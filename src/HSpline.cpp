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

	for(int i = 0; i < 100; i++)
    {
		gp_Pnt p;
		m_spline->D0(i * .01,p);
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
//	m_spline.Transform(mat);
	return false;
}

void HSpline::GetBox(CBox &box){
/*	gp_Dir x_axis = m_spline.XAxis().Direction();
	gp_Dir y_axis = m_spline.YAxis().Direction();
	gp_XYZ c = m_spline.Location().XYZ();
	gp_XYZ x = x_axis.XYZ() * m_spline.MajorRadius();
	gp_XYZ y = y_axis.XYZ() * m_spline.MinorRadius();

	gp_Pnt p[4];
	p[0] = gp_Pnt(c - x - y);
	p[1] = gp_Pnt(c + x - y);
	p[2] = gp_Pnt(c + x + y);
	p[3] = gp_Pnt(c - x + y);

	for(int i = 0; i<4; i++)
	{
		double pp[3];
		extract(p[i], pp);
		box.Insert(pp);
	} */
}

void HSpline::GetGripperPositions(std::list<double> *list, bool just_for_endof){
	if(!just_for_endof)
	{
/*		gp_Dir x_axis = m_spline.XAxis().Direction();
		gp_Dir y_axis = m_spline.YAxis().Direction();
		gp_XYZ c = m_spline.Location().XYZ();
		double maj_r = m_spline.MajorRadius();
		double min_r = m_spline.MinorRadius();
		gp_Pnt maj_s(c + x_axis.XYZ() * maj_r);
		gp_Pnt min_s(c + y_axis.XYZ() * min_r);
		gp_Pnt rot(c+x_axis.XYZ() * maj_r * -1);

		list->push_back(GripperTypeStretch);
		list->push_back(maj_s.X());
		list->push_back(maj_s.Y());
		list->push_back(maj_s.Z());

		list->push_back(GripperTypeStretch);
		list->push_back(min_s.X());
		list->push_back(min_s.Y());
		list->push_back(min_s.Z());

	        list->push_back(GripperTypeRotate);
	        list->push_back(rot.X());
	        list->push_back(rot.Y());
	        list->push_back(rot.Z());
*/
	} 
}

static void on_set_centre(const double *vt, HeeksObj* object){
//	((HSpline*)object)->m_spline.SetLocation(make_point(vt));
	wxGetApp().Repaint();
}

static void on_set_axis(const double *vt, HeeksObj* object){
//	gp_Ax1 a = ((HSpline*)object)->m_spline.Axis();
//	a.SetDirection(make_vector(vt));
//	((HSpline*)object)->m_spline.SetAxis(a);
	wxGetApp().Repaint(); 
}

void HSpline::GetProperties(std::list<Property *> *list){
/*	double c[3], a[3];
	extract(m_spline.Location(), c);
	extract(m_spline.Axis().Direction(), a);
	double rot = GetRotation();
	list->push_back(new PropertyVertex(_("centre"), c, this, on_set_centre));
	list->push_back(new PropertyVertex(_("axis"), a, this, on_set_axis));
	list->push_back(new PropertyDouble(_("major radius"), m_spline.MajorRadius(), this, on_set_major_radius));
	list->push_back(new PropertyDouble(_("minor radius"), m_spline.MinorRadius(), this, on_set_minor_radius));
	list->push_back(new PropertyDouble(_("rotation"), rot, this, on_set_rotation)); */
	list->push_back(new PropertyDouble(_("argh"),1,this,0));
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

/*	//TODO: 
        // 1. When the major and minor axis swap, the unused handle switches sides.
	// 2. The handle switches to the other radius if you go past Pi/4

/	gp_Pnt vp = make_point(p);
	gp_Pnt zp(0,0,0);
	gp_Dir up(0, 0, 1);
	gp_Vec vshift = make_vector(shift);

	double rot = GetRotation();

	gp_Dir x_axis = m_spline.XAxis().Direction();
	gp_Dir y_axis = m_spline.YAxis().Direction();
	gp_Pnt c = m_spline.Location();
	double maj_r = m_spline.MajorRadius();
	double min_r = m_spline.MinorRadius();
	gp_Pnt maj_s(c.XYZ() + x_axis.XYZ() * maj_r);
	gp_Pnt min_s(c.XYZ() + y_axis.XYZ() * min_r);

        double d = c.Distance(vp);
        double f = DistanceToFoci(vp,m_spline)/2;

	if(c.IsEqual(vp, wxGetApp().m_geom_tol)){
		m_spline.SetLocation(c.XYZ() + vshift.XYZ());
	}
        else if(f < m_spline.MajorRadius() +  wxGetApp().m_geom_tol && d > m_spline.MinorRadius() -  wxGetApp().m_geom_tol)
	{
		//We have to rotate the incoming vector to be in our coordinate system
		gp_Pnt cir = vp.XYZ() + vshift.XYZ() - c.XYZ();
		cir.Rotate(gp_Ax1(zp,up),-rot);

		//This is shockingly simple
		if( d > min_r + (maj_r - min_r)/2)
		{
			double nradius = 1/sqrt((1-(1/min_r)*(1/min_r)*cir.X()*cir.X()) / cir.Y() / cir.Y());
			if(nradius > min_r)
				m_spline.SetMajorRadius(nradius); 
			else
			{
				m_spline.SetMajorRadius(min_r);
				m_spline.SetMinorRadius(nradius);
				SetRotation(GetRotation()-Pi/2);
			}
		}
		else
		{
			double nradius = 1/sqrt((1-(1/maj_r)*(1/maj_r)*cir.Y()*cir.Y()) / cir.X() / cir.X());
			if(nradius < maj_r)
				m_spline.SetMinorRadius(nradius); 
			else
			{
				m_spline.SetMinorRadius(maj_r);
				m_spline.SetMajorRadius(nradius);
				SetRotation(GetRotation()+Pi/2);
			}
		}
	} */
	return false; 
}

bool HSpline::GetCentrePoint(double* pos)
{
//	extract(m_spline.Location(), pos);
	return true;
}

void HSpline::WriteXML(TiXmlNode *root)
{
/*	TiXmlElement * element;
	element = new TiXmlElement( "Circle" );
	root->LinkEndChild( element );  
	element->SetAttribute("col", color.COLORREF_color());
	element->SetDoubleAttribute("maj", m_spline.MajorRadius());
	element->SetDoubleAttribute("min", m_spline.MinorRadius());
	gp_Pnt C = m_spline.Location();
	gp_Dir D = m_spline.Axis().Direction();
	element->SetDoubleAttribute("cx", C.X());
	element->SetDoubleAttribute("cy", C.Y());
	element->SetDoubleAttribute("cz", C.Z());
	element->SetDoubleAttribute("ax", D.X());
	element->SetDoubleAttribute("ay", D.Y());
	element->SetDoubleAttribute("az", D.Z());
	WriteBaseXML(element); */
}

// static member function
HeeksObj* HSpline::ReadFromXMLElement(TiXmlElement* pElem)
{
/*	gp_Pnt centre;
	double axis[3];
	double maj = 0.0;
	double min = 0.0;
	HeeksColor c;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){c = HeeksColor(a->IntValue());}
		else if(name == "maj"){maj = a->DoubleValue();}
		else if(name == "mIN"){min = a->DoubleValue();}
		else if(name == "cx"){centre.SetX(a->DoubleValue());}
		else if(name == "cy"){centre.SetY(a->DoubleValue());}
		else if(name == "cz"){centre.SetZ(a->DoubleValue());}
		else if(name == "ax"){axis[0] = a->DoubleValue();}
		else if(name == "ay"){axis[1] = a->DoubleValue();}
		else if(name == "az"){axis[2] = a->DoubleValue();}
	}

	Geom_BSplineCurve ellipse(gp_Ax2(centre, gp_Dir(make_vector(axis))), maj,min);

	HSpline* new_object = new HSpline(ellipse, &c);
	new_object->ReadBaseXML(pElem);

	return new_object;  */
	return 0;
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

