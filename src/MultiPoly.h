// MultiPoly.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.


#pragma once

#include "BentleyOttmann.h"
#include "NearMap.h"
#include "WrappedCurves.h"

std::vector<TopoDS_Face> MultiPoly(std::list<CSketch*> sketches);

std::vector<CompoundSegment*> find_level(bool odd, 
				std::vector<std::pair<CompoundSegment*,std::vector<CompoundSegment*> > > &pRet,
				std::vector<CompoundSegment*>& closed_shapes, 
				std::vector<std::vector<CompoundSegment*> >& inside_of, 
				std::vector<CompoundSegment*> parents);

std::vector<TopoDS_Face> TopoDSFaceAdaptor(
	std::vector<std::pair<CompoundSegment*,std::vector<CompoundSegment*> > > &data);

void ConcatSegments(double x_coord, double y_coord, CompoundSegment* seg1, CompoundSegment* seg2, TwoDNearMap &bcurves);
void AnalyzeNearMap(TwoDNearMap &bcurves);

