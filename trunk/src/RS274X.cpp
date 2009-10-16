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

#include <sstream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <set>
#include <list>
#include <map>
#include <vector>

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

	m_LayerName = "";

	m_active_aperture = -1;	// None selected yet.

	m_area_fill = false;
	m_mirror_image = false;

	m_current_line = -1;
} // End constructor


// Filter out '\r' and '\n' characters.
char RS274X::ReadChar( const char *data, int *pos, const int max_pos )
{
	if (*pos < max_pos)
	{
		while ( ((*pos) < max_pos) && ((data[*pos] == '\r') || (data[*pos] == '\n')) )
		{
			if(data[*pos] == '\n') m_current_line++;
			(*pos)++;
		}
		if (*pos < max_pos)
		{
			return(data[(*pos)++]);
		} // End if - then>	HeeksCAD.exe!RS274X::ReadBlock(const char * data=0x060625f8, int * pos=0x0012f488, const int max_pos=15861)  Line 98 + 0x14 bytes	C++

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
        char c = ReadChar(data,pos,max_pos);

        if (c < 0) return(std::string(""));

        if (c == '%') delimiter = '%';
        else delimiter = '*';

        l_ossBlock << c;

        while (((c = ReadChar(data,pos,max_pos)) > 0) && (c != delimiter))
        {
                l_ossBlock << c;
        } // End while

        return(l_ossBlock.str());
} // End ReadBlock() method


bool RS274X::Read( const char *p_szFileName)
{
	printf("RS274X::Read(%s)\n", p_szFileName );

	std::ifstream input( p_szFileName, std::ios::in|std::ios::ate );
	if (input.is_open())
	{
		int size = (int) input.tellg();

		char *memblock = new char[size];

		input.seekg (0, std::ios::beg);
		input.read (memblock, size);
		input.close();

		m_current_line = 0;

		Polygon_t temppolygon;
		m_polygons.push_back(temppolygon);

		int pos = 0;
		while (pos < size)
		{
			std::string block = ReadBlock( memblock, &pos, size );
			if (block.size() > 0)
			{
				if (block[0] == '%')
				{
					// We're reading a parameter.
					if (! ReadParameters( block )) return(false);
				} // End if - then
				else
				{
					// It's a normal data block.
					if (! ReadDataBlock( block )) return(false);
				} // End if - else
			} // End if - then
		} // End while

		delete [] memblock;

		//std::cout<<"collecting Polygons from polygons and traces"<<std::endl;

		std::list<CPolygon> polygons_list;

		for (std::list<CPolygon>::iterator it_poly = m_polygons.begin(); it_poly != m_polygons.end(); it_poly++)
		{
			//std::cout<<"\t"<<it_poly->str()<<": "<<PDstr(it_poly->Direction())<<std::endl;
			//at least the last polygon should be empty, so skip it
			if(it_poly->empty()) continue;
			if(it_poly->Direction() == PolyUndefinedW)
			{
				std::cout<<"\tshould not happen: direction undefinable, not adding: "<<it_poly->str()<<std::endl;
				continue;
			}
			if(it_poly->Direction() != PolyCW)
			{
				std::cout<<"\treversing polygon"<<std::endl;
				it_poly->reverse();
				std::cout<<"\t->"<<it_poly->str()<<": "<<PDstr(it_poly->Direction())<<std::endl;
			}
			if(it_poly->Direction() != PolyCW)
			{
				std::cout<<"\tpolygon direction not cw"<<std::endl;
				continue;
			}
			polygons_list.push_back(*it_poly);
		}

		for (Traces_t::iterator l_itTrace = m_traces.begin(); l_itTrace != m_traces.end(); l_itTrace++ )
		{
			CPolygon polygon;
			l_itTrace->MakePolygon(polygon);
			if(polygon.Direction() == PolyCCW)
			{

				std::cout<<"\treversing polygon"<<std::endl;
				polygon.reverse();
			}
			if(polygon.empty())
			{
				std::cout<<"\tl_itTrace->MakePolygon() gave an empty polygon"<<std::endl;
				continue;
			}
			if(polygon.Direction() == PolyUndefinedW)
			{
				std::cout<<"\tshould not happen: direction undefinable, not adding: "<<polygon.str()<<std::endl;
				continue;
			}
			/*if(polygon.Direction() != PolyCW)
			{
				std::cout<<"\tpolygon direction not cw"<<std::endl;
				continue;
			}*/
			polygons_list.push_back(polygon);
		}

		/*const char *polygon_names[] =
		{
			"kalle","ike","happo","taateli","aku","hessu","iines","roope","mikki",
			"minni","kakka","kirje","biltema","sp-ele","paristo","teippi","ampuu"
		};
		int polygon_name_count = sizeof(polygon_names)/sizeof(char*);
		int i = 0;
		for(std::list<CPolygon>::iterator ipoly = polygons_list.begin();
				ipoly != polygons_list.end(); ipoly++)
		{
			if(i < polygon_name_count) ipoly->name = polygon_names[i];
			else{
				int num = (i / polygon_name_count)+1;
				std::stringstream ss;
				ss<<polygon_names[i%polygon_name_count]<<num;
				ipoly->name = ss.str();
			}
			i++;
		}*/

		std::list<CPolygon> result_list;
		//bool union_succeeded = UnionPolygons(polygons_list, result_list);
		bool union_succeeded = UnionPolygons_old(polygons_list, result_list);

		/*std::cout<<"polygons_list:"<<std::endl;
		for(std::list<CPolygon>::iterator i=polygons_list.begin(); i!=polygons_list.end(); i++)
		{
			std::cout<<"\t"<<i->str()<<": "<<PDstr(i->Direction())<<std::endl;
		}

		std::cout<<"result_list:"<<std::endl;
		for(std::list<CPolygon>::iterator i=result_list.begin(); i!=result_list.end(); i++)
		{
			std::cout<<"\t"<<i->str()<<": "<<PDstr(i->Direction())<<std::endl;
		}*/


		for (std::list<CPolygon>::iterator ipoly = result_list.begin(); ipoly != result_list.end(); ipoly++)
		{
			if(ipoly->empty()) continue;
			CSketch *sketch = ipoly->MakeSketch();
			if(sketch==NULL) continue;
			//if(sketch->GetSketchOrder() == SketchOrderTypeCloseCCW) sketch->ReverseSketch(false);
			wxGetApp().Add(sketch, NULL);
		}

		/*for (std::list<CPolygon>::iterator ipoly = polygons_list.begin(); ipoly != polygons_list.end(); ipoly++)
		{
			if(ipoly->empty()) continue;
			ipoly->Move(gp_Vec(50., 0., 0.));
			CSketch *sketch = ipoly->MakeSketch();
			if(sketch==NULL) continue;
			//if(sketch->GetSketchOrder() == SketchOrderTypeCloseCCW) sketch->ReverseSketch(false);
			if(undoably)wxGetApp().AddUndoably(sketch, NULL, NULL);
			else wxGetApp().Add(sketch, NULL);
		}*/


		return union_succeeded;

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

	if (_params.substr(0,4) == "MOIN") m_units = 25.4;
	else if (_params.substr(0,4) == "MOMM") m_units = 1.0;
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
		char *end = NULL;
		int tool_number = strtoul( _params.c_str(), &end, 10 );
		if ((end == NULL) || (end == _params.c_str()))
		{
			printf("Expected Aperture Type character\n");
			return(false);
		} // End if - then
		_params.erase(0, end - _params.c_str());

		char aperture_type = _params[0];
		_params.erase(0,1);	// Remove aperture type character.
		if (_params[0] == ',') _params.erase(0,1);	// Remove comma

		double modifier = strtod( _params.c_str(), &end );
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
					aperture.XAxisHoleDimension( double(strtod( _params.c_str(), &end )) * m_units );
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
					aperture.YAxisHoleDimension( double(strtod( _params.c_str(), &end )) * m_units );
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
				printf("Rectangular apertures are not yet supported\n");

				//TODO: just use a circle for now
				{
				// Push a circle within an outside diameter of modifier onto the list of apertures
				Aperture aperture;

				aperture.Type( Aperture::eCircular );
				aperture.OutsideDiameter( modifier * m_units );
				m_aperture_table.insert( std::make_pair( tool_number, aperture ) );
				}
				break;

			case 'O':	// Obround
				printf("ObRound apertures are not yet supported\n");
				//TODO: just use a circle for now
				{
				// Push a circle within an outside diameter of modifier onto the list of apertures
				Aperture aperture;

				aperture.Type( Aperture::eCircular );
				aperture.OutsideDiameter( modifier * m_units );
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
	else if (_params.substr(0,2) == "MI")
	{
		// Mirror Image = on.
		_params.erase(0,2);	// Remove MI
		m_mirror_image = true;
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
		while (_coord.size() < (unsigned int) digits_right_of_point) _coord.push_back('0');

		// use the end of the string as the reference point.
		result = atof( _coord.substr( 0, _coord.size() - digits_right_of_point ).c_str() );
		result += (atof( _coord.substr( _coord.size() - digits_right_of_point ).c_str() ) / pow(10.0, digits_right_of_point));
	} // End if - then
	else
	{
		while (_coord.size() < (unsigned int) digits_left_of_point) _coord.insert(0,"0");

		// use the beginning of the string as the reference point.
		result = atof( _coord.substr( 0, digits_left_of_point ).c_str() );
		result += (atof( _coord.substr( digits_left_of_point ).c_str() ) / pow(10.0, (int)(_coord.size() - digits_left_of_point)));
	} // End if - else

	result *= multiplier;

	// printf("RS274X::InterpretCoord(%s) = %lf\n", coordinate, result );
	return(result);
} // End InterpretCoord() method


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

	while (_data.size() > 0)
	{
		if (_data.substr(0,3) == "G04")
		{
			// Ignore data block.
			return(true);
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
		}
		else if (_data.substr(0,3) == "G37")
		{
			_data.erase(0,3);
			Polygons_t::iterator l_itPolygon = m_polygons.end();
			l_itPolygon--;
			//add a new polygon if needed
			AddPolygonIfNeeded();
			m_area_fill = false;
		}
		else if (_data.substr(0,1) == "I")
		{
			_data.erase(0,1);
			char *end = NULL;
			std::string sign;

			if (_data[0] == '-')
			{
				sign = "-";
				_data.erase(0,1);
			} // End if - then

			if (_data[0] == '+')
			{
				sign = "+";
				_data.erase(0,1);
			} // End if - then

			strtoul( _data.c_str(), &end, 10 );
			if ((end == NULL) || (end == _data.c_str()))
			{
				printf("Expected value for I parameter\n");
				return(false);
			} // End if - then
			std::string i_param = sign + _data.substr(0, end - _data.c_str());
			_data.erase(0, end - _data.c_str());
			i_term = InterpretCoord( i_param.c_str(), 
							m_YDigitsLeftOfPoint, 
							m_YDigitsRightOfPoint, 
							m_leadingZeroSuppression, 
							m_trailingZeroSuppression );
		}
		else if (_data.substr(0,1) == "J")
		{
			_data.erase(0,1);
			char *end = NULL;
			std::string sign;

			if (_data[0] == '-')
			{
				sign = "-";
				_data.erase(0,1);
			} // End if - then

			if (_data[0] == '+')
			{
				sign = "+";
				_data.erase(0,1);
			} // End if - then

			strtoul( _data.c_str(), &end, 10 );
			if ((end == NULL) || (end == _data.c_str()))
			{
				printf("Expected value for J parameter\n");
				return(false);
			} // End if - then
			std::string j_param = sign + _data.substr(0, end - _data.c_str());
			_data.erase(0, end - _data.c_str());
			j_term = InterpretCoord( j_param.c_str(), 
							m_YDigitsLeftOfPoint, 
							m_YDigitsRightOfPoint, 
							m_leadingZeroSuppression, 
							m_trailingZeroSuppression );
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
			char *end = NULL;
			std::string sign;

			if (_data[0] == '-')
			{
				sign = "-";
				_data.erase(0,1);
			} // End if - then

			if (_data[0] == '+')
			{
				sign = "+";
				_data.erase(0,1);
			} // End if - then

			strtoul( _data.c_str(), &end, 10 );
			if ((end == NULL) || (end == _data.c_str()))
			{
				printf("Expected aperture number following 'D'\n");
				return(false);
			} // End if - then
			std::string x = sign + _data.substr(0, end - _data.c_str());
			_data.erase(0, end - _data.c_str());
			position.SetX( InterpretCoord( x.c_str(), 
							m_YDigitsLeftOfPoint, 
							m_YDigitsRightOfPoint, 
							m_leadingZeroSuppression, 
							m_trailingZeroSuppression ) );
			if (m_mirror_image) position.SetX( position.X() * -1.0 ); // mirror about Y axis
		}
		else if (_data.substr(0,1) == "Y")
		{
			_data.erase(0,1);	// Erase Y
			char *end = NULL;
			std::string sign;

			if (_data[0] == '-')
			{
				sign = "-";
				_data.erase(0,1);
			} // End if - then

			if (_data[0] == '+')
			{
				sign = "+";
				_data.erase(0,1);
			} // End if - then

			strtoul( _data.c_str(), &end, 10 );
			if ((end == NULL) || (end == _data.c_str()))
			{
				printf("Expected aperture number following 'D'\n");
				return(false);
			} // End if - then
			std::string y = sign + _data.substr(0, end - _data.c_str());
			_data.erase(0, end - _data.c_str());
			position.SetY( InterpretCoord( y.c_str(), 
							m_YDigitsLeftOfPoint, 
							m_YDigitsRightOfPoint, 
							m_leadingZeroSuppression, 
							m_trailingZeroSuppression ));
		}
		else if (_data.substr(0,3) == "D01")
		{
			_data.erase(0,3);

			// We're going from the current position to x,y in
			// either a linear interpolated path or a circular
			// interpolated path.  We end up resetting our current
			// location to x,y

			if (m_part_circular_interpolation)
			{
				// arc
				// circular interpolation.
				
				if(m_area_fill)
				{
					Polygons_t::iterator l_itPolygon = m_polygons.end();
					l_itPolygon--;
					//TODO: create arc
					//l_itPolygon->push_back(trace);
				}
				else
				{
					Trace trace( m_aperture_table[m_active_aperture], Trace::eCircular );
					trace.Start( m_current_position );
					trace.End( position );
					trace.Clockwise( m_cw_circular_interpolation );
					trace.I( i_term );
					trace.J( j_term );
					m_traces.push_back( trace );
				}

				m_current_position = position;
			} // End if - then
			else if (m_full_circular_interpolation)
			{
				// full circle
				// circular interpolation.
				double radius = sqrt((i_term * i_term) + (j_term * j_term));
			
				if(m_area_fill)
				{
					Polygons_t::iterator l_itPolygon = m_polygons.end();
					l_itPolygon--;
					
					gp_Vec vx(1.0, 0.0, 0.0);
					gp_Vec vy(0.0, 1.0, 0.0);
					
					for(int i=0; i<8; i++){
						double angle = -PI*2.0*8.0*(double)i;
						l_itPolygon->push_back(position.Translated(vx*radius*cos(angle)+vy*radius*sin(angle)));
					}
				}
				else
				{
					Trace trace( m_aperture_table[m_active_aperture], Trace::eCircular );
					trace.Radius(radius);
					trace.Start( position );
					trace.End( position );
					trace.Clockwise( m_cw_circular_interpolation );
					m_traces.push_back( trace );
				}

				m_current_position = position;
			}
			else
			{
				// linear interpolation.

				if(m_area_fill)
				{
					Polygons_t::iterator l_itPolygon = m_polygons.end();
					l_itPolygon--;
					l_itPolygon->push_back(m_current_position);
				}
				else
				{
					Trace trace( m_aperture_table[m_active_aperture], Trace::eLinear );
					trace.Start( m_current_position );
					trace.End( position );
					m_traces.push_back( trace );
				}

				m_current_position = position;
			} // End if - else
		}
		else if (_data.substr(0,3) == "D02")
		{
			_data.erase(0,3);
			if(m_area_fill){
				AddPolygonIfNeeded();
			}
			m_current_position = position;
		}
		else if (_data.substr(0,3) == "D03")
		{
			//std::cout<<"d03"<<std::endl;
			_data.erase(0,3);
			m_current_position = position;
			if (m_aperture_table.find( m_active_aperture ) == m_aperture_table.end())
			{
				printf("Flash (D03) command issued without first selecting an aperture\n");
				return(false);
			} // End if - then

			Aperture aperture = m_aperture_table[ m_active_aperture ];
			/*Trace trace( aperture, Trace::eCircular );
			trace.Start( position );
			trace.End( position );
			trace.Radius( aperture.OutsideDiameter() / 2 );*/

			double radius = aperture.OutsideDiameter() / 2;

			Polygons_t::iterator l_itPolygon = m_polygons.end();
			l_itPolygon--;

			gp_Vec vx(1.0, 0.0, 0.0);
			gp_Vec vy(0.0, 1.0, 0.0);
			
			for(int i=0; i<8; i++){
				double angle = -PI*2.0*(double)i/8.0;
				l_itPolygon->push_back(position.Translated(vx*radius*cos(angle)+vy*radius*sin(angle)));
			}

			//add a new polygon
			AddPolygonIfNeeded();
			if (aperture.OutsideDiameter() < 0.0001) printf("WARNING: D03 found without radius information\n");
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
			_data.erase(0, end - _data.c_str());
	
			m_active_aperture = aperture_number;
			return(true);
		}
		else
		{
			printf("Unexpected command '%s'\n", _data.c_str() );
			return(false);
		} // End if - else
	} // End while

	return(true);
} // End ReadDataBlock() method

