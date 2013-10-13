// HeeksObj.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "HeeksObj.h"
#include "PropertyString.h"
#include "PropertyInt.h"
#include "PropertyColor.h"
#include "PropertyCheck.h"
#ifdef HEEKSCAD
#include "../tinyxml/tinyxml.h"
#include "ObjList.h"
#include "../src/Gripper.h"
#include "../src/HeeksFrame.h"
#include "../src/ObjPropsCanvas.h"
#include "../src/Sketch.h"
#include "../src/Pad.h"
#else
#include "GripperTypes.h"
#include "GripData.h"
#endif

HeeksObj::HeeksObj(void): m_skip_for_undo(false), m_id(0), m_layer(0), m_visible(true), m_preserving_id(false), m_index(0), m_owner(NULL)
{
}

HeeksObj::HeeksObj(const HeeksObj& ho): m_skip_for_undo(false), m_id(0), m_layer(0), m_visible(true),m_preserving_id(false), m_index(0), m_owner(NULL)
{
	operator=(ho);
}

const HeeksObj& HeeksObj::operator=(const HeeksObj &ho)
{
	// don't copy the ID or the owner
	m_layer = ho.m_layer;
	m_visible = ho.m_visible;
	m_skip_for_undo = ho.m_skip_for_undo;

	if(ho.m_preserving_id)
		m_id = ho.m_id;

	return *this;
}

HeeksObj::~HeeksObj()
{
	if(m_owner)m_owner->Remove(this);

#ifdef HEEKSCAD
	if (m_index) wxGetApp().ReleaseIndex(m_index);
#else
	if (m_index) heeksCAD->ReleaseIndex(m_index);
#endif
}

HeeksObj* HeeksObj::MakeACopyWithID()
{
	m_preserving_id = true;
	HeeksObj* ret = MakeACopy();
	m_preserving_id = false;
	return ret;
}

void on_edit_string(const wxChar* value, HeeksObj* object)
{
	object->OnEditString(value);
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

const wxBitmap &HeeksObj::GetIcon()
{
	static wxBitmap* icon = NULL;
#ifdef HEEKSCAD
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/unknown.png")));
#else
	if(icon == NULL)icon = new wxBitmap(wxImage(heeksCAD->GetResFolder() + _T("/icons/unknown.png")));
#endif
	return *icon;
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

bool HeeksObj::StretchTemporaryTransformed(const double *p, const double* shift, void* data)
{
#ifdef HEEKSCAD
	gp_Trsf mat;

	HeeksObj* owner = m_owner;

	CSketch *sketch = dynamic_cast<CSketch*>(owner);

	if(sketch && sketch->m_coordinate_system)
		mat = sketch->m_coordinate_system->GetMatrix();

	//mat.Invert();

	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	//vp.Transform(mat);
	//vshift.Transform(mat);

	double np[3];
	double nshift[3];
	extract(vp,np);
	extract(vshift,nshift);

	return StretchTemporary(np,nshift,data);
#else
	return StretchTemporary(p,shift,data);
#endif
}

void HeeksObj::GetGripperPositionsTransformed(std::list<GripData> *list, bool just_for_endof)
{
#ifdef HEEKSCAD

	//TODO: We want to transform these coords by whatever has happened to the draw matrix on the way down to our level
	//For right now we are just grabbing the sketches coord system, but this isn't right and won't work when parts or
	//assemblies come around.
	//For that matter it has gotten out of control with the addition of faces and edges to pads
	std::list<GripData> newlist;
	GetGripperPositions(&newlist,just_for_endof);

	gp_Trsf mat;

	CSketch *sketch = dynamic_cast<CSketch*>(m_owner);

	if(sketch && sketch->m_coordinate_system)
		mat = sketch->m_coordinate_system->GetMatrix();

	CPad *pad = dynamic_cast<CPad*>(m_owner);
	if(!pad && m_owner)
		pad = dynamic_cast<CPad*>(m_owner->m_owner);

	if(pad && pad->m_sketch->m_coordinate_system)
		mat = pad->m_sketch->m_coordinate_system->GetMatrix();

	std::list<GripData>::iterator it;
	for(it = newlist.begin(); it != newlist.end(); ++it)
	{
		GripData gd = *it;

		gp_Pnt pnt(gd.m_x,gd.m_y,gd.m_z);
		pnt.Transform(mat);
		gd.m_x = pnt.X();
		gd.m_y = pnt.Y();
		gd.m_z = pnt.Z();
		list->push_back(gd);
	}
#else
	GetGripperPositions(list,just_for_endof);
#endif
}

void HeeksObj::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
//#ifdef HEEKSCAD
	CBox box;
	GetBox(box);
	if(!box.m_valid)return;

	//TODO: This is a tab bit of a strange thing to do. Especially for planar objects like faces
	//ones that are on a plane like y-z or x-z will have all gripper merged togeather.
	list->push_back(GripData(GripperTypeTranslate,box.m_x[0],box.m_x[1],box.m_x[2],NULL));
	list->push_back(GripData(GripperTypeRotateObject,box.m_x[3],box.m_x[1],box.m_x[2],NULL));
	list->push_back(GripData(GripperTypeRotateObject,box.m_x[0],box.m_x[4],box.m_x[2],NULL));
	list->push_back(GripData(GripperTypeScale,box.m_x[3],box.m_x[4],box.m_x[2],NULL));
//#endif
}

bool HeeksObj::Add(HeeksObj* object, HeeksObj* prev_object)
{
	object->m_owner = this;
	object->OnAdd();
	return true;
}

void HeeksObj::OnRemove()
{
	if(m_owner == NULL)KillGLLists();
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

HeeksObj *HeeksObj::Find( const int type, const unsigned int id )
{
	if ((type == this->GetType()) && (this->m_id == id)) return(this);
	return(NULL);
}

#ifdef WIN32
#define snprintf _snprintf
#endif

void HeeksObj::ToString(char *str, unsigned int* rlen, unsigned int len)
{
	unsigned int printed;
	*rlen = 0;

	printed = snprintf(str,len,"ID: 0x%X, Type: 0x%X, MarkingMask: 0x%X, IDGroup: 0x%X\n",GetID(),GetType(),(unsigned int)GetMarkingMask(),GetIDGroupType());
	if(printed >= len)
		goto abort;
	*rlen += printed; len -= printed;

abort:
	*rlen = 0;
}

unsigned int HeeksObj::GetIndex() {
#ifdef HEEKSCAD
	if (!m_index) m_index = wxGetApp().GetIndex(this);
#else
	if (!m_index) m_index = heeksCAD->GetIndex(this);
#endif
	return m_index;
}
