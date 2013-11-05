// PointDrawing.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "PointDrawing.h"
#include "HPoint.h"

PointDrawing point_drawing;

PointDrawing::PointDrawing(void)
{
}

PointDrawing::~PointDrawing(void)
{
}

bool PointDrawing::calculate_item(DigitizedPoint &end)
{
	if(end.m_type == DigitizeNoItemType)return false;

	if(TempObject() && TempObject()->GetType() != PointType){
		ClearObjectsMade();
	}

	if(TempObject() == NULL){
		AddToTempObjects(new HPoint(end.m_point, &wxGetApp().current_color));
	}
	else{
		((HPoint*)TempObject())->m_p = end.m_point;
	}

	return true;
}
