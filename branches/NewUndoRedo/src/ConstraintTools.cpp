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
#include "MultiPoly.h"

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
		SolveSketch((CSketch*)last->Owner());
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Perpendicular");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these lines to be perpendicular");}
};

class SetLineTangent:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		ConstrainedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			ConstrainedObject* obj = (ConstrainedObject*)*It;
			if(last)
			{
				if(last->GetType() == LineType)
					obj->SetTangentConstraint(last);
				else
					last->SetTangentConstraint(obj);
			}
			last=obj;
		}
		SolveSketch((CSketch*)last->Owner());
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
		SolveSketch((CSketch*)last->Owner());
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
		SolveSketch((CSketch*)last->Owner());
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
		SolveSketch((CSketch*)last->Owner());
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Colinear");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these lines to be colinear");}
};

class SetEqualRadius:public Tool{
	// set world coordinate system active again
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		ConstrainedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			ConstrainedObject* obj = (ConstrainedObject*)*It;
			if(last)
				if(obj->SetEqualRadiusConstraint(last))
					break;
			last=obj;
		}
		SolveSketch((CSketch*)last->Owner());
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Equal Radius");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these arcs to have equal radii");}
};

class SetConcentric:public Tool{
	// set world coordinate system active again
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		ConstrainedObject* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			ConstrainedObject* obj = (ConstrainedObject*)*It;
			if(last)
				if(obj->SetConcentricConstraint(last))
					break;
			last=obj;
		}
		SolveSketch((CSketch*)last->Owner());
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Concentric");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these arcs to be concentric");}
};

class SetPointsCoincident:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		HPoint* last=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			HPoint* obj = (HPoint*)*It;
			if(last)
			{
				obj->SetCoincidentPoint(last,true);
				break;
			}
			last=obj;
		}
		SolveSketch((CSketch*)last->Owner()->Owner());
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Coincident");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set these points to be coincident");}
};

class SetPointOnLine:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* line=NULL;
		HPoint* point=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = dynamic_cast<EndedObject*>(*It);
			if(obj)
				line=obj;
			HPoint* pobj = dynamic_cast<HPoint*>(*It);
			if(pobj)
				point=pobj;
		}
		line->SetPointOnLineConstraint(point);

		SolveSketch((CSketch*)line->Owner());
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Point On Line");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set this point on line");}
};

class SetPointOnMidpoint:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* line=NULL;
		HPoint* point=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = dynamic_cast<EndedObject*>(*It);
			if(obj)
				line=obj;
			HPoint* pobj = dynamic_cast<HPoint*>(*It);
			if(pobj)
				point=pobj;
		}
		line->SetPointOnLineMidpointConstraint(point);

		SolveSketch((CSketch*)line->Owner());
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Point On Midpoint");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set this point on the midpoint of line");}
};

class SetPointOnArcMidpoint:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* line=NULL;
		HPoint* point=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = dynamic_cast<EndedObject*>(*It);
			if(obj)
				line=obj;
			HPoint* pobj = dynamic_cast<HPoint*>(*It);
			if(pobj)
				point=pobj;
		}
		line->SetPointOnArcMidpointConstraint(point);

		SolveSketch((CSketch*)line->Owner());
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Point On Midpoint");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set this point on the midpoint of arc");}
};

class SetPointOnArc:public Tool{
public:
	void Run(){
		std::list<HeeksObj*>::const_iterator It;
		EndedObject* line=NULL;
		HPoint* point=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
			EndedObject* obj = dynamic_cast<EndedObject*>(*It);
			if(obj)
				line=obj;
			HPoint* pobj = dynamic_cast<HPoint*>(*It);
			if(pobj)
				point=pobj;
		}
		line->SetPointOnArcConstraint(point);

		SolveSketch((CSketch*)line->Owner());
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Set Point On Arc");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set this point on arc");}
};

void Test2DNearMap();

class RunTest:public Tool{
public:
	void Run(){
		std::list<CSketch*> list;
		list.push_back((CSketch*)(*wxGetApp().m_marked_list->list().begin())->Owner());
		MultiPoly(list);
	//	Test2DNearMap();
	//	TestMatrixFac();
	}
	const wxChar* GetTitle(){return _T("Run Test");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Run Test");}
};


static SetEqualRadius set_equal_radius;
static SetConcentric set_concentric;
static SetLinesParallel set_lines_parallel;
static SetLinesEqualLength set_lines_equal_length;
static SetLinesColinear set_lines_colinear;
static SetLinesPerpendicular set_lines_perpendicular;
static SetLineTangent set_line_tangent;
static SetPointOnLine set_point_on_line;
static SetPointOnMidpoint set_point_on_midpoint;
static SetPointsCoincident set_points_coincident;
static SetPointOnArc set_point_on_arc;
static SetPointOnArcMidpoint set_point_on_arc_midpoint;
static RunTest run_test;

void GetConstraintMenuTools(std::list<Tool*>* t_list){
	int line_count = 0;
	int arc_count = 0;
	int point_count = 0;
	int circle_count = 0;

	// check to see what types have been marked
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case PointType:
				point_count++;
				break;
			case LineType:
				line_count++;
				break;
			case ArcType:
				arc_count++;
				break;
			case CircleType:
				circle_count++;
				break;
			default:
				return;
		}
	}
#ifdef MULTIPOLYTEST
	t_list->push_back(&run_test);
#endif

	int total_count = line_count + arc_count + point_count + circle_count;

	if(total_count < 2)
		return;

	if(line_count == 2 && arc_count == 0 && point_count == 0 && circle_count == 0)
		t_list->push_back(&set_lines_perpendicular);

	if(arc_count == 0 && point_count == 0 && circle_count == 0)
	{
		t_list->push_back(&set_lines_parallel);
		t_list->push_back(&set_lines_equal_length);
		t_list->push_back(&set_lines_colinear);
	}

	if(line_count == 0 && arc_count == 0 && circle_count == 0)
		t_list->push_back(&set_points_coincident);

	if(line_count == 1 && point_count == 1 && arc_count == 0 && circle_count == 0)
	{
		t_list->push_back(&set_point_on_line);
		t_list->push_back(&set_point_on_midpoint);
	}

	if(line_count == 0 && point_count == 1 && arc_count == 1 && circle_count == 0)
	{
		t_list->push_back(&set_point_on_arc);
		t_list->push_back(&set_point_on_arc_midpoint);
	}

	if(line_count == 0 && point_count == 0 && (arc_count + circle_count == 2))
	{
		t_list->push_back(&set_concentric);
		t_list->push_back(&set_equal_radius);
	}

	if(line_count == 1 && (circle_count + arc_count == 1))
	{
		t_list->push_back(&set_line_tangent);
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
						eobj->A->SetCoincidentPoint(eobj2->A,false);
						shared_points = true;
					}
					if(eobj->A->m_p.Distance(eobj2->B->m_p) < wxGetApp().m_geom_tol)
					{
						//A to B coincidant
						eobj->A->SetCoincidentPoint(eobj2->B,false);
						shared_points = true;
					}
					if(eobj->B->m_p.Distance(eobj2->A->m_p) < wxGetApp().m_geom_tol)
					{
						//B to A coincidant
						eobj->B->SetCoincidentPoint(eobj2->A,false);
						shared_points = true;
					}
					if(eobj->B->m_p.Distance(eobj2->B->m_p) < wxGetApp().m_geom_tol)
					{
						//B's coincidant
						eobj->B->SetCoincidentPoint(eobj2->B,false);
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

