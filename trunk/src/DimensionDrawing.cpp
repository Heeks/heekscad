// DimensionDrawing.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "DimensionDrawing.h"
#include "Sketch.h"
#include "HLine.h"
#include "HArc.h"
#include "../interface/PropertyChoice.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyInt.h"
#include "HeeksFrame.h"
#include "InputModeCanvas.h"

DimensionDrawing dimension_drawing;

DimensionDrawing::DimensionDrawing(void)
{
	m_mode = TwoPointsDimensionMode;
}

DimensionDrawing::~DimensionDrawing(void)
{
}

bool DimensionDrawing::calculate_item(DigitizedPoint &end)
{
	if(end.m_type == DigitizeNoItemType)return false;

	if(TempObject() && TempObject()->GetType() != DimensionType){
		ClearObjectsMade();
	}

	gp_Trsf mat = wxGetApp().GetDrawMatrix(true);

	// make sure dimension exists
	if(TempObject()==NULL){
		AddToTempObjects(new HDimension(mat, gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 0), m_mode, DimensionUnitsGlobal, &(wxGetApp().current_color)));
	}

	gp_Pnt p0, p1, p2;
	if(GetDrawStep() == 1)
	{
		p0 = GetStartPos().m_point;
		p1 = end.m_point;
		p2 = end.m_point;
	}
	else if(GetDrawStep() == 2)
	{
		p0 = GetBeforeStartPos().m_point;
		p1 = GetStartPos().m_point;
		p2 = end.m_point;
	}
	else
	{
		return false;
	}

	// double distance = p0.Distance(p1);

	((HDimension*)TempObject())->m_trsf = mat;
	((HDimension*)TempObject())->A = p0;
	((HDimension*)TempObject())->B = p1;
	((HDimension*)TempObject())->m_p2 = p2;
	((HDimension*)TempObject())->m_mode = m_mode;

	return true;
}

static DimensionDrawing* DimensionDrawing_for_GetProperties = NULL;

static void on_set_mode(int value, HeeksObj* object)
{
	DimensionDrawing_for_GetProperties->m_mode = (DimensionMode)value;
	wxGetApp().Repaint();
}

void DimensionDrawing::GetProperties(std::list<Property *> *list){
	// add drawing mode
	std::list< wxString > choices;
	choices.push_back ( wxString ( _("between two points") ) );
	choices.push_back ( wxString ( _("between two points, XY only") ) );
	choices.push_back ( wxString ( _("between two points, X only") ) );
	choices.push_back ( wxString ( _("between two points, Y only") ) );
	choices.push_back ( wxString ( _("between two points, Z only") ) );
	choices.push_back ( wxString ( _("orthogonal") ) );
	DimensionDrawing_for_GetProperties = this;
	list->push_back ( new PropertyChoice ( _("mode"),  choices, m_mode, NULL, on_set_mode ) );

	Drawing::GetProperties(list);
}

void DimensionDrawing::GetTools(std::list<Tool*> *f_list, const wxPoint *p){
	Drawing::GetTools(f_list, p);
}
