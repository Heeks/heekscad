// RS274X.cpp
// Copyright (c) 2009, David Nicholls, Perttu "celero55" Ahola
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "RS274X.h"
#include "Geom.h"
#include "ConversionTools.h"
#include "Sketch.h"

#include <sstream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <set>
#include <list>
#include <map>
#include <vector>

extern CHeeksCADInterface heekscad_interface;

RS274X::RS274X()
{
	m_units = 25.4;	// inches.
	m_leadingZeroSuppression = false;
	m_trailingZeroSuppression = false;
	m_absoluteCoordinatesMode = true;

	m_XDigitsLeftOfPoint = 2;
	m_XDigitsRightOfPoint = 4;

	m_YDigitsLeftOfPoint = 2;
	m_YDigitsRightOfPoint = 4;

	m_full_circular_interpolation = false;
	m_part_circular_interpolation = false;
	m_cw_circular_interpolation = false;	// this flag only makes sense if m_part_circular_interpolation is true.

	m_current_position = gp_Pnt(0.0, 0.0, 0.0);
	m_lamp_on = false;

	m_LayerName = "";

	m_active_aperture = -1;	// None selected yet.

	m_area_fill = false;
	m_mirror_image = false;

	m_bWithinApertureMacroDefinition = false;

	m_current_line = -1;
} // End constructor


// Filter out spaces
char RS274X::ReadChar( const char *data, int *pos, const int max_pos )
{
	if (*pos < max_pos)
	{
		while ( ((*pos) < max_pos) && ((data[*pos] == ' ')) )
		{
			if(data[*pos] == '\n') m_current_line++;
			(*pos)++;
		}
		if (*pos < max_pos)
		{
			return(data[(*pos)++]);
		} // End if - then
		else
		{
			return(-1);
		} // End if - else
	} // End if - then
	else
	{
		return(-1);
	} // End if - else
} // End ReadChar() method

std::string RS274X::ReadBlock( const char *data, int *pos, const int max_pos )
{
	char delimiter;
	std::ostringstream l_ossBlock;

	// Read first char to determine if it's a parameter or not.
	char c;
	while (((c = ReadChar(data,pos,max_pos)) > 0) && ((c == '\n') || (c == '\r'))) { }

	if (c < 0) return(std::string(""));

	if (c == '%') delimiter = '%';
	else delimiter = '*';

	l_ossBlock << c;

	while (((c = ReadChar(data,pos,max_pos)) > 0) && (c != delimiter) && ((c != '\r') && (c != '\n')))
	{
		l_ossBlock << c;
	} // End while

	return(l_ossBlock.str());
} // End ReadBlock() method

bool RS274X::Read( const char *p_szFileName, const FileInterpretation_t file_interpretation, const bool force_mirror_on /* = false */ )
{
	printf("RS274X::Read(%s)\n", p_szFileName );

	if (force_mirror_on)
	{
		m_mirror_image = true;
	}

	std::ifstream input( p_szFileName, std::ios::in|std::ios::ate );
	if (input.is_open())
	{
		int size = (int) input.tellg();

		char *memblock = new char[size];

		input.seekg (0, std::ios::beg);
		input.read (memblock, size);
		input.close();

		m_current_line = 0;

		int pos = 0;
		while (pos < size)
		{
			std::string block = ReadBlock( memblock, &pos, size );
			if (block.size() > 0)
			{
			    while (block[0] == ' ') block.erase( block.begin() );
			    while (block[0] == '\t') block.erase( block.begin() );
			    while (*(block.rbegin()) == ' ') block.erase( block.size()-1 );
			    while (*(block.rbegin()) == '\t') block.erase( block.size()-1 );

				if (block[0] == '%')
				{
					// We're reading a parameter.
					if (! ReadParameters( block ))
					{
					    return(false);
					}
				} // End if - then
				else
				{
					// It's a normal data block.
					if (! ReadDataBlock( block ))
					{
					    return(false);
					}
				} // End if - else
			} // End if - then
		} // End while

		delete [] memblock;

        if ((file_interpretation == IsolationRouting) || (file_interpretation == Both))
        {
            // Generate and add the sketch objects that represent the boundaries of the traces.
            FormNetworks();
        } // End if - then

        if ((file_interpretation == CentreLines) || (file_interpretation == Both))
        {
            DrawCentrelines();
        } // End if - then

		if (file_interpretation == RasterImage)
        {
            // Generate a raster image (bitmap) that represents the traces.
            Bitmap pcb = RenderToBitmap();

            // Save the PCB bitmap to a RAW raster file for debugging purposes.
            // use the following command to convert this RAW image into a GIF file for viewing
            // (substitute the width and height integers based on the boards dimensions - that
            // appear in the file's name)
            //
            // eg: If file name is "pcb_width_3312_height_5678.raw" then the command would be;
            // rawtoppm < /home/david/_width_3312_height_5678.raw 3312 5678 | ppmtogif > pcb.gif
            {
                wxString file_name;
                file_name << _("pcb") << _("_width_") << pcb.PixelsPerRow() << _("_height_") << pcb.PixelsPerColumn() << _T(".raw");
                pcb.Save( file_name );
            }
        } // End if - then

		return true;

	} // End if - then
	else
	{
		// Couldn't read file.
		printf("Could not open '%s' for reading\n", p_szFileName );
		return(false);
	} // End if - else
} // End Read() method

bool RS274X::ReadParameters( const std::string & parameters )
{
	// printf("RS274X::ReadParameters('%s')\n", parameters.c_str() );
	std::string _params( parameters );
	std::string::size_type offset;
	while ((offset = _params.find('%')) != _params.npos) _params.erase(offset,1);
	while ((offset = _params.find('*')) != _params.npos) _params.erase(offset,1);

    if ((_params.size() == 0) && (m_bWithinApertureMacroDefinition))
    {
        m_bWithinApertureMacroDefinition = false;
        return(true);
    }

    if (m_bWithinApertureMacroDefinition)
    {
        printf("Ignoring aperture macro definition %s\n", _params.c_str());
        return(true);
    }

	if (_params.substr(0,4) == "MOIN")
	{
	    m_units = 25.4;
	    return(true);
	}
	else if (_params.substr(0,4) == "MOMM")
	{
	    m_units = 1.0;
	    return(true);
	}
	else if (_params.substr(0,2) == "AD")
	{
		// Aperture Definition.
		// %ADD<D-code number><aperture type>,<modifier>[X<modifer>]*%
		_params.erase(0,2);	// Remove AD
		if (_params[0] != 'D')
		{
			printf("Expected 'D' after Aperture Defintion sequence 'AD'\n");
			return(false);
		} // End if - then
		_params.erase(0,1);	// Remove 'D'
		const char *end = NULL;
		int tool_number = strtoul( _params.c_str(), (char **) &end, 10 );
		if ((end == NULL) || (end == _params.c_str()))
		{
			printf("Expected Aperture Type character\n");
			return(false);
		} // End if - then
		_params.erase(0, end - _params.c_str());

		char aperture_type = _params[0];
		_params.erase(0,1);	// Remove aperture type character.
		if (_params[0] == ',') _params.erase(0,1);	// Remove comma

		double modifier = special_strtod( _params.c_str(), &end );
		if ((end == NULL) || (end == _params.c_str()))
		{
			printf("Expected modifier\n");
			return(false);
		} // End if - then
		_params.erase(0, end - _params.c_str());

		switch (aperture_type)
		{
			case 'C':	// Circle.
				{
				// Push a circle within an outside diameter of modifier onto the list of apertures
				Aperture aperture;

				aperture.Type( Aperture::eCircular );
				aperture.OutsideDiameter( modifier * m_units );

				if ((_params.size() > 0) && (_params[0] == 'X'))
				{
					// It has a hole in it. (either circular or rectangular)
					_params.erase(0,1); // Erase the X

					aperture.XAxisHoleDimension( double(special_strtod( _params.c_str(), &end )) * m_units );
					if ((end == NULL) || (end == _params.c_str()))
					{
						printf("Expected modifier\n");
						return(false);
					} // End if - then
					_params.erase(0, end - _params.c_str());
				} // End if - then

				if ((_params.size() > 0) && (_params[0] == 'X'))
				{
					// It has a rectangular hole in it.
					_params.erase(0,1); // Erase the X
					aperture.YAxisHoleDimension( double(special_strtod( _params.c_str(), &end )) * m_units );
					if ((end == NULL) || (end == _params.c_str()))
					{
						printf("Expected modifier\n");
						return(false);
					} // End if - then
					_params.erase(0, end - _params.c_str());
				} // End if - then

				m_aperture_table.insert( std::make_pair( tool_number, aperture ) );
				}
				break;

			case 'R':	// Rectangle
				{
				// Push a circle within an outside diameter of modifier onto the list of apertures
				Aperture aperture;

				aperture.Type( Aperture::eRectangular );
				aperture.XAxisOutsideDimension( modifier * m_units );

				if ((_params.size() > 0) && (_params[0] == 'X'))
				{
					_params.erase(0,1); // Remove the 'X'
					aperture.YAxisOutsideDimension( double(special_strtod( _params.c_str(), &end )) * m_units );
					_params.erase(0, end - _params.c_str());
				} // End if - then

				m_aperture_table.insert( std::make_pair( tool_number, aperture ) );
				}
				break;

			case 'O':	// Obround
				{
				// Push a circle within an outside diameter of modifier onto the list of apertures
				Aperture aperture;

				aperture.Type( Aperture::eObRound );
				aperture.XAxisOutsideDimension( modifier * m_units );

				if ((_params.size() > 0) && (_params[0] == 'X'))
				{
					_params.erase(0,1); // Remove the 'X'
					aperture.YAxisOutsideDimension( double(special_strtod( _params.c_str(), &end )) * m_units );
					_params.erase(0, end - _params.c_str());
				} // End if - then

				m_aperture_table.insert( std::make_pair( tool_number, aperture ) );
				}
				break;

			case 'P':	// Rectangular Polygon (diamond)
				printf("Polygon apertures are not yet supported\n");
				return(false);

			case 'T':	// Unknown - but valid
				printf("T-type apertures are not yet supported\n");
				return(false);

			default:
				printf("Expected aperture type 'C', 'R', 'O', 'P' or 'T'\n");
				return(false);
		} // End switch
	}
	else if (_params.substr(0,2) == "LN")
	{
		// %LN<character string>*%
		_params.erase(0,2);	// Remove LN.
		m_LayerName = _params;
		return(true);
	}
	else if (_params.substr(0,2) == "AM")
	{
		// Aperture Macro
		_params.erase(0,2);	// Remove AM
		printf("Ignoring Aperture Macro command\n");
		m_bWithinApertureMacroDefinition = true;
		return(true);
	}
	else if (_params.substr(0,2) == "MI")
	{
		// Mirror Image = on.
		_params.erase(0,2);	// Remove MI
		m_mirror_image = true;
		return(true);
	}
	else if (_params.substr(0,2) == "AS")
	{
		// Axis Select
		_params.erase(0,2);	// Remove AS
		printf("Ignoring Axis Select command\n");

		while ((_params[0] == 'A') || (_params[0] == 'B'))
		{
		    if ((_params[0] == '=') && (_params.size() > 1))
		    {
		        _params.erase(0,2);
		    }
		}

		return(true);
	}
	else if (_params.substr(0,2) == "OF")
	{
		// Offset
		_params.erase(0,2);	// Remove OF
		printf("Ignoring Offset command\n");
		while ((_params[0] == 'A') || (_params[0] == 'B'))
		{
		    const char *end = NULL;
		    _params.erase(0,1); // Remove 'A' or 'B'
		    special_strtod( _params.c_str(), &end );
            _params.erase(0, end - _params.c_str());
		}

		return(true);
	}
	else if (_params.substr(0,2) == "IO")
	{
		// Image Offset
		_params.erase(0,2);	// Remove IO
		printf("Ignoring Image Offset command\n");
		while ((_params[0] == 'A') || (_params[0] == 'B'))
		{
		    const char *end = NULL;
		    _params.erase(0,1); // Remove 'A' or 'B'
		    special_strtod( _params.c_str(), &end );
            _params.erase(0, end - _params.c_str());
		}

		return(true);
	}
	else if (_params.substr(0,2) == "SR")
	{
		// Step and Repeat
		_params.erase(0,2);	// Remove SR
		printf("Ignoring Step and Repeat command\n");
		while ((_params[0] == 'A') || (_params[0] == 'B'))
		{
		    const char *end = NULL;
		    _params.erase(0,1); // Remove 'A' or 'B'
		    special_strtod( _params.c_str(), &end );
            _params.erase(0, end - _params.c_str());
		}

		return(true);
	}
	else if (_params.substr(0,2) == "IP")
	{
		// Image Polarity
		_params.erase(0,2);	// Remove IP
		printf("Ignoring Image Polarity command\n");
		if (_params.substr(0,3) == "POS") _params.erase(0,3);
		if (_params.substr(0,3) == "NEG") _params.erase(0,3);
		if (_params.substr(0,8) == "POSITIVE") _params.erase(0,8);
		if (_params.substr(0,8) == "NEGATIVE") _params.erase(0,8);
		return(true);
	}
	else if (_params.substr(0,2) == "LP")
	{
		// Layer Polarity
		_params.erase(0,2);	// Remove LP
		printf("Ignoring Layer Polarity command\n");
		if (_params.substr(0,3) == "POS") _params.erase(0,3);
		if (_params.substr(0,3) == "NEG") _params.erase(0,3);
		if (_params.substr(0,8) == "POSITIVE") _params.erase(0,8);
		if (_params.substr(0,8) == "NEGATIVE") _params.erase(0,8);
		return(true);
	}
	else if (_params.substr(0,2) == "KO")
	{
		// Knock Out
		_params.erase(0,2);	// Remove KO
		printf("Ignoring Knock Out command\n");
		if (_params.substr(0,3) == "OFF") _params.erase(0,3);
		if (_params.substr(0,3) == "Off") _params.erase(0,3);
		if (_params.substr(0,2) == "ON") _params.erase(0,2);
		if (_params.substr(0,2) == "On") _params.erase(0,2);
		return(true);
	}
	else if (_params.substr(0,2) == "IR")
	{
		// Image Rotation
		_params.erase(0,2);	// Remove IR
		printf("Ignoring Image Rotation command\n");
		if (_params.size() > 0)
		{
            const char *end = NULL;
            special_strtod( _params.c_str(), &end );
            _params.erase(0, end - _params.c_str());
		}
		return(true);
	}
	else if (_params.substr(0,2) == "RO")
	{
		// Rotation
		_params.erase(0,2);	// Remove RO
		printf("Ignoring Rotation command\n");
		if (_params.size() > 0)
		{
            const char *end = NULL;
            special_strtod( _params.c_str(), &end );
            _params.erase(0, end - _params.c_str());
		}
		return(true);
	}
	else if (_params.substr(0,2) == "PF")
	{
		// Plotter Film
		_params.erase(0,2);	// Remove PF
		printf("Ignoring Plotter Film command\n");

		return(true);
	}
	else if (_params.substr(0,2) == "SF")
	{
		// Scale Factor 'A=1.0,B=1.0'
		_params.erase(0,2);	// Remove OF
		printf("Ignoring Offset command\n");
		while ((_params[0] == 'A') || (_params[0] == 'B') || (_params[0] == ','))
		{
		    if (_params[0] == ',')
		    {
		        _params.erase(0,1);
		    }
		    else
		    {
		        _params.erase(0,1); // Remove 'A' or 'B'
		        if (_params.size() > 0) _params.erase(0,1); // Remove '='
                const char *end = NULL;
                special_strtod( _params.c_str(), &end );
                _params.erase(0, end - _params.c_str());
		    }
		}

		return(true);
	}
	else if (_params.substr(0,2) == "IJ")
	{
		// Image Justify
		_params.erase(0,2);	// Remove IJ
		printf("Ignoring Image Justify command\n");

		return(true);
	}
	else if (_params.substr(0,2) == "FS")
	{
		// <L or T><A or I>[Nn][Gn]<Xn><Yn>[Dn][Mn]
		_params.erase(0,2);	// Remove FS. We already know it's there.

		while (_params.size() > 0)
		{
			switch (_params[0])
			{
				case 'M':	// Ignore this.
					_params.erase(0,1);
					break;

				case 'D':	// Ignore this.
					_params.erase(0,1);
					break;

				case 'G':	// Ignore this.
					_params.erase(0,1);
					break;

				case 'N':	// Ignore this.
					_params.erase(0,1);
					break;

				case 'L':
					m_leadingZeroSuppression = true;
					_params.erase(0,1);
					break;

				case 'T':
					m_trailingZeroSuppression = true;
					_params.erase(0,1);
					break;

				case 'A':
					m_absoluteCoordinatesMode = true;
					_params.erase(0,1);
					break;

				case 'I':
					m_absoluteCoordinatesMode = false;
					_params.erase(0,1);
					break;

				case 'X':
					_params.erase(0,1);	// erase X

					if ((_params[0] < '0') || (_params[0] > '9'))
					{
						printf("Expected number following 'X'\n");
						return(false);
					} // End if - then

					m_XDigitsLeftOfPoint = atoi(_params.substr(0,1).c_str());
					_params.erase(0,1);

					m_XDigitsRightOfPoint = atoi(_params.substr(0,1).c_str());
					_params.erase(0,1);
					break;

				case 'Y':
					_params.erase(0,1);	// erase Y

					if ((_params[0] < '0') || (_params[0] > '9'))
					{
						printf("Expected number following 'Y'\n");
						return(false);
					} // End if - then

					m_YDigitsLeftOfPoint = atoi(_params.substr(0,1).c_str());
					_params.erase(0,1);

					m_YDigitsRightOfPoint = atoi(_params.substr(0,1).c_str());
					_params.erase(0,1);
					break;

				default:
					printf("Unrecognised argument to 'FS' parameter '%s'\n", _params.c_str() );
					return(false);
			} // End switch
		} // End while
	}
	else {
		printf("line %i: Unrecognised parameter '%s'\n", m_current_line, _params.c_str() );
		return(false);
	} // End if - then

	return(true);
} // End ReadParameters() method


double RS274X::InterpretCoord(
	const char *coordinate,
	const int digits_left_of_point,
	const int digits_right_of_point,
	const bool leading_zero_suppression,
	const bool trailing_zero_suppression ) const
{

	double multiplier = m_units;
	double result;
	std::string _coord( coordinate );

	if (_coord[0] == '-')
	{
		multiplier *= -1.0;
		_coord.erase(0,1);
	} // End if - then

	if (_coord[0] == '+')
	{
		multiplier *= +1.0;
		_coord.erase(0,1);
	} // End if - then

	if (leading_zero_suppression)
	{
		while (_coord.size() < (unsigned int) digits_right_of_point) _coord.insert(0,"0");

		// use the end of the string as the reference point.
		result = atof( _coord.substr( 0, _coord.size() - digits_right_of_point ).c_str() );
		result += (atof( _coord.substr( _coord.size() - digits_right_of_point ).c_str() ) / pow(10.0, digits_right_of_point));
	} // End if - then
	else
	{
		while (_coord.size() < (unsigned int) digits_left_of_point) _coord.push_back('0');

		// use the beginning of the string as the reference point.
		result = atof( _coord.substr( 0, digits_left_of_point ).c_str() );
		result += (atof( _coord.substr( digits_left_of_point ).c_str() ) / pow(10.0, (int)(_coord.size() - digits_left_of_point)));
	} // End if - else

	result *= multiplier;

	// printf("RS274X::InterpretCoord(%s) = %lf\n", coordinate, result );
	return(result);
} // End InterpretCoord() method


/**
	This routine is the same as the normal strtod() routine except that it
	doesn't accept 'd' or 'D' as radix values.  Some locale configurations
	use 'd' or 'D' as radix values just as 'e' or 'E' might be used.  This
	confuses subsequent commands held on the same line as the coordinate.
 */
double RS274X::special_strtod( const char *value, const char **end ) const
{
	std::string _value(value);
	char *_end = NULL;

	std::string::size_type offset = _value.find_first_of( "dD" );
	if (offset != std::string::npos)
	{
		_value.erase(offset);
	}

	double dval = strtod( _value.c_str(), &_end );
	if (end)
	{
		*end = value + (_end - _value.c_str());
	}
	return(dval);
}


bool RS274X::ReadDataBlock( const std::string & data_block )
{
	// printf("RS274X::ReadDataBlock('%s')\n", data_block.c_str() );
	std::string _data( data_block );
	std::string::size_type offset;
	while ((offset = _data.find('%')) != _data.npos) _data.erase(offset,1);
	while ((offset = _data.find('*')) != _data.npos) _data.erase(offset,1);
	while ((offset = _data.find(' ')) != _data.npos) _data.erase(offset,1);
	while ((offset = _data.find('\t')) != _data.npos) _data.erase(offset,1);

	char buffer[1024];
	memset(buffer,'\0', sizeof(buffer));
	strcpy( buffer, data_block.c_str() );

	double i_term = 0.0;
	double j_term = 0.0;
	gp_Pnt position( m_current_position );

	if ((_data.size() == 0) && (m_bWithinApertureMacroDefinition))
    {
        m_bWithinApertureMacroDefinition = false;
        return(true);
    }

    if (m_bWithinApertureMacroDefinition)
    {
        printf("Ignoring aperture macro definition %s\n", _data.c_str());
        return(true);
    }

	while (_data.size() > 0)
	{
		if (_data.substr(0,3) == "G04")
		{
			// Ignore data block.
			return(true);
		}
		else if (_data.substr(0,3) == "G70")
		{
			_data.erase(0,3);
			m_units = 25.4;
		}
		else if (_data.substr(0,3) == "G71")
		{
			_data.erase(0,3);
			m_units = 1.0;
		}
		else if (_data.substr(0,3) == "G90")
		{
			_data.erase(0,3);
			m_absoluteCoordinatesMode = true;
		}
		else if (_data.substr(0,3) == "G91")
		{
			_data.erase(0,3);
			m_absoluteCoordinatesMode = false;
		}
		else if (_data.substr(0,3) == "G75")
		{
			_data.erase(0,3);
			m_full_circular_interpolation = true;
		}
		else if (_data.substr(0,3) == "G36")
		{
			_data.erase(0,3);
			m_area_fill = true;
			m_filled_area_traces.clear();
		}
		else if (_data.substr(0,3) == "G37")
		{
			_data.erase(0,3);

			m_area_fill = false;
			// Convert the list of traces that bound the filled area into a single
			// face object.
			if (m_filled_area_traces.size() > 0)
			{
				m_filled_areas.push_back( m_filled_area_traces );
				m_filled_area_traces.clear();
			} // End if - then
		}
		else if (_data.substr(0,1) == "I")
		{
		    _data.erase(0,1);	// Erase I
			const char *end = NULL;

			double _value = special_strtod( _data.c_str(), &end );
			if ((end == NULL) || (end == _data.c_str()))
			{
				printf("Expected number following 'I'\n");
				return(false);
			} // End if - then
			std::string _string = _data.substr(0, end - _data.c_str());
			_data.erase(0, end - _data.c_str());

            if (_string.find('.') == std::string::npos)
            {

                i_term = InterpretCoord( _string.c_str(),
                                m_YDigitsLeftOfPoint,
                                m_YDigitsRightOfPoint,
                                m_leadingZeroSuppression,
                                m_trailingZeroSuppression );

            }
            else
            {
                // The number had a decimal point explicitly defined within it.  Read it as a correctly
                // represented number as is.
                i_term = _value;
            }
		}
		else if (_data.substr(0,1) == "J")
		{
			_data.erase(0,1);	// Erase J
			const char *end = NULL;

			double _value = special_strtod( _data.c_str(), &end );
			if ((end == NULL) || (end == _data.c_str()))
			{
				printf("Expected number following 'I'\n");
				return(false);
			} // End if - then
			std::string _string = _data.substr(0, end - _data.c_str());
			_data.erase(0, end - _data.c_str());

            if (_string.find('.') == std::string::npos)
            {

                j_term = InterpretCoord( _string.c_str(),
                                m_YDigitsLeftOfPoint,
                                m_YDigitsRightOfPoint,
                                m_leadingZeroSuppression,
                                m_trailingZeroSuppression );

            }
            else
            {
                // The number had a decimal point explicitly defined within it.  Read it as a correctly
                // represented number as is.
                j_term = _value;
            }
		}
		else if (_data.substr(0,3) == "M00")
		{
			_data.erase(0,3);
			return(true);	// End of program
		}
		else if (_data.substr(0,3) == "M02")
		{
			_data.erase(0,3);
			return(true);	// End of program
		}
		else if (_data.substr(0,3) == "G74")
		{
			_data.erase(0,3);
			m_full_circular_interpolation = false;
			m_part_circular_interpolation = false;
		}
		else if (_data.substr(0,3) == "G01")
		{
			_data.erase(0,3);
			m_part_circular_interpolation = false;
			m_cw_circular_interpolation = false;
			m_full_circular_interpolation = false;
		}
		else if (_data.substr(0,3) == "G02")
		{
			_data.erase(0,3);
			m_part_circular_interpolation = true;
			m_cw_circular_interpolation = true;
		}
		else if (_data.substr(0,3) == "G03")
		{
			_data.erase(0,3);
			m_part_circular_interpolation = true;
			m_cw_circular_interpolation = false;
		}
		else if (_data.substr(0,1) == "X")
		{
			_data.erase(0,1);	// Erase X
			const char *end = NULL;

			double x = special_strtod( _data.c_str(), &end );
			if ((end == NULL) || (end == _data.c_str()))
			{
				printf("Expected number following 'X'\n");
				return(false);
			} // End if - then
			std::string x_string = _data.substr(0, end - _data.c_str());
			_data.erase(0, end - _data.c_str());

            if (x_string.find('.') == std::string::npos)
            {

                double x = InterpretCoord( x_string.c_str(),
                                m_YDigitsLeftOfPoint,
                                m_YDigitsRightOfPoint,
                                m_leadingZeroSuppression,
                                m_trailingZeroSuppression );

                if (m_absoluteCoordinatesMode)
                {
                    position.SetX( x );
                }
                else
                {
                    // Incremental position.
                    position.SetX( position.X() + x );
                }
            }
            else
            {
                // The number had a decimal point explicitly defined within it.  Read it as a correctly
                // represented number as is.
                if (m_absoluteCoordinatesMode)
                {
                    position.SetX( x );
                }
                else
                {
                    // Incremental position.
                    position.SetX( position.X() + x );
                }
            }
		}
		else if (_data.substr(0,1) == "Y")
		{
			_data.erase(0,1);	// Erase Y
			const char *end = NULL;

			double y = special_strtod( _data.c_str(), &end );
			if ((end == NULL) || (end == _data.c_str()))
			{
				printf("Expected number following 'Y'\n");
				return(false);
			} // End if - then

			std::string y_string = _data.substr(0, end - _data.c_str());
			_data.erase(0, end - _data.c_str());

            if (y_string.find('.') == std::string::npos)
            {
                double y = InterpretCoord( y_string.c_str(),
                                m_YDigitsLeftOfPoint,
                                m_YDigitsRightOfPoint,
                                m_leadingZeroSuppression,
                                m_trailingZeroSuppression );

                if (m_absoluteCoordinatesMode)
                {
                    position.SetY( y );
                }
                else
                {
                    // Incremental position.
                    position.SetY( position.Y() + y );
                }
            }
            else
            {
                    // The number already has a decimal point explicitly defined within it.

                    if (m_absoluteCoordinatesMode)
                    {
                        position.SetY( y );
                    }
                    else
                    {
                        // Incremental position.
                        position.SetY( position.Y() + y );
                    }
            }
		}
		else if (_data.substr(0,3) == "D01")
		{
			_data.erase(0,3);
			m_lamp_on = true;
		}
		else if (_data.substr(0,3) == "D02")
		{
			_data.erase(0,3);

			if((m_area_fill) && (m_filled_area_traces.size() > 0)) {
				m_filled_areas.push_back( m_filled_area_traces );
				m_filled_area_traces.clear();
			}
			m_current_position = position;
			m_lamp_on = false;
		}
		else if (_data.substr(0,3) == "D03")
		{
			_data.erase(0,3);
			m_lamp_on = false;
			m_current_position = position;
			if (m_aperture_table.find( m_active_aperture ) == m_aperture_table.end())
			{
				printf("Flash (D03) command issued without first selecting an aperture\n");
				return(false);
			} // End if - then

			Aperture aperture = m_aperture_table[ m_active_aperture ];
			if ((aperture.Type() == Aperture::eCircular) && (aperture.OutsideDiameter() < 0.0001)) printf("WARNING: D03 found without radius information\n");
			Trace trace( aperture, Trace::eFlash );
			trace.Start( position );
			m_traces.push_back( trace );
		}
		else if (_data.substr(0,3) == "G54")
		{
			// Prepare tool
			_data.erase(0,3);	// Erase 'G54'
			if (_data[0] != 'D')
			{
				printf("Expected aperture number argument 'D' in G54 command\n");
				return(false);
			} // End if - then

			_data.erase(0,1);	// Erase 'D'

			char *end = NULL;
			int aperture_number = strtoul( _data.c_str(), &end, 10 );
			if ((end == NULL) || (end == _data.c_str()))
			{
				printf("Expected aperture number following 'D'\n");
				return(false);
			} // End if - then
			_data.erase(0, end - _data.c_str() + 1);

			m_active_aperture = aperture_number;
			return(true);
		}
		else if (_data.substr(0,1) == "D")
		{
		    // The RS247X format document says that a new aperture is normally selected along
		    // with a G54 command but the G54 is optional. Accept this is the number matches
		    // an aperture that is already defined.
		    _data.erase(0,1);   // Erase the 'D'

		    char *end = NULL;
            m_active_aperture = strtoul(_data.c_str(), &end, 10 );
            if ((end == NULL) || (end == _data.c_str()))
			{
				printf("Expected aperture number following 'D'\n");
				return(false);
			} // End if - then
			_data.erase(0, end - _data.c_str() + 1);

            if (m_aperture_table.find( m_active_aperture ) == m_aperture_table.end())
			{
				printf("Aperture selection command selected undefined aperture\n");
				return(false);
			} // End if - then
		}
		else
		{
			printf("Unexpected command '%s'\n", _data.c_str() );
			return(false);
		} // End if - else
	} // End while

	if ((position.Distance( m_current_position ) > wxGetApp().m_geom_tol) && (m_lamp_on == true))
	{
		// We may have just parsed a modal command.  If the lamp is on then
		// add the appropriate trace.  If the movement had already been
		// handled then the m_current_position would already have been
		// updated.

		if (m_part_circular_interpolation)
		{
		    if (m_full_circular_interpolation)
            {
                // full circle
                // circular interpolation.
                double radius = sqrt((i_term * i_term) + (j_term * j_term));

                Trace trace( m_aperture_table[m_active_aperture], Trace::eCircular );
                trace.Radius(radius);
                trace.Start( position );
                trace.End( position );
                trace.Clockwise( m_cw_circular_interpolation );

                if(m_area_fill)
                {
                    m_filled_area_traces.push_back( trace );
                }
                else
                {
                    m_traces.push_back( trace );
                }

                m_current_position = position;
            }
            else
            {
                // arc
                // circular interpolation.

                Trace trace( m_aperture_table[m_active_aperture], Trace::eCircular );
                trace.Start( m_current_position );
                trace.End( position );
                trace.Clockwise( m_cw_circular_interpolation );
                trace.I( i_term );
                trace.J( j_term );
                trace.Radius( sqrt((i_term * i_term) + (j_term * j_term)) );

                if(m_area_fill)
                {
                    m_filled_area_traces.push_back( trace );
                }
                else
                {
                    m_traces.push_back( trace );
                }

                m_current_position = position;
            }
		} // End if - then
		else
		{
			// linear interpolation.

			Trace trace( m_aperture_table[m_active_aperture], Trace::eLinear );
			trace.Start( m_current_position );
			trace.End( position );

			if(m_area_fill)
			{
				m_filled_area_traces.push_back( trace );
			}
			else
			{
				m_traces.push_back( trace );
			}

			m_current_position = position;
		} // End if - else
	}

	return(true);
} // End ReadDataBlock() method



bool RS274X::AggregateFaces( const TopoDS_Face lhs, const TopoDS_Face rhs, TopoDS_Face *pResult ) const
{
    const bool l_bSuccess = true;
    const bool l_bFailure = false;

    TopoDS_Face empty;

	if ((Trace::Area(lhs) < 0.00001) || (Trace::Area(rhs) < 0.00001))
	{
        return(l_bFailure);
	}

	Bnd_Box lhs_bounding_box, rhs_bounding_box;
    BRepBndLib::Add(lhs, lhs_bounding_box);
    BRepBndLib::Add(rhs, rhs_bounding_box);

    Standard_Real lhs_box[6];
    Standard_Real rhs_box[6];

    lhs_bounding_box.Get( lhs_box[0], lhs_box[1], lhs_box[2], lhs_box[3], lhs_box[4], lhs_box[5] );
    rhs_bounding_box.Get( rhs_box[0], rhs_box[1], rhs_box[2], rhs_box[3], rhs_box[4], rhs_box[5] );

    if ((Trace::Area(lhs) - Trace::Area(rhs)) < 0.00001)
	{
	    bool matches = true;
	    for (unsigned int i=0; i<(sizeof(lhs_box)/sizeof(lhs_box[0])); i++)
	    {
	        if ((lhs_box[i] - rhs_box[i]) < 0.00001) matches = false;
	    }

	    if (matches)
	    {
            // They're the same (conincident).
            *pResult = rhs;
            return(l_bSuccess);
	    }
	}

    TopoDS_Shape shape;
    TopoDS_Shape lhs_shape = BRepPrimAPI_MakePrism(lhs, gp_Vec(0,0,1));
    TopoDS_Shape rhs_shape = BRepPrimAPI_MakePrism(rhs, gp_Vec(0,0,1));

    try {
        BRepAlgo_Fuse fused( lhs_shape, rhs_shape );
        fused.Build();
        if (fused.IsDone())
        {
            shape = fused.Shape();

            for (TopExp_Explorer expFace(shape, TopAbs_FACE); expFace.More(); expFace.Next())
            {
                TopoDS_Face aFace = TopoDS::Face(expFace.Current());
                Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace);
                if(aSurface->DynamicType() == STANDARD_TYPE(Geom_Plane))
                {
                    bool bottom_surface = true;
                    TopoDS_Wire wire=BRepTools::OuterWire(aFace);
                    BRepTools_WireExplorer explorer;
                    for (explorer.Init(TopoDS::Wire(wire)) ; bottom_surface && explorer.More(); explorer.Next())
                    {
                        Standard_Real start_u, end_u;

                        Handle(Geom_Curve) curve = BRep_Tool::Curve(explorer.Current(),start_u,end_u);
                        BRepAdaptor_Curve adaptor = BRepAdaptor_Curve(explorer.Current());

                        if ((curve->Value(start_u).Z() > 0.0001) ||
                            (curve->Value(end_u).Z() > 0.0001))
                        {
                            bottom_surface = false;
                        }
                    }

                    if (bottom_surface)
                    {
                        *pResult = aFace;
                        return(l_bSuccess);
                    }
                }
            }
        } // End if - then
    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        return(l_bFailure);
    }

    return(l_bFailure);
}


bool RS274X::AggregateFilledArea( const RS274X::Traces_t & traces, TopoDS_Face *pResult ) const
{
	// Aggregate the face objects from all the component traces and find the outer-most
	// edge to form one large (all encompasing) face.

    const bool l_bSuccess = true;
    const bool l_bFailure = false;

	if (traces.size() == 0)
	{
		return(l_bFailure);
	}

	TopoDS_Shape shape;
	BRepBuilderAPI_MakeWire wire;

	for (Traces_t::const_iterator l_itTrace = traces.begin(); l_itTrace != traces.end(); l_itTrace++)
	{
		if (Trace::Area(l_itTrace->Face()) < 0.00001)
		{
		    printf("Discarding trace due to face size of %lf\n", Trace::Area(l_itTrace->Face()));
		    continue;
		}
		if (l_itTrace->Length() < 0.00001)
		{
		    printf("Discarding trace due to length of %lf\n", l_itTrace->Length());
		    continue;
		}

		TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(l_itTrace->Start(), l_itTrace->End());
		wire.Add(edge);
	}
	// Now add a block to fill in the centre of the filled area.
	TopoDS_Face face = BRepBuilderAPI_MakeFace(wire.Wire());
	*pResult = face;
	return(l_bSuccess);
}









/* static */ bool RS274X::FacesIntersect( const TopoDS_Face lhs, const TopoDS_Face rhs )
{
    Bnd_Box lhs_bounding_box, rhs_bounding_box;
    BRepBndLib::Add(lhs, lhs_bounding_box);
    BRepBndLib::Add(rhs, rhs_bounding_box);

    if (lhs_bounding_box.IsOut(rhs_bounding_box)) return(false);

    IntTools_FaceFace intersect;
    intersect.Perform( lhs, rhs );
    if (intersect.IsDone())
    {
        // The OpenCascade routine BRepAlgo_Fuse() can't handle fusing two
        // shapes that touch each other but don't overlap each other.  We need
        // to find out if this is the case for these two faces.

        HeeksObj *lhs_sketch = Sketch( lhs );
        HeeksObj *rhs_sketch = Sketch( rhs );

        std::list< double > intersections;
        ((CSketch *) lhs_sketch)->Intersects( rhs_sketch, &intersections );

		delete lhs_sketch; lhs_sketch = (HeeksObj *) NULL;
		delete rhs_sketch; rhs_sketch = (HeeksObj *) NULL;

        std::set<RS274X::Point> points;
        while (intersections.size() > 2)
        {
            RS274X::Point point;
            point.SetX( *(intersections.begin()) ); intersections.erase( intersections.begin() );
            point.SetY( *(intersections.begin()) ); intersections.erase( intersections.begin() );
            point.SetZ( *(intersections.begin()) ); intersections.erase( intersections.begin() );

            points.insert(point);
        }

        return( points.size() > 1);
    }
    else
    {
        return(false);
    }
}


bool RS274X::Trace::Intersects( const Trace & rhs ) const
{
    return(FacesIntersect( Face(), rhs.Face() ));
} // End Intersects() method


/* static */ HeeksObj *RS274X::Sketch( const TopoDS_Face face )
{
    CSketch *sketch = new CSketch();

    TopoDS_Wire wire=BRepTools::OuterWire(face);
    BRepTools_WireExplorer explorer;
    for (explorer.Init(TopoDS::Wire(wire)) ; explorer.More(); explorer.Next())
    {
        Standard_Real start_u, end_u;

        Handle(Geom_Curve) curve = BRep_Tool::Curve(explorer.Current(),start_u,end_u);
        BRepAdaptor_Curve adaptor = BRepAdaptor_Curve(explorer.Current());

        switch (adaptor.GetType())
        {
        case GeomAbs_Circle:
            {
                HArc *arc = new HArc(curve->Value(start_u),
                        curve->Value(end_u),
                        adaptor.Circle(),
                        &wxGetApp().current_color );
                sketch->Add( arc, NULL );
            }
            break;

        case GeomAbs_Line:
            {
                HLine *line = new HLine( curve->Value(start_u), curve->Value(end_u), &wxGetApp().current_color );
                sketch->Add( line, NULL );
            }
            break;

        case GeomAbs_Ellipse:
        case GeomAbs_Hyperbola:
        case GeomAbs_Parabola:
        case GeomAbs_BezierCurve:
        case GeomAbs_BSplineCurve:
        case GeomAbs_OtherCurve:
            // The routines that produce these curves don't use these
            // last few curve types.  These are only listed within the
            // switch stastement to avoid the compiler warning.
            break;
        } // End switch

    } // End for

    return(sketch);
}



RS274X::Bitmap RS274X::RenderToBitmap()
{
    // Define a blank bitmap to represent the printed circuit board.
	Bitmap pcb( BoundingBox() );

    // Expose the PCB to the traces and aperture flashes defined by the traces.
	for (Traces_t::iterator l_itTrace = m_traces.begin(); l_itTrace != m_traces.end(); l_itTrace++ )
	{
	    l_itTrace->ExposeFilm( pcb );
	}

    // Expose the PCB to the filled areas made up by the boundary of traces.
	for (FilledAreas_t::iterator l_itArea = m_filled_areas.begin(); l_itArea != m_filled_areas.end(); l_itArea++)
	{
		// TODO Fill in the filled area.
		for (Traces_t::iterator l_itTrace = l_itArea->begin(); l_itTrace != l_itArea->end(); l_itTrace++)
		{
		    l_itTrace->ExposeFilm( pcb );
		}
	}

	return(pcb);
}



void RS274X::DrawCentrelines()
{
   	// Now aggregate the traces based on how they intersect each other.  We want all traces
	// that touch to become one large object.

    typedef std::list<HeeksObj *> Objects_t;
    Objects_t objects;

    for (Traces_t::iterator l_itTrace = m_traces.begin(); l_itTrace != m_traces.end(); l_itTrace++ )
	{
	    HeeksObj *object = l_itTrace->CentrelineGraphics();
	    if (object != NULL)
	    {
			if (m_mirror_image)
			{
				double mirror[16];
				gp_Ax1 mirror_axis = gp_Ax1(gp_Pnt(0,0,0), gp_Dir(1,0,0));
				gp_Trsf rotation;
				rotation.SetRotation( mirror_axis, M_PI );
				extract(rotation, mirror);
				object->ModifyByMatrix(mirror);
			}

            objects.push_back( object );
	    }
	}

	for (FilledAreas_t::iterator l_itArea = m_filled_areas.begin(); l_itArea != m_filled_areas.end(); l_itArea++)
	{
	    for (Traces_t::iterator l_itTrace = l_itArea->begin(); l_itTrace != l_itArea->end(); l_itTrace++ )
	    {
	        HeeksObj *object = l_itTrace->CentrelineGraphics();
            if (object != NULL)
            {
				if (m_mirror_image)
				{
					double mirror[16];
					gp_Ax1 mirror_axis = gp_Ax1(gp_Pnt(0,0,0), gp_Dir(1,0,0));
					gp_Trsf rotation;
					rotation.SetRotation( mirror_axis, M_PI );
					extract(rotation, mirror);
					object->ModifyByMatrix(mirror);
				}

                objects.push_back( object );
            }
	    }
	}


    while (objects.size() > 1)
    {
        // Join all other sketches to the first one.  Keep joining until we can't find any to join.
        if ((*(objects.begin()))->GetType() != SketchType)
        {
            // We're going to aggregate all elements with this first one.  If we haven't already,convert
            // the first element into a sketch including itself.  This way we can just add to the
            // sketch parent object when aggregating them.

            HeeksObj *sketch = heekscad_interface.NewSketch();
            ((CSketch *)sketch)->Add( *(objects.begin()), NULL );
            *(objects.begin()) = sketch;
        }

        bool joins_made = false;
        std::list<Objects_t::iterator> already_joined;
        Objects_t::iterator itObject = objects.begin(); itObject++;
        for ( ; (! joins_made) && (itObject != objects.end()); itObject++)
        {
            std::list<double> intersections;
            if ((*itObject)->Intersects( *(objects.begin()),  &intersections) > 0)
            {
                ((CSketch *)*(objects.begin()))->Add( *itObject, NULL );
                joins_made = true;
                already_joined.push_back(itObject);
            } // End if - then
        } // End for

        for (std::list<Objects_t::iterator>::iterator itRemove = already_joined.begin(); itRemove != already_joined.end(); itRemove++)
        {
            objects.erase( *itRemove );
        }

        if (joins_made == false)
        {
            // This is as big as it's ever going to get.
            heekscad_interface.Add( *(objects.begin()), NULL );
            objects.erase( objects.begin() );
        }
    } // End while

	if (objects.size() > 0)
	{
		heekscad_interface.Add( *(objects.begin()), NULL );
        objects.erase( objects.begin() );
	}
}


int RS274X::FormNetworks()
{
	// Now aggregate the traces based on how they intersect each other.  We want all traces
	// that touch to become one large object.
    Faces_t faces;
    int number_of_networks = 0;

    static std::list<HeeksObj *> objects;

    for (Traces_t::iterator l_itTrace = m_traces.begin(); l_itTrace != m_traces.end(); l_itTrace++ )
	{
        faces.push_back(l_itTrace->Face());
	}

	for (FilledAreas_t::const_iterator l_itArea = m_filled_areas.begin(); l_itArea != m_filled_areas.end(); l_itArea++)
	{
	    TopoDS_Face face;
	    if (AggregateFilledArea( *l_itArea, &face ))
	    {
                faces.push_back( face );
	    }
	}

    // We have a list of faces that represent all the copper areas we want.  We now need to intersect
	// all the touching faces to produce a single (larger) face.  The ones that intersect each other
	// should be replaced by the larger (combined) face.  We should repeat this until we find no
	// further intersecting faces.


    while (faces.size() > 1)
    {
        // Join all other faces to the first one.  Keep joining until we can't find any to join.
        bool joins_made = false;
        std::list<Faces_t::iterator> already_joined;
        Faces_t::iterator itFace = faces.begin(); itFace++;
        for ( ; (! joins_made) && (itFace != faces.end()); itFace++)
        {
            if (FacesIntersect( *(faces.begin()), *itFace ))
            {
                TopoDS_Face combination;
                if (AggregateFaces(*(faces.begin()), *itFace, &combination))
                {
                    *(faces.begin()) = combination;
                    joins_made = true;
                    already_joined.push_back(itFace);
                }
            } // End if - then
        } // End for

        for (std::list<Faces_t::iterator>::iterator itRemove = already_joined.begin(); itRemove != already_joined.end(); itRemove++)
        {
            faces.erase( *itRemove );
        }

        if (joins_made == false)
        {
            // This is as big as it's ever going to get.
			TopoDS_Face face(*(faces.begin()));

            HeeksObj *sketch = this->Sketch( face );
            for (int i=0; (((CSketch *)sketch)->GetSketchOrder() != SketchOrderTypeCloseCCW) && (i<4); i++)
            {
                ((CSketch *)sketch)->ReOrderSketch( SketchOrderTypeCloseCCW );  // At least try to make them all consistently oriented.
            }

			if (m_mirror_image)
			{
				double mirror[16];
				gp_Ax1 mirror_axis = gp_Ax1(gp_Pnt(0,0,0), gp_Dir(1,0,0));
				gp_Trsf rotation;
				rotation.SetRotation( mirror_axis, M_PI );
				extract(rotation, mirror);
				sketch->ModifyByMatrix(mirror);
			}

            heekscad_interface.Add( sketch, NULL );
            number_of_networks++;

            faces.erase( faces.begin() );
        }
    } // End while

	if (faces.size() > 0)
	{
		TopoDS_Face face(*(faces.begin()));

	    HeeksObj *sketch = this->Sketch( face );
	    for (int i=0; (((CSketch *)sketch)->GetSketchOrder() != SketchOrderTypeCloseCCW) && (i<4); i++)
	    {
            ((CSketch *)sketch)->ReOrderSketch( SketchOrderTypeCloseCCW );  // At least try to make them all consistently oriented.
	    }

		if (m_mirror_image)
		{
			double mirror[16];
			gp_Ax1 mirror_axis = gp_Ax1(gp_Pnt(0,0,0), gp_Dir(1,0,0));
			gp_Trsf rotation;
			rotation.SetRotation( mirror_axis, M_PI );
			extract(rotation, mirror);
			sketch->ModifyByMatrix(mirror);
		}

		heekscad_interface.Add( sketch, NULL );
		number_of_networks++;
        faces.erase( faces.begin() );
	}

    return(number_of_networks);

} // End FormNetworks() method


TopoDS_Shape RS274X::Aperture::Shape( const gp_Pnt & location ) const
{
	return(BRepPrimAPI_MakePrism(Face(location), gp_Vec(0,0,1)));
}

TopoDS_Shape RS274X::Trace::Shape() const
{
	return(BRepPrimAPI_MakePrism(Face(), gp_Vec(0,0,1)));
}

TopoDS_Face RS274X::Aperture::Face(const gp_Pnt & location) const
{
	switch (m_type)
	{
		case eCircular:
		{
			gp_Pnt left( location ); left.SetX( left.X() - (OutsideDiameter()/2.0) );
			gp_Pnt right( location ); right.SetX( right.X() + (OutsideDiameter()/2.0) );
			gp_Circ circ(gp_Ax2(location,gp_Dir(0,0,-1)), (OutsideDiameter()/2.0));

			Handle(Geom_TrimmedCurve) left_half = GC_MakeArcOfCircle(circ, left, right, true );
			Handle(Geom_TrimmedCurve) right_half = GC_MakeArcOfCircle(circ, right, left, true );

			TopoDS_Edge left_edge = BRepBuilderAPI_MakeEdge(left_half);
			TopoDS_Edge right_edge = BRepBuilderAPI_MakeEdge(right_half);

			BRepBuilderAPI_MakeWire wire_maker;

			wire_maker.Add(left_edge);
			wire_maker.Add(right_edge);

			TopoDS_Face face = BRepBuilderAPI_MakeFace(wire_maker.Wire());
			return(face);
		}

		case eObRound:
		{
		    if (XAxisOutsideDimension() > YAxisOutsideDimension())
		    {
		        // It's horozontal in orientation.  i.e. the two half circles are
		        // at the left and right end while horozontal lines join them together.

                gp_Pnt top_left( location ); top_left.SetX( top_left.X() - (XAxisOutsideDimension() / 2.0) + (YAxisOutsideDimension() / 2.0) );
                top_left.SetY( top_left.Y() + (YAxisOutsideDimension()/2.0) );

                gp_Pnt top_right( location ); top_right.SetX( top_right.X() + (XAxisOutsideDimension() / 2.0) - (YAxisOutsideDimension() / 2.0) );
                top_right.SetY( top_right.Y() + (YAxisOutsideDimension()/2.0) );

                gp_Pnt bottom_left( location ); bottom_left.SetX( bottom_left.X() - (XAxisOutsideDimension() / 2.0) + (YAxisOutsideDimension() / 2.0) );
                bottom_left.SetY( bottom_left.Y() - (YAxisOutsideDimension()/2.0) );

                gp_Pnt bottom_right( location ); bottom_right.SetX( bottom_right.X() + (XAxisOutsideDimension() / 2.0) - (YAxisOutsideDimension() / 2.0) );
                bottom_right.SetY( bottom_right.Y() - (YAxisOutsideDimension()/2.0) );

                gp_Pnt left_centre( location ); left_centre.SetX(left_centre.X() - (XAxisOutsideDimension() / 2.0) );
                gp_Pnt right_centre( location ); right_centre.SetX(right_centre.X() + (XAxisOutsideDimension() / 2.0) );

                Handle(Geom_TrimmedCurve) left_half_circle = GC_MakeArcOfCircle(top_left, left_centre, bottom_left );
                Handle(Geom_TrimmedCurve) bottom_segment = GC_MakeSegment(bottom_left, bottom_right);
                Handle(Geom_TrimmedCurve) right_half_circle = GC_MakeArcOfCircle(bottom_right, right_centre, top_right );
                Handle(Geom_TrimmedCurve) top_segment = GC_MakeSegment(top_right, top_left);

                TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(left_half_circle);
                TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(bottom_segment);
                TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(right_half_circle);
                TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(top_segment);

                BRepBuilderAPI_MakeWire wire_maker;

                wire_maker.Add(edge1);
                wire_maker.Add(edge2);
                wire_maker.Add(edge3);
                wire_maker.Add(edge4);

                TopoDS_Face face = BRepBuilderAPI_MakeFace(wire_maker.Wire());
                return(face);
		    }
		    else if (XAxisOutsideDimension() < YAxisOutsideDimension())
		    {
		        // It's vertical in orientation.  i.e. the two half circles are at
		        // the top and bottom while vertical lines join them together.

		        gp_Pnt top_left( location ); top_left.SetX( top_left.X() - (XAxisOutsideDimension() / 2.0) );
                top_left.SetY( top_left.Y() + (YAxisOutsideDimension()/2.0) - (XAxisOutsideDimension() / 2.0) );

                gp_Pnt top_right( location ); top_right.SetX( top_right.X() + (XAxisOutsideDimension() / 2.0) );
                top_right.SetY( top_right.Y() + (YAxisOutsideDimension()/2.0) - (XAxisOutsideDimension() / 2.0) );

                gp_Pnt bottom_left( location ); bottom_left.SetX( bottom_left.X() - (XAxisOutsideDimension() / 2.0) );
                bottom_left.SetY( bottom_left.Y() - (YAxisOutsideDimension()/2.0) + (XAxisOutsideDimension() / 2.0));

                gp_Pnt bottom_right( location ); bottom_right.SetX( bottom_right.X() + (XAxisOutsideDimension() / 2.0) );
                bottom_right.SetY( bottom_right.Y() - (YAxisOutsideDimension()/2.0) + (XAxisOutsideDimension() / 2.0));

                gp_Pnt top_centre( location ); top_centre.SetY(top_centre.Y() + (YAxisOutsideDimension() / 2.0) );
                gp_Pnt bottom_centre( location ); bottom_centre.SetY(bottom_centre.Y() - (YAxisOutsideDimension() / 2.0) );

                Handle(Geom_TrimmedCurve) top_half_circle = GC_MakeArcOfCircle(top_left, top_centre, top_right );
                Handle(Geom_TrimmedCurve) right_segment = GC_MakeSegment(top_right, bottom_right);
                Handle(Geom_TrimmedCurve) bottom_half_circle = GC_MakeArcOfCircle(bottom_right, bottom_centre, bottom_left );
                Handle(Geom_TrimmedCurve) left_segment = GC_MakeSegment(bottom_left, top_left);

                TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(top_half_circle);
                TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(right_segment);
                TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(bottom_half_circle);
                TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(left_segment);

                BRepBuilderAPI_MakeWire wire_maker;

                wire_maker.Add(edge1);
                wire_maker.Add(edge2);
                wire_maker.Add(edge3);
                wire_maker.Add(edge4);

                TopoDS_Face face = BRepBuilderAPI_MakeFace(wire_maker.Wire());
                return(face);
		    }
		    else
		    {
                    // It's a circle rather than an obround.

                    gp_Pnt left( location ); left.SetX( left.X() - (OutsideDiameter()/2.0) );
                    gp_Pnt right( location ); right.SetX( right.X() + (OutsideDiameter()/2.0) );
                    gp_Circ circ(gp_Ax2(location,gp_Dir(0,0,-1)), (OutsideDiameter()/2.0));

                    Handle(Geom_TrimmedCurve) left_half = GC_MakeArcOfCircle(circ, left, right, true );
                    Handle(Geom_TrimmedCurve) right_half = GC_MakeArcOfCircle(circ, right, left, true );

                    TopoDS_Edge left_edge = BRepBuilderAPI_MakeEdge(left_half);
                    TopoDS_Edge right_edge = BRepBuilderAPI_MakeEdge(right_half);

                    BRepBuilderAPI_MakeWire wire_maker;

                    wire_maker.Add(left_edge);
                    wire_maker.Add(right_edge);

                    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire_maker.Wire());
                    return(face);
		    }
		}

		case eRectangular:
		{
			gp_Pnt top_left( location );
			gp_Pnt top_right( location );
			gp_Pnt bottom_left( location );
			gp_Pnt bottom_right( location );

			top_left.SetX( location.X() - (XAxisOutsideDimension() / 2.0));
			bottom_left.SetX( location.X() - (XAxisOutsideDimension() / 2.0));

			top_right.SetX( location.X() + (XAxisOutsideDimension() / 2.0));
			bottom_right.SetX( location.X() + (XAxisOutsideDimension() / 2.0));

			top_left.SetY( location.Y() + (YAxisOutsideDimension() / 2.0));
			bottom_left.SetY( location.Y() - (YAxisOutsideDimension() / 2.0));

			top_right.SetY( location.Y() + (YAxisOutsideDimension() / 2.0));
			bottom_right.SetY( location.Y() - (YAxisOutsideDimension() / 2.0));

			Handle(Geom_TrimmedCurve) seg1 = GC_MakeSegment(top_left, bottom_left);
			Handle(Geom_TrimmedCurve) seg2 = GC_MakeSegment(bottom_left, bottom_right);
			Handle(Geom_TrimmedCurve) seg3 = GC_MakeSegment(bottom_right, top_right);
			Handle(Geom_TrimmedCurve) seg4 = GC_MakeSegment(top_right, top_left);

			TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(seg1);
			TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(seg2);
			TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(seg3);
			TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(seg4);

			BRepBuilderAPI_MakeWire wire_maker;

			wire_maker.Add(edge1);
			wire_maker.Add(edge2);
			wire_maker.Add(edge3);
			wire_maker.Add(edge4);

			TopoDS_Face face1 = BRepBuilderAPI_MakeFace(wire_maker.Wire());
			return(face1);
		}


		default:
			printf("Unsupported aperture shape found\n");
			TopoDS_Face empty;
			return(empty);
	} // End switch
} // End MakePolygon() method


RS274X::Trace::Trace( const Aperture & aperture, const eInterpolation_t interpolation ) : m_aperture(aperture), m_interpolation( interpolation )
{
	m_start.SetX(0.0);
	m_start.SetY(0.0);
	m_start.SetZ(0.0);

	m_end.SetX(0.0);
	m_end.SetY(0.0);
	m_end.SetZ(0.0);

	m_i_term = 0.0;
	m_j_term = 0.0;

	if (m_interpolation == eFlash)
	{
		m_radius = m_aperture.OutsideDiameter() / 2.0;
	}
	else
	{
		m_radius = 0.0;
	}

	m_clockwise = true;

	m_tolerance = wxGetApp().m_geom_tol;

	m_pHeeksObject = NULL;
}


double RS274X::Trace::Radius() const
{
	if (m_radius < m_tolerance)
	{
		m_radius = sqrt( ( m_i_term * m_i_term ) + ( m_j_term * m_j_term ) );
	}

	return(m_radius);
}
void RS274X::Trace::Radius( const double value ) { m_radius = value; }

gp_Pnt RS274X::Trace::Centre() const
{
	if ((Interpolation() == eCircular) && (abs(m_i_term) < m_tolerance) && (abs(m_j_term) < m_tolerance))
	{
		// It's a full circle.  Set the centre to the start/end points.
		return(m_start);
	} // End if - then

	switch (Interpolation())
	{
		case eLinear:
		{
			return(gp_Pnt(  ((Start().X() - End().X())/2) + Start().X(),
							((Start().Y() - End().Y())/2) + Start().Y(),
							((Start().Z() - End().Z())/2) + Start().Z() ));
		} // End if - then


		case eCircular:
		{
			// It must be an arc.
			// The i and j parameters are unsigned in the RS274 standard as the
			// sign can be inferred from the start and end position.  The radius
			// will be the pythagorean distance of i (x axis component) and
			// j (y axis component).  The sign of i and j can be determined based
			// on the distance between the centre point and each of the start
			// and end points.  i.e. both distances must be equal to the radius.

			double radius = Radius();

			// There are four possible centre points based on the ± sign applied
			// to each of the i and j terms.  The correct one will have a distance
			// of radius between it and each of the two endpoints.

			std::list<gp_Pnt> possible_centres;

			possible_centres.push_back( gp_Pnt( m_start.X() - m_i_term, m_start.Y() - m_j_term, m_start.Z() ) );
			possible_centres.push_back( gp_Pnt( m_start.X() + m_i_term, m_start.Y() - m_j_term, m_start.Z() ) );
			possible_centres.push_back( gp_Pnt( m_start.X() - m_i_term, m_start.Y() + m_j_term, m_start.Z() ) );
			possible_centres.push_back( gp_Pnt( m_start.X() + m_i_term, m_start.Y() + m_j_term, m_start.Z() ) );

			for (std::list<gp_Pnt>::iterator l_itPoint = possible_centres.begin(); l_itPoint != possible_centres.end(); l_itPoint++)
			{
				if (((l_itPoint->Distance( m_start ) - radius) < m_tolerance) &&
					((l_itPoint->Distance( m_end   ) - radius) < m_tolerance))
				{
					return( *l_itPoint );
				}
			} // End for

			return(gp_Pnt(0,0,0));	// It shouldn't get here.
		}

		case eFlash:
		default:
			return(m_start);

	} // End switch
}

gp_Dir RS274X::Trace::Direction() const
{
	switch(Interpolation())
	{
		case eCircular:
			if (m_clockwise) return(gp_Dir(0,0,-1));
			else	return(gp_Dir(0,0,1));

		case eLinear:
			return( gp_Dir( m_end.X() - m_start.X(), m_end.Y() - m_start.Y(), m_end.Z() - m_start.Z() ));

		default:
		case eFlash:
			return(gp_Dir(0,0,1));	// It doesn't matter.
	}
}

// Get gp_Circle for arc
gp_Circ RS274X::Trace::Circle() const
{
	gp_Ax1 axis( Centre(), Direction() );
	return gp_Circ(gp_Ax2(Centre(),Direction()), Radius());
}

gp_Lin RS274X::Trace::Line() const
{
	return(gp_Lin(Start(),Direction()));
}

			/*bool Intersects( const Trace & rhs ) const
			{
				std::list<double> points;
				return( HeeksObject()->Intersects( rhs.HeeksObject(), &points ) > 0);
			} // End Intersects() method*/


/* static */ double RS274X::AngleBetweenVectors(
        const gp_Pnt & vector_1_start_point,
        const gp_Pnt & vector_1_end_point,
        const gp_Pnt & vector_2_start_point,
        const gp_Pnt & vector_2_end_point,
        const double minimum_angle )
{
    gp_Vec vector1( vector_1_start_point, vector_1_end_point );
    gp_Vec vector2( vector_2_start_point, vector_2_end_point );
    gp_Vec reference( 0, 0, 1 );    // Looking from the top down.

    double angle = vector1.AngleWithRef( vector2, reference );
    while (angle < minimum_angle) angle += (2 * M_PI);
    return(angle);
}


double RS274X::Trace::StartAngle() const
{
    if (Clockwise())
    {
        double end_angle = RS274X::AngleBetweenVectors( Centre(), End(), gp_Pnt(0,0,0), gp_Pnt(1,0,0), 0.0 );
        double start_angle = RS274X::AngleBetweenVectors( Centre(), Start(), gp_Pnt(0,0,0), gp_Pnt(1,0,0), end_angle );
        return(start_angle);
    }
    else
    {
        return(RS274X::AngleBetweenVectors( Centre(), Start(), gp_Pnt(0,0,0), gp_Pnt(1,0,0), 0.0 ));
     }
}

double RS274X::Trace::EndAngle() const
{
    if (Clockwise())
    {
        return(RS274X::AngleBetweenVectors( Centre(), End(), gp_Pnt(0,0,0), gp_Pnt(1,0,0), 0.0 ));
    }
    else
    {
        double start_angle = RS274X::AngleBetweenVectors( Centre(), Start(), gp_Pnt(0,0,0), gp_Pnt(1,0,0), 0.0 );
        double end_angle = RS274X::AngleBetweenVectors( Centre(), End(), gp_Pnt(0,0,0), gp_Pnt(1,0,0), start_angle );
        return(end_angle);
     }
}

double RS274X::Trace::Length() const
{
	switch (Interpolation())
	{
		case eLinear:
			return(Start().Distance(End()));
			break;

		case eCircular:
			if ((abs(m_i_term) < m_tolerance) && (abs(m_j_term) < m_tolerance) && (Radius() > m_tolerance))
			{
				// It's a full circle.
				return(2.0 * M_PI * Radius());
			} // End if - then
			else
			{
				if ((Start().Distance(End()) < 0.000001) && (Radius() < 0.00001)) return(0.0);

				double arc_angle = EndAngle() - StartAngle();
				double arc_length = (arc_angle / (2.0 * M_PI)) * (2.0 * M_PI * Radius());
				return(abs(arc_length));
			} // End if - else
			break;

		case eFlash:
		default:
			return( 2.0 * M_PI * Radius() );
	} // End switch
} // End Length() method

bool RS274X::Trace::operator==( const Trace & rhs ) const
{
	if (Interpolation() != rhs.Interpolation()) return(false);
	if (Start().X() != rhs.Start().X()) return(false);
	if (Start().Y() != rhs.Start().Y()) return(false);
	if (Start().Z() != rhs.Start().Z()) return(false);

	if (End().X() != rhs.End().X()) return(false);
	if (End().Y() != rhs.End().Y()) return(false);
	if (End().Z() != rhs.End().Z()) return(false);

	if (I() != rhs.I()) return(false);
	if (J() != rhs.J()) return(false);

	if (Clockwise() != rhs.Clockwise()) return(false);

	return(true);	// They're equal
} // End equivalence operator

/* static */ double RS274X::Trace::Area( const TopoDS_Face & face )
{
	GProp_GProps properties;
	BRepGProp::LinearProperties(face, properties);

	return(properties.Mass());
} // End Area() method


HeeksObj *RS274X::Trace::CentrelineGraphics() const
{
	switch (Interpolation())
	{
		case eLinear:
		{
			//it's a line
			if (Length() < 0.00001)
            {
                return(NULL);
            }

			if (Length() > 0)
			{

                double start[3]; Point(Start()).ToDoubleArray(start);
                double end[3];   Point(End()).ToDoubleArray(end);
                return( heekscad_interface.NewLine( start, end ) );
			}
		}

		case eCircular:
		{
		    if (Length() < 0.00001)
            {
               return(NULL);
            }

			if ((abs(m_i_term) < m_tolerance) && (abs(m_j_term) < m_tolerance) && (Radius() > m_tolerance))
			{
				// It's a full circle.
				gp_Circ circ(gp_Ax2(Start(),gp_Dir(0,0,-1)), (m_aperture.OutsideDiameter()/2.0));
				return( new HCircle( circ, &wxGetApp().current_color ) );
			}
			else
			{
				// It's an arc
                double start[3]; Point(Start()).ToDoubleArray(start);
                double end[3];   Point(End()).ToDoubleArray(end);
                double centre[3]; Point(Centre()).ToDoubleArray(centre);
                double up[3];

                up[0] = 0;
                up[1] = 0;
                up[2] = (Clockwise()?-1:+1);

				return(heekscad_interface.NewArc(start, end, centre, up ));
			}
			break;
		}

		case eFlash:
		{
			return(NULL);   // We don't want to draw the outlines of aperture flashes for a centre-line drawing.
		}
	} // End switch

	return(NULL);
}

TopoDS_Face RS274X::Trace::Face() const
{
	BRepBuilderAPI_MakeWire make_wire;
	TopoDS_Wire wire;



	switch (Interpolation())
	{
		case eLinear:
		{
			//it's a line
			if (Length() < 0.00001)
            {
                TopoDS_Face empty;
                return(empty);
            }

			gp_Vec v(Start(), End());
			v = v.Normalized();
			//vector in 90 degree angle to the left
			gp_Vec n(-v.Y(), v.X(), 0);
			double d = m_aperture.OutsideDiameter();
			//first line is on the right side of the original

			if (Length() > 0)
			{
				// Draw a closed shape consisting of two straight lines and two half circles (to
				// tap the ends with).  Ideally we would add a m_aperture.Face(Start()) face to
				// a square ended straight (wide) line so that we would inerit whatever shape
				// the aperture is.  Unfortunately, I still can't figure out how to union faces
				// in OpenCascade so, for now, we will just assume a round aperture was used to
				// paint this line.

				gp_Circ arc_at_start(gp_Ax2(Start(),gp_Dir(0,0,-1)), (m_aperture.OutsideDiameter()/2.0));
				gp_Circ arc_at_end(gp_Ax2(End(),gp_Dir(0,0,-1)), (m_aperture.OutsideDiameter()/2.0));

				Handle(Geom_TrimmedCurve) seg1 = GC_MakeSegment(End().Translated(-n*d*.5), Start().Translated(-n*d*.5));
				Handle(Geom_TrimmedCurve) seg2 = GC_MakeArcOfCircle(arc_at_start, Start().Translated(-n*d*.5), Start().Translated( n*d*.5), true );
				Handle(Geom_TrimmedCurve) seg3 = GC_MakeSegment(Start().Translated(n*d*.5),    End().Translated( n*d*.5));
				Handle(Geom_TrimmedCurve) seg4 = GC_MakeArcOfCircle(arc_at_end, End().Translated( n*d*.5),   End().Translated(-n*d*.5), true );

				TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(seg1);
				TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(seg2);
				TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(seg3);
				TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(seg4);

				BRepBuilderAPI_MakeWire wire_maker;

				wire_maker.Add(edge1);
				wire_maker.Add(edge2);
				wire_maker.Add(edge3);
				wire_maker.Add(edge4);

				TopoDS_Face face1 = BRepBuilderAPI_MakeFace(wire_maker.Wire());
				return(face1);
			}
		}

		case eCircular:
		{
		    if (Length() < 0.00001)
            {
                TopoDS_Face empty;
                return(empty);
            }

			if ((abs(m_i_term) < m_tolerance) && (abs(m_j_term) < m_tolerance) && (Radius() > m_tolerance))
			{
				// It's a full circle.
				gp_Pnt left( Start() ); left.SetX( left.X() - (m_aperture.OutsideDiameter()/2.0) );
				gp_Pnt right( Start() ); right.SetX( right.X() + (m_aperture.OutsideDiameter()/2.0) );
				gp_Circ circ(gp_Ax2(Start(),gp_Dir(0,0,-1)), (m_aperture.OutsideDiameter()/2.0));

				Handle(Geom_TrimmedCurve) left_half = GC_MakeArcOfCircle(circ, left, right, true );
				Handle(Geom_TrimmedCurve) right_half = GC_MakeArcOfCircle(circ, right, left, true );

				TopoDS_Edge left_edge = BRepBuilderAPI_MakeEdge(left_half);
				TopoDS_Edge right_edge = BRepBuilderAPI_MakeEdge(right_half);

				BRepBuilderAPI_MakeWire wire_maker;

				wire_maker.Add(left_edge);
				wire_maker.Add(right_edge);

				TopoDS_Face face = BRepBuilderAPI_MakeFace(wire_maker.Wire());
				return(face);
			}
			else
			{
				// It's an arc
				gp_Vec vx(1.0, 0.0, 0.0);
				gp_Vec vy(0.0, 1.0, 0.0);

				double radius = (m_aperture.OutsideDiameter()/2.0);
				gp_Pnt p1 = Start().Translated(vx*radius*cos(StartAngle())+vy*radius*sin(StartAngle()));

				radius = (m_aperture.OutsideDiameter()/2.0);
				gp_Pnt p2 = Start().Translated(vx*radius*cos(StartAngle() + M_PI)+vy*radius*sin(StartAngle() + M_PI));

				radius = (m_aperture.OutsideDiameter()/2.0);
				gp_Pnt p3 = End().Translated(vx*radius*cos(EndAngle())+vy*radius*sin(EndAngle()));

				radius = (m_aperture.OutsideDiameter()/2.0);
				gp_Pnt p4 = End().Translated(vx*radius*cos(EndAngle() + M_PI)+vy*radius*sin(EndAngle() + M_PI));

				gp_Dir dir1, dir2;

				gp_Circ arc_at_start;
				gp_Circ arc_at_end;

				if (Clockwise())
				{
					dir1 = gp_Dir(0,0,-1);
					dir2 = gp_Dir(0,0,+1);

					arc_at_start = gp_Circ(gp_Ax2(Start(),dir2), (m_aperture.OutsideDiameter()/2.0));
					arc_at_end = gp_Circ(gp_Ax2(End(),dir1), (m_aperture.OutsideDiameter()/2.0));
				}
				else
				{
					dir1 = gp_Dir(0,0,+1);
					dir2 = gp_Dir(0,0,-1);

					arc_at_start = gp_Circ(gp_Ax2(Start(),dir1), (m_aperture.OutsideDiameter()/2.0));
					arc_at_end = gp_Circ(gp_Ax2(End(),dir1), (m_aperture.OutsideDiameter()/2.0));
				}

				gp_Circ inside_arc(gp_Ax2(Centre(),dir1), Radius() - (m_aperture.OutsideDiameter()/2.0));
				gp_Circ outside_arc(gp_Ax2(Centre(),dir2), Radius() + (m_aperture.OutsideDiameter()/2.0));

				Handle(Geom_TrimmedCurve) seg1 = GC_MakeArcOfCircle(arc_at_start, p1, p2, true );
				Handle(Geom_TrimmedCurve) seg2 = GC_MakeArcOfCircle(inside_arc, p2, p3, true );
				Handle(Geom_TrimmedCurve) seg3 = GC_MakeArcOfCircle(arc_at_end, p3, p4, true );
				Handle(Geom_TrimmedCurve) seg4 = GC_MakeArcOfCircle(outside_arc, p4, p1, true );

				TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(seg1);
				TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(seg2);
				TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(seg3);
				TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(seg4);

				BRepBuilderAPI_MakeWire wire_maker;

				wire_maker.Add(edge1);
				wire_maker.Add(edge2);
				wire_maker.Add(edge3);
				wire_maker.Add(edge4);

				TopoDS_Face face = BRepBuilderAPI_MakeFace(wire_maker.Wire());
				return(face);
			}
			break;
		}

		case eFlash:
		{
			return(m_aperture.Face(m_start));
		}
	} // End switch
	return(TopoDS_Face());
}


void RS274X::Trace::ExposeFilm( RS274X::Bitmap & pcb )
{
	switch (Interpolation())
	{
		case eLinear:
		{
			//it's a line.  Expose the aperture at one-pixel spacing along the line's path.
			unsigned int number_of_points = (unsigned int) (floor(Length() * Bitmap::PixelsPerMM()));
			for (unsigned int i=0; i<number_of_points; i++)
			{
			    double x = (double(double(i) / double(number_of_points)) * (End().X() - Start().X())) + Start().X();
			    double y = (double(double(i) / double(number_of_points)) * (End().Y() - Start().Y())) + Start().Y();

				pcb.ExposeFilm( *(m_aperture.GetBitmap()), gp_Pnt( x, y, 0.0 ) );
			} // End for

			return;
		}

		case eCircular:
		{
			if ((abs(m_i_term) < m_tolerance) && (abs(m_j_term) < m_tolerance) && (Radius() > m_tolerance))
			{
				// It's a full circle.
				unsigned int i = 0;
				unsigned int number_of_points = (unsigned int) (floor(Length() * Bitmap::PixelsPerMM()));
				double alpha = 3.1415926 * 2 / number_of_points;
				while( i++ < number_of_points )
				{
					double theta = alpha * i;

					double x = (cos( theta ) * Radius()) + Centre().X();
					double y = (sin( theta ) * Radius()) + Centre().Y();

					pcb.ExposeFilm( *(m_aperture.GetBitmap()), gp_Pnt( x, y, 0 ));
				} // End while
				return;
			}
			else
			{
				// It's an arc
				if (Clockwise())
				{
				    // We're turning clockwise so we want the end_angle to be smaller than the start_angle.
					double increment = (StartAngle() - EndAngle()) / (Length() * Bitmap::PixelsPerMM());
					for (double angle = EndAngle(); angle <= StartAngle(); angle += increment)
					{
						double x = (cos( angle ) * Radius()) + Centre().X();
						double y = (sin( angle ) * Radius()) + Centre().Y();

						pcb.ExposeFilm( *(m_aperture.GetBitmap()), gp_Pnt( x, y, 0 ));
					} // End for
					return;
				} // End if - then
				else
				{
					// Counter-Clockwise
					// We're turning clockwise so we want the start_angle to be smaller than the end_angle.
					double increment = (EndAngle() - StartAngle()) / (Length() * Bitmap::PixelsPerMM());
					for (double angle = StartAngle(); angle <= EndAngle(); angle += increment)
					{
						double x = (cos( angle ) * Radius()) + Centre().X();
						double y = (sin( angle ) * Radius()) + Centre().Y();

						pcb.ExposeFilm( *(m_aperture.GetBitmap()), gp_Pnt( x, y, 0 ));
					} // End for
				} // End if - else
				return;
			}
			break;
		}

		case eFlash:
		{
			gp_Pnt point(Start());

			pcb.ExposeFilm( *(m_aperture.GetBitmap()), point );
			return;
		}
	} // End switch

} // End Expose() method.



const RS274X::Bitmap *RS274X::Aperture::GetBitmap()
{
	if (m_pBitmap.get() != NULL)
	{
		return(m_pBitmap.get());
	}

	switch (m_type)
	{
		case eCircular:
		{
			m_pBitmap = std::auto_ptr<Bitmap>(new Bitmap( BoundingBox() ));

			// We need to fill in a solid circle.  Start from the centre and a small radius
			// and run around the circle marking pixels black.  Then increase the radius
			// and repeat.  This goes all the way up to the final radius.  The number of
			// steps around the circle and the increase in radius is all down to the resolution
			// of the bitmap.

			double radius = OutsideDiameter() / 2.0;
			for (double x=-1.0 * radius; x<= radius; x += Bitmap::MMPerPixel())
			{
				for (double y=-1.0 * radius; y<= radius; y += Bitmap::MMPerPixel())
				{
					double distance = (double) sqrt( double(double(x * x) + double(y * y)) );
					if (distance <= OutsideDiameter() / 2.0)
					{
						m_pBitmap->operator ()(x, y) = ~0;	// black.
					} // End if - then
				} // End for
			} // End for

			return(m_pBitmap.get());
		}

		case eRectangular:
		{
			m_pBitmap = std::auto_ptr<Bitmap>(new Bitmap( BoundingBox() ));

			for (double x=-1.0 * XAxisOutsideDimension() / 2.0; x< +1.0 * XAxisOutsideDimension() / 2.0; x += Bitmap::MMPerPixel())
			{
				for (double y=-1.0 * YAxisOutsideDimension() / 2.0; y<+1.0 * YAxisOutsideDimension() / 2.0; y += Bitmap::MMPerPixel())
				{
					m_pBitmap->operator ()( x, y ) = ~0;	// Black.
				} // End for
			} // End for

			return(m_pBitmap.get());
		}

		default:
			printf("Unsupported aperture shape found\n");
			m_pBitmap = std::auto_ptr<Bitmap>(new Bitmap( BoundingBox() ));
			return(m_pBitmap.get());
	} // End switch
}


CBox RS274X::Aperture::BoundingBox() const
{
	switch (m_type)
	{
		case eCircular:
		{
			CBox bounding_box;
			bounding_box.Insert(0,0,0);
			bounding_box.Insert(OutsideDiameter(), OutsideDiameter(), 0);
			return(bounding_box);
		}

		case eRectangular:
		{
			CBox bounding_box;
			bounding_box.Insert(0,0,0);
			bounding_box.Insert(XAxisOutsideDimension(), YAxisOutsideDimension(), 0);
			return(bounding_box);
		}


		default:
			printf("Unsupported aperture shape found\n");
			return(CBox());
	} // End switch
} // End BoundingBox() method


CBox RS274X::Trace::BoundingBox() const
{
	switch (Interpolation())
	{
		case eLinear:
		{
			CBox bounding_box;
			bounding_box.Insert( Start().X() - m_aperture.BoundingBox().Width(),
								 Start().Y() - m_aperture.BoundingBox().Height(),
								 0.0 );

			bounding_box.Insert( Start().X() + m_aperture.BoundingBox().Width(),
								 Start().Y() + m_aperture.BoundingBox().Height(),
								 0.0 );

			bounding_box.Insert( End().X() + m_aperture.BoundingBox().Width(),
								 End().Y() + m_aperture.BoundingBox().Height(),
								 0.0 );

			bounding_box.Insert( End().X() - m_aperture.BoundingBox().Width(),
								 End().Y() - m_aperture.BoundingBox().Height(),
								 0.0 );

			return(bounding_box);
		}

		case eCircular:
		{
			CBox bounding_box;
			bounding_box.Insert(0,0,0);

			if ((abs(m_i_term) < m_tolerance) && (abs(m_j_term) < m_tolerance) && (Radius() > m_tolerance))
			{
				// It's a full circle.
				unsigned int i = 0;
				unsigned int number_of_points = (unsigned int) (floor(Length() * Bitmap::PixelsPerMM()));
				double alpha = 3.1415926 * 2 / number_of_points;
				while( i++ < number_of_points )
				{
					double theta = alpha * i;
					double x = cos( theta ) * Radius();
					double y = sin( theta ) * Radius();
					bounding_box.Insert( x + m_aperture.BoundingBox().Width(), y + m_aperture.BoundingBox().Height(), 0);
					bounding_box.Insert( x - m_aperture.BoundingBox().Width(), y - m_aperture.BoundingBox().Height(), 0);
				} // End while
			}
			else
			{
				// It's an arc
				if (Clockwise())
				{
					double increment = (EndAngle() - StartAngle()) / (Length() * Bitmap::PixelsPerMM());
					for (double angle = EndAngle(); angle <= StartAngle(); angle -= increment)
					{
						double x = cos( angle ) * Radius();
						double y = sin( angle ) * Radius();
						bounding_box.Insert( x + m_aperture.BoundingBox().Width(), y + m_aperture.BoundingBox().Height(), 0);
						bounding_box.Insert( x - m_aperture.BoundingBox().Width(), y - m_aperture.BoundingBox().Height(), 0);
					} // End for
				} // End if - then
				else
				{
					// Counter-Clockwise
					double increment = (EndAngle() - StartAngle()) / (Length() * Bitmap::PixelsPerMM());
					for (double angle = StartAngle(); angle <= EndAngle(); angle += increment)
					{
						double x = cos( angle ) * Radius();
						double y = sin( angle ) * Radius();
						bounding_box.Insert( x + m_aperture.BoundingBox().Width(), y + m_aperture.BoundingBox().Height(), 0);
						bounding_box.Insert( x - m_aperture.BoundingBox().Width(), y - m_aperture.BoundingBox().Height(), 0);
					} // End for
				} // End if - else

				return(bounding_box);
			}
			break;
		}

		case eFlash:
		default:
		{
			return(m_aperture.BoundingBox());
		}
	} // End switch

	return(m_aperture.BoundingBox());

} // End BoundingBox() method

CBox RS274X::BoundingBox() const
{
    CBox bounding_box;

    for (Traces_t::const_iterator l_itTrace = m_traces.begin(); l_itTrace != m_traces.end(); l_itTrace++ )
	{
        bounding_box.Insert( l_itTrace->BoundingBox() );
	}

	for (FilledAreas_t::const_iterator l_itArea = m_filled_areas.begin(); l_itArea != m_filled_areas.end(); l_itArea++)
	{
		for (Traces_t::const_iterator l_itTrace = l_itArea->begin(); l_itTrace != l_itArea->end(); l_itTrace++)
		{
			bounding_box.Insert( l_itTrace->BoundingBox() );
		}
	}

	return(bounding_box);
}



bool RS274X::Bitmap::Save( const wxString file_name ) const
{
	const bool success = true;
	const bool failure = false;

	FILE *fp = fopen(Ttc(file_name.c_str()),"w");
	if (! fp)
	{
		printf("Could not open '%s' for writing\n", Ttc(file_name));
		return(failure);
	}

	for (int row=PixelsPerColumn(); row>=0; row--)  // Raster images have positive Y from the top down where we use bottom up.
	{
	    for (int col=0; col < PixelsPerRow(); col++)
	    {
	        fputc( m_bitmap[ (row * PixelsPerRow()) + col ], fp );	// Red
	        fputc( m_bitmap[ (row * PixelsPerRow()) + col ], fp );	// Green
	        fputc( m_bitmap[ (row * PixelsPerRow()) + col ], fp );	// Blue
	    }
	}

	fclose(fp);

	wxString output_file_name(file_name.BeforeLast('.'));
	if (output_file_name.length() == 0)
	{
	    output_file_name = file_name;
	}
	// output_file_name << _T(".gif");

	wxString script_file_name;
	script_file_name << output_file_name << _T(".gif");

	// Now generate a script that can be used to convert this file into a GIF format file for viewing.
    fp = fopen(Ttc(script_file_name.c_str()),"w");
    if (fp != NULL)
    {
        wxString contents;

        contents << _T("#!/bin/bash\n\n");
        contents << _("# This file converts the RAW image file generated by the HeeksCAD import of\n");
        contents << _("# a GERBER format data file into a GIF format file.   This just makes it\n");
        contents << _("# more convenient for viewing\n\n");
        contents << _T("rawtoppm < \"") << file_name << _T("\" ") << PixelsPerRow() << _T(" ") << PixelsPerColumn() << _T(" | ppmtogif > \"") << output_file_name << _T(".gif") << _T("\"\n");

        fprintf(fp,"%s\n", Ttc(contents.c_str()));
    }

    fclose(fp);
	return(success);

} // End Save() method


static double proportion( const double width, const int total_pixels, const int pixel )
{
    double _total_pixels(total_pixels);
    double _pixel(pixel);

    return(((_total_pixels - _pixel) / _total_pixels) * width);
}



bool RS274X::Bitmap::ExposeFilm( const Bitmap & pattern, const gp_Pnt & location )
{
    const bool success = true;
    const bool failure = false;

    if ((pattern.Width() >= Width()) || (pattern.Height() >= Height()))
    {
        printf("Pattern is smaller than this bitmap\n");
        return(failure);
    }

    // The real-world coordinates of 'this' bitmap have the 0,0 point at the bottom left
    // corner while the 0,0 point of the rhs map is in the centre.  This assumes that the
    // rhs bitmap represents an aperture while 'this' one represents the PCB.

    for (int pattern_row = 0; pattern_row < pattern.PixelsPerRow(); pattern_row++)
    {
        for (int pattern_col = 0; pattern_col < pattern.PixelsPerColumn(); pattern_col++)
        {
            double pattern_offset_x = proportion( pattern.Width(), pattern.PixelsPerRow(), pattern_row ) - (0.5 * pattern.Width());
            double pattern_offset_y = proportion( pattern.Height(), pattern.PixelsPerColumn(), pattern_col) - (0.5 * pattern.Height());

            int row = int(floor(double((location.Y() + pattern_offset_y - m_box.MinY()) * PixelsPerMM()))) + Boarder();
            int col = int(floor(double((location.X() + pattern_offset_x - m_box.MinX()) * PixelsPerMM()))) + Boarder();

            int pixel = int(floor(double((PixelsPerRow() * row) + col)));
            if ((pixel < 0) || (pixel > Size()-1))
            {
                printf("Pixel is out of range for bitmap\n");
                return(false);
            }

            int pattern_pixel = (pattern.PixelsPerRow() * pattern_row) + pattern_col;
            if ((pattern_pixel < 0) || (pattern_pixel > Size()-1))
            {
                printf("Pixel is out of range for pattern's bitmap\n");
                return(false);
            }

            m_bitmap[pixel] |= pattern.m_bitmap[ pattern_pixel ];
        }
    }

    return(success);
}

