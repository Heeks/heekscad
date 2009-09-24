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

#include <gp_Pnt.hxx>
#include <gp_Lin.hxx>
#include <gp_Circ.hxx>
#include <gp_Vec.hxx>

#include "HLine.h"
#include "HArc.h"
#include "HCircle.h"
#include "HEllipse.h"
#include "HSpline.h"
#include "Sketch.h"

#include "Polygon.h"

class RS274X
{

	public:
		RS274X();
		~RS274X() { }

		bool Read( const char *p_szFileName);

	private:
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

			public:
				void Type( const eType_t type ) { m_type = type; }
				eType_t Type() const { return(m_type); }

				void OutsideDiameter( const double value ) { m_outside_diameter = value; }
				double OutsideDiameter() const { return(m_outside_diameter); }

				void XAxisHoleDimension( const double value ) { m_x_axis_hole_dimension = value; }
				double XAxisHoleDimension() const { return(m_x_axis_hole_dimension); }

				void YAxisHoleDimension( const double value ) { m_y_axis_hole_dimension = value; }
				double YAxisHoleDimension() const { return(m_y_axis_hole_dimension); }

				void XAsixOutsideDimension( const double value ) { m_x_axis_outside_dimension = value; }
				double XAsixOutsideDimension() const { return(m_x_axis_outside_dimension); }

				void YAsixOutsideDimension( const double value ) { m_y_axis_outside_dimension = value; }
				double YAsixOutsideDimension() const { return(m_y_axis_outside_dimension); }

				void DegreeOfRotation( const double value ) { m_degree_of_rotation = value; }
				double DegreeOfRotation() const { return(m_degree_of_rotation); }


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
		}; // End Aperture class defintion.
		
	public:
		typedef CPolygon Polygon_t;
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
			typedef enum { eLinear = 0, eCircular } eInterpolation_t;

			Trace( const Aperture & aperture, const eInterpolation_t interpolation ) : m_aperture(aperture), m_interpolation( interpolation )
			{ 
				m_start.SetX(0.0);
				m_start.SetY(0.0);
				m_start.SetZ(0.0);

				m_end.SetX(0.0);
				m_end.SetY(0.0);
				m_end.SetZ(0.0);

				m_i_term = 0.0;
				m_j_term = 0.0;

				m_radius = 0.0;

				m_clockwise = true;

				m_tolerance = wxGetApp().m_geom_tol;

				m_pHeeksObject = NULL;
			}

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

			double Radius() const
			{
				if (m_radius < m_tolerance)
				{
					m_radius = sqrt( ( m_i_term * m_i_term ) + ( m_j_term * m_j_term ) );
				}

				return(m_radius);
			}
			void Radius( const double value ) { m_radius = value; }

			gp_Pnt Centre() const
			{
				if ((Interpolation() == eCircular) && (m_i_term < m_tolerance) && (m_j_term < m_tolerance))
				{
					// It's a full circle.  Set the centre to the start/end points.
					return(m_start);
				} // End if - then

				if (Interpolation() == eLinear)
				{
					return(gp_Pnt(  ((Start().X() - End().X())/2) + Start().X(),
									((Start().Y() - End().Y())/2) + Start().Y(),
									((Start().Z() - End().Z())/2) + Start().Z() ));
				} // End if - then

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

			gp_Dir Direction() const
			{
				switch(Interpolation())
				{
					case eCircular:
						if (m_clockwise) return(gp_Dir(0,0,-1));
						else	return(gp_Dir(0,0,1));

					case eLinear:
						return( gp_Dir( m_end.X() - m_start.X(), m_end.Y() - m_start.Y(), m_end.Z() - m_start.Z() ));
				}
			}

			// Get gp_Circle for arc
			gp_Circ Circle() const
			{
				gp_Ax1 axis( Centre(), Direction() );
				return gp_Circ(gp_Ax2(Centre(),Direction()), Radius());
			}

			gp_Lin Line() const
			{
				return(gp_Lin(Start(),Direction()));
			}

			/*bool Intersects( const Trace & rhs ) const
			{
				std::list<double> points;
				return( HeeksObject()->Intersects( rhs.HeeksObject(), &points ) > 0);
			} // End Intersects() method*/

			double Length() const
			{
				switch (Interpolation())
				{
					case eLinear:
						return(Start().Distance(End()));
						break;

					default:
					case eCircular:
						if ((m_i_term < m_tolerance) && (m_j_term < m_tolerance) && (Radius() > m_tolerance))
						{
							// It's a full circle.
							return(2.0 * PI * Radius());
						} // End if - then
						else
						{
							gp_Vec vx(1.0, 0.0, 0.0);
							gp_Vec vy(0.0, 1.0, 0.0);

							gp_Vec start_vector( Centre(), Start() );
							gp_Vec end_vector( Centre(), End() );

							double start_angle = start_vector.Angle( vx );
							if (start_angle < 0.0) start_angle += (2.0 * PI);

							double end_angle = end_vector.Angle( vx );
							if (end_angle < 0.0) end_angle += (2.0 * PI);

							double arc_angle = end_angle - start_angle;
							double arc_length = (arc_angle / (2.0 * PI)) * (2.0 * PI * Radius());
							return(abs(arc_length));
						} // End if - else
						break;
				} // End switch
			} // End Length() method

			bool operator==( const Trace & rhs ) const
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

			bool MakePolygon(Polygon_t & polygon) const
			{
				switch (Interpolation())
				{
					case eLinear:
					{
						//it's a line
						gp_Vec v(Start(), End());
						v = v.Normalized();
						//vector in 90 degree angle to the left
						gp_Vec n(-v.Y(), v.X(), 0);
						float d = m_aperture.OutsideDiameter();
						//first line is on the right side of the original
						polygon.push_back(End().Translated(-n*d*.5));
						polygon.push_back(Start().Translated(-n*d*.5));
						polygon.push_back(Start().Translated(-n*d*sin(PI/4)/2- v*d*sin(PI/4)/2));
						polygon.push_back(Start().Translated(-v*d*0.5));
						polygon.push_back(Start().Translated(n*d*sin(PI/4)/2 - v*d*sin(PI/4)/2));
						polygon.push_back(Start().Translated(n*d*.5));
						polygon.push_back(End().Translated(n*d*.5));
						polygon.push_back(End().Translated(n*d*sin(PI/4)/2 + v*d*sin(PI/4)/2));
						polygon.push_back(End().Translated(v*d*0.5));
						polygon.push_back(End().Translated(-n*d*sin(PI/4)/2 + v*d*sin(PI/4)/2));
						return true;
					}

					case eCircular:
					{
						if ((m_i_term < m_tolerance) && (m_j_term < m_tolerance) && (Radius() > m_tolerance))
						{
							// It's a full circle.
							
							gp_Vec vx(1.0, 0.0, 0.0);
							gp_Vec vy(0.0, 1.0, 0.0);
							
							for(int i=0; i<8; i++){
								double angle = -PI*2.0*(double)i/8.0;
								double radius = Radius() + m_aperture.OutsideDiameter()*0.5;
								polygon.push_back(Centre().Translated(vx*radius*cos(angle)+vy*radius*sin(angle)));
							}
							for(int i=0; i<8; i++){
								double angle = -PI*2.0*(double)i/8.0;
								double radius = Radius() - m_aperture.OutsideDiameter()*0.5;
								if(radius > m_tolerance)
								{
									polygon.push_back(Centre().Translated(vx*radius*cos(angle)+vy*radius*sin(angle)));
								}
							}
							return true;
						}
						else
						{
							// It's an arc
							double start[3];
							double end[3];
							double centre[3];
							double up[3];

							start[0] = Start().X();
							start[1] = Start().Y();
							start[2] = Start().Z();

							end[0] = End().X();
							end[1] = End().Y();
							end[2] = End().Z();
		
							centre[0] = Centre().X();
							centre[1] = Centre().Y();
							centre[2] = Centre().Z();

							if (Clockwise())
							{
								up[0] = 0;
								up[1] = 0;
								up[2] = -1;
							}
							else
							{
								up[0] = 0;
								up[1] = 0;
								up[2] = 1;
							}
						
							gp_Pnt p0 = make_point(start);
							gp_Pnt p1 = make_point(end);
							gp_Dir dir(up[0], up[1], up[2]);
							gp_Pnt pc = make_point(centre);
							gp_Circ circle(gp_Ax2(pc, dir), p1.Distance(pc));

							gp_Vec vx(1.0, 0.0, 0.0);
							gp_Vec vy(0.0, 1.0, 0.0);

							double start_angle = vx.Angle( gp_Vec( Centre(), Start() ));
							if (start_angle < 0) start_angle += (2.0 * PI);
							
							double end_angle = vx.Angle( gp_Vec( Centre(), End() ));
							if (end_angle < 0) end_angle += (2.0 * PI);

							if (start_angle > end_angle)
							{
								// Swap them.
								double temp = start_angle;
								start_angle = end_angle;
								end_angle = temp;
							} // End if - then

							double number_of_segments = Length() / (0.05 * 25.4);
							double angle_increment = (end_angle - start_angle) / number_of_segments;

							double radius = Radius() + (m_aperture.OutsideDiameter()*0.5);
							for (double angle = start_angle; angle <= end_angle; angle += angle_increment)
							{
								if (angle > m_tolerance)
								{
									polygon.push_back(Centre().Translated(vx*radius*cos(angle)+vy*radius*sin(angle)));
								} // End if - then
							} // End for
							if (end_angle > m_tolerance)
							{
								polygon.push_back(Centre().Translated(vx*radius*cos(end_angle)+vy*radius*sin(end_angle)));
							} // End if - then

							radius = Radius() - (m_aperture.OutsideDiameter()*0.5);
							for (double angle = end_angle; angle >= start_angle; angle -= angle_increment)
							{
								if (angle > m_tolerance)
								{
									polygon.push_back(Centre().Translated(vx*radius*cos(angle)+vy*radius*sin(angle)));
								} // End if - then
							} // End for
							if (start_angle > m_tolerance)
							{
								polygon.push_back(Centre().Translated(vx*radius*cos(start_angle)+vy*radius*sin(start_angle)));
							} // End if - then

							return(true);
						}
						break;
					}
				} // End switch
				return false;
			}

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

		std::string m_LayerName;
		int	m_active_aperture;
		gp_Pnt	m_current_position;

		typedef std::map< unsigned int, Aperture > ApertureTable_t;
		ApertureTable_t m_aperture_table;

		typedef std::list<Trace> Traces_t;
		Traces_t	m_traces;
	
		typedef std::list<Polygon_t> Polygons_t;
		Polygons_t	m_polygons;

		CSketch *MakeSketch(Polygon_t polygon) const
		{
			CSketch *sketch = new CSketch();

			gp_Pnt oldp = *(polygon.begin());
			
			for(std::list<gp_Pnt>::const_iterator ilp = ++polygon.begin();
					ilp != polygon.end(); ilp++)
			{
				if(!oldp.IsEqual(*ilp, wxGetApp().m_geom_tol))
				{
					HLine *line = new HLine(oldp, *ilp, &(wxGetApp().current_color));
					sketch->Add(line, NULL);
				}
				oldp = *ilp;
			}
			if(!oldp.IsEqual(*(polygon.begin()), wxGetApp().m_geom_tol))
			{
				sketch->Add(new HLine(oldp, *(polygon.begin()), &(wxGetApp().current_color)), NULL);
			}
			return sketch;
		}

		void AddPolygonIfNeeded()
		{
			std::list<Polygon_t>::iterator ipoly = m_polygons.end();
			ipoly--;
			if(!ipoly->empty()){
				Polygon_t polygon;
				m_polygons.push_back(polygon);
			}
		}
};


