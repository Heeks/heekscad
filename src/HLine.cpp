// HLine.cpp
#include "stdafx.h"

#include "HLine.h"
#include "HILine.h"
#include "HCircle.h"
#include "HArc.h"
#include "../interface/PropertyDouble.h"
#include "PropertyVertex.h"
#include "../tinyxml/tinyxml.h"

wxIcon* HLine::m_icon = NULL;

HLine::HLine(const HLine &line){
	operator=(line);
}

HLine::HLine(const gp_Pnt &a, const gp_Pnt &b, const HeeksColor* col){
	A = a;
	B = b;
	color = *col;
}

HLine::~HLine(){
}

const HLine& HLine::operator=(const HLine &b){
	HeeksObj::operator=(b);
	A = b.A;
	B = b.B;
	color = b.color;
	return *this;
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
}

HeeksObj *HLine::MakeACopy(void)const{
		HLine *new_object = new HLine(*this);
		return new_object;
}

void HLine::ModifyByMatrix(const double* m, bool for_undo){
	gp_Trsf mat = make_matrix(m);
	A.Transform(mat);
	B.Transform(mat);
}

wxIcon* HLine::GetIcon(){
	if(m_icon == NULL)
	{
		wxString exe_folder = wxGetApp().GetExeFolder();
		m_icon = new wxIcon(exe_folder + _T("/icons/line.png"), wxBITMAP_TYPE_PNG);
	}
	return m_icon;
}

void HLine::GetBox(CBox &box){
	box.Insert(A.X(), A.Y(), A.Z());
	box.Insert(B.X(), B.Y(), B.Z());
}

void HLine::GetGripperPositions(std::list<double> *list, bool just_for_endof){
	list->push_back(0);
	list->push_back(A.X());
	list->push_back(A.Y());
	list->push_back(A.Z());
	list->push_back(0);
	list->push_back(B.X());
	list->push_back(B.Y());
	list->push_back(B.Z());
}

static void on_set_start(const gp_Pnt &vt, HeeksObj* object){
	((HLine*)object)->A = vt;
	wxGetApp().Repaint();
}

static void on_set_end(const gp_Pnt &vt, HeeksObj* object){
	((HLine*)object)->B = vt;
	wxGetApp().Repaint();
}

void HLine::GetProperties(std::list<Property *> *list){
	__super::GetProperties(list);

	list->push_back(new PropertyVertex(_T("start"), A, this, on_set_start));
	list->push_back(new PropertyVertex(_T("end"), B, this, on_set_end));
	double length = A.Distance(B);
	list->push_back(new PropertyDouble(_T("Length"), length, NULL));
}

bool HLine::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
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

void HLine::Stretch(const double *p, const double* shift, double* new_position){
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	if(A.IsEqual(vp, wxGetApp().m_geom_tol)){
		A = A.XYZ() + vshift.XYZ();
		extract(A, new_position);
	}
	else if(B.IsEqual(vp, wxGetApp().m_geom_tol)){
		B = B.XYZ() + vshift.XYZ();
		extract(B, new_position);
	}
}

gp_Lin HLine::GetLine()const{
	gp_Vec v(A, B);
	return gp_Lin(A, v);
}

int HLine::Intersects(const HeeksObj *object, std::list< double > *rl)const{
	int numi = 0;

	switch(object->GetType())
	{
	case LineType:
		{
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
			intersect(GetLine(), ((HArc*)object)->m_circle, plist);
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
			intersect(GetLine(), ((HCircle*)object)->m_circle, plist);
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
	gp_Lin& this_line = GetLine();
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

bool HLine::GetStartPoint(double* pos)
{
	extract(A, pos);
	return true;
}

bool HLine::GetEndPoint(double* pos)
{
	extract(B, pos);
	return true;
}

gp_Vec HLine::GetSegmentVector(double fraction)
{
	gp_Vec line_vector(A, B);
	if(line_vector.Magnitude() < 0.000000001)return gp_Vec(0, 0, 0);
	return gp_Vec(A, B).Normalized();
}

void HLine::WriteXML(TiXmlElement *root)
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
	gp_Pnt p0, p1;
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

	return new_object;
}