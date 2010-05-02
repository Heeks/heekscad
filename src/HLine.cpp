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
#include "SolveSketch.h"

HLine::HLine(const HLine &line):EndedObject(&line.color){
	operator=(line);
}

HLine::HLine(const gp_Pnt &a, const gp_Pnt &b, const HeeksColor* col):EndedObject(col){
	A->m_p = a;
	B->m_p = b;
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

class SetLineHorizontal:public Tool{
public:
	void Run(){
		line_for_tool->SetAbsoluteAngleConstraint(AbsoluteAngleHorizontal);
		SolveSketch((CSketch*)line_for_tool->Owner());
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Toggle Horizontal");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set this line to be horizontal");}
};
static SetLineHorizontal horizontal_line_toggle;

class SetLineVertical:public Tool{
public:
	void Run(){
		line_for_tool->SetAbsoluteAngleConstraint(AbsoluteAngleVertical);
		SolveSketch((CSketch*)line_for_tool->Owner());
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Toggle Vertical");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set this line to be vertical");}
};
static SetLineVertical vertical_line_toggle;

class SetLineLength:public Tool{
public:
	void Run(){
		line_for_tool->SetLineLengthConstraint(line_for_tool->A->m_p.Distance(line_for_tool->B->m_p));
		SolveSketch((CSketch*)line_for_tool->Owner());
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Toggle Line Length");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Set this lines length as constrained");}
};
static SetLineLength line_length_toggle;

const wxBitmap &HLine::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/line.png")));
	return *icon;
}

void HLine::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	line_for_tool = this;
	t_list->push_back(&horizontal_line_toggle);
	t_list->push_back(&vertical_line_toggle);
	t_list->push_back(&line_length_toggle);
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
	glVertex3d(A->m_p.X(), A->m_p.Y(), A->m_p.Z());
	glVertex3d(B->m_p.X(), B->m_p.Y(), B->m_p.Z());
	glEnd();
	if(marked){
		glLineWidth(1);
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}

	if(!A->m_p.IsEqual(B->m_p, wxGetApp().m_geom_tol))
	{
		gp_Pnt mid_point = A->m_p.XYZ() + (B->m_p.XYZ() - A->m_p.XYZ())/2;
		gp_Dir dir = B->m_p.XYZ() - mid_point.XYZ();
		gp_Ax1 ax(mid_point,dir);
		//gp_Dir up(0,0,1);
		//ax.Rotate(gp_Ax1(mid_point,up),Pi/2);
		ConstrainedObject::glCommands(color,ax);
	}

	EndedObject::glCommands(select,marked,no_color);
}

void HLine::Draw(wxDC& dc)
{
	wxGetApp().PlotSetColor(color);
	double s[3], e[3];
	extract(A->m_p, s);
	extract(B->m_p, e);
	wxGetApp().PlotLine(s, e);
}

HeeksObj *HLine::MakeACopy(void)const{
	HLine *new_object = new HLine(*this);
	return new_object;
}

void HLine::GetBox(CBox &box){
	box.Insert(A->m_p.X(), A->m_p.Y(), A->m_p.Z());
	box.Insert(B->m_p.X(), B->m_p.Y(), B->m_p.Z());
}

void HLine::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	EndedObject::GetGripperPositions(list,just_for_endof);
}

static void on_set_start(const double *vt, HeeksObj* object){
	((HLine*)object)->A->m_p = make_point(vt);
	wxGetApp().Repaint();
}

static void on_set_end(const double *vt, HeeksObj* object){
	((HLine*)object)->B->m_p = make_point(vt);
	wxGetApp().Repaint();
}

static void on_set_length(double v, HeeksObj* object){
	((HLine*)object)->SetLineLength(v);
	if(wxGetApp().autosolve_constraints)
		SolveSketch((CSketch*)object->Owner());
	wxGetApp().Repaint();
}

void HLine::GetProperties(std::list<Property *> *list){
	double a[3], b[3];
	extract(A->m_p, a);
	extract(B->m_p, b);
	list->push_back(new PropertyVertex(_("start"), a, this, on_set_start));
	list->push_back(new PropertyVertex(_("end"), b, this, on_set_end));
	double length = A->m_p.Distance(B->m_p);
	list->push_back(new PropertyLength(_("Length"), length, this, on_set_length));

	HeeksObj::GetProperties(list);
}

bool HLine::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
	// The OpenCascade libraries throw an exception when one tries to
	// create a gp_Lin() object using a vector that doesn't point
	// anywhere.  If this is a zero-length line then we're in
	// trouble.  Don't bother with it.
	if ((A->m_p.X() == B->m_p.X()) &&
	    (A->m_p.Y() == B->m_p.Y()) &&
	    (A->m_p.Z() == B->m_p.Z())) return(false);

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
	gp_Vec v(A->m_p, B->m_p);
	return gp_Lin(A->m_p, v);
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
			if ((A->m_p.X() == B->m_p.X()) &&
			    (A->m_p.Y() == B->m_p.Y()) &&
			    (A->m_p.Z() == B->m_p.Z())) break;

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
	double dpA = gp_Vec(A->m_p.XYZ()) * v;
	double dpB = gp_Vec(B->m_p.XYZ()) * v;
	double dp = gp_Vec(pnt.XYZ()) * v;
	return dp >= dpA - wxGetApp().m_geom_tol && dp <= dpB + wxGetApp().m_geom_tol;
}

void HLine::GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point)const{
	if(want_start_point)
	{
		double p[3];
		extract(A->m_p, p);
		(*callbackfunc)(p);
	}

	double p[3];
	extract(B->m_p, p);
	(*callbackfunc)(p);
}

gp_Vec HLine::GetSegmentVector(double fraction)
{
	gp_Vec line_vector(A->m_p, B->m_p);
	if(line_vector.Magnitude() < 0.000000001)return gp_Vec(0, 0, 0);
	return gp_Vec(A->m_p, B->m_p).Normalized();
}

void HLine::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Line" );
	root->LinkEndChild( element );
	element->SetAttribute("col", color.COLORREF_color());

#ifdef OLDLINES
	element->SetDoubleAttribute("sx", A->m_p.X());
	element->SetDoubleAttribute("sy", A->m_p.Y());
	element->SetDoubleAttribute("sz", A->m_p.Z());
	element->SetDoubleAttribute("ex", B->m_p.X());
	element->SetDoubleAttribute("ey", B->m_p.Y());
	element->SetDoubleAttribute("ez", B->m_p.Z());
#endif
	WriteBaseXML(element);
}

// static member function
HeeksObj* HLine::ReadFromXMLElement(TiXmlElement* pElem)
{
	gp_Pnt p0(0, 0, 0), p1(0, 0, 0);
	HeeksColor c;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){c = HeeksColor(a->IntValue());}
		else if(name == "sx"){p0.SetX(a->DoubleValue());}
		else if(name == "sy"){p0.SetY(a->DoubleValue());}
		else if(name == "sz"){p0.SetZ(a->DoubleValue());}
		else if(name == "ex"){p1.SetX(a->DoubleValue());}
		else if(name == "ey"){p1.SetY(a->DoubleValue());}
		else if(name == "ez"){p1.SetZ(a->DoubleValue());}
	}

	HLine* new_object = new HLine(p0, p1, &c);
	new_object->ReadBaseXML(pElem);

	if(new_object->GetNumChildren()>2)
	{
		//This is a new style line, with children points
		new_object->Remove(new_object->A);
		new_object->Remove(new_object->B);
		delete new_object->A;
		delete new_object->B;
		new_object->A = (HPoint*)new_object->GetFirstChild();
		new_object->B = (HPoint*)new_object->GetNextChild();
		new_object->A->m_draw_unselected = false;
		new_object->B->m_draw_unselected = false;
		new_object->A->SetSkipForUndo(true);
		new_object->B->SetSkipForUndo(true);
	}

	// The OpenCascade libraries throw an exception when one tries to
	// create a gp_Lin() object using a vector that doesn't point
	// anywhere.  If this is a zero-length line then we're in
	// trouble.  Don't bother with it.
	if (new_object->A == NULL || new_object->B == NULL || ((new_object->A->m_p.X() == new_object->B->m_p.X()) &&
		(new_object->A->m_p.Y() == new_object->B->m_p.Y()) &&
		(new_object->A->m_p.Z() == new_object->B->m_p.Z())))
	{
		delete new_object;
		return(NULL);
	}

	return new_object;
}

void HLine::Reverse()
{
	HPoint* temp = A;
	A = B;
	B = temp;
	m_objects.pop_front();
	m_objects.pop_front();
	m_objects.push_front(B);
	m_objects.push_front(A);
}

