// BezierCurve.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "BezierCurve.h"
#if 0
#include "Point.h"
#include "Arc.h"
#endif

void split_bezier_curve(int level, const gp_Pnt& vt0, const gp_Pnt& vt2, const gp_Pnt& vt02, const gp_Pnt& vt20, void(*call_back)(const gp_Pnt& vt0, const gp_Pnt& vt1))
{
	if(level > 0)
	{
		// split into two sub lines
		for(int i = 0; i<2; i++){
			gp_Pnt nvt0 = vt0;
			gp_Pnt nvt2 = vt2;
			gp_Pnt nvt20 = vt20;
			gp_Pnt nvt02 = vt02;

			if(i == 1){
				// mirror
				gp_Pnt t2 = nvt2;
				gp_Pnt t20 = nvt20;
				nvt2 = nvt0;
				nvt20 = nvt02;
				nvt0 = t2;
				nvt02 = t20;
			}

			nvt2=(nvt20.XYZ()+nvt2.XYZ())/2;
			nvt20=(nvt02.XYZ()+nvt20.XYZ())/2;
			nvt2=(nvt20.XYZ()+nvt2.XYZ())/2;
			nvt02=(nvt0.XYZ()+nvt02.XYZ())/2;
			nvt20=(nvt02.XYZ()+nvt20.XYZ())/2;
			nvt2=(nvt20.XYZ()+nvt2.XYZ())/2;

			if(i == 1){
				// unmirror
				gp_Pnt t2 = nvt2;
				gp_Pnt t20 = nvt20;
				nvt2 = nvt0;
				nvt20 = nvt02;
				nvt0 = t2;
				nvt02 = t20;
			}

			// recursive
			split_bezier_curve(level - 1, nvt0, nvt2, nvt02, nvt20, call_back);
		}
		
	}
	else{
		// render line
		(*call_back)(vt0, vt2);
	}
}

#if 0

void biarc(const Point& P0, const Point& TS, const Point& P4, const Point& TE, double r, Point& P1, Point& P2, Point& P3, double &alpha, double &beta))
{
    TS = TS.unit();
    TE = TE.unit();

    Point v = P0 - P4;

    Point c = v*v
    double b = 2*v*(TS*r+TE)
    double a = 2*r*(TS*TE-1)

    if(fabs(a)<0.0000000000001)
	return;

    double discr = b*b-4*a*c;
    if(discr < 0)
	return;

    double disq = discr**.5;
    double beta1 = (-b - disq) / 2 / a;
    double beta2 = (-b + disq) / 2 / a;

    if (beta1 > 0.0000000000001 && beta2 > 0.000000000001)
	return;

	double beta = beta1;
	if(beta2 > beta)beta = beta2;

    if (beta < -0.0000000000001)
	return;

    double alpha = beta * r;
    double ab = alpha + beta;
    P1 = P0 + TS * alpha;
    P3 = P4 - TE * beta;
    P2 = P1 * (beta / ab) + P3 * (alpha / ab);
}

double radiusDifference(const Arc& arc, const Point& p)
{
	if(arc.m_c == arc.m_p0)return 0.0;

        return (arc.m_c - p).magn() - fabs(arc.m_p0.dist(arc.m_c));
}

void giarc(const Point& PS, const Point& TS, const Point& PE, const Point& TE, double r, Arc& arc1, Arc& arc2)
{
	Point TSu = TS.unit();
	Point TEu = TE.unit();
	Point P1, P2, P3;
	double alpha, beta;
	biarc(PS, TS, PE, TE, r, P1, P2, P3, alpha, beta);

	arc1 = Arc(PS, P2, TS);
	arc2 = Arc(P2, PE, P3 - P2);
}

// Calculate the the location of the point on the curve at t
void calculateSplinePoint(self, t):
        return (self.P0 * (1 - t) ** 3 +
                3 * self.P1 * t * (1 - t) ** 2 +
                3 * self.P2 * t ** 2 * (1-t) +
                self.P3 * t ** 3)

    # Calculate the the tangent on the curve at t
    def calculateTangent(self, t):
        return (self.Q0 * (1 - t) ** 2 +
                2 * self.Q1 * t * (1 - t) +
                self.Q2 * t ** 2)

    # Create biarcs for a piece of the curve. The first arc will start
    # on the curve at tS, the second one will end at tE. If the curve
    # distance from the center of the arcs to the midpoint of the half
    # of the curve segment the arc represents is larger than
    # max_divergence, split the curve and repeat until the arcs stay
    # within the limit. This function returns the location and tangent
    # at tE
    def createArcs(self, PS, TS, tS, tE):
        PE = self.calculateSplinePoint(tE)
        TE = self.calculateTangent(tE)

        tDelta = tE - tS

        a1, a2 = giarc(PS, TS, PE, TE)

        PM1 = self.calculateSplinePoint(tS + (tDelta * 0.25))
        PM2 = self.calculateSplinePoint(tS + (tDelta * 0.75))

        if ((abs(a1.radiusDifference(PM1) ) > self.max_divergence) or
            (abs(a2.radiusDifference(PM2) ) > self.max_divergence)):
            tM = tS + (tDelta * 0.5)

            PM, TM = self.createArcs(PS, TS, tS, tM)
            self.createArcs(PM, TM, tM, tE)
        else:
            self.arcs.append(a1)
            self.arcs.append(a2)

        return PE, TE

    # Output the G-code for the arcs describing this curve.
    def printGCode(self, output):
        if len(self.arcs) == 0:
            PS = self.P0
            TS = self.P1 - self.P0
            tS = 0

            for i in range(0, self.n):
                tE = (i + 1) * 1. / self.n

                PS, TS = self.createArcs(PS, TS, tS, tE)
                tS = tE

        for arc in self.arcs:
            arc.printGCode(output)

if __name__ == '__main__':
    import sys
    print "G0X0Y0Z0"
    print "F100"
    s = Spline(P(0,0), P(.25,1), P(1,1), P(2,0), 2)
    s.max_divergence = 2
    s.printGCode(sys.stdout)
    print "G0X0Y0Z0"
    s = Spline(P(0,0), P(.25,1), P(1,1), P(2,0), 4)
    s.max_divergence = 2
    s.printGCode(sys.stdout)
    print "G0X0Y0Z0"
    s = Spline(P(0,0), P(.25,1), P(1,1), P(2,0), 8)
    s.max_divergence = 2
    s.printGCode(sys.stdout)
    print "M2"


void bezier_curve_to_arcs(double maximum_deviation, const gp_Pnt& vt0, const gp_Pnt& vt2, const gp_Pnt& vt02, const gp_Pnt& vt20, void(*call_back)(const gp_Pnt& vt0, const gp_Pnt& vt1, const gp_Pnt& c, bool dir))
{
	gp_Pnt arc0_p0
}
#endif
