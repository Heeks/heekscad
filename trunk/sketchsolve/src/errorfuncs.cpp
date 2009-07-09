/*
 * solve.cpp
 *
 *  Created on: May 4, 2009
 *      Author: Jonathan George
 *      This
 *      Copyright (c) 2009, Jonathan George
 *      This program is released under the BSD license. See the file COPYING for details.
 *
 */

#include "solve.h"
#include <cmath>
#include <stdlib.h>
#include <sstream>

using namespace std;

double ParallelError(std::vector<double> parms)
{
     double dx = parms[2] - parms[0];
     double dy = parms[3] - parms[1];
     double dx2 = parms[6] - parms[4];
     double dy2 = parms[7] - parms[5];

     double hyp1=sqrt(dx*dx+dy*dy);
     double hyp2=sqrt(dx2*dx2+dy2*dy2);

     dx=dx/hyp1;
     dy=dy/hyp1;
     dx2=dx2/hyp2;
     dy2=dy2/hyp2;

     double temp = dy*dx2-dx*dy2;
     return (temp)*(temp);
}

double HorizontalError(std::vector<double> parms)
{
   double ody = parms[3] - parms[1];
   return ody*ody*1000;
}

double VerticalError(std::vector<double> parms)
{
   double odx = parms[2] - parms[0];
   return odx*odx*1000;
}

double PointOnPointError(std::vector<double> parms)
{
    //Hopefully avoid this constraint, make coincident points use the same parameters
	double dx = parms[0] - parms[2];
	double dy = parms[1] - parms[3];
    return dx*dx + dy*dy;
}

double P2PDistanceError(std::vector<double> parms)
{
	double dx = parms[0] - parms[2];
	double dy = parms[1] - parms[3];
	double d = parms[4];
    return dx*dx+dy*dy - d * d;
}



