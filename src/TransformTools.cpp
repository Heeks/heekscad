// TransformTools.cpp

#include "stdafx.h"
#include "TransformTools.h"
#include "MarkedList.h"

static double from[3];
static double centre[3];

static void on_move_translate(const double* to)
{
	wxGetApp().m_drag_matrix.SetTranslationPart(gp_Vec(make_point(from), make_point(to)));
	wxGetApp().Repaint(true);
}

//static
void TransformTools::MoveTranslate()
{
	// pick items
	if(wxGetApp().m_marked_list->size() == 0){
		wxGetApp().PickObjects("Pick objects to move");
	}
	if(wxGetApp().m_marked_list->size() == 0)return;
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear();

	// pick "from" position
	if(!wxGetApp().PickPosition("Click position to move from", from))return;

	// pick "to" position
	wxGetApp().m_marked_list->Add(selected_items);
	wxGetApp().CreateTransformGLList(false);
	wxGetApp().m_drag_matrix = gp_Trsf();
	wxGetApp().HideMarkedList();
	double to[3];
	bool success = wxGetApp().PickPosition("Click position to move to", to, on_move_translate);
	wxGetApp().UnHideMarkedList();
	wxGetApp().DestroyTransformGLList();
	wxGetApp().m_marked_list->Clear();

	// transform the objects
	gp_Trsf mat;
	mat.SetTranslationPart(make_vector(make_point(from), make_point(to)));
	double m[16];
	extract(mat, m);
	wxGetApp().TransformUndoably(selected_items, m);
}

//static
void TransformTools::MoveRotate()
{
	// pick items
	if(wxGetApp().m_marked_list->size() == 0){
		wxGetApp().PickObjects("Pick objects to rotate");
	}
	if(wxGetApp().m_marked_list->size() == 0)return;
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear();

	// pick "centre" position
	if(!wxGetApp().PickPosition("Click centre position to rotate about", centre))return;

	// enter angle
	double angle = 45.0;
	if(!wxGetApp().InputDouble("Enter angle to rotate by", "angle", angle))return;

	// transform the objects
	gp_Trsf mat;
	mat.SetRotation(gp_Ax1(make_point(centre), gp_Dir(0, 0, 1)), angle * Pi/180);
	double m[16];
	extract(mat, m);
	wxGetApp().TransformUndoably(selected_items, m);
}

//static
void TransformTools::CopyTranslate()
{
	// pick items
	if(wxGetApp().m_marked_list->size() == 0){
		wxGetApp().PickObjects("Pick Objects To Move");
	}
	if(wxGetApp().m_marked_list->size() == 0)return;

	// input "number of copies"
	double number_of_copies = 1.0;
	wxGetApp().InputDouble("Enter number of copies", "number of copies", number_of_copies);
	int ncopies = (int)(number_of_copies + 0.5);
	if(ncopies < 1)return;

	// clear the selection
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear();

	// pick "from" position
	if(!wxGetApp().PickPosition("Click Position To Move From", from))return;

	// pick "to" position
	wxGetApp().m_marked_list->Add(selected_items);
	wxGetApp().CreateTransformGLList(false);
	wxGetApp().m_drag_matrix = gp_Trsf();
//	wxGetApp().HideMarkedList();
	double to[3];
	bool success = wxGetApp().PickPosition("Click Position To Move To", to, on_move_translate);
//	wxGetApp().UnHideMarkedList();
	wxGetApp().DestroyTransformGLList();
	wxGetApp().m_marked_list->Clear();

	// transform the objects
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

//static
void TransformTools::CopyRotate()
{
	// pick items
	if(wxGetApp().m_marked_list->size() == 0){
		wxGetApp().PickObjects("Pick objects to rotate");
	}
	if(wxGetApp().m_marked_list->size() == 0)return;

	// input "number of copies"
	double number_of_copies = 1.0;
	wxGetApp().InputDouble("Enter number of copies", "number of copies", number_of_copies);
	int ncopies = (int)(number_of_copies + 0.5);
	if(ncopies < 1)return;

	// clear the selection
	std::list<HeeksObj *> selected_items = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear();

	// pick "centre" position
	if(!wxGetApp().PickPosition("Click centre position to rotate about", centre))return;

	// enter angle
	double angle = 45.0;
	if(!wxGetApp().InputDouble("Enter angle to rotate by", "angle", angle))return;

	// transform the objects
	wxGetApp().StartHistory(_T("Copy translate"));
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
	wxGetApp().m_marked_list->Clear();
}
