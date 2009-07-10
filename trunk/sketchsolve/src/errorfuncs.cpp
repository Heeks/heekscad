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

double P2PDistanceHorzError(std::vector<double> parms)
{
	double dx = parms[0] - parms[2];
	double d = parms[4];
    return dx*dx - d * d;
}

double P2PDistanceVertError(std::vector<double> parms)
{
	double dy = parms[1] - parms[3];
	double d = parms[4];
    return dy*dy - d * d;
}

double PointOnLineError(std::vector<double> parms)
{
	double dx = parms[0] - parms[2];
	double dy = parms[1] - parms[3];

    double m=dy/dx; //Slope
    double n=dx/dy; //1/Slope

    if(m<=1 && m>=-1)
    {
       //Calculate the expected y point given the x coordinate of the point
       double Ey=parms[1]+m*(parms[4]-parms[0]);
       return (Ey-parms[5])*(Ey-parms[5]);
    }
    else
    {
       //Calculate the expected x point given the y coordinate of the point
       double Ex=parms[0]+n*(parms[5]-parms[1]);
       return (Ex-parms[4])*(Ex-parms[4]);
    }
}

double P2LDistanceError(std::vector<double> parms)                      
{
	double dx = parms[0] - parms[2];
	double dy = parms[1] - parms[3];

    double radsq = parms[6] * parms[6];
    double t=-(parms[0]*dx-parms[4]*dx+parms[1]*dy-parms[5]*dy)/(dx*dx+dy*dy);
    double Xint=parms[0]+dx*t;
    double Yint=parms[1]+dy*t;
    double temp= sqrt((parms[4] - Xint)*(parms[4] - Xint)+(parms[5] - Yint)*(parms[5] - Yint)) - sqrt(radsq);
    return temp*temp/100;
}

double P2LDistanceVertError(std::vector<double> parms)
{
	double dx = parms[0] - parms[2];
	double dy = parms[1] - parms[3];

    double t=(parms[4]- parms[0])/dx;
    double Yint=parms[1]+dy*t;
    double temp= fabs((parms[5] - Yint)) - parms[6];
    return temp*temp;
}

double P2LDistanceHorzError(std::vector<double> parms)
{
	double dx = parms[0] - parms[2];
	double dy = parms[1] - parms[3];

    double t=(parms[5]- parms[1])/dy;
    double Xint=parms[0]+dx*t;
    double temp= fabs((parms[4] - Xint)) - parms[6];
    return temp*temp/10;
}

double LineLengthError(std::vector<double> parms)
{
	double dx = parms[0] - parms[2];
	double dy = parms[1] - parms[3];
    double temp= sqrt(dx*dx+dy*dy) - parms[4];
	return temp*temp*100;
}
			

double EqualLengthError(std::vector<double> parms)
{
	double dx = parms[0] - parms[2];
	double dy = parms[1] - parms[3];
	double dx2 = parms[4] - parms[6];
	double dy2 = parms[5] - parms[7];

	double temp = sqrt(dx*dx+dy*dy) -  sqrt(dx2*dx2+dy2*dy2);
    return temp*temp;
}

double EqualScalarError(std::vector<double> parms)
{
    double temp= parms[0] - parms[1];
    return temp*temp;
}

double PointOnArcAngleError(std::vector<double> parms)
{
	double a1x = sin(parms[5]) * parms[4] + parms[2];
	double a1y = cos(parms[5]) * parms[4] + parms[3];
	double dx = parms[0] - a1x;
	double dy = parms[1] - a1y;
    return dx*dx + dy*dy;
}

