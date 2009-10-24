
#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <wx/string.h>
#include <wx/window.h>

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


HeeksObj *CxfFont::Glyph::Line::Sketch( const gp_Pnt & location, const gp_Trsf & transformation_matrix ) const
{
	gp_Pnt start_point( location );
	gp_Pnt end_point( location );

	start_point.SetX( location.X() + m_x1 );
	start_point.SetY( location.Y() + m_y1 );

	end_point.SetX( location.X() + m_x2 );
	end_point.SetY( location.Y() + m_y2 );

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

void CxfFont::Glyph::Line::glCommands( const gp_Pnt & starting_point, const bool select, const bool marked, const bool no_color) const
{
	gp_Pnt from( starting_point );
	gp_Pnt to( starting_point );

	from.SetX( starting_point.X() + m_x1);
	from.SetY( starting_point.Y() + m_y1);
	from.SetZ( starting_point.Z() );

	to.SetX( starting_point.X() + m_x2);
	to.SetY( starting_point.Y() + m_y2);
	to.SetZ( starting_point.Z() );

	glBegin(GL_LINE_STRIP);
	glVertex3d(from.X(), from.Y(), from.Z());
	glVertex3d(to.X(), to.Y(), to.Z());
	glEnd();
} // End glCommands() method



std::list<gp_Pnt> CxfFont::Glyph::Arc::Interpolate(const gp_Pnt & location, const unsigned int number_of_points ) const
{
	std::list<gp_Pnt> points;

	gp_Pnt origin(location);
	origin.SetX( origin.X() + m_xcentre );
	origin.SetY( origin.Y() + m_ycentre );

	double start_angle = m_start_angle;
	double end_angle = m_end_angle;

	if (start_angle > end_angle)
	{
		end_angle += (2 * PI);
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


HeeksObj *CxfFont::Glyph::Arc::Sketch( const gp_Pnt & location, const gp_Trsf & transformation_matrix ) const
{
	double start[3];
	double end[3];
	double centre[3];
	double up[3];

	gp_Pnt centre_point( location.X() + m_xcentre, location.Y() + m_ycentre, location.Z() );
	gp_Pnt start_point( centre_point.X() + m_radius, centre_point.Y(), centre_point.Z() );
	gp_Pnt end_point( centre_point.X() + m_radius, centre_point.Y(), centre_point.Z() );	

	gp_Dir z_direction( 0, 0, 1 );

	gp_Trsf start_rotation_matrix;
	start_rotation_matrix.SetRotation( gp_Ax1(centre_point, z_direction), m_start_angle );
	start_point.Transform(start_rotation_matrix);	// Rotate to start_angle
	start_point.Transform(transformation_matrix);	// Move it as well as rotate it.

	start[0] = start_point.X();
	start[1] = start_point.Y();
	start[2] = start_point.Z();

	gp_Trsf end_rotation_matrix;
	end_rotation_matrix.SetRotation( gp_Ax1(centre_point, z_direction), m_end_angle );
	end_point.Transform(end_rotation_matrix);	// Rotate to start_angle
	end_point.Transform(transformation_matrix);	// Move it as well as rotate it.

	end[0] = end_point.X();
	end[1] = end_point.Y();
	end[2] = end_point.Z();

	centre_point.Transform(transformation_matrix);
	centre[0] = centre_point.X();
	centre[1] = centre_point.Y();
	centre[2] = centre_point.Z();

	gp_Pnt up_point( 0.0, 0.0, 1.0 );
	up_point.Transform( transformation_matrix );

	// For counter-clockwise (always in this font format)
	up[0] = up_point.X();
	up[1] = up_point.Y();
	up[2] = up_point.Z();

	HeeksObj *arc = heekscad_interface.NewArc( start, end, centre, up );
	return(arc);
} // End Sketch() method

void CxfFont::Glyph::Arc::glCommands( const gp_Pnt & starting_point, const bool select, const bool marked, const bool no_color) const
{
	glBegin(GL_LINE_STRIP);
	std::list<gp_Pnt> vertices = Interpolate( starting_point, 20 );
	for (std::list<gp_Pnt>::const_iterator l_itVertex = vertices.begin(); l_itVertex != vertices.end(); l_itVertex++)
	{
		glVertex3d(l_itVertex->X(), l_itVertex->Y(), l_itVertex->Z());
	} // End for
	glEnd();
} // End glCommands() method

CxfFont::Glyph::Glyph( const std::list<std::string> &definition )
{
	for (std::list<std::string>::const_iterator l_itLine = definition.begin(); l_itLine != definition.end(); l_itLine++)
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
				Line *line = new Line( strtod( Ttc(tokens[1].c_str()), NULL ),
												 strtod( Ttc(tokens[2].c_str()), NULL ),
												 strtod( Ttc(tokens[3].c_str()), NULL ),
												 strtod( Ttc(tokens[4].c_str()), NULL ) );
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
					Arc *arc = new Arc( strtod( Ttc(tokens[1].c_str()), NULL ),
													 strtod( Ttc(tokens[2].c_str()), NULL ),
													 strtod( Ttc(tokens[3].c_str()), NULL ),
													 strtod( Ttc(tokens[5].c_str()), NULL ),
													 strtod( Ttc(tokens[4].c_str()), NULL ) );
					m_graphics_list.push_back( arc );
					m_bounding_box.Insert( arc->BoundingBox() );
				} // End if - then
				else
				{
					Arc *arc = new Arc( strtod( Ttc(tokens[1].c_str()), NULL ),
												 strtod( Ttc(tokens[2].c_str()), NULL ),
												 strtod( Ttc(tokens[3].c_str()), NULL ),
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

CxfFont::Glyph::~Glyph()
{
	for (GraphicsList_t::iterator l_itGraphic = m_graphics_list.begin();
		l_itGraphic != m_graphics_list.end(); l_itGraphic++)
	{
		delete *l_itGraphic;
	} // End for

	m_graphics_list.clear();
} // End destructor


CxfFont::Glyph::Glyph( const CxfFont::Glyph & rhs )
{
	*this = rhs;	// call the assignment operator
}

CxfFont::Glyph & CxfFont::Glyph::operator= ( const CxfFont::Glyph & rhs )
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
HeeksObj *CxfFont::Glyph::Sketch( const gp_Pnt & location, const gp_Trsf & transformation_matrix ) const
{
	HeeksObj *sketch = heekscad_interface.NewSketch();
	
	for (GraphicsList_t::const_iterator l_itGraphic = m_graphics_list.begin(); l_itGraphic != m_graphics_list.end(); l_itGraphic++)
	{
		sketch->Add((*l_itGraphic)->Sketch( location, transformation_matrix ), NULL);
	} // End for

	return(sketch);
} // End Sketch() method

std::list<HeeksObj *> CxfFont::Glyph::GetGraphics( const gp_Pnt & location, const gp_Trsf & transformation_matrix ) const
{
	std::list<HeeksObj *> results;
	
	for (GraphicsList_t::const_iterator l_itGraphic = m_graphics_list.begin(); l_itGraphic != m_graphics_list.end(); l_itGraphic++)
	{
		results.push_back( (*l_itGraphic)->Sketch( location, transformation_matrix ) );
	} // End for

	return(results);
} // End Graphics() method


void CxfFont::Glyph::glCommands( const gp_Pnt & starting_point, const bool select, const bool marked, const bool no_color) const
{
	for (GraphicsList_t::const_iterator l_itGraphic = m_graphics_list.begin(); l_itGraphic != m_graphics_list.end(); l_itGraphic++)
	{
		(*l_itGraphic)->glCommands( starting_point, select, marked, no_color );
	} // End for
} // End glCommands() method

 

/* static */ std::list<wxString> CxfFonts::GetFileNames( const wxString & Root )
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
						
			results.push_back( file_data.cFileName );
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


CxfFont::CxfFont( const wxChar *p_szFile )
{
	m_letter_spacing = 3.0;
	m_word_spacing = 6.75;
	m_line_spacing_factor = 1.0;
	m_name = _T("");
	wxChar character_name = 0;

	std::ifstream file(Ttc(p_szFile));
	if (file.is_open())
	{
		wxString symbol;
		std::list<std::string> lines;
		std::string line;
		while (! file.eof() )
		{
			std::getline (file,line);
			if (line.size() >= 5)
			{
				if (line[0] == '[')
				{
					if (symbol.Length() > 1)
					{
						// It must be a multi-byte character.  I don't know how to do these yet.
						// printf("Ignoring multi-byte character '%s' for now\n", Ttc(symbol.c_str()));
						if (((symbol[0] == '<') && (symbol[symbol.Length()-1] == '>')) ||
							(symbol[0] == '#'))
						{
							std::vector<wxString> tokens = Tokens( symbol, _T("#<> \t") );
							if (tokens.size() > 0)
							{
								long lCharacterName = tokens[0][0];
								tokens[0].ToLong( &lCharacterName, 16 );
								character_name = wxChar(lCharacterName);
							}
						}
						else
						{
							character_name = symbol[0];
						}
					}
					else
					{
						m_glyphs.insert(std::make_pair(character_name, Glyph(lines)));
						m_bounding_box.Insert( Glyph(lines).BoundingBox() );
					}

					lines.clear();
					symbol.Clear();

					std::vector<wxString> tokens = Tokens( ss_to_wxstring(line), wxString(_T("[] \r\n\t")) );
					if (tokens.size() >= 2)
					{
						symbol = tokens[0];
						if (((symbol[0] == '<') && (symbol[symbol.Length()-1] == '>')) ||
							(symbol[0] == '#'))
						{
							std::vector<wxString> tokens = Tokens( symbol, _T("#<> \t") );
							if (tokens.size() > 0)
							{
								long lCharacterName = tokens[0][0];
								tokens[0].ToLong( &lCharacterName, 16 );
								character_name = wxChar(lCharacterName);
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

					std::vector<wxString> tokens = Tokens( ss_to_wxstring(line), _T("# \r\t\n:") );
					if (line.find("LetterSpacing") != line.npos)
					{
						tokens.rbegin()->ToDouble(&m_letter_spacing);
					} else if (line.find("WordSpacing") != line.npos)
					{
						tokens.rbegin()->ToDouble(&m_word_spacing);
					} else if (line.find("LineSpacingFactor") != line.npos)
					{
						tokens.rbegin()->ToDouble(&m_line_spacing_factor);
					} else if (line.find("Name") != line.npos)
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

		if (symbol.Length() > 1)
		{
			// It must be a multi-byte character.  I don't know how to do these yet.
			// printf("Ignoring multi-byte character '%s' for now\n", Ttc(symbol.c_str()));
		}
		else
		{
			m_glyphs.insert(std::make_pair(character_name, Glyph(lines)));
			lines.clear();
			symbol.Clear();
			m_bounding_box.Insert( Glyph(lines).BoundingBox() );
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

HeeksObj *CxfFont::Sketch( const wxString & text, const gp_Trsf & transformation_matrix ) const
{
	HeeksObj *sketch = heekscad_interface.NewSketch();
	sketch->OnEditString(text.c_str());

	gp_Pnt location( 0.0, 0.0, 0.0 );	// The transformation matrix will put it in the right place.
	for (wxString::size_type offset = 0; offset < text.Length(); offset++)
	{
		if (text[offset] == ' ')
		{
			// It's a space.  Just move on.  Nothing to see here.
			location.SetX( location.X()+ BoundingBox().Width() + WordSpacing() );
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
				// Get the lines and arcs that represent the character (glyph)
				std::list<HeeksObj *> graphics = l_itGlyph->second.GetGraphics( location, transformation_matrix );
				for (std::list<HeeksObj *>::iterator l_itGraphic = graphics.begin(); l_itGraphic != graphics.end(); l_itGraphic++)
				{
					// label this piece of graphics with the character it's representing
					(*l_itGraphic)->OnEditString(text.Mid(offset,1).c_str()); 

					// And add it to the sketch that represents the whole text string.
					sketch->Add( *l_itGraphic, NULL );
				} // End for

				location.SetX( location.X() + l_itGlyph->second.BoundingBox().Width() );
			} // End if - then
		} // End if - else

		location.SetX(location.X() + LetterSpacing());
	} // End for

	((CSketch *) sketch)->ReOrderSketch( SketchOrderTypeMultipleCurves );
	return(sketch);
}


void CxfFont::Glyph::get_text_size( float *pWidth, float *pHeight ) const
{
	*pWidth = m_bounding_box.Width();
	*pHeight = m_bounding_box.Height();
} // End get_text_size() method



bool CxfFont::get_text_size( const wxString & text, float *pWidth, float *pHeight ) const
{
	*pWidth = 0.0;
	*pHeight = m_line_spacing_factor;

	for (wxString::size_type offset = 0; offset < text.Length(); offset++)
	{
		if (text[offset] == '\n') 
		{
			*pHeight += m_line_spacing_factor;
		}
		else 
		{
			if (m_glyphs.find( text[offset] ) != m_glyphs.end())
			{
				float width = 0.0; float height = 0.0;
				m_glyphs.find( text[offset] )->second.get_text_size( &width, &height );
				*pWidth += width;
			}
		}
	} // End for

	return(true);
} // End get_text_size() method


void CxfFont::glCommands(const wxString & text, const gp_Pnt &start_point, const bool select, const bool marked, const bool no_color) const
{
	gp_Pnt location( start_point );
	for (wxString::size_type offset = 0; offset < text.Length(); offset++)
	{
		if (text[offset] == ' ')
		{
			// It's a space.  Just move on.  Nothing to see here.
			location.SetX( location.X()+ BoundingBox().Width() + WordSpacing() );
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

			glBegin(GL_LINE_STRIP);
			glVertex3d(top_left.X(), top_left.Y(), top_left.Z());
			glVertex3d(top_right.X(), top_right.Y(), top_right.Z());
			glVertex3d(bottom_right.X(), bottom_right.Y(), bottom_right.Z());
			glVertex3d(bottom_left.X(), bottom_left.Y(), bottom_left.Z());
			glVertex3d(top_left.X(), top_left.Y(), top_left.Z());
			glEnd();

			location.SetX( location.X() + BoundingBox().Width() );
		} // End if - then
		else
		{
			Glyphs_t::const_iterator l_itGlyph = m_glyphs.find( text[offset] );
			if (l_itGlyph != m_glyphs.end())
			{
				l_itGlyph->second.glCommands(location, select, marked, no_color);
				location.SetX( location.X() + l_itGlyph->second.BoundingBox().Width() );
			} // End if - then
		} // End if - else

		location.SetX(location.X() + LetterSpacing());
	} // End for

} // End glCommands() method




/* static */ bool CxfFonts::ValidExtension( const wxString &file_name )
{
	wxString _file( file_name );	// Take a copy so we can convert it to uppercase.
	if (_file.Length() < 4) return(false);	// too short to have a '.cxf' extension

	wxString _extension( file_name.Mid( _file.Length() - 3 ) );

	if (_extension.MakeUpper() == wxString(_T("CXF"))) return(true);
	return(false);
}

CxfFonts::CxfFonts( const CxfFont::Name_t & directory )
{
	Add( directory );
}

void CxfFonts::Add( const CxfFont::Name_t & directory )
{
	std::list<wxString> files = GetFileNames( directory.c_str() );
	for (std::list<wxString>::const_iterator l_itFile = files.begin(); l_itFile != files.end(); l_itFile++)
	{
		try {
			if (ValidExtension( *l_itFile ))
			{
				wxString path( directory );
				path = path + _T("/");
				path = path + l_itFile->c_str();
				CxfFont *pFont = new CxfFont( path.c_str() );
				m_fonts.insert( std::make_pair( pFont->Name(), pFont ) );
			} // End if - then
		} // End try
		catch( const std::exception & error)
		{
			printf("Failed to load font.  Error is %s\n", error.what() );
		} // End catch
	} // End for

	printf("Read %d Cxf-format font files\n", (unsigned)m_fonts.size());
} // End Add() method

CxfFonts::~CxfFonts()
{
	for (Fonts_t::iterator l_itFont = m_fonts.begin(); l_itFont != m_fonts.end(); l_itFont++)
	{
		delete l_itFont->second;
	} // End for
	m_fonts.clear();
}

std::set<CxfFont::Name_t> CxfFonts::FontNames() const
{
	std::set<CxfFont::Name_t> names;
	for (Fonts_t::const_iterator l_itFont = m_fonts.begin(); l_itFont != m_fonts.end(); l_itFont++)
	{
		names.insert(l_itFont->first.c_str());
	} // End for

	return(names);
} // End FontNames() method


CxfFont *CxfFonts::Font( const CxfFont::Name_t & name ) const
{
	for (Fonts_t::const_iterator l_itFont = m_fonts.begin(); l_itFont != m_fonts.end(); l_itFont++)
	{
		if (l_itFont->first == name) return(l_itFont->second);
	} // End for

	return(NULL);
} // End Font() method




