// HeeksObj.cpp

#include "stdafx.h"
#include "HeeksObj.h"
#include "PropertyString.h"
#include "PropertyInt.h"
#include "PropertyColor.h"
#ifdef HEEKSCAD
#include "ObjList.h"
#include "../src/Gripper.h"
#include "../src/HeeksFrame.h"
#include "../src/ObjPropsCanvas.h"
#endif

HeeksObj::HeeksObj(void): m_owner(NULL), m_id(0), m_layer(0), m_visible(true){}

HeeksObj::HeeksObj(const HeeksObj& ho): m_owner(NULL), m_id(0), m_layer(0), m_visible(true){operator=(ho);}

const HeeksObj& HeeksObj::operator=(const HeeksObj &ho)
{
	// don't copy the ID or the owner
	m_layer = ho.m_layer;
	m_visible = ho.m_visible;

	return *this;
}

void on_edit_string(const wxChar* value, HeeksObj* object)
{
	object->OnEditString(value);

	// to do , reconnect these two
//	wxGetApp().WasModified(object_for_properties);
//	wxGetApp().Repaint();
}

static void on_set_color(HeeksColor value, HeeksObj* object)
{
	object->SetColor(value);


#ifdef HEEKSCAD
	wxGetApp().m_frame->m_properties->OnApply2();
	wxGetApp().Repaint();
#else
	heeksCAD->PropertiesOnApply2();
	heeksCAD->Repaint();
#endif
}

static void on_set_id(int value, HeeksObj* object)
{
	object->SetID(value);
}

void HeeksObj::GetProperties(std::list<Property *> *list)
{
	bool editable = CanEditString();
	list->push_back(new PropertyString(_T("object type"), GetTypeString(), NULL));
	if(GetShortString())list->push_back(new PropertyString(_T("object title"), GetShortString(), this, editable ? on_edit_string : NULL));
	list->push_back(new PropertyInt(_T("ID"), m_id, this, on_set_id));
	const HeeksColor* c = GetColor();
	if(c)
	{
		list->push_back ( new PropertyColor ( _T("color"),  *c, this, on_set_color ) );
	}
}

bool HeeksObj::GetScaleAboutMatrix(double *m)
{
#ifdef HEEKSCAD
	// return the bottom left corner of the box
	CBox box;
	GetBox(box);
	if(!box.m_valid)return false;
	gp_Trsf mat;
	mat.SetTranslationPart(gp_Vec(box.m_x[0], box.m_x[1], box.m_x[2]));
	extract(mat, m);
	return true;
#else
	return false;
#endif
}

void HeeksObj::GetGripperPositions(std::list<double> *list, bool just_for_endof)
{
#ifdef HEEKSCAD
	CBox box;
	GetBox(box);
	if(!box.m_valid)return;
	list->push_back(GripperTypeTranslate);
	list->push_back(box.m_x[0]);
	list->push_back(box.m_x[1]);
	list->push_back(box.m_x[2]);
	list->push_back(GripperTypeRotateObject);
	list->push_back(box.m_x[3]);
	list->push_back(box.m_x[1]);
	list->push_back(box.m_x[2]);
	list->push_back(GripperTypeRotateObject);
	list->push_back(box.m_x[0]);
	list->push_back(box.m_x[4]);
	list->push_back(box.m_x[2]);
	list->push_back(GripperTypeRotateObject);
	list->push_back(box.m_x[0]);
	list->push_back(box.m_x[1]);
	list->push_back(box.m_x[5]);
	list->push_back(GripperTypeScale);
	list->push_back(box.m_x[3]);
	list->push_back(box.m_x[4]);
	list->push_back(box.m_x[5]);
	list->push_back(GripperTypeRotate);
	list->push_back(box.m_x[3]);
	list->push_back(box.m_x[4]);
	list->push_back(box.m_x[2]);
	list->push_back(GripperTypeRotate);
	list->push_back(box.m_x[0]);
	list->push_back(box.m_x[4]);
	list->push_back(box.m_x[5]);
	list->push_back(GripperTypeRotate);
	list->push_back(box.m_x[3]);
	list->push_back(box.m_x[1]);
	list->push_back(box.m_x[5]);
#endif
}

void HeeksObj::OnRemove()
{
	KillGLLists();
}

void HeeksObj::SetID(int id)
{
#ifdef HEEKSCAD
	wxGetApp().SetObjectID(this, id);
#else
	heeksCAD->SetObjectID(this, id);
#endif
}

void HeeksObj::WriteBaseXML(TiXmlElement *element)
{
#ifdef HEEKSCAD
	wxGetApp().WriteIDToXML(this, element);
#else
	heeksCAD->WriteIDToXML(this, element);
#endif
}

void HeeksObj::ReadBaseXML(TiXmlElement* element)
{
#ifdef HEEKSCAD
	wxGetApp().ReadIDFromXML(this, element);
#else
	heeksCAD->ReadIDFromXML(this, element);
#endif
}

bool HeeksObj::OnVisibleLayer()
{
	// to do, support multiple layers.

	// for now, layer -1 is invisible, all others are visible
	return m_layer != -1;
}