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
#include "HeeksConfig.h"

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
	HeeksConfig config;
	int ncopies;
	config.Read(_T("TranslateNumCopies"), &ncopies, 1);
	if(copy)
	{
		// check for uncopyable objects
		RemoveUncopyable();
		if(wxGetApp().m_marked_list->size() == 0)return;

		// input "number of copies"
		if(!wxGetApp().InputInt(_("Enter number of copies"), _("number of copies"), ncopies))return;
		if(ncopies < 1)return;
		config.Write(_T("TranslateNumCopies"), ncopies);
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

	bool move_to_accepted = wxGetApp().PickPosition(_("Click position to move to"), to, on_move_translate);

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

	if(!move_to_accepted)return;

	wxGetApp().CreateUndoPoint();

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
#ifdef MULTIPLE_OWNERS
				object->HEEKSOBJ_OWNER->Add(new_object, NULL);
#else
				object->m_owner->Add(new_object, NULL);
#endif
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

	wxGetApp().Changed();
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
	HeeksConfig config;
	int ncopies;
	config.Read(_T("RotateNumCopies"), &ncopies, 1);
	if(copy)
	{
		// check for uncopyable objects
		RemoveUncopyable();
		if(wxGetApp().m_marked_list->size() == 0)return;
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear(true);

	double angle;
	config.Read(_T("RotateAngle"), &angle, 90.0);

	// enter angle, plane and position
	double axis[3];
	double pos[3];
	config.Read(_T("RotateAxisX"), &axis[0], 0.0);
	config.Read(_T("RotateAxisY"), &axis[1], 0.0);
	config.Read(_T("RotateAxisZ"), &axis[2], 1.0);
	config.Read(_T("RotatePosX"), &pos[0], 0.0);
	config.Read(_T("RotatePosY"), &pos[1], 0.0);
	config.Read(_T("RotatePosZ"), &pos[2], 0.0);

	double axial_shift;
	config.Read(_T("RotateAxialShift"), &axial_shift, 0.0);

	if(!wxGetApp().InputAngleWithPlane(angle, axis, pos, copy ? &ncopies : NULL, &axial_shift))return;
	if(copy)
	{
		if(ncopies < 1)return;
		config.Write(_T("RotateNumCopies"), ncopies);
	}
	config.Write(_T("RotateAngle"), angle);
	config.Write(_T("RotateAxialShift"), axial_shift);
	config.Write(_T("RotateAxisX"), axis[0]);
	config.Write(_T("RotateAxisY"), axis[1]);
	config.Write(_T("RotateAxisZ"), axis[2]);
	config.Write(_T("RotatePosX"), pos[0]);
	config.Write(_T("RotatePosY"), pos[1]);
	config.Write(_T("RotatePosZ"), pos[2]);
	axis_Dir = gp_Dir(axis[0], axis[1], axis[2]);
	line_Pos = gp_Pnt(pos[0], pos[1], pos[2]);

	// transform the objects
	wxGetApp().CreateUndoPoint();
	if(copy)
	{
		for(int i = 0; i<ncopies; i++)
		{
			gp_Trsf mat;
			mat.SetRotation(gp_Ax1(line_Pos, axis_Dir), angle * M_PI/180 * (i+1));
			gp_Trsf tmat;
			tmat.SetTranslation(gp_Vec(axis_Dir.XYZ() * (axial_shift * ((double)(i+1)) / ncopies)));
			mat = tmat * mat;
			double m[16];
			extract(mat, m);
			for(std::list<HeeksObj*>::iterator It = selected_items.begin(); It != selected_items.end(); It++)
			{
				HeeksObj* object = *It;
				HeeksObj* new_object = object->MakeACopy();
				new_object->ModifyByMatrix(m);              // Rotate the duplicate object.
#ifdef MULTIPLE_OWNERS
				object->HEEKSOBJ_OWNER->Add(new_object, NULL);     // And add it to this object's owner
#else
				object->m_owner->Add(new_object, NULL);     // And add it to this object's owner
#endif
			}
		}
		wxGetApp().m_marked_list->Clear(true);
	}
	else
	{
		gp_Trsf mat;
		mat.SetRotation(gp_Ax1(line_Pos, axis_Dir), angle * M_PI/180);
		gp_Trsf tmat;
		tmat.SetTranslation(gp_Vec(axis_Dir.XYZ() * axial_shift));
		mat = tmat * mat;
		double m[16];
		extract(mat, m);
		wxGetApp().Transform(selected_items, m);
	}
	wxGetApp().Changed();
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
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear(true);

	// pick a line to mirror about
	bool line_found = false;
	gp_Lin line;
	int save_filter = wxGetApp().m_marked_list->m_filter;
	wxGetApp().PickObjects(_("Pick line to mirror about"), MARKING_FILTER_LINE | MARKING_FILTER_ILINE, true);
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
	wxGetApp().CreateUndoPoint();
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
#ifdef MULTIPLE_OWNERS
			object->HEEKSOBJ_OWNER->Add(new_object, NULL);
#else
			object->m_owner->Add(new_object, NULL);
#endif
			new_object->ModifyByMatrix(m);
		}
		wxGetApp().m_marked_list->Clear(true);
	}
	else
	{
		wxGetApp().Transform(selected_items, m);
	}
	wxGetApp().Changed();
}

void TransformTools::Scale(bool copy)
{
	// pick items
	if(wxGetApp().m_marked_list->size() == 0){
		wxGetApp().PickObjects(_("Pick objects to scale"));
	}
	if(wxGetApp().m_marked_list->size() == 0)return;

	// get number of copies
	int ncopies;
	HeeksConfig config;
	config.Read(_T("ScaleNumCopies"), &ncopies, 1);
	if(copy)
	{
		// check for uncopyable objects
		RemoveUncopyable();
		if(wxGetApp().m_marked_list->size() == 0)return;

		// input "number of copies"
		if(!wxGetApp().InputInt(_("Enter number of copies"), _("number of copies"), ncopies))return;
		if(ncopies < 1)return;
		config.Write(_T("ScaleNumCopies"), ncopies);
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear(true);

	// pick "centre" position
	if(!wxGetApp().PickPosition(_("Click centre position to scale about"), centre))return;

	// enter scale factor
	double scale;
	config.Read(_T("ScaleFactor"), &scale, 2.0);
	if(!wxGetApp().InputDouble(_("Enter scale factor"), _("scale factor"), scale))return;
	config.Write(_T("ScaleFactor"), scale);

	// transform the objects
	wxGetApp().CreateUndoPoint();
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
#ifdef MULTIPLE_OWNERS
				object->HEEKSOBJ_OWNER->Add(new_object, NULL);
#else
				object->m_owner->Add(new_object, NULL);
#endif
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
	wxGetApp().Changed();
}
