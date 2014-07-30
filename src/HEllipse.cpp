// HEllipse.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HEllipse.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "../interface/PropertyChoice.h"
#include "../interface/PropertyVertex.h"
#include "HLine.h"
#include "HILine.h"
#include "HArc.h"
#include "Gripper.h"

HEllipse::HEllipse(const HEllipse &e){
	operator=(e);
}

HEllipse::HEllipse(const gp_Elips &e, const HeeksColor* col):color(*col){
	C = new HPoint(e.Location(),col);
	C->m_draw_unselected = false;
	C->SetSkipForUndo(true);
	Add(C,NULL);

	m_start = 0;
	m_end = 2*M_PI;
	SetEllipse(e);
}

HEllipse::HEllipse(const gp_Elips &e, double start, double end, const HeeksColor* col):color(*col){
	m_start = start; m_end = end;
	C = new HPoint(e.Location(),col);
	C->m_draw_unselected = false;
	C->SetSkipForUndo(true);
	Add(C,NULL);

	SetEllipse(e);
}

HEllipse::~HEllipse(){
}

const HEllipse& HEllipse::operator=(const HEllipse &e){
#ifdef MULTIPLE_OWNERS
	ObjList::operator=(e);
#else
	HeeksObj::operator =(e);
#endif
	m_start = e.m_start; m_end = e.m_end;
	color = e.color;
	C = (HPoint*)GetFirstChild();
	C->SetSkipForUndo(true);
	SetEllipse(e.GetEllipse());

	return *this;
}


const wxBitmap &HEllipse::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/ellipse.png")));
	return *icon;
}

//segments - number of segments per full revolution!
void HEllipse::GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point)const
{
	gp_Dir x_axis = m_xdir;
	gp_Dir y_axis = m_zdir ^ m_xdir;
	gp_Pnt centre = C->m_p;

	double radius = m_majr;
	double min_radius = m_minr;

	double ratio = min_radius/radius;

	gp_Pnt zp(0,0,0);
	gp_Dir up(0,0,1);

	double end_angle = m_end;
	double start_angle = m_start;

	double d_angle = end_angle - start_angle;

	if(fabs(d_angle) < wxGetApp().m_geom_tol)
		d_angle = 2*M_PI;

	if(d_angle < 0)
		d_angle += 2*M_PI;

	int segments = (int)(fabs(pixels_per_mm * radius * d_angle / (2*M_PI) + 1));
	if(segments > 1000)
		segments = 1000;

    double theta = d_angle / (double)segments;
	while(theta>1.0){segments*=2;theta = d_angle / (double)segments;}
    double tangetial_factor = tan(theta);
    double radial_factor = 1 - cos(theta);

    double x = radius * cos(start_angle);
    double y = radius * sin(start_angle);

	double pp[3];

   for(int i = 0; i < segments + 1; i++)
    {
		gp_Pnt p = centre.XYZ() + x * x_axis.XYZ() + y * y_axis.XYZ()  * ratio;
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
//		if (wxGetApp().m_allow_opengl_stippling)
//		{
//			glEnable(GL_LINE_STIPPLE);
//			glLineStipple(3, 0xaaaa);
//		}
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
//		if (wxGetApp().m_allow_opengl_stippling)
//		{
//			glDisable(GL_LINE_STIPPLE);
//		}
	}
}

HeeksObj *HEllipse::MakeACopy(void)const{
		HEllipse *new_object = new HEllipse(*this);
		return new_object;
}

void HEllipse::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	SetEllipse(GetEllipse().Transformed(mat));
}

void HEllipse::GetBox(CBox &box){
	gp_Elips m_ellipse = GetEllipse();
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

void HEllipse::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	gp_Elips m_ellipse = GetEllipse();
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

		list->push_back(GripData(GripperTypeStretch,maj_s.X(),maj_s.Y(),maj_s.Z(),&m_majr));

		list->push_back(GripData(GripperTypeStretch,min_s.X(),min_s.Y(),min_s.Z(),&m_minr));

		list->push_back(GripData(GripperTypeStretch,c.X(),c.Y(),c.Z(),(void*)&C));

	    list->push_back(GripData(GripperTypeRotate,rot.X(),rot.Y(),rot.Z(),NULL));

	}
}

static void on_set_centre(const double *vt, HeeksObj* object){
	((HEllipse*)object)->C->m_p = make_point(vt);
	wxGetApp().Repaint();
}

static void on_set_axis(const double *vt, HeeksObj* object){
	HEllipse *e = (HEllipse*)object;
	gp_Ax2 a(e->C->m_p,e->m_zdir,e->m_xdir);
	a.SetDirection(make_vector(vt));
	e->m_zdir = a.Direction();
	e->m_xdir = a.XDirection();
	wxGetApp().Repaint();
}

static void on_set_major_radius(double value, HeeksObj* object){
	((HEllipse*)object)->m_majr = value;
	wxGetApp().Repaint();
}

static void on_set_minor_radius(double value, HeeksObj* object){
	((HEllipse*)object)->m_minr = value;
	wxGetApp().Repaint();
}

static void on_set_rotation(double value, HeeksObj* object){
    ((HEllipse*)object)->SetRotation(value);
	wxGetApp().Repaint();
}

static void on_set_start_angle(double value, HeeksObj* object){
    ((HEllipse*)object)->m_start = value;
	wxGetApp().Repaint();
}

static void on_set_end_angle(double value, HeeksObj* object){
    ((HEllipse*)object)->m_end = value;
	wxGetApp().Repaint();
}

void HEllipse::SetRotation(double value)
{
	gp_Dir up(0, 0, 1);
	gp_Pnt zp(0,0,0);
    double rot = GetRotation();

	gp_Ax2 a(C->m_p,m_zdir,m_xdir);
	a.Rotate(gp_Ax1(C->m_p,up),value-rot);
	m_zdir = a.Direction();
	m_xdir = a.XDirection();
}

double HEllipse::GetRotation()const
{
	return GetEllipseRotation(GetEllipse());
}

void HEllipse::GetProperties(std::list<Property *> *list){
	double c[3], a[3];
	extract(C->m_p, c);
	extract(m_zdir, a);
	double rot = GetRotation();
	list->push_back(new PropertyVertex(_("centre"), c, this, on_set_centre));
	list->push_back(new PropertyVector(_("axis"), a, this, on_set_axis));
	list->push_back(new PropertyLength(_("major radius"), m_majr, this, on_set_major_radius));
	list->push_back(new PropertyLength(_("minor radius"), m_minr, this, on_set_minor_radius));
	list->push_back(new PropertyDouble(_("rotation"), rot, this, on_set_rotation));
	list->push_back(new PropertyDouble(_("start angle"), m_start, this, on_set_start_angle));
	list->push_back(new PropertyDouble(_("end angle"), m_end, this, on_set_end_angle));
	HeeksObj::GetProperties(list);
}

bool HEllipse::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
	gp_Lin ray(make_point(ray_start), make_vector(ray_direction));
	std::list< gp_Pnt > rl;
	ClosestPointsLineAndEllipse(ray, GetEllipse(), rl);
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

bool HEllipse::Stretch(const double *p, const double* shift, void* data){

	//TODO:
        // 1. When the major and minor axis swap, the unused handle switches sides.
	// 2. The handle switches to the other radius if you go past M_PI/4

	gp_Pnt vp = make_point(p);
	gp_Pnt zp(0,0,0);
	gp_Dir up(0, 0, 1);
	gp_Vec vshift = make_vector(shift);

	double rot = GetRotation();

	gp_Dir x_axis = m_xdir;
	gp_Dir y_axis = m_zdir ^ m_xdir;
	gp_Pnt c = C->m_p;
	double maj_r = m_majr;
	double min_r = m_minr;
	gp_Pnt maj_s(c.XYZ() + x_axis.XYZ() * maj_r);
	gp_Pnt min_s(c.XYZ() + y_axis.XYZ() * min_r);

	gp_Pnt np = vp.XYZ() + vshift.XYZ();
#if 0
	// these cause compiler warnings, so I have commented them out
    double d = c.Distance(np);
    double f = DistanceToFoci(np,m_ellipse)/2;
#endif
	if(data == &C){
		C->m_p = np;
	}
    else if(data == &m_majr || data == &m_minr)
	{
		//We have to rotate the incoming vector to be in our coordinate system
		gp_Pnt cir = np.XYZ() - c.XYZ();
		cir.Rotate(gp_Ax1(zp,up),-rot);

		//This is shockingly simple
		if( data == (void*)1)
		{
			double nradius = 1/sqrt((1-(1/min_r)*(1/min_r)*cir.X()*cir.X()) / cir.Y() / cir.Y());
			if(nradius > 1 / wxGetApp().m_geom_tol || nradius != nradius)
				nradius = 1 / wxGetApp().m_geom_tol;

			if(nradius > min_r)
				m_majr = nradius;
			else
			{
				m_majr = min_r;
				m_minr = nradius;
				SetRotation(GetRotation()-M_PI/2);
				m_start += M_PI/2;
				m_end += M_PI/2;
			}
		}
		else
		{
			double nradius = 1/sqrt((1-(1/maj_r)*(1/maj_r)*cir.Y()*cir.Y()) / cir.X() / cir.X());
			if(nradius > 1 / wxGetApp().m_geom_tol || nradius != nradius)
				nradius = 1 / wxGetApp().m_geom_tol;
			if(nradius < maj_r)
				m_minr = nradius;
			else
			{
				m_minr = maj_r;
				m_majr = nradius;
				SetRotation(GetRotation()+M_PI/2);
				m_start-=M_PI/2;
				m_end-=M_PI/2;
			}
		}
	}
	return false;
}

bool HEllipse::GetCentrePoint(double* pos)
{
	extract(C->m_p, pos);
	return true;
}

void HEllipse::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Ellipse" );
	root->LinkEndChild( element );
	element->SetAttribute("col", color.COLORREF_color());
	element->SetDoubleAttribute("maj", m_majr);
	element->SetDoubleAttribute("min", m_minr);
	element->SetDoubleAttribute("rot", GetRotation());
	gp_Dir D = m_zdir;
	element->SetDoubleAttribute("ax", D.X());
	element->SetDoubleAttribute("ay", D.Y());
	element->SetDoubleAttribute("az", D.Z());
	element->SetDoubleAttribute("start", m_start);
	element->SetDoubleAttribute("end", m_end);
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
	double start=0;
	double end=2*M_PI;
	HeeksColor c;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){c = HeeksColor((long)(a->IntValue()));}
		else if(name == "maj"){maj = a->DoubleValue();}
		else if(name == "min"){min = a->DoubleValue();}
		else if(name == "rot"){rot = a->DoubleValue();}
		else if(name == "cx"){centre.SetX(a->DoubleValue());}
		else if(name == "cy"){centre.SetY(a->DoubleValue());}
		else if(name == "cz"){centre.SetZ(a->DoubleValue());}
		else if(name == "ax"){axis[0] = a->DoubleValue();}
		else if(name == "ay"){axis[1] = a->DoubleValue();}
		else if(name == "az"){axis[2] = a->DoubleValue();}
		else if(name == "start"){start = a->DoubleValue();}
		else if(name == "end"){end = a->DoubleValue();}
	}

	gp_Elips ellipse(gp_Ax2(centre, gp_Dir(make_vector(axis))), maj,min);

	HEllipse* new_object = new HEllipse(ellipse,start,end,&c);
	new_object->SetRotation(rot);
	new_object->ReadBaseXML(pElem);

	if(new_object->GetNumChildren() > 0)
	{
		new_object->Remove(new_object->C);
		new_object->ReloadPointers();
		new_object->C->SetSkipForUndo(true);
	}

	return new_object;
}

void HEllipse::ReloadPointers()
{
	C = (HPoint*)GetFirstChild();
#ifdef MULTIPLE_OWNERS
	ObjList::ReloadPointers();
#else
	HeeksObj::ReloadPointers();
#endif
}

void HEllipse::SetEllipse(gp_Elips e)
{
	C->m_p = e.Location();
	m_zdir = e.Axis().Direction();
	m_xdir = e.XAxis().Direction();
	m_majr = e.MajorRadius();
	m_minr = e.MinorRadius();
}

gp_Elips HEllipse::GetEllipse() const
{
	return gp_Elips(gp_Ax2(C->m_p,m_zdir,m_xdir),m_majr,m_minr);
}

int HEllipse::Intersects(const HeeksObj *object, std::list< double > *rl)const
{
	int numi = 0;
	gp_Elips m_ellipse = GetEllipse();

	switch(object->GetType())
	{
    case SketchType:
        return( ((CSketch *)object)->Intersects( this, rl ));

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

void HEllipse::LoadFromDoubles()
{
	C->LoadFromDoubles();
	if(m_minr > m_majr)
	{
		double t = m_majr;
		m_majr = m_minr;
		m_minr = t;
		SetRotation(m_rot+M_PI/2);
	}
	else
		SetRotation(m_rot);
}

void HEllipse::LoadToDoubles()
{
	C->LoadToDoubles();
	m_rot = GetRotation();
}
