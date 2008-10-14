// TransformTools.cpp

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
void TransformTools::Translate(bool copy)
{
	// pick items
	if(wxGetApp().m_marked_list->size() == 0){
		wxGetApp().PickObjects(_T("Pick objects to move"));
	}
	if(wxGetApp().m_marked_list->size() == 0)return;

	// get number of copies
	int ncopies = 1;
	if(copy)
	{
		// input "number of copies"
		double number_of_copies = 1.0;
		wxGetApp().InputDouble(_T("Enter number of copies"), _T("number of copies"), number_of_copies);
		ncopies = (int)(number_of_copies + 0.5);
		if(ncopies < 1)return;
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear();

	// pick "from" position
	if(!wxGetApp().PickPosition(_T("Click position to move from"), from))return;

	// pick "to" position
	wxGetApp().m_marked_list->Add(selected_items);
	wxGetApp().CreateTransformGLList(false);
	wxGetApp().m_drag_matrix = gp_Trsf();
	if(!copy)wxGetApp().HideMarkedList();
	double to[3];
	bool success = wxGetApp().PickPosition(_T("Click position to move to"), to, on_move_translate);
	if(!copy)wxGetApp().UnHideMarkedList();
	wxGetApp().DestroyTransformGLList();
	wxGetApp().m_marked_list->Clear();

	// transform the objects
	if(copy)
	{
		wxGetApp().StartHistory(_T("Copy Translate"));
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
				wxGetApp().AddUndoably(new_object, object->m_owner, NULL);
				new_object->ModifyByMatrix(m);
			}
		}
		wxGetApp().EndHistory();
		wxGetApp().m_marked_list->Clear();
	}
	else
	{
		gp_Trsf mat;
		mat.SetTranslationPart(make_vector(make_point(from), make_point(to)));
		double m[16];
		extract(mat, m);
		wxGetApp().TransformUndoably(selected_items, m);
	}
}

//static
void TransformTools::Rotate(bool copy)
{
	// pick items
	if(wxGetApp().m_marked_list->size() == 0){
		wxGetApp().PickObjects(_T("Pick objects to rotate"));
	}
	if(wxGetApp().m_marked_list->size() == 0)return;

	// get number of copies
	int ncopies = 1;
	if(copy)
	{
		// input "number of copies"
		double number_of_copies = 1.0;
		wxGetApp().InputDouble(_T("Enter number of copies"), _T("number of copies"), number_of_copies);
		ncopies = (int)(number_of_copies + 0.5);
		if(ncopies < 1)return;
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear();

	// pick "centre" position
	if(!wxGetApp().PickPosition(_T("Click centre position to rotate about"), centre))return;

	// enter angle
	double angle = 45.0;
	if(!wxGetApp().InputDouble(_T("Enter angle to rotate by"), _T("angle"), angle))return;

	// transform the objects
	if(copy)
	{
		wxGetApp().StartHistory(_T("Copy rotate"));
		for(int i = 0; i<ncopies; i++)
		{
			gp_Trsf mat;
			mat.SetRotation(gp_Ax1(make_point(centre), gp_Dir(0, 0, 1)), angle * Pi/180 * (i+1));
			double m[16];
			extract(mat, m);
			for(std::list<HeeksObj*>::iterator It = selected_items.begin(); It != selected_items.end(); It++)
			{
				HeeksObj* object = *It;
				HeeksObj* new_object = object->MakeACopy();
				wxGetApp().AddUndoably(new_object, object->m_owner, NULL);
				new_object->ModifyByMatrix(m);
			}
		}
		wxGetApp().EndHistory();
		wxGetApp().m_marked_list->Clear();	}
	else
	{
		gp_Trsf mat;
		mat.SetRotation(gp_Ax1(make_point(centre), gp_Dir(0, 0, 1)), angle * Pi/180);
		double m[16];
		extract(mat, m);
		wxGetApp().TransformUndoably(selected_items, m);
	}
}

//static
void TransformTools::Mirror(bool copy)
{
	// pick items
	if(wxGetApp().m_marked_list->size() == 0){
		wxGetApp().PickObjects(_T("Pick objects to mirror"));
	}
	if(wxGetApp().m_marked_list->size() == 0)return;

	if(copy)
	{
		// input "number of copies"
		double number_of_copies = 1.0;
		wxGetApp().InputDouble(_T("Enter number of copies"), _T("number of copies"), number_of_copies);
		int ncopies = (int)(number_of_copies + 0.5);
		if(ncopies < 1)return;
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear();

	// pick a line to mirror about
	bool line_found = false;
	gp_Lin line;
	wxGetApp().m_marked_list->m_filter = MARKING_FILTER_LINE | MARKING_FILTER_ILINE;
	wxGetApp().PickObjects(_T("Pick line to mirror about"));
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
		wxGetApp().StartHistory(_T("Copy Mirror"));
		for(std::list<HeeksObj*>::iterator It = selected_items.begin(); It != selected_items.end(); It++)
		{
			HeeksObj* object = *It;
			HeeksObj* new_object = object->MakeACopy();
			wxGetApp().AddUndoably(new_object, object->m_owner, NULL);
			new_object->ModifyByMatrix(m);
		}
		wxGetApp().EndHistory();
		wxGetApp().m_marked_list->Clear();
	}
	else
	{
		wxGetApp().TransformUndoably(selected_items, m);
	}
}

void TransformTools::Scale(bool copy)
{
	// pick items
	if(wxGetApp().m_marked_list->size() == 0){
		wxGetApp().PickObjects(_T("Pick objects to scale"));
	}
	if(wxGetApp().m_marked_list->size() == 0)return;

	// get number of copies
	int ncopies = 1;
	if(copy)
	{
		// input "number of copies"
		double number_of_copies = 1.0;
		wxGetApp().InputDouble(_T("Enter number of copies"), _T("number of copies"), number_of_copies);
		ncopies = (int)(number_of_copies + 0.5);
		if(ncopies < 1)return;
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear();

	// pick "centre" position
	if(!wxGetApp().PickPosition(_T("Click centre position to scale about"), centre))return;

	// enter scale factor
	double scale = 2.0;
	if(!wxGetApp().InputDouble(_T("Enter scale factor"), _T("scale factor"), scale))return;

	// transform the objects
	if(copy)
	{
		wxGetApp().StartHistory(_T("Copy scale"));
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
				wxGetApp().AddUndoably(new_object, object->m_owner, NULL);
				new_object->ModifyByMatrix(m);
			}
		}
		wxGetApp().EndHistory();
		wxGetApp().m_marked_list->Clear();	}
	else
	{
		gp_Trsf mat;
		mat.SetScale(make_point(centre), scale);
		double m[16];
		extract(mat, m);
		wxGetApp().TransformUndoably(selected_items, m);
	}
}
