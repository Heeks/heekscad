#ifdef wxUSE_UNICODE
	#ifndef _UNICODE
		#define _UNICODE
	#endif
#endif

#ifndef CXF_FONT_CLASS_DEFINITION
#define CXF_FONT_CLASS_DEFINITION

#include <stdio.h>
#include <math.h>
#include <wx/string.h>
#include <wx/window.h>

#include <gp_Pnt.hxx>

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <stdexcept>
#include <memory>

#include "../interface/HeeksCADInterface.h"
#include "../interface/Box.h"
#include "OrientationModifier.h"


/**
 	The CxfFont class holds enough information to draw a single font.
 */
class VectorFont
{
public:
	typedef wxString Name_t;

protected:
	/**
	 	A Glyph is the term used to describe the graphics required to draw a single character
		in a font.  It includes a list of Graphics classes.  The Graphics class is just an
		inheritance base class that allows the list to contain either a Line or an Arc object.
	 */
	class Glyph
	{
	public:
		class Graphics
		{
		public:
			virtual HeeksObj *Sketch( const gp_Pnt & location, const gp_Trsf & transformation_matrix, const float width, COrientationModifier *pOrientationModifier ) const = 0;
			virtual void glCommands( const gp_Pnt & starting_point,
									 const bool select,
									 const bool marked,
									 const bool no_color,
									 COrientationModifier *pOrientationModifier,
									 gp_Trsf transformation,
									 const float width) const = 0;
			virtual CBox BoundingBox() const = 0;
			virtual Graphics *Duplicate() = 0;
		}; // End Graphics class defintion.

		class GlyphLine : public Graphics
		{
		public:
			GlyphLine( const double x1, const double y1, const double x2, const double y2 ) :
			  m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2)
			{
				// Setup the bounding box values.
				double point[3];
				point[0] = m_x1;
				point[1] = m_y1;
				point[2] = 0.0;

				m_bounding_box.Insert( point );

				point[0] = m_x2;
				point[1] = m_y2;
				point[2] = 0.0;

				m_bounding_box.Insert( point );
			}

			GlyphLine() : m_x1(0.0), m_y1(0.0), m_x2(0.0), m_y2(0.0)
			{
				// Setup the bounding box values.
				double point[3];
				point[0] = m_x1;
				point[1] = m_y1;
				point[2] = 0.0;

				m_bounding_box.Insert( point );

				point[0] = m_x2;
				point[1] = m_y2;
				point[2] = 0.0;

				m_bounding_box.Insert( point );
			}

		    HeeksObj *Sketch( const gp_Pnt & location, const gp_Trsf & transformation_matrix, const float width, COrientationModifier *pOrientationModifier ) const;
			void glCommands( const gp_Pnt & starting_point,
							 const bool select,
							 const bool marked,
							 const bool no_color,
							 COrientationModifier *pOrientationModifier,
							 gp_Trsf transformation,
							 const float width ) const;
			CBox BoundingBox() const { return(m_bounding_box); }
			Graphics *Duplicate() { return(new GlyphLine(*this)); }

		private:
			double m_x1;
			double m_y1;
			double m_x2;
			double m_y2;

			CBox m_bounding_box;
		};

		class GlyphArc : public Graphics
		{
		public:
			GlyphArc( const double xcentre, const double ycentre, const double radius, const double start_angle, const double end_angle ) :
			  m_xcentre(xcentre), m_ycentre(ycentre), m_radius(radius),
				m_start_angle((start_angle / 360.0) * (2 * M_PI)),
				m_end_angle((end_angle / 360.0) * (2 * M_PI))
			  {
				if (m_start_angle < 0) m_start_angle += (2 * M_PI);
				if (m_end_angle < 0) m_end_angle += (2 * M_PI);

				std::list<gp_Pnt> points = Interpolate(gp_Pnt(0.0, 0.0, 0.0), 20 );
				for (std::list<gp_Pnt>::const_iterator l_itPoint = points.begin();
					l_itPoint != points.end(); l_itPoint++)
				{
					double point[3];
					point[0] = l_itPoint->X();
					point[1] = l_itPoint->Y();
					point[2] = l_itPoint->Z();

					m_bounding_box.Insert( point );
				} // End for
			  }

			GlyphArc() : m_xcentre(0.0), m_ycentre(0.0), m_radius(0.0), m_start_angle(0.0), m_end_angle(0.0) { }
			Graphics *Duplicate() { return(new GlyphArc(*this)); }

			HeeksObj *Sketch( const gp_Pnt & location, const gp_Trsf & transformation_matrix, const float width, COrientationModifier *pOrientationModifier ) const;
			void glCommands( const gp_Pnt & starting_point,
							 const bool select,
							 const bool marked,
							 const bool no_color,
							 COrientationModifier *pOrientationModifier,
							 gp_Trsf transformation,
							 const float width ) const;
			CBox BoundingBox() const { return(m_bounding_box); }

			std::list<gp_Pnt> Interpolate(const gp_Pnt & location, const unsigned int number_of_points ) const;


		private:
			double m_xcentre;
			double m_ycentre;
			double m_radius;
			double m_start_angle;
			double m_end_angle;

			CBox	m_bounding_box;
		}; // End GlyphArc class defintion.


	public:
		Glyph( const std::list<std::string> &cxf_glyph_definition, const double word_space_percentage, const double character_space_percentage );
		Glyph( const std::string & hershey_glyph_definition, const double word_space_percentage, const double character_space_percentage );
		Glyph( const Glyph & rhs );
		Glyph & operator= ( const Glyph & rhs );
		~Glyph();

		HeeksObj *Sketch( const gp_Pnt & location, const gp_Trsf & transformation_matrix, const float width, COrientationModifier *pOrientationModifier ) const;
		std::list< HeeksObj * > GetGraphics( const gp_Pnt & location, const gp_Trsf & transformation_matrix, const float width, COrientationModifier *pOrientationModifier ) const;
		void glCommands( const gp_Pnt & starting_point,
						 const bool select,
						 const bool marked,
						 const bool no_color,
						 COrientationModifier *pOrientationModifier,
						 gp_Trsf transformation, const float width) const;
		void get_text_size( float *pWidth, float *pHeight ) const;
		CBox BoundingBox() const { return(m_bounding_box); }
		void SetWordSpacePercentage( const double value );
        void SetCharacterSpacePercentage( const double value );

	private:
		typedef std::list< Graphics * > GraphicsList_t;

		GraphicsList_t	m_graphics_list;
		CBox	m_bounding_box;
		double m_word_space_percentage;
		double m_character_space_percentage;

		wxString PrepareStringForConversion( wxString &value ) const;
		double PointToMM( const double points ) const;
	}; // End Glyph class definition

public:
	VectorFont(const double word_space_percentage, const double character_space_percentage)
	{
		m_word_space_percentage = word_space_percentage;
		m_character_space_percentage = character_space_percentage;
	}

	unsigned int NumGlyphs() const { return((unsigned int) m_glyphs.size()); }
	double LetterSpacing(const Glyph & glyph) const;
	double WordSpacing() const { return(m_bounding_box.Width() * m_word_space_percentage / 100.0); }
	double LineSpacingFactor() const { return(m_bounding_box.Height() * 10.0 / 100.0); }
	Name_t Name() const { return(m_name); }

	HeeksObj *Sketch( const wxString & string, const gp_Trsf & transformation_matrix, const float width, COrientationModifier *pOrientationModifier ) const;
	bool get_text_size( const wxString & text, float *pWidth, float *pHeight ) const;
	void glCommands(const wxString & text,
					const gp_Pnt & starting_point,
					const bool select,
					const bool marked,
					const bool no_color,
					COrientationModifier *pOrientationModifier,
					gp_Trsf transformation,
					const float width ) const;
	CBox BoundingBox() const { return(m_bounding_box); }
	void SetWordSpacePercentage( const double value );
	void SetCharacterSpacePercentage( const double value );
	virtual gp_Pnt StartingLocation() const { return(gp_Pnt(0.0, 0.0, 0.0)); }

protected:
	typedef std::map< wxChar, Glyph > Glyphs_t;
	Glyphs_t m_glyphs;
	double m_line_spacing_factor;
	Name_t m_name;
	CBox	m_bounding_box;		// box surrounding the largest character.
	double m_word_space_percentage;
	double m_character_space_percentage;

}; // End VectorFont class definition


class CxfFont : public VectorFont
{
public:
	CxfFont( const wxChar *p_szFile, const double word_space_percentage, const double character_space_percentage );
	static bool ValidExtension( const wxString &file_name );
	/* virtual */ gp_Pnt StartingLocation() const { return(gp_Pnt(0.0, 0.0, 0.0)); }

}; // End CxfFont class definition


class HersheyFont : public VectorFont
{
public:
	HersheyFont( const wxChar *p_szFile, const double word_space_percentage, const double character_space_percentage );
	static bool ValidExtension( const wxString &file_name );
	/* virtual */ gp_Pnt StartingLocation() const { return(gp_Pnt(0.0, BoundingBox().Height()/2.0, 0.0)); }

}; // End CxfFont class definition


class VectorFonts
{
public:
	typedef std::map< VectorFont::Name_t, VectorFont * > Fonts_t;

	VectorFonts(const VectorFont::Name_t &directory, const double word_space_percentage, const double character_space_percentage);
	~VectorFonts();

	void Add( const VectorFont::Name_t &directory );

	static std::list<wxString> GetFileNames( const wxString & Root );

	std::set<VectorFont::Name_t> FontNames() const;
	VectorFont *Font( const VectorFont::Name_t & name ) const;

	void SetWordSpacePercentage( const double value );
	void SetCharacterSpacePercentage( const double value );

private:
	Fonts_t	m_fonts;
	double m_word_space_percentage;
	double m_character_space_percentage;
}; // End CxfFonts class definition.






#endif // CXF_FONT_CLASS_DEFINITION


