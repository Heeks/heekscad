// BezierCurve.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

void split_bezier_curve(int level, const gp_Pnt& vt0, const gp_Pnt& vt2, const gp_Pnt& vt02, const gp_Pnt& vt20, void(*call_back)(const gp_Pnt& vt0, const gp_Pnt& vt1));

