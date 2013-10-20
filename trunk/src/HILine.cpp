// HILine.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HILine.h"
#include "HLine.h"
#include "HArc.h"
#include "HCircle.h"
#include "HPoint.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "../interface/PropertyVertex.h"
#include "Gripper.h"

HILine::HILine(const HILine &line):EndedObject(){
	operator=(line);
}

HILine::HILine(const gp_Pnt &a, const gp_Pnt &b, const HeeksColor* col):EndedObject(){
	A = a;
	B = b;
	SetColor(*col);
}

HILine::~HILine(){
}

const HILine& HILine::operator=(const HILine &b){
	EndedObject::operator=(b);
	return *this;
}

const wxBitmap &HILine::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/iline.png")));
	return *icon;
}

void HILine::glCommands(bool select, bool marked, bool no_color)
{
	if(!no_color)
	{
		wxGetApp().glColorEnsuringContrast(*GetColor());
		if (wxGetApp().m_allow_opengl_stippling)
		{
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(3, 0xaaaa);
		}
	}
	GLfloat save_depth_range[2];
	if(marked)
	{
		glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
		glDepthRange(0, 0);
		glLineWidth(2);
	}

	gp_Vec v(A, B);
	if(v.Magnitude() > 0.0000000001)
	{
		v.Normalize();

		gp_Pnt p1 = A.XYZ() - v.XYZ() * 10000;
		gp_Pnt p2 = A.XYZ() + v.XYZ() * 10000;

		glBegin(GL_LINES);
		glVertex3d(p1.X(), p1.Y(), p1.Z());
		glVertex3d(p2.X(), p2.Y(), p2.Z());
		glEnd();
	}

	if(marked)
	{
		glLineWidth(1);
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}
	if(!no_color)
	{
		if (wxGetApp().m_allow_opengl_stippling)
		{
			glDisable(GL_LINE_STIPPLE);
		}
	}
}

HeeksObj *HILine::MakeACopy(void)const{
		HILine *new_object = new HILine(*this);
		return new_object;
}

void HILine::GetBox(CBox &box){
	box.Insert(A.X(), A.Y(), A.Z());
	box.Insert(B.X(), B.Y(), B.Z());
}

void HILine::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	if(!just_for_endof) // we don't want to snap to these for endof
	{
		EndedObject::GetGripperPositions(list,just_for_endof);
	}
}

static void on_set_start(const double *vt, HeeksObj* object){
	((HILine*)object)->A = make_point(vt);
	wxGetApp().Repaint();
}

static void on_set_end(const double *vt, HeeksObj* object){
	((HILine*)object)->B = make_point(vt);
	wxGetApp().Repaint();
}

void HILine::GetProperties(std::list<Property *> *list){
	double a[3], b[3];
	extract(A, a);
	extract(B, b);
	list->push_back(new PropertyVertex(_("start"), a, this, on_set_start));
	list->push_back(new PropertyVertex(_("end"), b, this, on_set_end));
	double length = A.Distance(B);
	list->push_back(new PropertyLength(_("Length"), length, this));

	HeeksObj::GetProperties(list);
}

bool HILine::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
	gp_Lin ray(make_point(ray_start), make_vector(ray_direction));
	gp_Pnt p1, p2;
	ClosestPointsOnLines(GetLine(), ray, p1, p2);
	extract(p1, point);
	return true;
}

bool HILine::FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point){
	// any point on this line is a possible tangent point
	return FindNearPoint(ray_start, ray_direction, point);
}

gp_Lin HILine::GetLine()const{
	gp_Vec v(A,B);
	return gp_Lin(A, v);
}

int HILine::Intersects(const HeeksObj *object, std::list< double > *rl)const{
	int numi = 0;

	switch(object->GetType())
	{
    case SketchType:
        return( ((CSketch *)object)->Intersects( this, rl ));

	case LineType:
		{
			gp_Pnt pnt;
			if(intersect(GetLine(), ((HLine*)object)->GetLine(), pnt))
			{
				if(((HLine*)object)->Intersects(pnt)){
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
				if(rl)add_pnt_to_doubles(pnt, *rl);
				numi++;
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
			intersect(GetLine(), ((HCircle*)object)->GetCircle(), plist);
			if(rl)convert_pnts_to_doubles(plist, *rl);
			numi += plist.size();
		}
		break;
	}

	return numi;
}

bool HILine::GetStartPoint(double* pos)
{
	extract(A, pos);
	return true;
}

bool HILine::GetEndPoint(double* pos)
{
	extract(B, pos);
	return true;
}

void HILine::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "InfiniteLine" );
	root->LinkEndChild( element );
	WriteBaseXML(element);
}

// static member function
HeeksObj* HILine::ReadFromXMLElement(TiXmlElement* pElem)
{
	gp_Pnt p0, p1;
	HeeksColor c;

	HILine* new_object = new HILine(p0, p1, &c);
	new_object->ReadBaseXML(pElem);

	return new_object;
}

