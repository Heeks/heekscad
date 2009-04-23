// HSpline.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HSpline.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyChoice.h"
#include "../tinyxml/tinyxml.h"
#include "../interface/PropertyVertex.h"
#include "HLine.h"
#include "HILine.h"
#include "HArc.h"
#include "Gripper.h"
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>

HSpline::HSpline(const HSpline &s){
	operator=(s);
}

HSpline::HSpline(const Geom_BSplineCurve &s, const HeeksColor* col):color(*col){
	m_spline = Handle(Geom_BSplineCurve)::DownCast(s.Copy());	
}

HSpline::~HSpline(){
}

const HSpline& HSpline::operator=(const HSpline &s){
	HeeksObj::operator=(s);
	m_spline = Handle(Geom_BSplineCurve)::DownCast((s.m_spline)->Copy());;
	color = s.color;
	return *this;
}



//segments - number of segments per full revolution!
void HSpline::GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point)const
{

	//TODO: calculate length
	double length = 10;
    int segments = (int)(fabs(pixels_per_mm * length + 1));
	double pp[3];

	for(int i = 0; i <= segments; i++)
    {
		gp_Pnt p;
		m_spline->D0(i / (double)segments,p);
		extract(p, pp);
		(*callbackfunc)(pp);
    } 
}

static void glVertexFunction(const double *p){glVertex3d(p[0], p[1], p[2]);}

void HSpline::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		wxGetApp().glColorEnsuringContrast(color);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(3, 0xaaaa);
	}
	GLfloat save_depth_range[2];
	if(marked){
		glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
		glDepthRange(0, 0);
		glLineWidth(2);
	}

	glBegin(GL_LINE_STRIP);
	GetSegments(glVertexFunction, wxGetApp().GetPixelScale());
	glEnd();

	if(marked){
		glLineWidth(1);
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}
	if(!no_color)
	{
		glDisable(GL_LINE_STIPPLE);
	}
}

HeeksObj *HSpline::MakeACopy(void)const{
		HSpline *new_object = new HSpline(*this);
		return new_object;
}

bool HSpline::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	m_spline->Transform(mat);
	return false;
}

void HSpline::GetBox(CBox &box){
	//TODO: get rid of magic number
	double pp[3];
	for(int i = 0; i <= 100; i++)
    {
		gp_Pnt p;
		m_spline->D0(i * .01,p);
		extract(p, pp);
		box.Insert(pp);
    } 
}

void HSpline::GetGripperPositions(std::list<double> *list, bool just_for_endof){
	if(!just_for_endof)
	{
		for(int i=1; i <= m_spline->NbPoles(); i++)
		{
			gp_Pnt pole = m_spline->Pole(i);
			list->push_back(GripperTypeStretch);
			list->push_back(pole.X());
			list->push_back(pole.Y());
			list->push_back(pole.Z());
		}
	} 
}

void OnSetKnot(double v, HeeksObj* obj, int i)
{
	((HSpline*)obj)->m_spline->SetKnot(i,v);
}

void OnSetWeight(double v, HeeksObj* obj, int i)
{
	((HSpline*)obj)->m_spline->SetWeight(i,v);
}

void OnSetPole(const double *v, HeeksObj* obj, int i)
{
	gp_Pnt p = make_point(v);
	((HSpline*)obj)->m_spline->SetPole(i,p);
}


void HSpline::GetProperties(std::list<Property *> *list){
	wxChar str[512];
	for(int i=1; i <= m_spline->NbKnots(); i++)
	{
		wxSprintf(str,_("%s %d"), _("Knot"), i);
		list->push_back(new PropertyDouble(str,m_spline->Knot(i),this,OnSetKnot,i));
		//TODO: Should add the multiplicity property. Complicated to change though.
		//Will blow up bspline unless a bunch of other parameters change as well
		//sprintf(str,"%s %d", _("Multiplicity"), i);
		//list->push_back(new PropertyInt(str,m_spline->Mult(i),this,OnSetMult,i));
	}
	for(int i=1; i <= m_spline->NbPoles(); i++)
	{
		double p[3];
		extract(m_spline->Pole(i),p);
		wxSprintf(str,_("%s %d"), _("Pole"), i);
		list->push_back(new PropertyVertex(str,p,this,OnSetPole,i));
		wxSprintf(str,_("%s %d"), _("Weight"), i);
		list->push_back(new PropertyDouble(str,m_spline->Weight(i),this,OnSetWeight,i));
	}
	HeeksObj::GetProperties(list); 
}

bool HSpline::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
	gp_Lin ray(make_point(ray_start), make_vector(ray_direction));
	std::list< gp_Pnt > rl;

	return false; 
}

bool HSpline::FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point){
	// any point on this ellipse is a possible tangent point
	return FindNearPoint(ray_start, ray_direction, point);
}

bool HSpline::Stretch(const double *p, const double* shift){

	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	//There may be multiple control points at the same location, so we must move them all
	// This could break if user drags one set of points over another
	// The correct mapping of handles to control points can be determined
	// from the multiplicity set
	for(int i = 1; i <= m_spline->NbPoles(); i++)
	{
		if(m_spline->Pole(i).IsEqual(vp, wxGetApp().m_geom_tol))
		{
			m_spline->SetPole(i,m_spline->Pole(i).XYZ() + vshift.XYZ());
		}
  	}
	return false; 
}

void HSpline::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Spline" );
	root->LinkEndChild( element );  
	element->SetAttribute("col", color.COLORREF_color());
	element->SetAttribute("rational", m_spline->IsRational()?1:0);
	element->SetAttribute("periodic", m_spline->IsPeriodic()?1:0);
	element->SetAttribute("knots", m_spline->NbKnots());
	element->SetAttribute("poles", m_spline->NbPoles());
	element->SetAttribute("degree", m_spline->Degree());
	for(int i=1; i <= m_spline->NbPoles(); i++)
	{
		gp_Pnt polepnt = m_spline->Pole(i);
		double weight = m_spline->Weight(i);
		TiXmlElement *pole;
		pole = new TiXmlElement("Pole");
		element->LinkEndChild(pole);
		pole->SetAttribute("index",i);
		pole->SetDoubleAttribute("weight",weight);
		pole->SetDoubleAttribute("x", polepnt.X());
		pole->SetDoubleAttribute("y", polepnt.Y());
		pole->SetDoubleAttribute("z", polepnt.Z());
	}

	for(int i=1; i <= m_spline->NbKnots(); i++)
	{
		double knotdoub = m_spline->Knot(i);
		int mult = m_spline->Multiplicity(i);
		TiXmlElement *knot;
		knot = new TiXmlElement("Knot");
		element->LinkEndChild(knot);
		knot->SetAttribute("index",i);
		knot->SetDoubleAttribute("knot",knotdoub);
		knot->SetAttribute("mult", mult);
	}
	WriteBaseXML(element); 
}

// static member function
HeeksObj* HSpline::ReadFromXMLElement(TiXmlElement* pElem)
{
	HeeksColor c;
	bool rational = false;
	bool periodic = false;
	int nknots = 0;
	int npoles = 0;
	int degree = 0;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){c = HeeksColor(a->IntValue());}
		else if(name == "rational"){rational = a->IntValue() != 0;}
		else if(name == "periodic"){periodic = a->IntValue() != 0;}
		else if(name == "knots"){nknots = a->IntValue();}
		else if(name == "poles"){npoles = a->IntValue();}
		else if(name == "degree"){degree = a->IntValue();}
	}

	TColgp_Array1OfPnt tkcontrol (1,npoles);
	TColStd_Array1OfReal tkweight (1,npoles);
	TColStd_Array1OfReal tkknot (1,nknots);
	TColStd_Array1OfInteger tkmult (1,nknots);



	// get the children

	TiXmlElement *pPole = pElem->FirstChildElement("Pole");
	for(int i=1; i <= npoles; i++)
	{
		double c[3];
		double weight;
		int index;
		// get the attributes
		for(TiXmlAttribute* a = pPole->FirstAttribute(); a; a = a->Next())
		{
			std::string name(a->Name());
			if(name == "x"){c[0] = a->DoubleValue();}
			else if(name == "y"){c[1] = a->DoubleValue();}
			else if(name == "z"){c[2] = a->DoubleValue();}
			else if(name == "weight"){weight = a->DoubleValue();}
			else if(name == "index"){index = a->IntValue();}
		}

		tkcontrol.SetValue(index,make_point(c));
		tkweight.SetValue(index,weight);
		pPole = pPole->NextSiblingElement("Pole");
	}
	
	TiXmlElement *pKnot = pElem->FirstChildElement("Knot");
	for(int i=1; i <= nknots; i++)
	{
		double knot;
		int mult;
		int index;
		// get the attributes
		for(TiXmlAttribute* a = pKnot->FirstAttribute(); a; a = a->Next())
		{
			std::string name(a->Name());
			if(name == "knot"){knot = a->DoubleValue();}
			else if(name == "mult"){mult = a->IntValue();}
			else if(name == "index"){index = a->IntValue();}
		}
		tkknot.SetValue(index,knot);
		tkmult.SetValue(index,mult);
		pKnot = pKnot->NextSiblingElement("Knot");
	}

	Geom_BSplineCurve spline(tkcontrol,tkweight,tkknot,tkmult,degree,periodic,rational);

	HSpline* new_object = new HSpline(spline, &c);
	new_object->ReadBaseXML(pElem);

	return new_object;  
}

int HSpline::Intersects(const HeeksObj *object, std::list< double > *rl)const
{
/*	int numi = 0;

	switch(object->GetType())
	{
	case LineType:
		{
			std::list<gp_Pnt> plist;
			intersect(((HLine*)object)->GetLine(), m_spline, plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(((HLine*)object)->Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case ILineType:
		{
			std::list<gp_Pnt> plist;
			intersect(((HILine*)object)->GetLine(), m_spline, plist);
			if(rl)convert_pnts_to_doubles(plist, *rl);
			numi += plist.size();
		}
		break;

/*	case ArcType:
		{
			std::list<gp_Pnt> plist;
			intersect(m_spline, ((HArc*)object)->m_circle, plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(((HArc*)object)->Intersects(pnt))
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
			intersect(m_spline, ((HSpline*)object)->m_circle, plist);
			if(rl)convert_pnts_to_doubles(plist, *rl);
			numi += plist.size();
		}
		break; 
	}
*/
	return 0; //return numi; 
}

