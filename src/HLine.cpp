// HLine.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HLine.h"
#include "HILine.h"
#include "HCircle.h"
#include "HArc.h"
#include "../interface/Tool.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "../interface/PropertyVertex.h"
#include "Gripper.h"
#include "Sketch.h"
#include "Cylinder.h"
#include "Cone.h"
#include "DigitizeMode.h"
#include "Drawing.h"
#include "HPoint.h"

HLine::HLine(const HLine &line):EndedObject(&line.color){
	operator=(line);
}

HLine::HLine(const gp_Pnt &a, const gp_Pnt &b, const HeeksColor* col):EndedObject(col){
	A = a;
	B = b;
	color = *col;
}

HLine::~HLine(){
}

const HLine& HLine::operator=(const HLine &b){
	EndedObject::operator=(b);
	color = b.color;
	return *this;
}

HLine* line_for_tool = NULL;

class MakeCylinderOnLine:public Tool{
public:
	void Run(){
		gp_Vec v(line_for_tool->A, line_for_tool->B);
		CCylinder* new_object = new CCylinder(gp_Ax2(line_for_tool->A, v), 1.0, v.Magnitude(), _("Cylinder"), HeeksColor(191, 191, 240), 1.0f);
		wxGetApp().StartHistory();
		wxGetApp().AddUndoably(new_object,NULL,NULL);
		wxGetApp().EndHistory();
	}
	const wxChar* GetTitle(){return _("Make Cylinder On Line");}
	wxString BitmapPath(){return _T("cylonlin");}
};
static MakeCylinderOnLine make_cylinder_on_line;

class MakeConeOnLine:public Tool{
public:
	void Run(){
		gp_Vec v(line_for_tool->A, line_for_tool->B);
		CCone* new_object = new CCone(gp_Ax2(line_for_tool->A, v), 2.0, 1.0, v.Magnitude(), _("Cone"), HeeksColor(240, 240, 191), 1.0f);
		wxGetApp().StartHistory();
		wxGetApp().AddUndoably(new_object,NULL,NULL);
		wxGetApp().EndHistory();
	}
	const wxChar* GetTitle(){return _("Make Cone On Line");}
	wxString BitmapPath(){return _T("coneonlin");}
};
static MakeConeOnLine make_cone_on_line;


class ClickMidpointOnLine:public Tool{
public:
	void Run(){
		gp_Pnt midpoint((line_for_tool->A.XYZ() + line_for_tool->B.XYZ()) /2);

		wxGetApp().m_digitizing->digitized_point = DigitizedPoint(midpoint, DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}
	const wxChar* GetTitle(){return _("Click Midpoint On Line");}
	wxString BitmapPath(){return _T("click_line_midpoint");}
};
static ClickMidpointOnLine click_midpoint_on_line;


class ClickStartPointOnLine:public Tool{
public:
	void Run(){
		wxGetApp().m_digitizing->digitized_point = DigitizedPoint(line_for_tool->A, DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}
	const wxChar* GetTitle(){return _("Click Start Of Line");}
	wxString BitmapPath(){return _T("click_line_end_one");}
};
static ClickStartPointOnLine click_start_point_on_line;


class ClickEndPointOnLine:public Tool{
public:
	void Run(){
		wxGetApp().m_digitizing->digitized_point = DigitizedPoint(line_for_tool->B, DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}
	const wxChar* GetTitle(){return _("Click End Of Line");}
	wxString BitmapPath(){return _T("click_line_end_two");}
};
static ClickEndPointOnLine click_end_point_on_line;



const wxBitmap &HLine::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/line.png")));
	return *icon;
}

bool HLine::GetMidPoint(double* pos)
{
	extract((A.XYZ() + B.XYZ())/2, pos);
	return true;
}

void HLine::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	line_for_tool = this;
	t_list->push_back(&make_cylinder_on_line);
	t_list->push_back(&make_cone_on_line);

	Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
	if (pDrawingMode != NULL)
	{
		t_list->push_back(&click_start_point_on_line);
		t_list->push_back(&click_midpoint_on_line);
		t_list->push_back(&click_end_point_on_line);
	}
}

void HLine::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		wxGetApp().glColorEnsuringContrast(color);
	}
	GLfloat save_depth_range[2];
	if(marked){
		glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
		glDepthRange(0, 0);
		glLineWidth(2);
	}
	glBegin(GL_LINES);
	glVertex3d(A.X(), A.Y(), A.Z());
	glVertex3d(B.X(), B.Y(), B.Z());
	glEnd();
	if(marked){
		glLineWidth(1);
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}

	EndedObject::glCommands(select,marked,no_color);
}

void HLine::Draw(wxDC& dc)
{
	wxGetApp().PlotSetColor(color);
	double s[3], e[3];
	extract(A, s);
	extract(B, e);
	wxGetApp().PlotLine(s, e);
}

HeeksObj *HLine::MakeACopy(void)const{
	HLine *new_object = new HLine(*this);
	return new_object;
}

void HLine::GetBox(CBox &box){
	box.Insert(A.X(), A.Y(), A.Z());
	box.Insert(B.X(), B.Y(), B.Z());
}

void HLine::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	EndedObject::GetGripperPositions(list,just_for_endof);
}

static void on_set_start(const double *vt, HeeksObj* object){
	((HLine*)object)->A = make_point(vt);
	wxGetApp().Repaint();
}

static void on_set_end(const double *vt, HeeksObj* object){
	((HLine*)object)->B = make_point(vt);
	wxGetApp().Repaint();
}

void HLine::GetProperties(std::list<Property *> *list){
	double a[3], b[3];
	extract(A, a);
	extract(B, b);
	list->push_back(new PropertyVertex(_("start"), a, this, on_set_start));
	list->push_back(new PropertyVertex(_("end"), b, this, on_set_end));
	double length = A.Distance(B);
	list->push_back(new PropertyLength(_("Length"), length, this, NULL));

	HeeksObj::GetProperties(list);
}

bool HLine::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
	// The OpenCascade libraries throw an exception when one tries to
	// create a gp_Lin() object using a vector that doesn't point
	// anywhere.  If this is a zero-length line then we're in
	// trouble.  Don't bother with it.
	if ((A.X() == B.X()) &&
	    (A.Y() == B.Y()) &&
	    (A.Z() == B.Z())) return(false);

	gp_Lin ray(make_point(ray_start), make_vector(ray_direction));
	gp_Pnt p1, p2;
	ClosestPointsOnLines(GetLine(), ray, p1, p2);

	if(!Intersects(p1))
		return false;

	extract(p1, point);
	return true;
}

bool HLine::FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point){
	// any point on this line is a possible tangent point
	return FindNearPoint(ray_start, ray_direction, point);
}

gp_Lin HLine::GetLine()const{
	gp_Vec v(A, B);
	return gp_Lin(A, v);
}

int HLine::Intersects(const HeeksObj *object, std::list< double > *rl)const{
	int numi = 0;

	switch(object->GetType())
	{
    case SketchType:
        return( ((CSketch *)object)->Intersects( this, rl ));

	case LineType:
		{
			// The OpenCascade libraries throw an exception when one tries to
			// create a gp_Lin() object using a vector that doesn't point
			// anywhere.  If this is a zero-length line then we're in
			// trouble.  Don't bother with it.
			if ((A.X() == B.X()) &&
			    (A.Y() == B.Y()) &&
			    (A.Z() == B.Z())) break;

			gp_Pnt pnt;
			if(intersect(GetLine(), ((HLine*)object)->GetLine(), pnt))
			{
				if(Intersects(pnt) && ((HLine*)object)->Intersects(pnt)){
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case ILineType:
		{
			gp_Pnt pnt;
			if(intersect(GetLine(), ((HILine*)object)->GetLine(), pnt))
			{
				if(Intersects(pnt)){
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case ArcType:
		{
			std::list<gp_Pnt> plist;
			intersect(GetLine(), ((HArc*)object)->GetCircle(), plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(Intersects(pnt) && ((HArc*)object)->Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case CircleType:
		{
			std::list<gp_Pnt> plist;
			intersect(GetLine(), ((HCircle*)object)->GetCircle(), plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;
	}

	return numi;
}

bool HLine::Intersects(const gp_Pnt &pnt)const
{
	gp_Lin this_line = GetLine();
	if(!intersect(pnt, this_line))return false;

	// check it lies between A and B
	gp_Vec v = this_line.Direction();
	double dpA = gp_Vec(A.XYZ()) * v;
	double dpB = gp_Vec(B.XYZ()) * v;
	double dp = gp_Vec(pnt.XYZ()) * v;
	return dp >= dpA - wxGetApp().m_geom_tol && dp <= dpB + wxGetApp().m_geom_tol;
}

void HLine::GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point)const{
	if(want_start_point)
	{
		double p[3];
		extract(A, p);
		(*callbackfunc)(p);
	}

	double p[3];
	extract(B, p);
	(*callbackfunc)(p);
}

gp_Vec HLine::GetSegmentVector(double fraction)
{
	gp_Vec line_vector(A, B);
	if(line_vector.Magnitude() < 0.000000001)return gp_Vec(0, 0, 0);
	return gp_Vec(A, B).Normalized();
}

void HLine::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Line" );
	root->LinkEndChild( element );
	element->SetAttribute("col", color.COLORREF_color());
	element->SetDoubleAttribute("sx", A.X());
	element->SetDoubleAttribute("sy", A.Y());
	element->SetDoubleAttribute("sz", A.Z());
	element->SetDoubleAttribute("ex", B.X());
	element->SetDoubleAttribute("ey", B.Y());
	element->SetDoubleAttribute("ez", B.Z());
	WriteBaseXML(element);
}

// static member function
HeeksObj* HLine::ReadFromXMLElement(TiXmlElement* pElem)
{
	gp_Pnt p0(0, 0, 0), p1(0, 0, 0);
	HeeksColor c;

	// get the attributes
	int att_col;
	double x;
	if(pElem->Attribute("col", &att_col))c = HeeksColor((long)att_col);
	if(pElem->Attribute("sx", &x))p0.SetX(x);
	if(pElem->Attribute("sy", &x))p0.SetY(x);
	if(pElem->Attribute("sz", &x))p0.SetZ(x);
	if(pElem->Attribute("ex", &x))p1.SetX(x);
	if(pElem->Attribute("ey", &x))p1.SetY(x);
	if(pElem->Attribute("ez", &x))p1.SetZ(x);

	else
	{
		// try the version where the points were children
		bool p0_found = false;
		for(TiXmlElement* pElem2 = TiXmlHandle(pElem).FirstChildElement().Element(); pElem2;	pElem2 = pElem2->NextSiblingElement())
		{
			HeeksObj* object = wxGetApp().ReadXMLElement(pElem2);
			if(object->GetType() == PointType)
			{
				if(!p0_found)
				{
					p0 = ((HPoint*)object)->m_p;
					p0_found = true;
				}
				else
				{
					p1 = ((HPoint*)object)->m_p;
					delete object;
					break;
				}
			}
			delete object;
		}
	}

	HLine* new_object = new HLine(p0, p1, &c);
	new_object->ReadBaseXML(pElem);

	// The OpenCascade libraries throw an exception when one tries to
	// create a gp_Lin() object using a vector that doesn't point
	// anywhere.  If this is a zero-length line then we're in
	// trouble.  Don't bother with it.
	if ((new_object->A.X() == new_object->B.X()) &&
		(new_object->A.Y() == new_object->B.Y()) &&
		(new_object->A.Z() == new_object->B.Z()))
	{
		delete new_object;
		return(NULL);
	}

	return new_object;
}

void HLine::Reverse()
{
	gp_Pnt temp = A;
	A = B;
	B = temp;
}

