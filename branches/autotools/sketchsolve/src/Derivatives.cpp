/*
 * Derivatives.cpp
 *
 *  Created on: Jun 8, 2009
 *      Author: jgeorge
 */

#include <iostream>
#include <math.h>
#include "solve.h"
using namespace std;

void derivatives(double **x,double *gradF,int xLength, constraint * cons, int consLength)
{
	int position;

	for(int i=0;i<consLength;i++)
	{
		//////////////////////////////////////
		//Point on Point Constraint derivative
		//////////////////////////////////////
		if((cons[i]).type==pointOnPoint)
		{
			// Derivative with respect to p1x
			position = &P1_x - x[0];
			if(position >=0 & position<xLength)
				{
				gradF[position] += 2 * (P1_x-P2_x);
				}

			// Derivative with respect to p1y
			position = &P1_y - x[0];
			if(position >=0 & position<xLength)
				{
				gradF[position] += 2 * (P1_y - P2_y);
				}

			// Derivative with respect to p2x
			position = &P2_x - x[0];
			if(position >=0 & position<xLength)
				{
				gradF[position] += -2 * (P1_x - P2_x);
				}

			// Derivative with respect to p2y
			position = &P2_y - x[0];
			if(position >=0 & position<xLength)
				{
				gradF[position] += -2 * (P1_y - P2_y);
				}
		}
		//.........................................
		// End Point On Point Constraint derivative
		//.........................................

		//////////////////////////////////////
		//Point to Point Distance Constraint derivative
		//////////////////////////////////////
		if((cons[i]).type==P2PDistance)
		{
			// Derivative with respect to p1x
			position = &P1_x - x[0];
			if(position >=0 & position<xLength) gradF[position] += 2 * (P1_x-P2_x);

			// Derivative with respect to p1y
			position = &P1_y - x[0];
			if(position >=0 & position<xLength) gradF[position] += 2 * (P1_y - P2_y);

			// Derivative with respect to p2x
			position = &P2_x - x[0];
			if(position >=0 & position<xLength) gradF[position] += -2 * (P1_x - P2_x);

			// Derivative with respect to p2y
			position = &P2_y - x[0];
			if(position >=0 & position<xLength) gradF[position] += -2 * (P1_y - P2_y);

			// Derivative with respect to distance
			position = &distance - x[0];
			if(position >=0 & position<xLength) gradF[position] += -2 * distance;
		}
		//..................................................
		// End Point to Point distance Constraint derivative
		//..................................................

		//////////////////////////////////////
		//Point to Point Distance Vert Constraint derivative
		//////////////////////////////////////
		if((cons[i]).type==P2PDistance)
		{

			// Derivative with respect to p1y
			position = &P1_y - x[0];
			if(position >=0 & position<xLength) gradF[position] += 2 * (P1_y - P2_y);

			// Derivative with respect to p2y
			position = &P2_y - x[0];
			if(position >=0 & position<xLength) gradF[position] += -2 * (P1_y - P2_y);

			// Derivative with respect to distance
			position = &distance - x[0];
			if(position >=0 & position<xLength) gradF[position] += -2 * distance;
		}
		//........................................................
		// End Point to Point Vert distance Constraint derivative
		//........................................................

		//////////////////////////////////////
		//Point to Point Horz Distance Constraint derivative
		//////////////////////////////////////
		if((cons[i]).type==P2PDistance)
		{
			// Derivative with respect to p1x
			position = &P1_x - x[0];
			if(position >=0 & position<xLength) gradF[position] += 2 * (P1_x-P2_x);

			// Derivative with respect to p2x
			position = &P2_x - x[0];
			if(position >=0 & position<xLength) gradF[position] += -2 * (P1_x - P2_x);

			// Derivative with respect to distance
			position = &distance - x[0];
			if(position >=0 & position<xLength) gradF[position] += -2 * distance;
		}
		//.......................................................
		// End Point to Point Horz distance Constraint derivative
		//.......................................................

		//////////////////////////////////////
		//Point on line Constraint derivatives
		//////////////////////////////////////
		if((cons[i]).type==pointOnLine)
		{
			// Derivative with respect to p1x
			position = &P1_x - x[0];
			if(position >=0 & position<xLength) gradF[position] += 2 * (P1_x-P2_x);

			// Derivative with respect to p2x
			position = &P2_x - x[0];
			if(position >=0 & position<xLength) gradF[position] += -2 * (P1_x - P2_x);

			// Derivative with respect to distance
			position = &distance - x[0];
			if(position >=0 & position<xLength) gradF[position] += -2 * distance;
		}
		//.......................................................
		// End Point to Point Horz distance Constraint derivative
		//.......................................................


	}
}
