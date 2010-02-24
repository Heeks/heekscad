// SketchTools.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "MarkedList.h"
#include "Sketch.h"
#include "Group.h"
#include "Pad.h"
#include "Part.h"
#include "Pocket.h"

class PadSketch:public Tool{
public:
	void Run(){
		double height = 10;
		wxGetApp().InputDouble(_("Input extrusion height"), _("height"), height);
		CSketch* sketch = (CSketch*)(*wxGetApp().m_marked_list->list().begin());

		CPad::PadSketch(sketch,height);
	}
	const wxChar* GetTitle(){return _T("Pad Sketch");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Pad Sketch");}
};

class PocketSketch:public Tool{
public:
	void Run(){
		double height = 10;
		wxGetApp().InputDouble(_("Input pad depth"), _("height"), height);
		CSketch* sketch = (CSketch*)(*wxGetApp().m_marked_list->list().begin());

		CPocket::PocketSketch(sketch,height);
	}
	const wxChar* GetTitle(){return _T("Pocket Sketch");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Pocket Sketch");}
};


class MakeToPart:public Tool{
public:
	void Run(){
		CSketch* sketch = (CSketch*)(*wxGetApp().m_marked_list->list().begin());
		CPart* part = new CPart();
		wxGetApp().Add(part,NULL);
		sketch->Owner()->Remove(sketch);
		sketch->RemoveOwners();
		part->Add(sketch,NULL);
	}
	const wxChar* GetTitle(){return _T("Make To Part");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Make To Part");}
};

class AddToPart:public Tool{
public:
	void Run()
	{
		std::list<HeeksObj*>::const_iterator It;
		CSketch* sketch=NULL;
		CPad* pad=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
		{
			HeeksObj* obj = (HeeksObj*)*It;
			if(obj->GetType() == SketchType)
				sketch = (CSketch*)obj;
			else
				pad = (CPad*)obj;
		}
		sketch->Owner()->Remove(sketch);
		sketch->RemoveOwners();
		pad->Add(sketch,NULL);
	}
	const wxChar* GetTitle(){return _T("Add To Part");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Add To Part");}
};

static PadSketch pad_sketch;
static MakeToPart make_to_part;
static AddToPart add_to_part;
static PocketSketch pocket_sketch;


void GetSketchMenuTools(std::list<Tool*>* t_list){
	int count=0;
	bool gotsketch=false;
	bool gotpart=false;
	// check to see what types have been marked
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SketchType)
			gotsketch=true;
		if(object->GetType() == PartType)
			gotpart=true;
		count++;
	}

	if(count == 2 && gotsketch && gotpart)
		t_list->push_back(&add_to_part);

	if(count!=1 || !gotsketch)
		return;

	t_list->push_back(&pad_sketch);
	t_list->push_back(&pocket_sketch);
	t_list->push_back(&make_to_part);
}

