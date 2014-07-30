// RS274X.h
// Copyright (c) 2009, David Nicholls, Perttu "celero55" Ahola
// This program is released under the BSD license. See the file COPYING for details.

/*
This code was moved from heekscnc to here. First it will be mainly targeted
for isolation milling.
*/

#pragma once

#include <string>
#include <list>
#include <map>
#include <algorithm>
#include <iostream>

#include "HLine.h"
#include "HArc.h"
#include "HCircle.h"
#include "HEllipse.h"
#include "HSpline.h"
#include "Sketch.h"
#include "../interface/Box.h"

class RS274X
{
    public:
        typedef enum
        {
            IsolationRouting = 0,   // Produce graphics for the boundaries of the wide traces.
            CentreLines,            // Produce graphics for the centrelines of the traces.
			RasterImage,			// Produce RAW image file.  For debug purposes only at the moment.
            Both
        } FileInterpretation_t;

	public:
		RS274X();
		~RS274X() { }

		bool Read( const char *p_szFileName, const FileInterpretation_t interpretation, const bool force_mirror_on = false );

    private:
		double special_strtod( const char *value, const char **end ) const;

        /**
            This is simply a wrapper around the gp_Pnt class from the OpenCascade library
            that allows objects of this class to be used with methods such as std::sort() etc.
         */
        class Point : public gp_Pnt {
        public:
            Point() : gp_Pnt(0.0, 0.0, 0.0) { }
            Point( const double *xyz ) : gp_Pnt(xyz[0], xyz[1], xyz[2]) { }
            Point( const double &x, const double &y, const double &z ) : gp_Pnt(x,y,z) { }
            Point( const gp_Pnt & rhs ) : gp_Pnt(rhs) { }

            bool operator==( const Point & rhs ) const
            {
                if (X() != rhs.X()) return(false);
                if (Y() != rhs.Y()) return(false);
                if (Z() != rhs.Z()) return(false);

                return(true);
            } // End equivalence operator

            bool operator!=( const Point & rhs ) const
            {
                return(! (*this == rhs));
            } // End not-equal operator

            bool operator<( const Point & rhs ) const
            {
                if (X() > rhs.X()) return(false);
                if (X() < rhs.X()) return(true);
                if (Y() > rhs.Y()) return(false);
                if (Y() < rhs.Y()) return(true);
                if (Z() > rhs.Z()) return(false);
                if (Z() < rhs.Z()) return(true);

                return(false);	// They're equal
            } // End equivalence operator

            void ToDoubleArray( double *pArrayOfThree ) const
            {
                pArrayOfThree[0] = X();
                pArrayOfThree[1] = Y();
                pArrayOfThree[2] = Z();
            } // End ToDoubleArray() method
        }; // End Point class definition.




	private:
		/**
			The Bitmap class defines an array of colour values surrounding a CENTRAL
			position.  i.e. the position of a bitmap is defined as its centre point.
		 */
		class Bitmap
		{
		public:
			static double PixelsPerMM()
			{
				const double pixels_per_inch = 1 * 1000;	// 1 pixels per thousdands of an inch
				const double inches_per_mm = 1.0 / 25.4;
				const double pixels_per_mm = pixels_per_inch * inches_per_mm;
				return( pixels_per_mm );
			}

			static double MMPerPixel()
			{
				static double mm_per_pixel = ( 1.0 / PixelsPerMM());
				return(mm_per_pixel);
			}

			const int PixelsPerRow() const
			{
				return(int(floor(m_box.Width() * PixelsPerMM())));
			}

			const int PixelsPerColumn() const
			{
				return(int(floor(m_box.Height() * PixelsPerMM())));
			}

			const int Boarder() const
			{
			    return(3);
			}

		public:
			Bitmap( const CBox bounding_box )
			{
				m_box = bounding_box;

				if ((m_box.Width() > 0.0) && (m_box.Height() > 0.0))
				{
					m_bitmap = new char[ Size() ];
					memset( m_bitmap, 0, Size() );
				}
				else
				{
					m_bitmap = NULL;
				}
			}

			Bitmap & operator= ( const Bitmap & rhs )
			{
				if (this != &rhs)
				{
					if (m_bitmap != NULL)
					{
						delete [] m_bitmap;
						m_bitmap = NULL;
					}

					m_box = rhs.m_box;

					if ((m_box.Width() > 0) && (m_box.Height() > 0))
					{
						m_bitmap = new char[ Size() ];
						memset( m_bitmap, 0, Size() );

						if ((rhs.m_box.Width() <= m_box.Width()) &&
							(rhs.m_box.Height() <= m_box.Height()))
						{
							memcpy( m_bitmap, rhs.m_bitmap, rhs.Size() );
						}
					}
					else
					{
						m_bitmap = NULL;
					}
				}

				return(*this);
			}

			Bitmap( const Bitmap & rhs )
			{
				m_bitmap = NULL;
				*this = rhs;	// Call the assignment operator.
			}

			~Bitmap()
			{
				if (m_bitmap != NULL)
				{
					delete [] m_bitmap;
				}
			}

			double Width() const { return(m_box.Width()); }
			double Height() const { return(m_box.Height()); }

			int Size() const { return((PixelsPerRow() + Boarder() + Boarder()) * (PixelsPerColumn() + Boarder() + Boarder())); }

			// Return the pixel value for the real-world coordinates passed in.  Take note
			// that the real-world 0,0 point is in the centre of the bitmap array.
			char & operator() ( const double x, const double y ) const
			{
				int pixel_x = int(floor((x - m_box.MinX()) * PixelsPerMM()) + ((m_box.Width()  * PixelsPerMM()) / 2.0)) + Boarder();
				int pixel_y = int(floor((y - m_box.MinY()) * PixelsPerMM()) + ((m_box.Height() * PixelsPerMM()) / 2.0)) + Boarder();

				return(m_bitmap[(PixelsPerRow() * pixel_y) + pixel_x]);
			}

			bool ExposeFilm( const Bitmap & pattern, const gp_Pnt & location );
			bool Save( const wxString file_name ) const;

		private:
			CBox m_box;

			char *m_bitmap;

		}; // End Bitmap class definition.

		/**
			An aperture is similar to a cutting tool in that it defines the shape
			of an object that is dragged over the raw material (photographic
			film in the case of a Gerber photoplotter).  This class holds the
			geometric definition of a single aperture.
		 */
		class Aperture
		{
			public:
				typedef enum
				{
					eCircular = 0,
					eRectangular,
					eObRound,
					ePolygon
				} eType_t;

			public:
				Aperture() {
					m_type = eCircular;
					m_outside_diameter = 0.0;
					m_x_axis_hole_dimension = 0.0;
					m_y_axis_hole_dimension = 0.0;
					m_x_axis_outside_dimension = 0.0;
					m_y_axis_outside_dimension = 0.0;
					m_degree_of_rotation = 0.0;
				}

				~Aperture() { }

				Aperture( const Aperture & rhs ) { *this = rhs; }

				Aperture & operator= ( const Aperture & rhs )
				{
					if (this != &rhs)
					{
						m_type = rhs.m_type;

						// For circular
						m_outside_diameter = rhs.m_outside_diameter;
						m_x_axis_hole_dimension = rhs.m_x_axis_hole_dimension;
						m_y_axis_hole_dimension = rhs.m_y_axis_hole_dimension;

						// For rectangular
						m_x_axis_outside_dimension = rhs.m_x_axis_outside_dimension;
						m_y_axis_outside_dimension = rhs.m_y_axis_outside_dimension;

						// For polygon
						m_degree_of_rotation = rhs.m_degree_of_rotation;

						// std::auto_ptr<Bitmap>	m_pBitmap;
					}
					return(*this);
				}

			public:
				void Type( const eType_t type ) { m_type = type; }
				eType_t Type() const { return(m_type); }

				void OutsideDiameter( const double value ) { m_outside_diameter = value; }
				double OutsideDiameter() const { return(m_outside_diameter); }

				void XAxisHoleDimension( const double value ) { m_x_axis_hole_dimension = value; if (m_type != eCircular) m_outside_diameter = value; }
				double XAxisHoleDimension() const { return(m_x_axis_hole_dimension); }

				void YAxisHoleDimension( const double value ) { m_y_axis_hole_dimension = value; if (m_type != eCircular) m_outside_diameter = value; }
				double YAxisHoleDimension() const { return(m_y_axis_hole_dimension); }

				void XAxisOutsideDimension( const double value ) { m_x_axis_outside_dimension = value; if (m_type != eCircular) m_outside_diameter = value; }
				double XAxisOutsideDimension() const { return(m_x_axis_outside_dimension); }

				void YAxisOutsideDimension( const double value ) { m_y_axis_outside_dimension = value; if (m_type != eCircular) m_outside_diameter = value; }
				double YAxisOutsideDimension() const { return(m_y_axis_outside_dimension); }

				void DegreeOfRotation( const double value ) { m_degree_of_rotation = value; }
				double DegreeOfRotation() const { return(m_degree_of_rotation); }

				TopoDS_Face Face(const gp_Pnt & location) const;
				TopoDS_Shape Shape(const gp_Pnt & location) const;

				const Bitmap *GetBitmap();
				CBox BoundingBox() const;

			private:
				eType_t m_type;

				// For circular
				double m_outside_diameter;
				double m_x_axis_hole_dimension;
				double m_y_axis_hole_dimension;

				// For rectangular
				double m_x_axis_outside_dimension;
				double m_y_axis_outside_dimension;

				// For polygon
				double m_degree_of_rotation;

				std::auto_ptr<Bitmap>	m_pBitmap;
		}; // End Aperture class defintion.


	private:

		/**
			The Trace class holds the aperture used to plot a line (or arc) as well as
			the line/arc details.  It also has code to determine if one Trace intersects
			another Trace.  We will use this to aggregate traces together to form
			single 'face' objects for addition to the data model.
		 */
		class Trace
		{
		public:
			typedef enum { eLinear = 0, eCircular, eFlash } eInterpolation_t;

			Trace( const Aperture & aperture, const eInterpolation_t interpolation );
			~Trace() { /* Don't delete the HeeksObj */ }

		public:
			Aperture aperture() const { return(m_aperture); }
			void aperture( const Aperture & aperture ) { m_aperture = aperture; }

			gp_Pnt Start() const { return(m_start); }
			void Start( const gp_Pnt & value ) { m_start = value; }

			gp_Pnt End() const { return(m_end); }
			void End( const gp_Pnt & value ) { m_end = value; }

			double I() const { return(m_i_term); }
			void I( const double & value ) { m_i_term = value; }

			double J() const { return(m_j_term); }
			void J( const double & value ) { m_j_term = value; }

			bool Clockwise() const { return(m_clockwise); }
			void Clockwise( const bool direction ) { m_clockwise = direction; }

			eInterpolation_t Interpolation() const { return(m_interpolation); }
			void Interpolation( const eInterpolation_t value ) { m_interpolation = value; }

			double Radius() const;
			void Radius( const double value );

			gp_Pnt Centre() const;
			gp_Dir Direction() const;

			// Get gp_Circle for arc
			gp_Circ Circle() const;
			gp_Lin Line() const;

			double StartAngle() const;
			double EndAngle() const;

			double Length() const;
			bool operator==( const Trace & rhs ) const;
			static double Area( const TopoDS_Face & face );
			TopoDS_Face Face() const;
			TopoDS_Shape Shape() const;
			HeeksObj *CentrelineGraphics() const;       // Centreline only.
			bool Valid() const;

			bool Intersects( const Trace & rhs ) const;
			bool PointWithinBoundary( const gp_Pnt & point, const gp_Pnt & start, const gp_Pnt & end ) const;

			void ExposeFilm( Bitmap & pcb );
			CBox BoundingBox() const;

		private:
			Aperture m_aperture;
			gp_Pnt	m_start;
			gp_Pnt	m_end;
			double	m_i_term;
			double	m_j_term;
			mutable double	m_radius;
			eInterpolation_t m_interpolation;
			bool	m_clockwise;

			double	m_tolerance;
			mutable HeeksObj *m_pHeeksObject;
		}; // End Trace class definition.

		char ReadChar( const char *data, int *pos, const int max_pos );
		std::string ReadBlock( const char *data, int *pos, const int max_pos );

		bool ReadParameters( const std::string & parameters );
		bool ReadDataBlock( const std::string & data_block );

		double InterpretCoord(	const char *coordinate,
					const int digits_left_of_point,
					const int digits_right_of_point,
					const bool leading_zero_suppression,
					const bool trailing_zero_suppression ) const;

		int m_current_line;

		double m_units;	// 1 = mm, 25.4 = inches
		bool m_leadingZeroSuppression;
		bool m_trailingZeroSuppression;
		bool m_absoluteCoordinatesMode;

		unsigned int m_XDigitsLeftOfPoint;
		unsigned int m_XDigitsRightOfPoint;

		unsigned int m_YDigitsLeftOfPoint;
		unsigned int m_YDigitsRightOfPoint;

		bool m_full_circular_interpolation;
		bool m_part_circular_interpolation;
		bool m_cw_circular_interpolation;
		bool m_area_fill;
		bool m_mirror_image;
		bool m_bWithinApertureMacroDefinition;

		std::string m_LayerName;
		int	m_active_aperture;
		gp_Pnt	m_current_position;
		bool m_lamp_on;	// Is the lamp on or off.  We need to remember for modal movements.

		typedef std::map< unsigned int, Aperture > ApertureTable_t;
		ApertureTable_t m_aperture_table;

		typedef std::list<Trace> Traces_t;
		Traces_t	m_traces;

		typedef std::list< Traces_t >	FilledAreas_t;
		Traces_t	m_filled_area_traces;	// The current set being aggregated.
		FilledAreas_t	m_filled_areas;		// All filled areas so far.

		typedef std::list<TopoDS_Face> Faces_t;

		bool AggregateFilledArea( const Traces_t & traces, TopoDS_Face *pResult ) const;
		bool AggregateFaces( const TopoDS_Face lhs, const TopoDS_Face rhs, TopoDS_Face *pResult ) const;
		static HeeksObj *Sketch( const TopoDS_Face face );
		static bool FacesIntersect( const TopoDS_Face lhs, const TopoDS_Face rhs );

		int FormNetworks();
		void DrawCentrelines();
		Bitmap RenderToBitmap();
		CBox BoundingBox() const;
		static double AngleBetweenVectors(  const gp_Pnt & vector_1_start_point,
                                            const gp_Pnt & vector_1_end_point,
                                            const gp_Pnt & vector_2_start_point,
                                            const gp_Pnt & vector_2_end_point,
                                            const double minimum_angle );

		struct traces_intersect : std::unary_function< const Trace &, bool >
		{
			traces_intersect( const Trace & trace ) : m_trace(trace) { }

			bool operator()( const Trace & rhs ) const
			{
				return(m_trace.Intersects(rhs));
			}

			Trace m_trace;
		};
};


