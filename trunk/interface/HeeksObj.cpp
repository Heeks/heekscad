// HeeksObj.cpp

#include "stdafx.h"
#include "HeeksObj.h"
#include "PropertyString.h"
#include "PropertyInt.h"
#ifdef HEEKSCAD
#include "ObjList.h"
#endif

HeeksObj::HeeksObj(void): m_owner(NULL), m_id(0){}

HeeksObj::HeeksObj(const HeeksObj& ho): m_owner(NULL), m_id(0){operator=(ho);}

const HeeksObj& HeeksObj::operator=(const HeeksObj &ho)
{
	// don't copy the ID or the owner
	m_layer = ho.m_layer;

	return *this;
}

static HeeksObj* object_for_properties = NULL;

void on_edit_string(const wxChar* value)
{
	object_for_properties->OnEditString(value);

	// to do , reconnect these two
//	wxGetApp().WasModified(object_for_properties);
//	wxGetApp().Repaint();
}

static void on_set_id(int value)
{
	object_for_properties->SetID(value);
}

void HeeksObj::GetProperties(std::list<Property *> *list)
{
	bool editable = CanEditString();
	object_for_properties = this;
	list->push_back(new PropertyString(_T("object type"), GetTypeString(), NULL));
	if(GetShortString())list->push_back(new PropertyString(_T("object title"), GetShortString(), editable ? on_edit_string : NULL));
	list->push_back(new PropertyInt(_T("ID"), m_id, on_set_id));
}

void HeeksObj::GetGripperPositions(std::list<double> *list, bool just_for_endof)
{
	// default gripper positions; every corner of the box around the object
	CBox box;
	GetBox(box);
	if(box.m_valid)
	{
		list->push_back(0);
		list->push_back(box.m_x[0]);
		list->push_back(box.m_x[1]);
		list->push_back(box.m_x[2]);

		list->push_back(0);
		list->push_back(box.m_x[3]);
		list->push_back(box.m_x[1]);
		list->push_back(box.m_x[2]);

		list->push_back(0);
		list->push_back(box.m_x[3]);
		list->push_back(box.m_x[4]);
		list->push_back(box.m_x[2]);

		list->push_back(0);
		list->push_back(box.m_x[0]);
		list->push_back(box.m_x[4]);
		list->push_back(box.m_x[2]);

		list->push_back(0);
		list->push_back(box.m_x[0]);
		list->push_back(box.m_x[1]);
		list->push_back(box.m_x[5]);

		list->push_back(0);
		list->push_back(box.m_x[3]);
		list->push_back(box.m_x[1]);
		list->push_back(box.m_x[5]);

		list->push_back(0);
		list->push_back(box.m_x[3]);
		list->push_back(box.m_x[4]);
		list->push_back(box.m_x[5]);

		list->push_back(0);
		list->push_back(box.m_x[0]);
		list->push_back(box.m_x[4]);
		list->push_back(box.m_x[5]);
	}
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