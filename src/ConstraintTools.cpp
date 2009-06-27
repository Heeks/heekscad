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

class SetArcTangent:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = (EndedObject*)*It;
			if(last)
			{
				if(last->GetType() == ArcType)
					last->SetTangentConstraint(obj);
				else
					obj->SetTangentConstraint(last);
			}
			last=obj;
		}
		SolveSketch((CSketch*)last->m_owner);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Toggle Tangent");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set line tangent to arc");}
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
				if(obj->SetParallelConstraint(last))
					break;
			last=obj;
		}
		SolveSketch((CSketch*)last->m_owner);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Parallel");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these lines to be parallel");}
};

class SetLinesEqualLength:public Tool{
	// set world coordinate system active again
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = (EndedObject*)*It;
			if(last)
				if(obj->SetEqualLengthConstraint(last))
					break;
			last=obj;
		}
		SolveSketch((CSketch*)last->m_owner);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Equal Length");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these lines to be equal length");}
};

class SetLinesColinear:public Tool{
	// set world coordinate system active again
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = (EndedObject*)*It;
			if(last)
				if(obj->SetColinearConstraint(last))
					break;
			last=obj;
		}
		SolveSketch((CSketch*)last->m_owner);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Colinear");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these lines to be colinear");}
};

class SetArcsEqualRadius:public Tool{
	// set world coordinate system active again
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = (EndedObject*)*It;
			if(last)
				if(obj->SetEqualRadiusConstraint(last))
					break;
			last=obj;
		}
		SolveSketch((CSketch*)last->m_owner);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Equal Radius");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these arcs to have equal radii");}
};

class SetArcsConcentric:public Tool{
	// set world coordinate system active again
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = (EndedObject*)*It;
			if(last)
				if(obj->SetConcentricConstraint(last))
					break;
			last=obj;
		}
		SolveSketch((CSketch*)last->m_owner);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Concentric");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these arcs to be concentric");}
};

static SetArcsEqualRadius set_arcs_equal_radius;
static SetArcsConcentric set_arcs_concentric;
static SetLinesParallel set_lines_parallel;
static SetLinesEqualLength set_lines_equal_length;
static SetLinesColinear set_lines_colinear;
static SetLinesPerpendicular set_lines_perpendicular;
static SetArcTangent set_arc_tangent;

void GetConstraintMenuTools(std::list<Tool*>* t_list){
	int line_count = 0;
	int arc_count = 0;

	// check to see what types have been marked
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case LineType:
				line_count++;
				break;
			case ArcType:
				arc_count++;
				break;
			default:
				return;
		}
	}

	int total_count = line_count + arc_count;

	if(total_count < 2)
		return;

	if(line_count == 2 && arc_count == 0)
		t_list->push_back(&set_lines_perpendicular);

	if(arc_count == 0)
	{
		t_list->push_back(&set_lines_parallel);
		t_list->push_back(&set_lines_equal_length);
		t_list->push_back(&set_lines_colinear);
	}

	if(arc_count == 1 && line_count == 1)
		t_list->push_back(&set_arc_tangent);

	if(line_count == 0)
	{
		t_list->push_back(&set_arcs_equal_radius);
		t_list->push_back(&set_arcs_concentric);
	}

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
					if(eobj->A->m_p.Distance(eobj2->A->m_p) < wxGetApp().m_geom_tol)
					{
						//A's coincidant
						eobj->SetCoincidentPoint(eobj2,PointA,PointA);
						shared_points = true;
					}
					if(eobj->A->m_p.Distance(eobj2->B->m_p) < wxGetApp().m_geom_tol)
					{
						//A to B coincidant
						eobj->SetCoincidentPoint(eobj2,PointA,PointB);
						shared_points = true;
					}
					if(eobj->B->m_p.Distance(eobj2->A->m_p) < wxGetApp().m_geom_tol)
					{
						//B to A coincidant
						eobj->SetCoincidentPoint(eobj2,PointB,PointA);
						shared_points = true;
					}
					if(eobj->B->m_p.Distance(eobj2->B->m_p) < wxGetApp().m_geom_tol)
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