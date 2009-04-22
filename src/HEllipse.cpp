// HEllipse.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HEllipse.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyChoice.h"
#include "../tinyxml/tinyxml.h"
#include "../interface/PropertyVertex.h"
#include "HLine.h"
#include "HILine.h"
#include "HArc.h"
#include "Gripper.h"

HEllipse::HEllipse(const HEllipse &e){
	operator=(e);
}

HEllipse::HEllipse(const gp_Elips &e, const HeeksColor* col):color(*col), m_ellipse(e){
}

HEllipse::~HEllipse(){
}

const HEllipse& HEllipse::operator=(const HEllipse &e){
	HeeksObj::operator=(e);
	m_ellipse = e.m_ellipse;
	color = e.color;
	return *this;
}



//segments - number of segments per full revolution!
void HEllipse::GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point)const
{
	gp_Dir x_axis = m_ellipse.XAxis().Direction();
	gp_Dir y_axis = m_ellipse.YAxis().Direction();
	gp_Pnt centre = m_ellipse.Location();

	double radius = m_ellipse.MajorRadius();
	double min_radius = m_ellipse.MinorRadius();
	int segments = (int)(fabs(pixels_per_mm * radius + 1));
    
    double theta = 6.28318530717958 / (double)segments;
    double tangetial_factor = tan(theta);
    double radial_factor = 1 - cos(theta);
    
    double x = radius;
    double y = 0.0;

	double pp[3];

   for(int i = 0; i < segments + 1; i++)
    {
		gp_Pnt p = centre.XYZ() + x * x_axis.XYZ() + y * y_axis.XYZ()  * min_radius/radius;
		extract(p, pp);
		(*callbackfunc)(pp);
        
        double tx = -y;
        double ty = x;
        
        x += tx * tangetial_factor;
        y += ty * tangetial_factor;
        
        double rx = - x;
        double ry = - y;
        
        x += rx * radial_factor;
        y += ry * radial_factor;
    } 
}

static void glVertexFunction(const double *p){glVertex3d(p[0], p[1], p[2]);}

void HEllipse::glCommands(bool select, bool marked, bool no_color){
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

HeeksObj *HEllipse::MakeACopy(void)const{
		HEllipse *new_object = new HEllipse(*this);
		return new_object;
}

bool HEllipse::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	m_ellipse.Transform(mat);
	return false;
}

void HEllipse::GetBox(CBox &box){
	gp_Dir x_axis = m_ellipse.XAxis().Direction();
	gp_Dir y_axis = m_ellipse.YAxis().Direction();
	gp_XYZ c = m_ellipse.Location().XYZ();
	gp_XYZ x = x_axis.XYZ() * m_ellipse.MajorRadius();
	gp_XYZ y = y_axis.XYZ() * m_ellipse.MinorRadius();

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
	} 
}

void HEllipse::GetGripperPositions(std::list<double> *list, bool just_for_endof){
	if(!just_for_endof)
	{
		gp_Dir x_axis = m_ellipse.XAxis().Direction();
		gp_Dir y_axis = m_ellipse.YAxis().Direction();
		gp_XYZ c = m_ellipse.Location().XYZ();
		double maj_r = m_ellipse.MajorRadius();
		double min_r = m_ellipse.MinorRadius();
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

	} 
}

static void on_set_centre(const double *vt, HeeksObj* object){
	((HEllipse*)object)->m_ellipse.SetLocation(make_point(vt));
	wxGetApp().Repaint();
}

static void on_set_axis(const double *vt, HeeksObj* object){
	gp_Ax1 a = ((HEllipse*)object)->m_ellipse.Axis();
	a.SetDirection(make_vector(vt));
	((HEllipse*)object)->m_ellipse.SetAxis(a);
	wxGetApp().Repaint(); 
}

static void on_set_major_radius(double value, HeeksObj* object){
	((HEllipse*)object)->m_ellipse.SetMajorRadius(value);
	wxGetApp().Repaint(); 
}

static void on_set_minor_radius(double value, HeeksObj* object){
	((HEllipse*)object)->m_ellipse.SetMinorRadius(value);
	wxGetApp().Repaint(); 
}

static void on_set_rotation(double value, HeeksObj* object){
        ((HEllipse*)object)->SetRotation(value);
	wxGetApp().Repaint(); 
}

void HEllipse::SetRotation(double value)
{
	gp_Dir up(0, 0, 1);
        double rot = GetRotation();
	m_ellipse.Rotate(gp_Ax1(m_ellipse.Location(),up),value-rot);
}

double HEllipse::GetRotation()
{
	return GetEllipseRotation(m_ellipse);
}

void HEllipse::GetProperties(std::list<Property *> *list){
	double c[3], a[3];
	extract(m_ellipse.Location(), c);
	extract(m_ellipse.Axis().Direction(), a);
	double rot = GetRotation();
	list->push_back(new PropertyVertex(_("centre"), c, this, on_set_centre));
	list->push_back(new PropertyVertex(_("axis"), a, this, on_set_axis));
	list->push_back(new PropertyDouble(_("major radius"), m_ellipse.MajorRadius(), this, on_set_major_radius));
	list->push_back(new PropertyDouble(_("minor radius"), m_ellipse.MinorRadius(), this, on_set_minor_radius));
	list->push_back(new PropertyDouble(_("rotation"), rot, this, on_set_rotation));
	HeeksObj::GetProperties(list);
}

bool HEllipse::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
	gp_Lin ray(make_point(ray_start), make_vector(ray_direction));
	std::list< gp_Pnt > rl;
	ClosestPointsLineAndEllipse(ray, m_ellipse, rl);
	if(rl.size()>0)
	{
		extract(rl.front(), point);
		return true;
	}

	return false; 
}

bool HEllipse::FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point){
	// any point on this ellipse is a possible tangent point
	return FindNearPoint(ray_start, ray_direction, point);
}

bool HEllipse::Stretch(const double *p, const double* shift){

	//TODO: 
        // 1. When the major and minor axis swap, the unused handle switches sides.
	// 2. The handle switches to the other radius if you go past Pi/4

	gp_Pnt vp = make_point(p);
	gp_Pnt zp(0,0,0);
	gp_Dir up(0, 0, 1);
	gp_Vec vshift = make_vector(shift);

	double rot = GetRotation();

	gp_Dir x_axis = m_ellipse.XAxis().Direction();
	gp_Dir y_axis = m_ellipse.YAxis().Direction();
	gp_Pnt c = m_ellipse.Location();
	double maj_r = m_ellipse.MajorRadius();
	double min_r = m_ellipse.MinorRadius();
	gp_Pnt maj_s(c.XYZ() + x_axis.XYZ() * maj_r);
	gp_Pnt min_s(c.XYZ() + y_axis.XYZ() * min_r);

        double d = c.Distance(vp);
        double f = DistanceToFoci(vp,m_ellipse)/2;

	if(c.IsEqual(vp, wxGetApp().m_geom_tol)){
		m_ellipse.SetLocation(c.XYZ() + vshift.XYZ());
	}
        else if(f < m_ellipse.MajorRadius() +  wxGetApp().m_geom_tol && d > m_ellipse.MinorRadius() -  wxGetApp().m_geom_tol)
	{
		//We have to rotate the incoming vector to be in our coordinate system
		gp_Pnt cir = vp.XYZ() + vshift.XYZ() - c.XYZ();
		cir.Rotate(gp_Ax1(zp,up),-rot);

		//This is shockingly simple
		if( d > min_r + (maj_r - min_r)/2)
		{
			double nradius = 1/sqrt((1-(1/min_r)*(1/min_r)*cir.X()*cir.X()) / cir.Y() / cir.Y());
			if(nradius > min_r)
				m_ellipse.SetMajorRadius(nradius); 
			else
			{
				m_ellipse.SetMajorRadius(min_r);
				m_ellipse.SetMinorRadius(nradius);
				SetRotation(GetRotation()-Pi/2);
			}
		}
		else
		{
			double nradius = 1/sqrt((1-(1/maj_r)*(1/maj_r)*cir.Y()*cir.Y()) / cir.X() / cir.X());
			if(nradius < maj_r)
				m_ellipse.SetMinorRadius(nradius); 
			else
			{
				m_ellipse.SetMinorRadius(maj_r);
				m_ellipse.SetMajorRadius(nradius);
				SetRotation(GetRotation()+Pi/2);
			}
		}
	}
	return false; 
}

bool HEllipse::GetCentrePoint(double* pos)
{
	extract(m_ellipse.Location(), pos);
	return true;
}

void HEllipse::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Ellipse" );
	root->LinkEndChild( element );  
	element->SetAttribute("col", color.COLORREF_color());
	element->SetDoubleAttribute("maj", m_ellipse.MajorRadius());
	element->SetDoubleAttribute("min", m_ellipse.MinorRadius());
	element->SetDoubleAttribute("rot", GetRotation());
	gp_Pnt C = m_ellipse.Location();
	gp_Dir D = m_ellipse.Axis().Direction();
	element->SetDoubleAttribute("cx", C.X());
	element->SetDoubleAttribute("cy", C.Y());
	element->SetDoubleAttribute("cz", C.Z());
	element->SetDoubleAttribute("ax", D.X());
	element->SetDoubleAttribute("ay", D.Y());
	element->SetDoubleAttribute("az", D.Z());
	WriteBaseXML(element); 
}

// static member function
HeeksObj* HEllipse::ReadFromXMLElement(TiXmlElement* pElem)
{
	gp_Pnt centre;
	double axis[3];
	double maj = 0.0;
	double min = 0.0;
	double rot = 0;
	HeeksColor c;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){c = HeeksColor(a->IntValue());}
		else if(name == "maj"){maj = a->DoubleValue();}
		else if(name == "min"){min = a->DoubleValue();}
		else if(name == "rot"){rot = a->DoubleValue();}
		else if(name == "cx"){centre.SetX(a->DoubleValue());}
		else if(name == "cy"){centre.SetY(a->DoubleValue());}
		else if(name == "cz"){centre.SetZ(a->DoubleValue());}
		else if(name == "ax"){axis[0] = a->DoubleValue();}
		else if(name == "ay"){axis[1] = a->DoubleValue();}
		else if(name == "az"){axis[2] = a->DoubleValue();}
	}

	gp_Elips ellipse(gp_Ax2(centre, gp_Dir(make_vector(axis))), maj,min);

	HEllipse* new_object = new HEllipse(ellipse, &c);
	new_object->SetRotation(rot);
	new_object->ReadBaseXML(pElem);

	return new_object; 
}

int HEllipse::Intersects(const HeeksObj *object, std::list< double > *rl)const
{
	int numi = 0;

	switch(object->GetType())
	{
	case LineType:
		{
			std::list<gp_Pnt> plist;
			intersect(((HLine*)object)->GetLine(), m_ellipse, plist);
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
			intersect(((HILine*)object)->GetLine(), m_ellipse, plist);
			if(rl)convert_pnts_to_doubles(plist, *rl);
			numi += plist.size();
		}
		break;

/*	case ArcType:
		{
			std::list<gp_Pnt> plist;
			intersect(m_ellipse, ((HArc*)object)->m_circle, plist);
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
			intersect(m_ellipse, ((HEllipse*)object)->m_circle, plist);
			if(rl)convert_pnts_to_doubles(plist, *rl);
			numi += plist.size();
		}
		break; */
	}

	return numi; 
}

