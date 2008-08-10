// HLine.cpp
#include "stdafx.h"

#include "HLine.h"
#include "../interface/PropertyDouble.h"
#include "PropertyVertex.h"
#include "../tinyxml/tinyxml.h"

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
	HGeomObject::operator=(b);
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

void HLine::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	A.Transform(mat);
	B.Transform(mat);
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

HLine* line_for_properties = NULL;
void on_set_start(gp_Pnt &vt){
	line_for_properties->A = vt;
	wxGetApp().Repaint();
}

void on_set_end(gp_Pnt &vt){
	line_for_properties->B = vt;
	wxGetApp().Repaint();
}

void HLine::GetProperties(std::list<Property *> *list){
	__super::GetProperties(list);

	line_for_properties = this;
	list->push_back(new PropertyVertex("start", A, on_set_start));
	list->push_back(new PropertyVertex("end", B, on_set_end));
	double length = A.Distance(B);
	list->push_back(new PropertyDouble("Length", length, NULL));
}

bool HLine::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
	// to do, this only works for exact intersection
	gp_Lin ray(make_point(ray_start), make_vector(ray_direction));
	std::list< double > rl;
	if(Intersects(ray, &rl)>0){
		std::list< double >::iterator It = rl.begin();
		point[0] = *It;It++;
		point[1] = *It;It++;
		point[2] = *It;
		return true;
	}

	return false;
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
	if(object->GetType() == LineType){
		return ((HLine*)object)->Intersects(GetLine(), rl);
	}
	return 0;
}

int HLine::Intersects(const gp_Lin &line, std::list< double > *rl)const{
	std::list<gp_Pnt> rlist;
	gp_Lin this_line = GetLine();
	intersect(this_line, line, rlist);
	gp_Vec v = this_line.Direction();
	double dpA = gp_Vec(A.XYZ()) * v;
	double dpB = gp_Vec(B.XYZ()) * v;
	int num = 0;
	if(rlist.size() > 0){
		// check if they lie between A and B
		std::list<gp_Pnt>::iterator It;
		for(It = rlist.begin(); It != rlist.end(); It++){
			gp_Pnt &p = *It;
			double dp = gp_Vec(p.XYZ()) * v;
			if(dp >= dpA - wxGetApp().m_geom_tol && dp <= dpB + wxGetApp().m_geom_tol)
			{
				rl->push_back(p.X());
				rl->push_back(p.Y());
				rl->push_back(p.Z());
				num++;
			}
		}
	}

	return num;
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
}

// static member function
HeeksObj* HLine::ReadFromXMLElement(TiXmlElement* pElem)
{
	gp_Pnt p0, p1;
	HeeksColor c;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		wxString name(a->Name());
		if(name == "col"){c = HeeksColor(a->IntValue());}
		else if(name == "sx"){p0.SetX(a->DoubleValue());}
		else if(name == "sy"){p0.SetY(a->DoubleValue());}
		else if(name == "sz"){p0.SetZ(a->DoubleValue());}
		else if(name == "ex"){p1.SetX(a->DoubleValue());}
		else if(name == "ey"){p1.SetY(a->DoubleValue());}
		else if(name == "ez"){p1.SetZ(a->DoubleValue());}
	}

	return new HLine(p0, p1, &c);
}