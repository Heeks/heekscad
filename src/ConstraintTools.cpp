// ConstraintTools.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "ConstraintTools.h"
#include "MarkedList.h"
#include "HLine.h"
#include "HArc.h"
#include "Sketch.h"
#include "SolveSketch.h"

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
		SolveSketch((CSketch*)last->m_owner);
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
		SolveSketch((CSketch*)last->m_owner);
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

void ApplyCoincidentConstraints(HeeksObj* extobj, std::list<HeeksObj*> list)
{
	list.push_back(extobj);

	std::list<HeeksObj*>::iterator it;
	std::list<HeeksObj*>::iterator it2;

	//Search for A's matching
	for(it = list.begin(); it!= list.end(); ++it)
	{
		EndedObject* eobj = (EndedObject*)*it;
		if(eobj)
		{
			for(it2 = it; it2!= list.end(); ++it2)
			{
				EndedObject* eobj2 = (EndedObject*)*it2;
				if(eobj2 && eobj != eobj2)
				{
					bool shared_points = false;
					//Check if these two objects share any points
					if(eobj->A.Distance(eobj2->A) < wxGetApp().m_geom_tol)
					{
						//A's coincidant
						eobj->SetCoincidentPoint(eobj2,PointA,PointA);
						shared_points = true;
					}
					if(eobj->A.Distance(eobj2->B) < wxGetApp().m_geom_tol)
					{
						//A to B coincidant
						eobj->SetCoincidentPoint(eobj2,PointA,PointB);
						shared_points = true;
					}
					if(eobj->B.Distance(eobj2->A) < wxGetApp().m_geom_tol)
					{
						//B to A coincidant
						eobj->SetCoincidentPoint(eobj2,PointB,PointA);
						shared_points = true;
					}
					if(eobj->B.Distance(eobj2->B) < wxGetApp().m_geom_tol)
					{
						//B's coincidant
						eobj->SetCoincidentPoint(eobj2,PointB,PointB);
						shared_points = true;
					}

					if(shared_points)
					{
						if(eobj->GetType() == LineType && eobj2->GetType() == ArcType)
						{
							eobj2->SetTangentConstraint(eobj);
						}
						if(eobj2->GetType() == LineType && eobj->GetType() == ArcType)
						{
					//		eobj->SetTangentConstraint(eobj2);
						}
					}
				}
			}
		}
	}
}