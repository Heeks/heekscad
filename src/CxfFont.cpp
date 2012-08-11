#include "stdafx.h"
#ifdef wxUSE_UNICODE
	#ifndef _UNICODE
		#define _UNICODE
	#endif
#endif
#include <stdio.h>
#include <math.h>
#include <wx/string.h>
#include <wx/window.h>
#include <wx/strconv.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <sstream>
#include <stdexcept>
#include <memory>

#include "../interface/HeeksCADInterface.h"
#include "../interface/strconv.h"

#include "CxfFont.h"
#include <gp_Pnt.hxx>
#include <gp_Ax1.hxx>
#include <gp_Trsf.hxx>

extern CHeeksCADInterface heekscad_interface;


HeeksObj *VectorFont::Glyph::GlyphLine::Sketch( const gp_Pnt & location, const gp_Trsf & transformation_matrix, const float width, COrientationModifier *pOrientationModifier ) const
{
	gp_Pnt start_point( location );
	gp_Pnt end_point( location );

	start_point.SetX( location.X() + m_x1 );
	start_point.SetY( location.Y() + m_y1 );

	end_point.SetX( location.X() + m_x2 );
	end_point.SetY( location.Y() + m_y2 );

	if (pOrientationModifier)
	{
	    start_point = pOrientationModifier->Transform(transformation_matrix, location.Distance(gp_Pnt(0.0,0.0,0.0)), start_point, width );
	    end_point = pOrientationModifier->Transform(transformation_matrix, location.Distance(gp_Pnt(0.0,0.0,0.0)), end_point, width );
	}

	start_point.Transform( transformation_matrix );
	end_point.Transform( transformation_matrix );

	double start[3];
	double end[3];

	start[0] = start_point.X();
	start[1] = start_point.Y();
	start[2] = start_point.Z();

	end[0] = end_point.X();
	end[1] = end_point.Y();
	end[2] = end_point.Z();

	HeeksObj *line = heekscad_interface.NewLine( start, end );
	return(line);
} // End Sketch() method

void VectorFont::Glyph::GlyphLine::glCommands(
	const gp_Pnt & starting_point,
	const bool select,
	const bool marked,
	const bool no_color,
	COrientationModifier *pOrientationModifier,
	gp_Trsf transformation,
	const float width ) const
{
	gp_Pnt from( starting_point );
	gp_Pnt to( starting_point );

    from.SetX( starting_point.X() + m_x1);
	from.SetY( starting_point.Y() + m_y1);
	from.SetZ( starting_point.Z() );

	to.SetX( starting_point.X() + m_x2);
	to.SetY( starting_point.Y() + m_y2);
	to.SetZ( starting_point.Z() );

	if (pOrientationModifier)
	{
	    from = pOrientationModifier->Transform(transformation, starting_point.Distance(gp_Pnt(0.0,0.0,0.0)), from, width );
	    to = pOrientationModifier->Transform(transformation, starting_point.Distance(gp_Pnt(0.0,0.0,0.0)), to, width );
	}

	glBegin(GL_LINE_STRIP);
	glVertex3d(from.X(), from.Y(), from.Z());
	glVertex3d(to.X(), to.Y(), to.Z());
	glEnd();
} // End glCommands() method



std::list<gp_Pnt> VectorFont::Glyph::GlyphArc::Interpolate(const gp_Pnt & location, const unsigned int number_of_points ) const
{
	std::list<gp_Pnt> points;

	gp_Pnt origin(location);
	origin.SetX( origin.X() + m_xcentre );
	origin.SetY( origin.Y() + m_ycentre );

	double start_angle = m_start_angle;
	double end_angle = m_end_angle;

	if (start_angle > end_angle)
	{
		end_angle += (2 * M_PI);
	}

	double increment = (end_angle - start_angle) / number_of_points;

	gp_Dir z_direction( 0, 0, 1 );
	for (double angle = start_angle; angle <= end_angle; angle += increment)
	{
		gp_Pnt point( location.X() + m_xcentre + m_radius, location.Y() + m_ycentre, location.Z() );
		gp_Trsf rotation_matrix;
		rotation_matrix.SetRotation( gp_Ax1(origin, z_direction), angle );
		point.Transform(rotation_matrix);
		points.push_back(point);
	}

	return(points);
}


HeeksObj *VectorFont::Glyph::GlyphArc::Sketch( const gp_Pnt & location, const gp_Trsf & transformation_matrix, const float width, COrientationModifier *pOrientationModifier ) const
{
	double start[3];
	double end[3];
	double centre[3];
	double up[3];

	gp_Pnt centre_point( location.X() + m_xcentre, location.Y() + m_ycentre, location.Z() );
	gp_Pnt start_point( centre_point.X() + m_radius, centre_point.Y(), centre_point.Z() );
	gp_Pnt end_point( centre_point.X() + m_radius, centre_point.Y(), centre_point.Z() );

    gp_Dir z_direction( 0, 0, 1 );

    if (pOrientationModifier) centre_point = pOrientationModifier->Transform(transformation_matrix, location.Distance(gp_Pnt(0.0,0.0,0.0)), centre_point, width );

    if (pOrientationModifier) start_point = pOrientationModifier->Transform(transformation_matrix, location.Distance(gp_Pnt(0.0,0.0,0.0)), start_point, width );

	gp_Trsf start_rotation_matrix;
	start_rotation_matrix.SetRotation( gp_Ax1(centre_point, z_direction), m_start_angle );
	start_point.Transform(start_rotation_matrix);	// Rotate to start_angle

	start[0] = start_point.X();
	start[1] = start_point.Y();
	start[2] = start_point.Z();

	if (pOrientationModifier) end_point = pOrientationModifier->Transform(transformation_matrix, location.Distance(gp_Pnt(0.0,0.0,0.0)), end_point, width );

	gp_Trsf end_rotation_matrix;
	end_rotation_matrix.SetRotation( gp_Ax1(centre_point, z_direction), m_end_angle );
	end_point.Transform(end_rotation_matrix);	// Rotate to start_angle

	end[0] = end_point.X();
	end[1] = end_point.Y();
	end[2] = end_point.Z();



	centre[0] = centre_point.X();
	centre[1] = centre_point.Y();
	centre[2] = centre_point.Z();

	gp_Pnt up_point( 0.0, 0.0, 1.0 );

	// For counter-clockwise (always in this font format)
	up[0] = up_point.X();
	up[1] = up_point.Y();
	up[2] = up_point.Z();

	HeeksObj *arc = heekscad_interface.NewArc( start, end, centre, up );
	double m[16];
	extract(transformation_matrix,m);
	arc->ModifyByMatrix(m);
	return(arc);
} // End Sketch() method

void VectorFont::Glyph::GlyphArc::glCommands(
	const gp_Pnt & starting_point,
	const bool select,
	const bool marked,
	const bool no_color,
	COrientationModifier *pOrientationModifier,
	gp_Trsf transformation,
	const float width) const
{
	glBegin(GL_LINE_STRIP);
	std::list<gp_Pnt> vertices = Interpolate( starting_point, 20 );
	for (std::list<gp_Pnt>::iterator l_itVertex = vertices.begin(); l_itVertex != vertices.end(); l_itVertex++)
	{
		if (pOrientationModifier) *l_itVertex = pOrientationModifier->Transform(transformation, starting_point.Distance(gp_Pnt(0.0,0.0,0.0)), *l_itVertex, width );
		glVertex3d(l_itVertex->X(), l_itVertex->Y(), l_itVertex->Z());
	} // End for
	glEnd();
} // End glCommands() method


/**
    If the locale settings indicate that floating point numbers use a comma (,) as
    a decimal point then we're going to have a problem reading in the CXF font files.
    The CXF font files use commas as field separators and dots (.) as decimal
    points.  In order that the strtod() conversion work correctly, convert all the
    dot characters in the attached string into a comma so that the conversion
    to a floating point number works correctly.
    NOTE: This routine must not be called with a comma-separated list of
    values.  This list of values should be separated and each individual value
    passed in here separately.
 */
wxString VectorFont::Glyph::PrepareStringForConversion( wxString &value ) const
{
    typedef enum
    {
        eUnknown = 0,
        eConvertDotToComma,
        eNoConversionRequired
    } FloatingPointConversion_t;

    static FloatingPointConversion_t   floating_point_conversion = eUnknown;

    if (floating_point_conversion == eUnknown)
    {
        // We need to figure out if the local computer's locale settings use a comma
        // as a decimal point.
        wxString test;
        test << 3.1415;
        if (test.Find(',') != -1)
        {
            floating_point_conversion = eConvertDotToComma;
        }
        else
        {
            floating_point_conversion = eNoConversionRequired;
        }
    }

    if (floating_point_conversion == eConvertDotToComma)
    {
        value.Replace(_T("."), _T(","));
    }

    return(value);
}

/**
	In font terms, one point is 1/72 inches.  If we interpret the values in the
	source file as 'points' then we need to convert them to mm for HeeksCAD internal
	use.
 */
double VectorFont::Glyph::PointToMM( const double points ) const
{
	const double mm_per_point = (1.0 / 72.0) * 25.4;
	return(points * mm_per_point);
}


VectorFont::Glyph::Glyph( const std::list<std::string> &cxf_glyph_definition, const double word_space_percentage, const double character_space_percentage )
{
	m_word_space_percentage = word_space_percentage;
	m_character_space_percentage = character_space_percentage;

	for (std::list<std::string>::const_iterator l_itLine = cxf_glyph_definition.begin(); l_itLine != cxf_glyph_definition.end(); l_itLine++)
	{
		wxString line( Ctt(l_itLine->c_str()) );
		wxString delimiters( _T(" \t\r\n,") );

		std::vector<wxString> tokens = Tokens( line, delimiters );
		if (tokens.size() == 0)
		{
			std::ostringstream l_ossError;
			l_ossError << "Expected tokens in glyph definition";
			throw(std::runtime_error(l_ossError.str().c_str()));
		}

        // Replace dot (.) for comma (,) if the locale settings require it.
		for (std::vector<wxString>::iterator token = tokens.begin(); token != tokens.end(); token++)
		{
            *token = PrepareStringForConversion( *token );
		}

		switch (tokens[0][0])
		{
		case 'L':
			if (tokens.size() != 5)
			{
				std::ostringstream l_ossError;
				l_ossError << "Expected 5 tokens when defining a line.  We got "
							<< tokens.size() << " tokens from '" << l_itLine->c_str() << "\n";
				throw(std::runtime_error(l_ossError.str().c_str()));
			}
			else
			{
				GlyphLine *line = new GlyphLine(	 PointToMM(strtod( Ttc(tokens[1].c_str()), NULL )),
										 PointToMM(strtod( Ttc(tokens[2].c_str()), NULL )),
										 PointToMM(strtod( Ttc(tokens[3].c_str()), NULL )),
										 PointToMM(strtod( Ttc(tokens[4].c_str()), NULL )) );
				m_graphics_list.push_back( line );
				m_bounding_box.Insert( line->BoundingBox() );
			}
			break;

		case 'A':

			if (tokens.size() != 6)
			{
				std::ostringstream l_ossError;
				l_ossError << "Expected 6 tokens when defining an arc";
				throw(std::runtime_error(l_ossError.str().c_str()));
			}
			else
			{
				if ((tokens[0].size() == 2) && (tokens[0][1] == 'R'))
				{
					// Reverse the starting and ending points.
					GlyphArc *arc = new GlyphArc( PointToMM(strtod( Ttc(tokens[1].c_str()), NULL )),
										 PointToMM(strtod( Ttc(tokens[2].c_str()), NULL )),
										 PointToMM(strtod( Ttc(tokens[3].c_str()), NULL )),
										 strtod( Ttc(tokens[5].c_str()), NULL) ,
										 strtod( Ttc(tokens[4].c_str()), NULL ));
					m_graphics_list.push_back( arc );
					m_bounding_box.Insert( arc->BoundingBox() );
				} // End if - then
				else
				{
					GlyphArc *arc = new GlyphArc( PointToMM(strtod( Ttc(tokens[1].c_str()), NULL )),
										 PointToMM(strtod( Ttc(tokens[2].c_str()), NULL )),
										 PointToMM(strtod( Ttc(tokens[3].c_str()), NULL )),
										 strtod( Ttc(tokens[4].c_str()), NULL ),
										 strtod( Ttc(tokens[5].c_str()), NULL ) );
					m_graphics_list.push_back( arc );
					m_bounding_box.Insert( arc->BoundingBox() );
				}
			}
			break;

		default:
			std::ostringstream l_ossError;
			l_ossError << "Unexpected graphics element type '" << tokens[0][0] << "'";
			throw(std::runtime_error(l_ossError.str().c_str()));
		} // End switch
	} // End for
} // End constructor

VectorFont::Glyph::Glyph( const std::string &hershey_glyph_definition, const double word_space_percentage, const double character_space_percentage )
{
	m_word_space_percentage = word_space_percentage;
	m_character_space_percentage = character_space_percentage;

	std::string definition(hershey_glyph_definition);

	if (definition.size() >= 2)
	{
	    double left_edge = PointToMM(double(definition[0] - 'R'));
	    double right_edge = PointToMM(double( definition[1] - 'R'));
	    definition.erase(0,2);

        m_bounding_box.Insert( left_edge, 0, 0 );
        m_bounding_box.Insert( right_edge, 0, 0 );

		if (definition.size() >= 2)
		{
			double x = double(definition[0] - 'R');
			double y = double(definition[1] - 'R') * -1.0;
			definition.erase(0,2);

			while (definition.size() > 0)
			{
				if (definition.size() >= 2)
				{
					if ((definition[0] == ' ') && (definition[1] == 'R'))
					{
						// This is a 'pen up' item.  Set the new coordinates but don't draw anything.
						definition.erase(0,2);
						x = double(definition[0] - 'R');
						y = double(definition[1] - 'R') * -1.0;
					}
					else
					{
						double to_x = double(definition[0] - 'R');
						double to_y = double(definition[1] - 'R') * -1.0;

						GlyphLine *line = new GlyphLine( PointToMM(x), PointToMM(y), PointToMM(to_x), PointToMM(to_y) );
						m_graphics_list.push_back( line );
						m_bounding_box.Insert( line->BoundingBox() );
						x = to_x;
						y = to_y;
					}

				}

				definition.erase(0,2);
			}
		}
	} // End if - then
} // End constructor

VectorFont::Glyph::~Glyph()
{
	for (GraphicsList_t::iterator l_itGraphic = m_graphics_list.begin();
		l_itGraphic != m_graphics_list.end(); l_itGraphic++)
	{
		delete *l_itGraphic;
	} // End for

	m_graphics_list.clear();
} // End destructor


VectorFont::Glyph::Glyph( const VectorFont::Glyph & rhs )
{
	*this = rhs;	// call the assignment operator
}

VectorFont::Glyph & VectorFont::Glyph::operator= ( const VectorFont::Glyph & rhs )
{
	if (this != &rhs)
	{
		for (GraphicsList_t::iterator l_itGraphic = m_graphics_list.begin();
			l_itGraphic != m_graphics_list.end(); l_itGraphic++)
		{
			delete *l_itGraphic;
		} // End for

		m_graphics_list.clear();

		for (GraphicsList_t::const_iterator l_itGraphic = rhs.m_graphics_list.begin();
			l_itGraphic != rhs.m_graphics_list.end(); l_itGraphic++)
		{
			m_graphics_list.push_back( (*l_itGraphic)->Duplicate() );
		} // End for

		m_bounding_box = rhs.m_bounding_box;
	} // End if - then

	return(*this);
}

/**
	The location is relative both to (0,0,0) and, from there, to the point along the text string
	for this character.  The transformation matrix is both a translation (movement) and a
	rotation function that is maintained by the HText class based on where the operator has
	placed the text.  i.e. the location is in 'internal' coordinates and those are then
	transformed (moved and/or rotated) by the rotation matrix to determine the final
	coordinates.  This is handled differently to the glCommands() method because the OpenGL
	libraries will have had the transformation matrix pushed onto the stack so that all
	OpenGL coordinates will be implicitly transformed.
 */
HeeksObj *VectorFont::Glyph::Sketch( const gp_Pnt & location, const gp_Trsf & transformation_matrix, const float width, COrientationModifier *pOrientationModifier ) const
{
	HeeksObj *sketch = heekscad_interface.NewSketch();

	for (GraphicsList_t::const_iterator l_itGraphic = m_graphics_list.begin(); l_itGraphic != m_graphics_list.end(); l_itGraphic++)
	{
		sketch->Add((*l_itGraphic)->Sketch( location, transformation_matrix, width, pOrientationModifier ), NULL);
	} // End for

	return(sketch);
} // End Sketch() method

std::list<HeeksObj *> VectorFont::Glyph::GetGraphics( const gp_Pnt & location, const gp_Trsf & transformation_matrix, const float width, COrientationModifier *pOrientationModifier ) const
{
	std::list<HeeksObj *> results;

	for (GraphicsList_t::const_iterator l_itGraphic = m_graphics_list.begin(); l_itGraphic != m_graphics_list.end(); l_itGraphic++)
	{
		results.push_back( (*l_itGraphic)->Sketch( location, transformation_matrix, width, pOrientationModifier ) );
	} // End for

	return(results);
} // End Graphics() method


void VectorFont::Glyph::glCommands(
		const gp_Pnt & starting_point,
		const bool select,
		const bool marked,
		const bool no_color,
		COrientationModifier *pOrientationModifier,
		gp_Trsf transformation,
		const float width ) const
{
	for (GraphicsList_t::const_iterator l_itGraphic = m_graphics_list.begin(); l_itGraphic != m_graphics_list.end(); l_itGraphic++)
	{
		(*l_itGraphic)->glCommands( starting_point, select, marked, no_color, pOrientationModifier, transformation, width );
	} // End for
} // End glCommands() method



/* static */ std::list<wxString> VectorFonts::GetFileNames( const wxString & Root )
#ifdef WIN32
{
	std::list<wxString>	results;

	WIN32_FIND_DATA file_data;
	HANDLE hFind;

	std::string pattern = std::string(Ttc(Root.c_str())) + "\\*";
	hFind = FindFirstFile(Ctt(pattern.c_str()), &file_data);

	// Now recurse down until we find document files within 'current' directories.
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) continue;

			results.push_back(file_data.cFileName);
		} while (FindNextFile( hFind, &file_data));

		FindClose(hFind);
	} // End if - then

	return(results);
} // End of GetFileNames() method.
#else
{
	// We're in UNIX land now.

	std::list<wxString>	results;

	DIR *pdir = opendir(Ttc(Root.c_str()));
				// whose names begin with "default."
	if (pdir != NULL)
	{
		struct dirent *pent = NULL;
		while ((pent=readdir(pdir)))
		{
			results.push_back(Ctt(pent->d_name));
		} // End while
		closedir(pdir);
	} // End if - then

	return(results);
} // End of GetFileNames() method
#endif


CxfFont::CxfFont( const wxChar *p_szFile, const double word_space_percentage, const double character_space_percentage )
	: VectorFont(word_space_percentage, character_space_percentage)
{
	m_line_spacing_factor = 1.0;
	m_name = _T("");
	wxChar character_name = 0;

	std::ifstream file(Ttc(p_szFile));
	if (file.is_open())
	{
		std::list<std::string> lines;
		std::string line;
		while (! file.eof() )
		{
			std::getline (file,line);
			if (line.size() >= 5)
			{
				if (line[0] == '[')
				{
				    if ((lines.size() > 0) && (character_name != 0))
				    {
				        // Write out the last glyph read in before we start reading this new one.
				        Glyph glyph(lines, m_word_space_percentage, m_character_space_percentage);
                        m_glyphs.insert(std::make_pair(character_name, glyph));
						m_bounding_box.Insert( glyph.BoundingBox() );
						lines.clear();
                        character_name = 0;
					}

                    // Start reading the new glyph.
                    wxString symbol;
                    std::vector<wxString> tokens = Tokens( wxString::From8BitData(line.c_str()), _T("[] \r\n\t") );
                    if (tokens.size() > 0)
                    {
                        symbol = tokens[0];
                    }

					if (symbol.Length() > 0)
					{
						// It must be a multi-byte character.
						if (((symbol[0] == '<') && (symbol[symbol.Length()-1] == '>')) ||
							(symbol[0] == '#'))
						{
							std::vector<wxString> tokens = Tokens( symbol, _T("#<> \t") );
							if (tokens.size() > 0)
							{
								unsigned long lCharacterName;
								tokens[0].ToULong( &lCharacterName, 16 );
								character_name = (wxChar) lCharacterName;
							}
						}
						else
						{
							character_name = symbol[0];
						}
					}
                }
				else if (line[0] == '#')
				{
					// It's a comment.  See if it's one of the special comments we're interested in.
					std::vector<wxString> tokens = Tokens( wxString::From8BitData(line.c_str()), _T("# \r\t\n:") );
					if (line.find("LineSpacingFactor") != line.npos)
					{
						tokens.rbegin()->ToDouble(&m_line_spacing_factor);
					}
					else if (line.find("LetterSpacing") != line.npos)
					{
						// tokens.rbegin()->ToDouble(&m_letter_spacing);
						// m_letter_spacing = PointToMM(m_letter_spacing);
					}
					else if (line.find("Name") != line.npos)
					{
						// We want all tokens except the first.
						m_name.Clear();
						for (std::vector<wxString>::iterator l_itToken = tokens.begin();
							l_itToken != tokens.end(); l_itToken++)
						{
							if (l_itToken != tokens.begin())
							{
								if (m_name.Length() > 0)
								{
									m_name.Append(_T(" "));
								}
								m_name.Append( *l_itToken );
							}
						}
					}
				}
				else if ((line.size() > 0) && ((line[0] == 'L') || (line[0] == 'A')))
				{
					lines.push_back(line);
				}
			}
		}
		file.close();

        if ((lines.size() > 0) && (character_name != 0))
        {
            Glyph glyph(lines, m_word_space_percentage, m_character_space_percentage);
            m_glyphs.insert(std::make_pair(character_name, glyph));
            m_bounding_box.Insert( glyph.BoundingBox() );
            lines.clear();
            character_name = 0;
        }

//		printf("File '%s' contained %d glyphs\n", Ttc(p_szFile), m_glyphs.size() );
	}
	else
	{
		std::ostringstream l_ossError;
		l_ossError << "Unable to open file";
		throw(std::runtime_error(l_ossError.str().c_str()));
	}
}

struct LineEnding : public std::unary_function< char, bool >
{
	bool operator()( const char character )
	{
		if (character == '\n') return(true);
		if (character == '\r') return(true);
		return(false);
	}
};

HersheyFont::HersheyFont( const wxChar *p_szFile, const double word_space_percentage, const double character_space_percentage )
	: VectorFont(word_space_percentage, character_space_percentage)
{
	m_line_spacing_factor = 1.0;
	m_name = _T("Hershey");
	wxChar character_name = ' ';

	wxString name = p_szFile;
	int offset = -1;
	for (offset = name.Find('/'); offset >= 0; offset = name.Find('/'))
	{
        if (offset >= 0)	name.Remove(0, offset+1);
	}

	for (offset = name.Find('\\'); offset >= 0; offset = name.Find('/'))
	{
        if (offset >= 0)	name.Remove(0, offset+1);
	}

	offset = name.Find('.');
	if (offset >= 0) name.erase( offset, name.Length() - offset );

	m_name = _T("Hershey ") + name;

	std::ifstream file(Ttc(p_szFile));
	if (file.is_open())
	{
		LineEnding lineending;

		while (! file.eof() )
		{
		    std::string line;
			std::getline (file,line);
			// Remove newline and carriage return characters.
			std::remove_if( line.begin(), line.end(), lineending );

			if (line.size() >= 5)
			{
				line.erase(0,5);	// Ignore the first five characters.

				// the next three characters represent the number of vertices in the character.
				int number_of_vertices = strtoul( line.substr(0,3).c_str(), NULL, 10 );
				line.erase(0,3);

				while ((! file.eof()) && (line.size() < std::string::size_type(number_of_vertices * 2)))
				{
					std::string extra;
					std::getline( file, extra );

					// Remove newline and carriage return characters.
					std::remove_if( extra.begin(), extra.end(), lineending );

					line += extra;
				} // End while

				m_glyphs.insert(std::make_pair(character_name, Glyph(line, m_word_space_percentage, m_character_space_percentage)));
				m_bounding_box.Insert( Glyph(line, m_word_space_percentage, m_character_space_percentage).BoundingBox() );
				character_name += 1;
			}
		}
		file.close();
	}
	else
	{
		std::ostringstream l_ossError;
		l_ossError << "Unable to open file";
		throw(std::runtime_error(l_ossError.str().c_str()));
	}
}



HeeksObj *VectorFont::Sketch( const wxString & text, const gp_Trsf & transformation_matrix, const float width, COrientationModifier *pOrientationModifier ) const
{
    if (pOrientationModifier)
    {
        pOrientationModifier->InitializeFromSketch();
    }

	HeeksObj *sketch = heekscad_interface.NewSketch();
	sketch->OnEditString(text.c_str());

	gp_Pnt location(StartingLocation());	// The transformation matrix will put it in the right place.
	for (wxString::size_type offset = 0; offset < text.Length(); offset++)
	{
		if (text[offset] == ' ')
		{
			// It's a space.  Just move on.  Nothing to see here.
			location.SetX( location.X() + WordSpacing() );
		}
		else if (text[offset] == '\n')
		{
			location.SetX(0.0);
			location.SetY(location.Y() - LineSpacingFactor() - BoundingBox().Height());
		}
		else if (m_glyphs.find(text[offset]) == m_glyphs.end())
		{
			// We don't have a glyph for this symbol.  Draw a square around where
			// it would have been instead.  The BoundingBox() for this font is
			// the size of the largest glyph (graphics for character) in this font.
			// Make the square that big.

			CBox largest_glyph( BoundingBox() );

			gp_Pnt top_left( location );
			gp_Pnt top_right( location );
			gp_Pnt bottom_left( location );
			gp_Pnt bottom_right( location );

			top_left.SetX( location.X() + largest_glyph.MinX() );
			top_left.SetY( location.Y() + largest_glyph.MaxY() );

			top_right.SetX( location.X() + largest_glyph.MaxX() );
			top_right.SetY( location.Y() + largest_glyph.MaxY() );

			bottom_left.SetX( location.X() + largest_glyph.MinX() );
			bottom_left.SetY( location.Y() + largest_glyph.MinY() );

			bottom_right.SetX( location.X() + largest_glyph.MaxX() );
			bottom_right.SetY( location.Y() + largest_glyph.MinY() );

			if (pOrientationModifier)
			{
			    top_left = pOrientationModifier->Transform(transformation_matrix, location.X(), top_left, width );
			    top_right = pOrientationModifier->Transform(transformation_matrix, location.X(), top_right, width );
			    bottom_left = pOrientationModifier->Transform(transformation_matrix, location.X(), bottom_left, width );
			    bottom_right = pOrientationModifier->Transform(transformation_matrix, location.X(), bottom_right, width );
			}

			top_left.Transform( transformation_matrix );
			top_right.Transform( transformation_matrix );
			bottom_left.Transform( transformation_matrix );
			bottom_right.Transform( transformation_matrix );

			double top_left_point[3];
			double top_right_point[3];
			double bottom_left_point[3];
			double bottom_right_point[3];

			top_left_point[0] = top_left.X();
			top_left_point[1] = top_left.Y();
			top_left_point[2] = top_left.Z();

			top_right_point[0] = top_right.X();
			top_right_point[1] = top_right.Y();
			top_right_point[2] = top_right.Z();

			bottom_left_point[0] = bottom_left.X();
			bottom_left_point[1] = bottom_left.Y();
			bottom_left_point[2] = bottom_left.Z();

			bottom_right_point[0] = bottom_right.X();
			bottom_right_point[1] = bottom_right.Y();
			bottom_right_point[2] = bottom_right.Z();

			HeeksObj *line = heekscad_interface.NewLine( top_left_point, top_right_point );
			sketch->Add( line, NULL);

			line = heekscad_interface.NewLine( top_right_point, bottom_right_point );
			sketch->Add( line, NULL );

			line = heekscad_interface.NewLine( bottom_right_point, bottom_left_point );
			sketch->Add( line, NULL );

			line = heekscad_interface.NewLine( bottom_left_point, top_left_point );
			sketch->Add( line, NULL );

			location.SetX( location.X() + BoundingBox().Width() );
		} // End if - then
		else
		{
			Glyphs_t::const_iterator l_itGlyph = m_glyphs.find( text[offset] );
			if (l_itGlyph != m_glyphs.end())
			{
			    gp_Pnt original_location(location);

                // Adjust this glyph left or right so that it's bottom left hand corner is at the x=0 mark.
			    location.SetX( location.X() + (l_itGlyph->second.BoundingBox().MinX() * -1.0) );

				// Get the lines and arcs that represent the character (glyph)
				std::list<HeeksObj *> graphics = l_itGlyph->second.GetGraphics( location, transformation_matrix, width, pOrientationModifier );
				for (std::list<HeeksObj *>::iterator l_itGraphic = graphics.begin(); l_itGraphic != graphics.end(); l_itGraphic++)
				{
					// label this piece of graphics with the character it's representing
					(*l_itGraphic)->OnEditString(text.Mid(offset,1).c_str());

					// And add it to the sketch that represents the whole text string.
					sketch->Add( *l_itGraphic, NULL );
				} // End for

                location.SetX( original_location.X() + LetterSpacing( l_itGlyph->second ) );
			} // End if - then
		} // End if - else
	} // End for

	((CSketch *) sketch)->ReOrderSketch( SketchOrderTypeMultipleCurves );
	return(sketch);
}


void VectorFont::Glyph::get_text_size( float *pWidth, float *pHeight ) const
{
	*pWidth = (float) m_bounding_box.Width();
	*pHeight = (float) m_bounding_box.Height();
} // End get_text_size() method



bool VectorFont::get_text_size( const wxString & text, float *pWidth, float *pHeight ) const
{
	*pWidth = 0.0;
	*pHeight = 0.0;

	for (wxString::size_type offset = 0; offset < text.Length(); offset++)
	{
		if (text[offset] == '\n')
		{
			*pHeight += (float) LineSpacingFactor();
		}
		else
		{
		    if (text[offset] == ' ')
            {
                // It's a space.  Just move on.  Nothing to see here.
                *pWidth += (float) WordSpacing();
            }
            else
            {
                Glyphs_t::const_iterator l_itGlyph = m_glyphs.find( text[offset] );
                if (l_itGlyph != m_glyphs.end())
                {
                    float width = 0.0; float height = 0.0;
                    l_itGlyph->second.get_text_size( &width, &height );
                    *pWidth += (float) LetterSpacing( l_itGlyph->second );

                    if (*pHeight < height) *pHeight = height;
                }
                else
                {
                    *pWidth += (float) BoundingBox().Width();
                    if (*pHeight < BoundingBox().Height()) *pHeight = (float) BoundingBox().Height();
                }
            } // End if - else
		}
	} // End for

	return(true);
} // End get_text_size() method


double VectorFont::LetterSpacing( const Glyph & glyph ) const
{
    double offset = glyph.BoundingBox().MaxX();

    if (offset < heekscad_interface.GetTolerance())
    {
        offset += ((BoundingBox().Width() * m_character_space_percentage / 100.0) / 2.0);
    }

    offset += (BoundingBox().Width() * m_character_space_percentage / 100.0);

    return(offset);
}


void VectorFont::glCommands(const wxString & text,
							const gp_Pnt &start_point,
							const bool select,
							const bool marked,
							const bool no_color,
							COrientationModifier *pOrientationModifier,
							gp_Trsf transformation,
							const float width ) const
{
    if (pOrientationModifier)
    {
        pOrientationModifier->InitializeFromSketch();
    }

	gp_Pnt location( start_point );
	location.SetX( location.X() + StartingLocation().X() );
	location.SetY( location.Y() + StartingLocation().Y() );
	location.SetZ( location.Z() + StartingLocation().Z() );

	for (wxString::size_type offset = 0; offset < text.Length(); offset++)
	{
		if (text[offset] == ' ')
		{
			// It's a space.  Just move on.  Nothing to see here.
			location.SetX( location.X() + WordSpacing() );
		}
		else if (text[offset] == '\n')
		{
			location.SetX(0.0);
			location.SetY(location.Y() - LineSpacingFactor() - BoundingBox().Height());
		}
		else if (m_glyphs.find(text[offset]) == m_glyphs.end())
		{
			// We don't have a glyph for this symbol.  Draw a square around where
			// it would have been instead.  The BoundingBox() for this font is
			// the size of the largest glyph (graphics for character) in this font.
			// Make the square that big.

			CBox largest_glyph( BoundingBox() );

			gp_Pnt top_left( location );
			gp_Pnt top_right( location );
			gp_Pnt bottom_left( location );
			gp_Pnt bottom_right( location );

			top_left.SetX( location.X() + largest_glyph.MinX() );
			top_left.SetY( location.Y() + largest_glyph.MaxY() );

			top_right.SetX( location.X() + largest_glyph.MaxX() );
			top_right.SetY( location.Y() + largest_glyph.MaxY() );

			bottom_left.SetX( location.X() + largest_glyph.MinX() );
			bottom_left.SetY( location.Y() + largest_glyph.MinY() );

			bottom_right.SetX( location.X() + largest_glyph.MaxX() );
			bottom_right.SetY( location.Y() + largest_glyph.MinY() );

			if (pOrientationModifier)
			{
			    top_left = pOrientationModifier->Transform(transformation, location.X(), top_left, width );
			    top_right = pOrientationModifier->Transform(transformation, location.X(), top_right, width );
			    bottom_left = pOrientationModifier->Transform(transformation, location.X(), bottom_left, width );
			    bottom_right = pOrientationModifier->Transform(transformation, location.X(), bottom_right, width );
			}

			glBegin(GL_LINE_STRIP);
			glVertex3d(top_left.X(), top_left.Y(), top_left.Z());
			glVertex3d(top_right.X(), top_right.Y(), top_right.Z());
			glVertex3d(bottom_right.X(), bottom_right.Y(), bottom_right.Z());
			glVertex3d(bottom_left.X(), bottom_left.Y(), bottom_left.Z());
			glVertex3d(top_left.X(), top_left.Y(), top_left.Z());
			glEnd();

			location.SetX( location.X() + BoundingBox().MaxX() );
		} // End if - then
		else
		{
			Glyphs_t::const_iterator l_itGlyph = m_glyphs.find( text[offset] );
			if (l_itGlyph != m_glyphs.end())
			{
			    gp_Pnt original_location(location);

                // Adjust this glyph left or right so that it's bottom left hand corner is at the x=0 mark.
			    location.SetX( location.X() + (l_itGlyph->second.BoundingBox().MinX() * -1.0) );
				l_itGlyph->second.glCommands(location, select, marked, no_color,pOrientationModifier, transformation, width );

                location.SetX( original_location.X() + LetterSpacing( l_itGlyph->second ) );
			} // End if - then
		} // End if - else
	} // End for

} // End glCommands() method




/* static */ bool CxfFont::ValidExtension( const wxString &file_name )
{
	wxString _file( file_name );	// Take a copy so we can convert it to uppercase.
	if (_file.Length() < 4) return(false);	// too short to have a '.cxf' extension

	_file.UpperCase();
	return(_file.EndsWith(_T("CXF")));
}

/* static */ bool HersheyFont::ValidExtension( const wxString &file_name )
{
	wxString _file( file_name );	// Take a copy so we can convert it to uppercase.
	if (_file.Length() < 4) return(false);	// too short to have a '.jhf' extension

	_file.UpperCase();
	return(_file.EndsWith(_T("JHF")));
}


VectorFonts::VectorFonts( const VectorFont::Name_t & directory,
							const double word_space_percentage,
							const double character_space_percentage )
{
	m_word_space_percentage = word_space_percentage;
	m_character_space_percentage = character_space_percentage;

	Add( directory );
}

void VectorFonts::Add( const VectorFont::Name_t & directory )
{
    printf("Within directory %s\n", Ttc( directory.c_str()));

	std::list<wxString> files = GetFileNames( directory.c_str() );
	for (std::list<wxString>::const_iterator l_itFile = files.begin(); l_itFile != files.end(); l_itFile++)
	{
	    printf("Looking at %s\n", Ttc(l_itFile->c_str()));

		try {
			if (CxfFont::ValidExtension( *l_itFile ))
			{
				wxString path( directory );
				path = path + _T("/");
				path = path + l_itFile->c_str();
				CxfFont *pFont = new CxfFont( path.c_str(), m_word_space_percentage, m_character_space_percentage );
				m_fonts.insert( std::make_pair( pFont->Name(), pFont ) );
			} // End if - then
			else if (HersheyFont::ValidExtension( *l_itFile ))
			{
				wxString path( directory );
				path = path + _T("/");
				path = path + l_itFile->c_str();
				HersheyFont *pFont = new HersheyFont( path.c_str(), m_word_space_percentage, m_character_space_percentage );
				m_fonts.insert( std::make_pair( pFont->Name(), pFont ) );
			} // End if - then
		} // End try
		catch( const std::exception & error)
		{
			printf("Failed to load font.  Error is %s\n", error.what() );
		} // End catch
	} // End for

	printf("Read %d vector font files\n", (unsigned)m_fonts.size());
} // End Add() method

VectorFonts::~VectorFonts()
{
	for (Fonts_t::iterator l_itFont = m_fonts.begin(); l_itFont != m_fonts.end(); l_itFont++)
	{
		delete l_itFont->second;
	} // End for
	m_fonts.clear();
}

std::set<VectorFont::Name_t> VectorFonts::FontNames() const
{
	std::set<VectorFont::Name_t> names;
	for (Fonts_t::const_iterator l_itFont = m_fonts.begin(); l_itFont != m_fonts.end(); l_itFont++)
	{
		names.insert(l_itFont->first.c_str());
	} // End for

	return(names);
} // End FontNames() method


VectorFont *VectorFonts::Font( const VectorFont::Name_t & name ) const
{
	for (Fonts_t::const_iterator l_itFont = m_fonts.begin(); l_itFont != m_fonts.end(); l_itFont++)
	{
		if (l_itFont->first == name) return(l_itFont->second);
	} // End for

	return(NULL);
} // End Font() method

void VectorFonts::SetWordSpacePercentage( const double value )
{
    m_word_space_percentage = value;
    for (Fonts_t::iterator itFont = m_fonts.begin(); itFont != m_fonts.end(); itFont++)
    {
        itFont->second->SetWordSpacePercentage(value);
    }
}

void VectorFonts::SetCharacterSpacePercentage( const double value )
{
    m_character_space_percentage = value;
    for (Fonts_t::iterator itFont = m_fonts.begin(); itFont != m_fonts.end(); itFont++)
    {
        itFont->second->SetCharacterSpacePercentage(value);
    }
}

void VectorFont::SetWordSpacePercentage( const double value )
{
    m_word_space_percentage = value;
    for (Glyphs_t::iterator itGlyph = m_glyphs.begin(); itGlyph != m_glyphs.end(); itGlyph++)
    {
        itGlyph->second.SetWordSpacePercentage(value);
    }
}

void VectorFont::SetCharacterSpacePercentage( const double value )
{
    m_character_space_percentage = value;
    for (Glyphs_t::iterator itGlyph = m_glyphs.begin(); itGlyph != m_glyphs.end(); itGlyph++)
    {
        itGlyph->second.SetCharacterSpacePercentage(value);
    }
}

void VectorFont::Glyph::SetWordSpacePercentage( const double value )
{
    m_word_space_percentage = value;
}

void VectorFont::Glyph::SetCharacterSpacePercentage( const double value )
{
    m_character_space_percentage = value;
}




