/*
 * solve.h
 *
 *  Created on: May 4, 2009
 *      Author: Jonathan
 *      Copyright (c) 2009, Jonathan George
 *      This program is released under the BSD license. See the file COPYING for details.
 */
#include <iostream>

#ifndef WIN32
	#define _hypot hypot
#endif

//This define selects between the old arc define by start point end point and center point
// and the new arc defined by a centerpoint, radius, start angle, and end angle
//#define NEWARC
// Eventually after everything is changed we will get rid of the old arc type.

#ifndef SOLVE_H_
#define SOLVE_H_

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

#define pointOnPoint	 0
#define pointToLine 	 1
#define pointOnLine      2
#define horizontal       3
#define vertical         4
#define internalAngle	 5
#define radiusValue		 6
#define tangentToArc	 7
#define tangentToCircle	 8
#define arcRules 		 9
#define P2PDistance      10
#define P2PDistanceVert  11
#define P2PDistanceHorz  12
#define P2LDistance      13
#define P2LDistanceVert  14
#define P2LDistanceHorz  15
#define lineLength		 16
#define equalLegnth		 17
#define arcRadius		 18
#define equalRadiusArcs      19
#define equalRadiusCircles   20
#define equalRadiusCircArc   21
#define concentricArcs       22
#define concentricCircles    23
#define concentricCircArc    24
#define circleRadius         25
#define externalAngle    26
#define parallel         27
#define perpendicular    28
#define colinear	     29
#define pointOnCircle    30
#define pointOnArc       31
#define pointOnLineMidpoint  32
#define pointOnArcMidpoint   33
#define pointOnCircleQuad    34
#define symmetricPoints      35
#define symmetricLines       36
#define symmetricCircles	 37
#define symmetricArcs		 38

#define pointOnArcStart      39  
#define pointOnArcEnd        40
#define arcStartToArcEnd     41
#define arcStartToArcStart   42
#define arcEndtoArcEnd       43

#define arcTangentToArc		   44
#define circleTangentToCircle  45
#define circleTangentToArc     46




///////////////////////////////////////
/// BFGS Solver parameters
///////////////////////////////////////
#define pertMag           1e-6
#define pertMin			  1e-10
#define XconvergenceRough 1e-8
#define XconvergenceFine  1e-10
#define smallF            1e-20
#define validSolutionFine  1e-12
#define validSoltuionRough 1e-4
#define rough             0
#define fine              1
#define MaxIterations	  50 //Note that the total number of iterations allowed is MaxIterations *xLength

///////////////////////////////////////
/// Solve exit codes
///////////////////////////////////////

#define succsess 0
#define noSolution 1

///////////////////////////////////////////////////////////////////////
/// constraint defines (these make writing constraint equations easier
///////////////////////////////////////////////////////////////////////
#define P1_x		   *cons[i].point1.x
#define P1_y	       *cons[i].point1.y
#define P2_x           *cons[i].point2.x
#define P2_y           *cons[i].point2.y
#define L1_P1_x        *cons[i].line1.p1.x
#define L1_P1_y        *cons[i].line1.p1.y
#define L1_P2_x        *cons[i].line1.p2.x
#define L1_P2_y        *cons[i].line1.p2.y
#define L2_P1_x        *cons[i].line2.p1.x
#define L2_P1_y        *cons[i].line2.p1.y
#define L2_P2_x        *cons[i].line2.p2.x
#define L2_P2_y        *cons[i].line2.p2.y
#define C1_Center_x    *cons[i].circle1.center.x
#define C1_Center_y    *cons[i].circle1.center.y
#define C1_rad         *cons[i].circle1.rad
#define C1_radius	   *cons[i].parameter
#define C2_Center_x    *cons[i].circle2.center.x
#define C2_Center_y    *cons[i].circle2.center.y
#define C2_rad         *cons[i].circle2.rad

#ifndef NEWARC

	#define A1_Start_x     *cons[i].arc1.start.x
	#define A1_Start_y     *cons[i].arc1.start.y
	#define A1_End_x       *cons[i].arc1.end.x
	#define A1_End_y       *cons[i].arc1.end.y
	#define A1_Center_x    *cons[i].arc1.center.x
	#define A1_Center_y    *cons[i].arc1.center.y
	#define A1_radius      *cons[i].parameter
	#define A2_Start_x     *cons[i].arc2.start.x
	#define A2_Start_y     *cons[i].arc2.start.y
	#define A2_End_x       *cons[i].arc2.end.x
	#define A2_End_y       *cons[i].arc2.end.y
	#define A2_Center_x    *cons[i].arc2.center.x
	#define A2_Center_y    *cons[i].arc2.center.y

	#define A1_rad	   sqrt((A1_Start_x-A1_Center_x)*(A1_Start_x-A1_Center_x)+(A1_Start_y-A1_Center_y)*(A1_Start_y-A1_Center_y))
	#define A2_rad	   sqrt((A2_Start_x-A2_Center_x)*(A2_Start_x-A2_Center_x)+(A2_Start_y-A2_Center_y)*(A2_Start_y-A2_Center_y))

#else
	#define A1_startA	   *cons[i].arc1.startAngle
	#define A1_endA		   *cons[i].arc1.endAngle
	#define A1_radius      *cons[i].arc1.radius
	#define A1_Center_x    *cons[i].arc1.center.x
	#define A1_Center_y    *cons[i].arc1.center.y
	#define A2_startA	   *cons[i].arc2.startAngle
	#define A2_endA		   *cons[i].arc2.endAngle
	#define A2_radius      *cons[i].arc2.radius
	#define A2_Center_x    *cons[i].arc2.center.x
	#define A2_Center_y    *cons[i].arc2.center.y

	#define A1_Start_x     (A1_Center_x+A1_radius*cos(A1_startA))
	#define A1_Start_y     (A1_Center_y+A1_radius*sin(A1_startA))
	#define A1_End_x       (A1_Center_x+A1_radius*cos(A1_endA))
	#define A1_End_y     (A1_Center_y+A1_radius*sin(A1_endA))
	#define A2_Start_x     (A1_Center_x+A2_radius*cos(A2_startA))
	#define A2_Start_y     (A1_Center_y+A2_radius*sin(A2_startA))
	#define A2_End_x       (A1_Center_x+A2_radius*cos(A2_endA))
	#define A2_End_y     (A1_Center_y+A2_radius*sin(A2_endA))

#endif

#define length		   *cons[i].parameter
#define distance	   *cons[i].parameter
//#define radius		   *cons[i].parameter
#define angleP		   *cons[i].parameter
#define quadIndex      *cons[i].parameter
#define Sym_P1_x       *cons[i].SymLine.p1.x
#define Sym_P1_y       *cons[i].SymLine.p1.y
#define Sym_P2_x       *cons[i].SymLine.p2.x
#define Sym_P2_y       *cons[i].SymLine.p2.y


class point
{
public:
	point(){x = 0; y = 0;}
	double * x;
	double * y;
};

class line
{
public:
	line(){}
	point p1;
	point p2;
};

#ifndef NEWARC
	class arc
	{
	public:
		arc(){}
		point start;
		point end;
		point center;
	};

#else

	class arc
	{
	public:
		arc(){startAngle=0;endAngle=0;}
		double *startAngle;
		double *endAngle;
		double *radius;//This is called parameter in the constraint class.
		point center;
	};
#endif

class circle
{
public:
	circle(){rad = 0;}
	point center;
	double *rad;
};

class constraint
{
public:
	constraint(){parameter = 0;}
	int type;
	point point1;
	point point2;
	line line1;
	line line2;
	line SymLine;
	circle circle1;
	circle circle2;
	arc arc1;
	arc arc2;
	double *parameter; //radius, length, angle etc...
};



void debugprint(std::string s);

//Function Prototypes
int solve(double  **x,int xLength, constraint * cons, int consLength, int isFine);
double calc(constraint * cons, int consLength);
void derivatives(double **x,double *gradF,int xLength, constraint * cons, int consLength);

#endif /* SOLVE_H_ */
