// TransformTools.cpp
/*
 * Copyright (c) 2009, Dan Heeks
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */

#include "stdafx.h"
#include "TransformTools.h"
#include "MarkedList.h"
#include "HLine.h"
#include "HILine.h"

static double from[3];
static double centre[3];

static void on_move_translate(const double* to)
{
	wxGetApp().m_drag_matrix.SetTranslationPart(gp_Vec(make_point(from), make_point(to)));
	wxGetApp().Repaint(true);
}

//static
void TransformTools::RemoveUncopyable()
{
	std::list<HeeksObj*> uncopyable_objects;
	for(std::list<HeeksObj*>::const_iterator It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
	{
		HeeksObj* object = *It;
		if(!object->CanBeCopied())uncopyable_objects.push_back(object);
	}
	if(uncopyable_objects.size() > 0)wxGetApp().m_marked_list->Remove(uncopyable_objects, true);
}

//static
void TransformTools::Translate(bool copy)
{
	// pick items
	if(wxGetApp().m_marked_list->size() == 0){
		wxGetApp().PickObjects(_("Pick objects to move"));
	}
	if(wxGetApp().m_marked_list->size() == 0)return;

	// get number of copies
	int ncopies = 1;
	if(copy)
	{
		// check for uncopyable objects
		RemoveUncopyable();
		if(wxGetApp().m_marked_list->size() == 0)return;

		// input "number of copies"
		double number_of_copies = 1.0;
		wxGetApp().InputDouble(_("Enter number of copies"), _("number of copies"), number_of_copies);
		ncopies = (int)(number_of_copies + 0.5);
		if(ncopies < 1)return;
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear(true);

	// pick "from" position
	if(!wxGetApp().PickPosition(_("Click position to move from"), from))return;

	// pick "to" position
	wxGetApp().CreateTransformGLList(selected_items, false);
	wxGetApp().m_drag_matrix = gp_Trsf();
	if(!copy)
	{
		for(std::list<HeeksObj*>::const_iterator It = selected_items.begin(); It != selected_items.end(); It++){
			HeeksObj* object = *It;
			if(object->m_visible)wxGetApp().m_hidden_for_drag.push_back(object);
			object->m_visible = false;
		}
	}
	double to[3];
	wxGetApp().PickPosition(_("Click position to move to"), to, on_move_translate);
	if(!copy)
	{
		for(std::list<HeeksObj*>::iterator It = wxGetApp().m_hidden_for_drag.begin(); It != wxGetApp().m_hidden_for_drag.end(); It++)
		{
			HeeksObj* object = *It;
			object->m_visible = true;
		}
		wxGetApp().m_hidden_for_drag.clear();
	}
	wxGetApp().DestroyTransformGLList();

	// transform the objects
	if(copy)
	{
		for(int i = 0; i<ncopies; i++)
		{
			gp_Trsf mat;
			mat.SetTranslationPart(make_vector(make_point(from), make_point(to)) * (i + 1));
			double m[16];
			extract(mat, m);
			for(std::list<HeeksObj*>::iterator It = selected_items.begin(); It != selected_items.end(); It++)
			{
				HeeksObj* object = *It;
				HeeksObj* new_object = object->MakeACopy();
				object->Owner()->Add(new_object, NULL);
				new_object->ModifyByMatrix(m);
			}
		}
		wxGetApp().m_marked_list->Clear(true);
	}
	else
	{
		gp_Trsf mat;
		mat.SetTranslationPart(make_vector(make_point(from), make_point(to)));
		double m[16];
		extract(mat, m);
		wxGetApp().Transform(selected_items, m);
	}
}

//static
void TransformTools::Rotate(bool copy)
{
	//rotation axis - Z axis by default
	gp_Dir axis_Dir = gp_Dir(0,0,1);
	gp_Pnt line_Pos = gp_Pnt(0,0,0);

	// pick items
	if(wxGetApp().m_marked_list->size() == 0){
		wxGetApp().PickObjects(_("Pick objects to rotate"));
	}
	if(wxGetApp().m_marked_list->size() == 0)return;

	// get number of copies
	int ncopies = 1;
	if(copy)
	{
		// check for uncopyable objects
		RemoveUncopyable();
		if(wxGetApp().m_marked_list->size() == 0)return;

		// input "number of copies"
		double number_of_copies = 1.0;
		wxGetApp().InputDouble(_("Enter number of copies"), _("number of copies"), number_of_copies);
		ncopies = (int)(number_of_copies + 0.5);
		if(ncopies < 1)return;
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear(true);



	if(wxGetApp().allow3DRotaion)
	{
		// pick a line to use as rotation axis
		bool line_found = false;
		gp_Lin line;
		int save_filter = wxGetApp().m_marked_list->m_filter;
		wxGetApp().m_marked_list->m_filter = MARKING_FILTER_LINE | MARKING_FILTER_ILINE;
		wxGetApp().PickObjects(_("Pick line for rotation axis"), true);
		wxGetApp().m_marked_list->m_filter = save_filter;
		for(std::list<HeeksObj *>::const_iterator It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
		{
			HeeksObj* object = *It;
			if(object->GetType() == LineType)
			{
				line = ((HLine*)object)->GetLine();
				line_found = true;
			}
			else if(object->GetType() == ILineType)
			{
				line = ((HILine*)object)->GetLine();
				line_found = true;
			}
		}
		if(!line_found)return;
		axis_Dir=line.Direction();
		line_Pos= line.Location();
	}
	else
	{
		// pick "centre" position
		if(!wxGetApp().PickPosition(_("Click centre position to rotate about"), centre))return;
		line_Pos.SetXYZ(gp_XYZ(centre[0],centre[1],centre[2]));
	}

	// enter angle
	double angle = 45.0;
	if(!wxGetApp().InputDouble(_("Enter angle to rotate by"), _("angle"), angle))return;

	// transform the objects
	if(copy)
	{
		for(int i = 0; i<ncopies; i++)
		{
			gp_Trsf mat;
			mat.SetRotation(gp_Ax1(line_Pos, axis_Dir), angle * Pi/180 * (i+1));
			double m[16];
			extract(mat, m);
			for(std::list<HeeksObj*>::iterator It = selected_items.begin(); It != selected_items.end(); It++)
			{
				HeeksObj* object = *It;
				HeeksObj* new_object = object->MakeACopy();
				object->Owner()->Add(new_object, NULL);
				object->ModifyByMatrix(m);
			}
		}
		wxGetApp().m_marked_list->Clear(true);
	}
	else
	{
		gp_Trsf mat;
		mat.SetRotation(gp_Ax1(line_Pos, axis_Dir), angle * Pi/180);
		double m[16];
		extract(mat, m);
		wxGetApp().Transform(selected_items, m);
	}
}

//static
void TransformTools::Mirror(bool copy)
{
	// pick items
	if(wxGetApp().m_marked_list->size() == 0){
		wxGetApp().PickObjects(_("Pick objects to mirror"));
	}
	if(wxGetApp().m_marked_list->size() == 0)return;

	if(copy)
	{
		// check for uncopyable objects
		RemoveUncopyable();
		if(wxGetApp().m_marked_list->size() == 0)return;

		// input "number of copies"
		double number_of_copies = 1.0;
		wxGetApp().InputDouble(_("Enter number of copies"), _("number of copies"), number_of_copies);
		int ncopies = (int)(number_of_copies + 0.5);
		if(ncopies < 1)return;
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear(true);

	// pick a line to mirror about
	bool line_found = false;
	gp_Lin line;
	int save_filter = wxGetApp().m_marked_list->m_filter;
	wxGetApp().m_marked_list->m_filter = MARKING_FILTER_LINE | MARKING_FILTER_ILINE;
	wxGetApp().PickObjects(_("Pick line to mirror about"), true);
	wxGetApp().m_marked_list->m_filter = save_filter;
	for(std::list<HeeksObj *>::const_iterator It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
	{
		HeeksObj* object = *It;
		if(object->GetType() == LineType)
		{
			line = ((HLine*)object)->GetLine();
			line_found = true;
		}
		else if(object->GetType() == ILineType)
		{
			line = ((HILine*)object)->GetLine();
			line_found = true;
		}
	}
	if(!line_found)return;

	// transform the objects
	gp_Trsf mat;
	mat.SetMirror(gp_Ax1(line.Location(), line.Direction()));
	double m[16];
	extract(mat, m);

	if(copy)
	{
		for(std::list<HeeksObj*>::iterator It = selected_items.begin(); It != selected_items.end(); It++)
		{
			HeeksObj* object = *It;
			HeeksObj* new_object = object->MakeACopy();
			object->Owner()->Add(new_object, NULL);
			new_object->ModifyByMatrix(m);
		}
		wxGetApp().m_marked_list->Clear(true);
	}
	else
	{
		wxGetApp().Transform(selected_items, m);
	}
}

void TransformTools::Scale(bool copy)
{
	// pick items
	if(wxGetApp().m_marked_list->size() == 0){
		wxGetApp().PickObjects(_("Pick objects to scale"));
	}
	if(wxGetApp().m_marked_list->size() == 0)return;

	// get number of copies
	int ncopies = 1;
	if(copy)
	{
		// check for uncopyable objects
		RemoveUncopyable();
		if(wxGetApp().m_marked_list->size() == 0)return;

		// input "number of copies"
		double number_of_copies = 1.0;
		wxGetApp().InputDouble(_("Enter number of copies"), _("number of copies"), number_of_copies);
		ncopies = (int)(number_of_copies + 0.5);
		if(ncopies < 1)return;
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear(true);

	// pick "centre" position
	if(!wxGetApp().PickPosition(_("Click centre position to scale about"), centre))return;

	// enter scale factor
	double scale = 2.0;
	if(!wxGetApp().InputDouble(_("Enter scale factor"), _("scale factor"), scale))return;

	// transform the objects
	if(copy)
	{
		for(int i = 0; i<ncopies; i++)
		{
			gp_Trsf mat;
			mat.SetScale(make_point(centre), scale * (i+1));
			double m[16];
			extract(mat, m);
			for(std::list<HeeksObj*>::iterator It = selected_items.begin(); It != selected_items.end(); It++)
			{
				HeeksObj* object = *It;
				HeeksObj* new_object = object->MakeACopy();
				object->Owner()->Add(new_object, NULL);
				new_object->ModifyByMatrix(m);
			}
		}
		wxGetApp().m_marked_list->Clear(true);
	}
	else
	{
		gp_Trsf mat;
		mat.SetScale(make_point(centre), scale);
		double m[16];
		extract(mat, m);
		wxGetApp().Transform(selected_items, m);
	}
}
