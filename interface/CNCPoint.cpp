// CNCPoint.cpp
/*
 * Copyright (c) 2009, Dan Heeks, Perttu Ahola
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */

#include "stdafx.h"
#include "CNCPoint.h"

#ifdef HEEKSCAD
	#include "../interface/HeeksCADInterface.h"
	extern CHeeksCADInterface heekscad_interface;
#else
	#include "interface/HeeksCADInterface.h"
	extern CHeeksCADInterface* heeksCAD;
#endif

CNCPoint::CNCPoint() : gp_Pnt(0.0, 0.0, 0.0)
{
}

CNCPoint::CNCPoint( const double *xyz ) : gp_Pnt(xyz[0], xyz[1], xyz[2])
{
}

CNCPoint::CNCPoint( const double &x, const double &y, const double &z ) : gp_Pnt(x,y,z)
{
}
CNCPoint::CNCPoint( const gp_Pnt & rhs ) : gp_Pnt(rhs)
{
}

double CNCPoint::Tolerance() const
{
#ifdef HEEKSCAD
	return(heekscad_interface.GetTolerance());
#else
	return(heeksCAD->GetTolerance());
#endif
}

double CNCPoint::Units() const
{
#ifdef HEEKSCAD
	return(wxGetApp().m_view_units);
#else
	return heeksCAD->GetViewUnits();
#endif
}


double CNCPoint::X(const bool in_drawing_units /* = false */) const
{
    if (in_drawing_units == false) return(gp_Pnt::X());
    else return(gp_Pnt::X() / Units());
}

double CNCPoint::Y(const bool in_drawing_units /* = false */) const
{
    if (in_drawing_units == false) return(gp_Pnt::Y());
    else return(gp_Pnt::Y() / Units());
}

double CNCPoint::Z(const bool in_drawing_units /* = false */) const
{
    if (in_drawing_units == false) return(gp_Pnt::Z());
    else return(gp_Pnt::Z() / Units());
}

CNCPoint & CNCPoint::operator+= ( const CNCPoint & rhs )
{
    SetX( X() + rhs.X() );
    SetY( Y() + rhs.Y() );
    SetZ( Z() + rhs.Z() );

    return(*this);
}

CNCPoint CNCPoint::operator- ( const CNCPoint & rhs ) const
{
    CNCPoint result(*this);
    result.SetX( X() - rhs.X() );
    result.SetY( Y() - rhs.Y() );
    result.SetZ( Z() - rhs.Z() );

    return(result);
}

bool CNCPoint::operator==( const CNCPoint & rhs ) const
{
    // We use the sum of both point's tolerance values.
    return(Distance(rhs) < (Tolerance() + rhs.Tolerance()));
} // End equivalence operator

bool CNCPoint::operator!=( const CNCPoint & rhs ) const
{
    return(! (*this == rhs));
} // End not-equal operator

bool CNCPoint::operator<( const CNCPoint & rhs ) const
{
    if (*this == rhs) return(false);

    if (fabs(X() - rhs.X()) > Tolerance())
    {
        if (X() > rhs.X()) return(false);
        if (X() < rhs.X()) return(true);
    }

    if (fabs(Y() - rhs.Y()) > Tolerance())
    {
        if (Y() > rhs.Y()) return(false);
        if (Y() < rhs.Y()) return(true);
    }

    if (fabs(Z() - rhs.Z()) > Tolerance())
    {
        if (Z() > rhs.Z()) return(false);
        if (Z() < rhs.Z()) return(true);
    }

    return(false);	// They're equal
} // End equivalence operator

void CNCPoint::ToDoubleArray( double *pArrayOfThree ) const
{
    pArrayOfThree[0] = X();
    pArrayOfThree[1] = Y();
    pArrayOfThree[2] = Z();
} // End ToDoubleArray() method




CNCVector::CNCVector() : gp_Vec(0.0, 0.0, 0.0)
{
}

CNCVector::CNCVector( const double *xyz ) : gp_Vec(xyz[0], xyz[1], xyz[2])
{
}
CNCVector::CNCVector( const double &x, const double &y, const double &z ) : gp_Vec(x,y,z)
{
}

CNCVector::CNCVector( const gp_Vec & rhs ) : gp_Vec(rhs)
{
}

bool CNCVector::operator==( const CNCVector & rhs ) const
{
    return(this->IsEqual(rhs, Tolerance(), Tolerance()) == Standard_True);
} // End equivalence operator

bool CNCVector::operator!=( const CNCVector & rhs ) const
{
    return(! (*this == rhs));
} // End not-equal operator

bool CNCVector::operator<( const CNCVector & rhs ) const
{
    for (int offset=1; offset <=3; offset++)
    {
        if (fabs(Coord(offset) - rhs.Coord(offset)) < Tolerance()) continue;

        if (Coord(offset) > rhs.Coord(offset)) return(false);
        if (Coord(offset) < rhs.Coord(offset)) return(true);
    }

    return(false);	// They're equal
} // End equivalence operator

double CNCVector::Tolerance() const
{
#ifdef HEEKSCAD
	return(heekscad_interface.GetTolerance());
#else
	return(heeksCAD->GetTolerance());
#endif
}






