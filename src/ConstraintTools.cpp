// ConstraintTools.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "ConstraintTools.h"
#include "MarkedList.h"
#include "HLine.h"
#include "HArc.h"

class SetLinesPerpendicular:public Tool{
	// set world coordinate system active again
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = (EndedObject*)*It;
			if(last)
				obj->SetPerpendicularConstraint(last);
			last=obj;
		}
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Perpendicular");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these lines to be perpendicular");}
};

class SetLinesParallel:public Tool{
	// set world coordinate system active again
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = (EndedObject*)*It;
			if(last)
				obj->SetParallelConstraint(last);
			last=obj;
		}
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Parallel");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these lines to be parallel");}
};

static SetLinesParallel set_lines_parallel;
static SetLinesPerpendicular set_lines_perpendicular;

void GetConstraintMenuTools(std::list<Tool*>* t_list){
	int line_count = 0;

	// check to see what types have been marked
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case LineType:
				line_count++;
				break;
			default:
				return;
		}
	}

	if(line_count < 2)
		return;

	if(line_count == 2)
		t_list->push_back(&set_lines_perpendicular);

	t_list->push_back(&set_lines_parallel);

}

