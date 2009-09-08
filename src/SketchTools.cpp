// SketchTools.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "MarkedList.h"
#include "Sketch.h"
#include "Group.h"
#include "Pad.h"

class PadSketch:public Tool{
public:
	void Run(){
		double height = 10;
		wxGetApp().InputDouble(_("Input extrusion height"), _("height"), height);
		CSketch* sketch = (CSketch*)(*wxGetApp().m_marked_list->list().begin());

		CPad::PadSketch(sketch,height,true);
	}
	const wxChar* GetTitle(){return _T("Pad Sketch");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Pad Sketch");}
};

static PadSketch pad_sketch;


void GetSketchMenuTools(std::list<Tool*>* t_list){
	int count=0;
	bool gotsketch=false;
	// check to see what types have been marked
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		if(object->GetType()== SketchType)
			gotsketch=true;
		count++;
	}

	if(count!=1 || !gotsketch)
		return;

	t_list->push_back(&pad_sketch);
}

