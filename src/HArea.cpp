// HArea.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HArea.h"
#include "HLine.h"
#include "HILine.h"
#include "HCircle.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyInt.h"
#include "../interface/PropertyLength.h"
#include "../interface/PropertyChoice.h"
#include "../tinyxml/tinyxml.h"
#include "../interface/PropertyVertex.h"
#include "../interface/Tool.h"
#include "Gripper.h"
#include "Sketch.h"
#include "SolveSketch.h"
#include "Drawing.h"
#include "DigitizeMode.h"
#include "../interface/MarkedObject.h"
#include "Arc.h"
#include "HeeksConfig.h"

HArea::HArea(const HArea &a){
	operator=(a);
}

HArea::HArea(const CArea &a):m_area(a){
}

HArea::~HArea(){
}

const wxBitmap &HArea::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/area.png")));
	return *icon;
}

bool HArea::IsDifferent(HeeksObj* other)
{
	HArea* a = (HArea*)other;

	// to do

	return this != a;
}

const HArea& HArea::operator=(const HArea &b){
	m_area = b.m_area;
	return *this;
}

static HArea* object_for_tool = NULL;

class DeleteCurveTool:public Tool{
public:
	// Tool's virtual functions
	void Run(){
		for(std::list<CCurve>::iterator It = object_for_tool->m_area.m_curves.begin(); It != object_for_tool->m_area.m_curves.end(); It++)
		{
			CCurve& curve = *It;
			if(&curve == object_for_tool->m_selected_curve)
			{
				object_for_tool->m_area.m_curves.erase(It);
				break;
			}		
		}
	}
	const wxChar* GetTitle(){ return _("Delete Curve");}
	wxString BitmapPath(){return _T("delete");}
};

static DeleteCurveTool delete_selected_curve;

class OffsetAreaTool:public Tool{
public:
	// Tool's virtual functions
	void Run(){
		double offset_value = 2.0;
		HeeksConfig config;
		config.Read(_T("OffsetAreaValue"), &offset_value);
		if(wxGetApp().InputLength(_("Enter Offset Value, + for making bigger, - for making smaller"), _("Offset value"), offset_value))
		{
			try
			{
				CArea area = object_for_tool->m_area;
				area.Offset(-offset_value);
				HeeksObj* new_object = new HArea(area);
				object_for_tool->Owner()->Add(new_object, NULL);
				config.Write(_T("OffsetAreaValue"), offset_value);
			}
			catch (...) {
				wxMessageBox(_("Area offset error"));
			}
		}
	}
	const wxChar* GetTitle(){ return _("Offset Area");}
	wxString BitmapPath(){return _T("offsetarea");}
};

static OffsetAreaTool offset_area_tool;

class ObroundAreaTool:public Tool{
public:
	// Tool's virtual functions
	void Run(){
		double offset_value = 0.3;
		HeeksConfig config;
		config.Read(_T("ObroundAreaValue"), &offset_value);
		if(wxGetApp().InputLength(_("Enter Offset Value"), _("Offset value"), offset_value))
		{
			try
			{
				CArea area = object_for_tool->m_area;
				area.Thicken(fabs(offset_value));
				HeeksObj* new_object = new HArea(area);
				object_for_tool->Owner()->Add(new_object, NULL);
				config.Write(_T("ObroundAreaValue"), fabs(offset_value));
			}
			catch (...) {
				wxMessageBox(_("Area thicken error"));
			}
		}
	}
	const wxChar* GetTitle(){ return _("Thicken Area");}
	wxString BitmapPath(){return _T("thickenarea");}
};

static ObroundAreaTool obround_area_tool;

class ReorderAreaTool:public Tool{
public:
	// Tool's virtual functions
	void Run(){
		object_for_tool->m_area.Reorder();
	}
	const wxChar* GetTitle(){ return _("Reorder Area");}
	wxString BitmapPath(){return _T("reorderarea");}
};

static ReorderAreaTool reorder_area_tool;

class SplitAreaTool:public Tool{
public:
	// Tool's virtual functions
	void Run(){
		std::list<CArea> areas;
		object_for_tool->m_area.Split(areas);
		for(std::list<CArea>::iterator It = areas.begin(); It != areas.end(); It++)
		{
			CArea &area = *It;
			HArea* new_object = new HArea(area);
			wxGetApp().Add(new_object, NULL);
		}
		wxGetApp().Remove(object_for_tool);
	}
	const wxChar* GetTitle(){ return _("Split Area");}
	wxString BitmapPath(){return _T("areasplit");}
};

static SplitAreaTool split_area_tool;

void HArea::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	object_for_tool = this;
	if(this->m_selected_curve)t_list->push_back(&delete_selected_curve);
	t_list->push_back(&offset_area_tool);
	t_list->push_back(&obround_area_tool);
	t_list->push_back(&reorder_area_tool);
	t_list->push_back(&split_area_tool);
}

static void glVertexFunction(const double *p){glVertex3d(p[0], p[1], 0.0);}

static void render_curve(const CCurve& curve)
{
	glBegin(GL_LINE_STRIP);

	int i = 0; // for gl name stack, when I get round to being able to select individual vertices
	const CVertex* prev_vertex = NULL;
	for(std::list<CVertex>::const_iterator It = curve.m_vertices.begin(); It != curve.m_vertices.end(); It++, i++)
	{
		const CVertex& vertex = *It;
		if(prev_vertex && vertex.m_type)
		{
			CArc arc(Point(prev_vertex->m_p), Point(vertex.m_p), Point(vertex.m_c), (vertex.m_type == 1), vertex.m_user_data);
			arc.GetSegments(glVertexFunction, wxGetApp().GetPixelScale());
		}
		else
		{
			glVertex3d(vertex.m_p.x, vertex.m_p.y, 0.0);
		}
		prev_vertex = &vertex;
	}

	glEnd();
}

void HArea::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		wxGetApp().glColorEnsuringContrast(HeeksColor(0));
	}
	GLfloat save_depth_range[2];
	if(marked){
		glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
		glDepthRange(0, 0);
		glLineWidth(2);
	}

	unsigned int i = 0;
	for(std::list<CCurve>::iterator It = m_area.m_curves.begin(); It != m_area.m_curves.end(); It++, i++)
	{
		CCurve& curve = *It;
		if(select)glPushName(i);
	
		if(marked && (&curve == m_selected_curve))glColor3ub(0, 255, 0);
		render_curve(curve);
		if(marked && (&curve == m_selected_curve))wxGetApp().glColorEnsuringContrast(HeeksColor(0));
		if(select)glPopName();
	}

	if(marked){
		glLineWidth(1);
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}
}

void HArea::Draw(wxDC& dc)
{
}

HeeksObj *HArea::MakeACopy(void)const
{
	HArea *new_object = new HArea(*this);
	return new_object;
}

void HArea::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);

	for(std::list<CCurve>::iterator It = m_area.m_curves.begin(); It != m_area.m_curves.end(); It++)
	{
		CCurve& curve = *It;
		for(std::list<CVertex>::iterator VIt = curve.m_vertices.begin(); VIt != curve.m_vertices.end(); VIt++)
		{
			CVertex &v = *VIt;
			gp_Pnt p(v.m_p.x, v.m_p.y, 0.0);
			p.Transform(mat);
			v.m_p = Point(p.X(), p.Y());
			if(v.m_type != 0)
			{
				gp_Pnt c(v.m_c.x, v.m_c.y, 0.0);
				c.Transform(mat);
				v.m_c = Point(c.X(), c.Y());
			}
		}
	}
}

void HArea::GetBox(CBox &box){
	CBox2D b2;
	m_area.GetBox(b2);
	box.Insert(b2.m_minxy.x, b2.m_minxy.y, 0.0);
	box.Insert(b2.m_maxxy.x, b2.m_maxxy.y, 0.0);
}

void HArea::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	//list->push_back(GripData(GripperTypeStretch,C->m_p.X(),C->m_p.Y(),C->m_p.Z(),C));
}

void HArea::GetProperties(std::list<Property *> *list){
	list->push_back(new PropertyInt(_("number of curves"), this->m_area.m_curves.size(), this, NULL));
	HeeksObj::GetProperties(list);
}

bool HArea::Stretch(const double *p, const double* shift, void* data){
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);
#if 0
	if(A->m_p.IsEqual(vp, wxGetApp().m_geom_tol)){
		gp_Vec direction = -(GetSegmentVector(1.0));
		gp_Pnt centre;
		gp_Dir axis;
		gp_Pnt new_A = gp_Pnt(A->m_p.XYZ() + vshift.XYZ());
		if(HArea::TangentialArc(B->m_p, direction, new_A, centre, axis))
		{
			m_axis = gp_Ax1(centre, -axis);
			m_radius = new_A.Distance(centre);
			A->m_p = new_A;
		}
	}
#endif

	return false;
}

// static
void HArea::WriteVertex(const CVertex& vertex, TiXmlNode *root)
{
	TiXmlElement* element = new TiXmlElement( "vertex" );
	root->LinkEndChild( element );  

	element->SetAttribute("type", vertex.m_type);
	element->SetDoubleAttribute("x", vertex.m_p.x);
	element->SetDoubleAttribute("y", vertex.m_p.y);
	if(vertex.m_type != 0)
	{
		element->SetDoubleAttribute("i", vertex.m_c.x);
		element->SetDoubleAttribute("j", vertex.m_c.y);
	}
}

// static
void HArea::ReadVertex(CVertex& vertex, TiXmlElement *root)
{
	root->Attribute("type", &vertex.m_type);
	root->Attribute("x", &vertex.m_p.x);
	root->Attribute("y", &vertex.m_p.y);
	if(vertex.m_type != 0)
	{
		root->Attribute("i", &vertex.m_c.x);
		root->Attribute("j", &vertex.m_c.y);
	}
}

// static
void HArea::WriteCurve(const CCurve& curve, TiXmlNode *root)
{
	TiXmlElement* element = new TiXmlElement( "curve" );
	root->LinkEndChild( element );  
	for(std::list<CVertex>::const_iterator It = curve.m_vertices.begin(); It != curve.m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		WriteVertex(vertex, element);
	}
}

// static
void HArea::ReadCurve(CCurve& curve, TiXmlElement *root)
{
	for(TiXmlElement *pElem = root->FirstChildElement(); pElem;	pElem = pElem->NextSiblingElement())
	{
		CVertex vertex;
		ReadVertex(vertex, pElem);
		curve.m_vertices.push_back(vertex);
	}
}

// static
void HArea::WriteArea(const CArea& area, TiXmlNode *root)
{
	TiXmlElement* element = new TiXmlElement( "area" );
	root->LinkEndChild( element );  
	for(std::list<CCurve>::const_iterator It = area.m_curves.begin(); It != area.m_curves.end(); It++)
	{
		const CCurve& curve = *It;
		WriteCurve(curve, element);
	}
}

// static
void HArea::ReadArea(CArea& area, TiXmlElement *root)
{
	for(TiXmlElement *pElem = root->FirstChildElement(); pElem;	pElem = pElem->NextSiblingElement())
	{
		CCurve curve;
		ReadCurve(curve, pElem);
		area.m_curves.push_back(curve);
	}
}

void HArea::WriteXML(TiXmlNode *root)
{
	TiXmlElement *element = new TiXmlElement( "Area" );
	root->LinkEndChild( element );
	WriteArea(m_area, element);

	WriteBaseXML(element);
}

// static member function
HeeksObj* HArea::ReadFromXMLElement(TiXmlElement* root)
{
	// read area
	CArea area;
	ReadArea(area, root->FirstChildElement());

	HArea* new_object = new HArea(area);
	new_object->ReadBaseXML(root);

	return new_object;
}

void HArea::SetClickMarkPoint(MarkedObject* marked_object, const double* ray_start, const double* ray_direction)
{
	// set picked curve
	if(marked_object->GetNumCustomNames()>0)
	{
		unsigned int i = 0;
		for(std::list<CCurve>::iterator It = m_area.m_curves.begin(); It != m_area.m_curves.end(); It++, i++)
		{
			CCurve& curve = *It;
			if(i == marked_object->GetCustomNames()[0])
			{
				m_selected_curve = &curve;
				break;
			}
		}
	}
}
