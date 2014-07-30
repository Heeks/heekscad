// HAngularDimension.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "HAngularDimension.h"
#include "HDimension.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyChoice.h"
#include "PropertyTrsf.h"
#include "Gripper.h"
#include "HPoint.h"
#include "HeeksFrame.h"
#include "GraphicsCanvas.h"

HAngularDimension::HAngularDimension(const wxString &text, const gp_Pnt &p0, const gp_Pnt &p1, const gp_Pnt &p2, const gp_Pnt &p3, const gp_Pnt &p4, AngularDimensionTextMode text_mode, const HeeksColor* col):m_color(*col), m_text(text), m_text_mode(text_mode),  m_scale(1.0)
{
	m_p0 = new HPoint(p0,col);
	m_p1 = new HPoint(p1,col);
	m_p2 = new HPoint(p2,col);
	m_p3 = new HPoint(p3,col);
	m_p4 = new HPoint(p4,col);

	m_p0->m_draw_unselected = false;
	m_p1->m_draw_unselected = false;
	m_p2->m_draw_unselected = false;
	m_p3->m_draw_unselected = false;
	m_p4->m_draw_unselected = false;

	m_p0->SetSkipForUndo(true);
	m_p1->SetSkipForUndo(true);
	m_p2->SetSkipForUndo(true);
	m_p3->SetSkipForUndo(true);
	m_p4->SetSkipForUndo(true);

	Add(m_p0,NULL);
	Add(m_p1,NULL);
	Add(m_p2,NULL);
	Add(m_p3,NULL);
	Add(m_p4,NULL);
}

//Points loaded via objlist constructor
HAngularDimension::HAngularDimension(const wxString &text, AngularDimensionTextMode text_mode, const HeeksColor* col):m_color(*col), m_text(text), m_text_mode(text_mode),  m_scale(1.0)
{

}

HAngularDimension::HAngularDimension(const HAngularDimension &b)
{
	operator=(b);
}

HAngularDimension::~HAngularDimension(void)
{
}

const HAngularDimension& HAngularDimension::operator=(const HAngularDimension &b)
{
#ifdef MULTIPLE_OWNERS
	ObjList::operator=(b);
#else
	HeeksObj::operator=(b);
#endif
	m_text = b.m_text;
	m_text_mode = b.m_text_mode;
	m_color = b.m_color;
	m_scale = b.m_scale;

	ReloadPointers();
	return *this;
}

const wxBitmap &HAngularDimension::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/dimension.png")));
	return *icon;
}

bool HAngularDimension::IsDifferent(HeeksObj* other)
{
	HAngularDimension* dim = (HAngularDimension*)other;

	if(m_color.COLORREF_color() != dim->m_color.COLORREF_color() || m_scale != dim->m_scale || m_text_mode != dim->m_text_mode)
		return true;

	if(wxStrcmp(m_text,dim->m_text))
		return true;

	if(m_p0->m_p.Distance(dim->m_p0->m_p) > wxGetApp().m_geom_tol)
		return true;

	if(m_p1->m_p.Distance(dim->m_p1->m_p) > wxGetApp().m_geom_tol)
		return true;


	if(m_p2->m_p.Distance(dim->m_p2->m_p) > wxGetApp().m_geom_tol)
		return true;


	if(m_p3->m_p.Distance(dim->m_p3->m_p) > wxGetApp().m_geom_tol)
		return true;

	if(m_p4->m_p.Distance(dim->m_p4->m_p) > wxGetApp().m_geom_tol)
		return true;

#ifdef MULTIPLE_OWNERS
	return ObjList::IsDifferent(other);
#else
	return HeeksObj::IsDifferent(other);
#endif
}

void HAngularDimension::glCommands(bool select, bool marked, bool no_color)
{
	//Find the point where the 2 lines meet.

	gp_Pnt pnt;
	if(intersect(gp_Lin(m_p0->m_p,m_p1->m_p.XYZ()-m_p0->m_p.XYZ()),gp_Lin(m_p2->m_p,m_p3->m_p.XYZ()-m_p2->m_p.XYZ()),pnt))
	{
		//Find the distance our gripper is from said thing
		double r = m_p4->m_p.Distance(pnt);

		//Save the angle of the control point
		double ca = atan2(m_p4->m_p.Y()-pnt.Y(),m_p4->m_p.X()-pnt.X());

		//find the angle of the lines

		double l1d1 = m_p0->m_p.Distance(pnt);
		double l1d2 = m_p1->m_p.Distance(pnt);

		double a1=0;

		//Avoid singularity and instabillity
		if(l1d1 > l1d2)
			a1 = atan2(m_p0->m_p.Y()-pnt.Y(),m_p0->m_p.X()-pnt.X());
		else
			a1 = atan2(m_p1->m_p.Y()-pnt.Y(),m_p1->m_p.X()-pnt.X());

		double l2d1 = m_p2->m_p.Distance(pnt);
		double l2d2 = m_p3->m_p.Distance(pnt);

		double a2=0;

		//Avoid singularity and instabillity
		if(l2d1 > l2d2)
			a2 = atan2(m_p2->m_p.Y()-pnt.Y(),m_p2->m_p.X()-pnt.X());
		else
			a2 = atan2(m_p3->m_p.Y()-pnt.Y(),m_p3->m_p.X()-pnt.X());


		//Figure out if which way the lines should be pointing
		while(a1 - ca > M_PI)
			a1 -= M_PI;
		while(a1 - ca < -M_PI)
			a1 += M_PI;
		while(a2 - ca > M_PI)
			a2 -= M_PI;
		while(a2 - ca < -M_PI)
			a2 += M_PI;


		//Need to find DA and compensate for the 2*Pi period of atan2
		double da = a2 - a1;
		while(da > M_PI)
			da -= 2*M_PI;
		while(da < -M_PI)
			da += 2*M_PI;

		// double ma = a1 + da/2;

		//Project the line to our circle
		gp_Pnt pnt2(cos(a1)*r + pnt.X(),sin(a1)*r + pnt.Y(),0);

		if(m_p0->m_p.Distance(pnt2) < m_p1->m_p.Distance(pnt2))
			DrawLine(m_p0->m_p,pnt2);
		else
			DrawLine(m_p1->m_p,pnt2);

		//Project the line to out circle

		pnt2 = gp_Pnt(cos(a2)*r + pnt.X(),sin(a2)*r + pnt.Y(),0);

		if(m_p2->m_p.Distance(pnt2) < m_p3->m_p.Distance(pnt2))
			DrawLine(m_p2->m_p,pnt2);
		else
			DrawLine(m_p3->m_p,pnt2);

		//Figure out if the control point is inside the included part

		if((ca >= a1 && ca <= a1 + da) || (ca <= a1 && ca >= a1 + da))
		{
			DrawArc(pnt,r,a1,a1+da);
		}
		else
		{
			double offset = da > 0? M_PI/20 : -M_PI/20;
			if((ca < a1) ^ (da < 0))
			{
				DrawArc(pnt,r,ca,a1);
				DrawArc(pnt,r,a1+da,a1+da+offset);
			}
			else
			{
				DrawArc(pnt,r,a1+da,ca);
				DrawArc(pnt,r,a1,a1-offset);
			}
		}

		wxString text = MakeText(da);

		HDimension::RenderText(text, m_p4->m_p, gp_Dir(1, 0, 0), gp_Dir(0, 1, 0), m_scale);
	}
}

void HAngularDimension::DrawLine(gp_Pnt p1, gp_Pnt p2)
{
	if (wxGetApp().m_allow_opengl_stippling)
	{
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(3, 0xaaaa);
	}

	glBegin(GL_LINES);
	glVertex2d(p1.X(),p1.Y());
	glVertex2d(p2.X(),p2.Y());
	glEnd();
	if (wxGetApp().m_allow_opengl_stippling)
	{
		glDisable(GL_LINE_STIPPLE);
	}
}

void HAngularDimension::DrawArc(gp_Pnt center, double r, double a1, double a2)
{
	if (wxGetApp().m_allow_opengl_stippling)
	{
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(3, 0xaaaa);
	}

	glBegin(GL_LINE_STRIP);
	double da = a2 - a1;
	for(int i=0; i < 100; i++)
	{
		glVertex2d(center.X() + cos(a1 + da*i/100.0)*r,center.Y() + sin(a1 + da*i/100.0)*r);
	}
	glEnd();
	if (wxGetApp().m_allow_opengl_stippling)
	{
		glDisable(GL_LINE_STIPPLE);
	}
}

#ifdef MULTIPLE_OWNERS
void HAngularDimension::LoadToDoubles()
{
	m_p0->LoadToDoubles();
	m_p1->LoadToDoubles();
	m_p2->LoadToDoubles();
	m_p3->LoadToDoubles();
	m_p4->LoadToDoubles();
}

void HAngularDimension::LoadFromDoubles()
{
	m_p0->LoadFromDoubles();
	m_p1->LoadFromDoubles();
	m_p2->LoadFromDoubles();
	m_p3->LoadFromDoubles();
	m_p4->LoadFromDoubles();
}
#endif

HAngularDimension* angular_dimension_for_tool = NULL;

void HAngularDimension::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	angular_dimension_for_tool = this;
}

void HAngularDimension::GetBox(CBox &box)
{
	double p[3];

	float width, height;
	if(!wxGetApp().get_text_size(m_text, &width, &height))return;

	gp_Pnt point[3];
	point[0] = gp_Pnt(width, 0, 0);
	point[1] = gp_Pnt(0, -height, 0);
	point[2] = gp_Pnt(width, -height, 0);

	for(int i = 0; i<3; i++)
	{
		extract(point[i], p);
		box.Insert(p);
	}
}

HeeksObj *HAngularDimension::MakeACopy(void)const
{
	return new HAngularDimension(*this);
}

void HAngularDimension::ModifyByMatrix(const double *m)
{
	gp_Trsf mat = make_matrix(m);
}

void HAngularDimension::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	float width, height;
	if(!wxGetApp().get_text_size(m_text, &width, &height))return;

	gp_Pnt point[4];
	point[0] = gp_Pnt(0, 0, 0);
	point[1] = gp_Pnt(width, 0, 0);
	point[2] = gp_Pnt(0, -height, 0);
	point[3] = gp_Pnt(width, -height, 0);


	//list->push_back(GripData(GripperTypeTranslate,point[0].X(),point[0].Y(),point[0].Z(),NULL));
	//list->push_back(GripData(GripperTypeRotateObject,point[1].X(),point[1].Y(),point[1].Z(),NULL));
	//list->push_back(GripData(GripperTypeRotateObject,point[2].X(),point[2].Y(),point[2].Z(),NULL));
	//list->push_back(GripData(GripperTypeScale,point[3].X(),point[3].Y(),point[3].Z(),NULL));

	HeeksObj::GetGripperPositions(list,just_for_endof);
	list->push_back(GripData(GripperTypeStretch,m_p4->m_p.X(),m_p4->m_p.Y(),m_p4->m_p.Z(),&m_p4));
}

static void on_set_text_mode(int value, HeeksObj* object)
{
	HAngularDimension* dimension = (HAngularDimension*)object;
	dimension->m_text_mode = (AngularDimensionTextMode)value;
	wxGetApp().Repaint();
}

static void on_set_scale(double value, HeeksObj* object)
{
	HAngularDimension* dimension = (HAngularDimension*)object;
	dimension->m_scale = value;
	wxGetApp().Repaint();
}


void HAngularDimension::GetProperties(std::list<Property *> *list)
{
	std::list< wxString > choices;

	choices.clear();
	choices.push_back ( wxString ( _("string") ) );
	choices.push_back ( wxString ( _("pythagorean") ) );
	choices.push_back ( wxString ( _("horizontal") ) );
	choices.push_back ( wxString ( _("vertical") ) );
	list->push_back ( new PropertyChoice ( _("text mode"),  choices, m_text_mode, this, on_set_text_mode ) );

	list->push_back ( new PropertyDouble ( _("scale"),  m_scale, this, on_set_scale ) );

#ifdef MULTIPLE_OWNERS
	ObjList::GetProperties(list);
#else
	HeeksObj::GetProperties(list);
#endif
}

bool HAngularDimension::Stretch(const double *p, const double* shift, void* data)
{
#ifdef MULTIPLE_OWNERS
	ObjList::Stretch(p,shift,data);
#else
	HeeksObj::Stretch(p,shift,data);
#endif
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	if(data == &m_p4){
		m_p4->m_p = vp.XYZ() + vshift.XYZ();
	}
	return false;
}

void HAngularDimension::OnEditString(const wxChar* str){
	m_text.assign(str);
	wxGetApp().Changed();
}

void HAngularDimension::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "AngularDimension" );
	root->LinkEndChild( element );
	element->SetAttribute("text", m_text.utf8_str());

	element->SetAttribute("col", m_color.COLORREF_color());
	element->SetDoubleAttribute("scale",m_scale);
	element->SetAttribute("textmode", m_text_mode);

	WriteBaseXML(element);
}

// static
HeeksObj* HAngularDimension::ReadFromXMLElement(TiXmlElement* pElem)
{
	wxString text;
	HeeksColor c;
	double scale=1;

	AngularDimensionTextMode text_mode = StringAngularDimensionTextMode;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){c = HeeksColor((long)(a->IntValue()));}
		else if(name == "text"){text.assign(Ctt(a->Value()));}
		else if(name == "scale"){scale= a->DoubleValue();}
		else if(name == "textmode"){text_mode = (AngularDimensionTextMode)(a->IntValue());}
	}

	HAngularDimension* new_object = new HAngularDimension(text, text_mode, &c);
	new_object->ReadBaseXML(pElem);
	new_object->m_scale = scale;
	new_object->ReloadPointers();

	return new_object;
}

void HAngularDimension::ReloadPointers()
{
	m_p0 = (HPoint*)GetFirstChild();
	m_p1 = (HPoint*)GetNextChild();
	m_p2 = (HPoint*)GetNextChild();
	m_p3 = (HPoint*)GetNextChild();
	m_p4 = (HPoint*)GetNextChild();

#ifdef MULTIPLE_OWNERS
	ObjList::ReloadPointers();
#else
	HeeksObj::ReloadPointers();
#endif
}

wxString HAngularDimension::MakeText(double angle)
{
	wxString text;

	switch(m_text_mode)
	{
		case StringAngularDimensionTextMode:
			text = wxString::Format(_T("%s"), m_text.c_str());
			break;
		case DegreesAngularDimensionTextMode:
			text = wxString::Format(_T("%lg degrees"), angle * 180/M_PI);
			break;
		case RadiansAngularDimensionTextMode:
			text = wxString::Format(_T("%lg radians"), angle);
			break;
	}

	return text;
}
