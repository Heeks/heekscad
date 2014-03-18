// BezierCurve.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "BezierCurve.h"

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

