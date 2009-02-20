// HeeksObj.cpp

#include "stdafx.h"
#include "HeeksObj.h"
#include "PropertyString.h"
#include "PropertyInt.h"
#include "PropertyColor.h"
#include "PropertyCheck.h"
#include "../tinyxml/tinyxml.h"
#ifdef HEEKSCAD
#include "ObjList.h"
#include "../src/Gripper.h"
#include "../src/HeeksFrame.h"
#include "../src/ObjPropsCanvas.h"
#endif

HeeksObj::HeeksObj(void): m_id(0), m_layer(0), m_visible(true), m_owner(NULL){}

HeeksObj::HeeksObj(const HeeksObj& ho): m_id(0), m_layer(0), m_visible(true), m_owner(NULL){operator=(ho);}

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

static void on_set_visible(bool value, HeeksObj* object)
{
	object->m_visible = value;
#ifdef HEEKSCAD
	wxGetApp().Repaint();
#else
	heeksCAD->Repaint();
#endif
}

void HeeksObj::GetProperties(std::list<Property *> *list)
{
	bool editable = CanEditString();
	list->push_back(new PropertyString(_("object type"), GetTypeString(), NULL));
	if(GetShortString())list->push_back(new PropertyString(_("object title"), GetShortString(), this, editable ? on_edit_string : NULL));
	if(UsesID())list->push_back(new PropertyInt(_("ID"), m_id, this, on_set_id));
	const HeeksColor* c = GetColor();
	if(c)
	{
		list->push_back ( new PropertyColor ( _("color"),  *c, this, on_set_color ) );
	}
	list->push_back(new PropertyCheck(_("visible"), m_visible, this, on_set_visible));
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
	list->push_back(GripperTypeScale);
	list->push_back(box.m_x[3]);
	list->push_back(box.m_x[4]);
	list->push_back(box.m_x[2]);
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
	wxGetApp().ObjectWriteBaseXML(this, element);
#else
	heeksCAD->ObjectWriteBaseXML(this, element);
#endif
}

void HeeksObj::ReadBaseXML(TiXmlElement* element)
{
#ifdef HEEKSCAD
	wxGetApp().ObjectReadBaseXML(this, element);
#else
	heeksCAD->ObjectReadBaseXML(this, element);
#endif
}

bool HeeksObj::OnVisibleLayer()
{
	// to do, support multiple layers.
	return true;
}
