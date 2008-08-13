// BezierCurve.h

#pragma once

void split_bezier_curve(int level, const gp_Pnt& vt0, const gp_Pnt& vt2, const gp_Pnt& vt02, const gp_Pnt& vt20, void(*call_back)(const gp_Pnt& vt0, const gp_Pnt& vt1));