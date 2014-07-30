// Box.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include <string.h>	// for memcpy() prototype
#include <math.h>	// for sqrt() prototype

class CBox{
public:
	double m_x[6];
	bool m_valid;

	CBox():m_valid(false){}
	CBox(const double *e):m_valid(true){memcpy(m_x, e, 6*sizeof(double));}
	CBox(double xmin, double ymin, double zmin, double xmax, double ymax, double zmax):m_valid(true){m_x[0] = xmin; m_x[1] = ymin; m_x[2] = zmin; m_x[3] = xmax; m_x[4] = ymax; m_x[5] = zmax;}

	bool operator==( const CBox & rhs ) const
	{
		for (::size_t i=0; i<sizeof(m_x)/sizeof(m_x[0]); i++) if (m_x[i] != rhs.m_x[i]) return(false);
		if (m_valid != rhs.m_valid) return(false);

		return(true);
	}

	bool operator!=( const CBox & rhs ) const { return(! (*this == rhs)); }


	void Insert(const double *p){ // insert a point ( 3 doubles )
		if(m_valid){
			for(int i = 0; i<3; i++){
				if(p[i] < m_x[i])m_x[i] = p[i];
				if(p[i] > m_x[i+3])m_x[i+3] = p[i];
			}
		}
		else
		{
			m_valid = true;
			memcpy(m_x, p, 3*sizeof(double));
			memcpy(&m_x[3], p, 3*sizeof(double));
		}
	}
	void Insert(double x, double y, double z){ // insert a point
		if(m_valid){
			if(x < m_x[0])m_x[0] = x;
			if(x > m_x[3])m_x[3] = x;
			if(y < m_x[1])m_x[1] = y;
			if(y > m_x[4])m_x[4] = y;
			if(z < m_x[2])m_x[2] = z;
			if(z > m_x[5])m_x[5] = z;
		}
		else
		{
			m_valid = true;
			m_x[0] = m_x[3] = x;
			m_x[1] = m_x[4] = y;
			m_x[2] = m_x[5] = z;
		}
	}
	void Insert(const CBox& b){
		if(b.m_valid){
			if(m_valid){
				for(int i = 0; i<3; i++){
					if(b.m_x[i] < m_x[i])m_x[i] = b.m_x[i];
					if(b.m_x[i+3] > m_x[i+3])m_x[i+3] = b.m_x[i+3];
				}
			}
			else{
				m_valid = b.m_valid;
				memcpy(m_x, b.m_x, 6*sizeof(double));
			}
		}
	}
	void Centre(double *p) const {p[0] = (m_x[0] + m_x[3])/2; p[1] = (m_x[1] + m_x[4])/2; p[2] = (m_x[2] + m_x[5])/2;}
	double Width() const {if(m_valid)return m_x[3] - m_x[0]; else return 0.0;}
	double Height() const {if(m_valid)return m_x[4] - m_x[1]; else return 0.0;}
	double Depth() const {if(m_valid)return m_x[5] - m_x[2]; else return 0.0;}
	double Radius() const {return sqrt(Width() * Width() + Height() * Height() + Depth() * Depth()) /2;}
	double MinX() const { return(m_x[0]); }
	double MaxX() const { return(m_x[3]); }
	double MinY() const { return(m_x[1]); }
	double MaxY() const { return(m_x[4]); }
	double MinZ() const { return(m_x[2]); }
	double MaxZ() const { return(m_x[5]); }
	void vert(int index, double* p) const {
		switch(index){
			case 0:
				p[0] = m_x[0];
				p[1] = m_x[1];
				p[2] = m_x[2];
				break;

			case 1:
				p[0] = m_x[3];
				p[1] = m_x[1];
				p[2] = m_x[2];
				break;

			case 2:
				p[0] = m_x[3];
				p[1] = m_x[4];
				p[2] = m_x[2];
				break;

			case 3:
				p[0] = m_x[0];
				p[1] = m_x[4];
				p[2] = m_x[2];
				break;

			case 4:
				p[0] = m_x[0];
				p[1] = m_x[1];
				p[2] = m_x[5];
				break;

			case 5:
				p[0] = m_x[3];
				p[1] = m_x[1];
				p[2] = m_x[5];
				break;

			case 6:
				p[0] = m_x[3];
				p[1] = m_x[4];
				p[2] = m_x[5];
				break;

			case 7:
				p[0] = m_x[0];
				p[1] = m_x[4];
				p[2] = m_x[5];
				break;

			default:
				p[0] = 0;
				p[1] = 0;
				p[2] = 0;
				break;

		}
	}
};

