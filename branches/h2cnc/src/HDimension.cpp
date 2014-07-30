// HDimension.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "HDimension.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyChoice.h"
#include "PropertyTrsf.h"
#include "Gripper.h"
#include "HPoint.h"
#include "HeeksFrame.h"
#include "GraphicsCanvas.h"
#include "HeeksConfig.h"

bool HDimension::DrawFlat = true;

HDimension::HDimension(const gp_Trsf &trsf, const gp_Pnt &p0, const gp_Pnt &p1, const gp_Pnt &p2, DimensionMode mode, DimensionUnits units, const HeeksColor* col): EndedObject(col), m_color(*col), m_trsf(trsf), m_mode(mode),  m_units(units),  m_scale(1.0)
{
	m_p2 = new HPoint(p2,col);
	m_p2->m_draw_unselected = false;
	m_p2->SetSkipForUndo(true);
	Add(m_p2,NULL);
	A->m_p = p0;
	B->m_p = p1;
}

HDimension::HDimension(const HDimension &b):EndedObject(&b.m_color)
{
	operator=(b);
}

HDimension::~HDimension(void)
{
}

const HDimension& HDimension::operator=(const HDimension &b)
{
	EndedObject::operator=(b);
	m_trsf = b.m_trsf;
	m_units = b.m_units;
	m_color = b.m_color;
	m_mode = b.m_mode;
	m_scale = b.m_scale;

#ifdef MULTIPLE_OWNERS
	std::list<HeeksObj*>::iterator it = m_objects.begin();
	it++;it++;
	m_p2 = (HPoint*)(*it);
#endif

	return *this;
}

const wxBitmap &HDimension::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/dimension.png")));
	return *icon;
}

bool HDimension::IsDifferent(HeeksObj* other)
{
	HDimension* dim = (HDimension*)other;

	if(m_color.COLORREF_color() != dim->m_color.COLORREF_color() || m_mode != dim->m_mode || m_scale != dim->m_scale || m_units != dim->m_units)
		return true;

	if(m_p2->m_p.Distance(dim->m_p2->m_p) > wxGetApp().m_geom_tol)
		return true;

	return EndedObject::IsDifferent(other);
}

wxString HDimension::MakeText()
{
	wxString text;

	double units_factor = wxGetApp().m_view_units;
	switch(m_units)
	{
	case DimensionUnitsInches:
		units_factor = 25.4;
		break;
	case DimensionUnitsMM:
		units_factor = 1.0;
		break;
    case DimensionUnitsGlobal:
        units_factor = wxGetApp().m_view_units;
        break;
	}

	wxString units_str(_T(""));
	if(fabs(units_factor - 1.0) < 0.0000000001)units_str += wxString(_T(" ")) + _("mm");
	else if(fabs(units_factor - 25.4) < 0.000000001)units_str += wxString(_T(" ")) + _("inch");

	text = wxString::Format(_T("%lg %s"), A->m_p.Distance(GetB2())/units_factor, units_str.c_str());

	return text;
}

gp_Pnt HDimension::GetB2()
{
	// return B, possibly flattened
	if(m_mode == TwoPointsXYOnlyDimensionMode)
	{
		gp_Dir zdir = gp_Dir(0, 0, 1).Transformed(m_trsf);
		double dp = gp_Vec(B->m_p.XYZ()) * zdir - gp_Vec(A->m_p.XYZ()) * zdir;
		return gp_Pnt(B->m_p.XYZ() + zdir.XYZ() * (-dp));
	}
	else if(m_mode == TwoPointsXOnlyDimensionMode)
	{
		gp_Dir zdir = gp_Dir(0, 0, 1).Transformed(m_trsf);
		gp_Dir ydir = gp_Dir(0, 1, 0).Transformed(m_trsf);
		double dpz = gp_Vec(B->m_p.XYZ()) * zdir - gp_Vec(A->m_p.XYZ()) * zdir;
		double dpy = gp_Vec(B->m_p.XYZ()) * ydir - gp_Vec(A->m_p.XYZ()) * ydir;
		return gp_Pnt(B->m_p.XYZ() + zdir.XYZ() * (-dpz) + ydir.XYZ() * (-dpy));
	}
	else if(m_mode == TwoPointsYOnlyDimensionMode)
	{
		gp_Dir zdir = gp_Dir(0, 0, 1).Transformed(m_trsf);
		gp_Dir xdir = gp_Dir(1, 0, 0).Transformed(m_trsf);
		double dpz = gp_Vec(B->m_p.XYZ()) * zdir - gp_Vec(A->m_p.XYZ()) * zdir;
		double dpx = gp_Vec(B->m_p.XYZ()) * xdir - gp_Vec(A->m_p.XYZ()) * xdir;
		return gp_Pnt(B->m_p.XYZ() + zdir.XYZ() * (-dpz) + xdir.XYZ() * (-dpx));
	}
	else if(m_mode == TwoPointsZOnlyDimensionMode)
	{
		gp_Dir xdir = gp_Dir(1, 0, 0).Transformed(m_trsf);
		gp_Dir ydir = gp_Dir(0, 1, 0).Transformed(m_trsf);
		double dpx = gp_Vec(B->m_p.XYZ()) * xdir - gp_Vec(A->m_p.XYZ()) * xdir;
		double dpy = gp_Vec(B->m_p.XYZ()) * ydir - gp_Vec(A->m_p.XYZ()) * ydir;
		return gp_Pnt(B->m_p.XYZ() + xdir.XYZ() * (-dpx) + ydir.XYZ() * (-dpy));
	}

	return B->m_p;
}

gp_Pnt HDimension::GetC2()
{
	// return m_p2, possibly flattened
	if(m_mode == TwoPointsXYOnlyDimensionMode || m_mode == TwoPointsXOnlyDimensionMode)
	{
		gp_Dir zdir = gp_Dir(0, 0, 1).Transformed(m_trsf);
		double dp = gp_Vec(m_p2->m_p.XYZ()) * zdir - gp_Vec(A->m_p.XYZ()) * zdir;
		return gp_Pnt(m_p2->m_p.XYZ() + zdir.XYZ() * (-dp));
	}
#if 0
	else if(m_mode == TwoPointsXOnlyDimensionMode)
	{
		gp_Dir zdir = gp_Dir(0, 0, 1).Transformed(m_trsf);
		double zdp = gp_Vec(m_p2->m_p.XYZ()) * zdir - gp_Vec(A->m_p.XYZ()) * zdir;
		gp_Dir ydir = gp_Dir(0, 1, 0).Transformed(m_trsf);
		double ydp = gp_Vec(m_p2->m_p.XYZ()) * ydir - gp_Vec(A->m_p.XYZ()) * ydir;
		return gp_Pnt(m_p2->m_p.XYZ() + zdir.XYZ() * (-zdp) + ydir.XYZ() * (-ydp));
	}
#endif

	return m_p2->m_p;
}

void HDimension::glCommands(bool select, bool marked, bool no_color)
{
	gp_Pnt b = GetB2();
	
	if(A->m_p.IsEqual(b, wxGetApp().m_geom_tol))return;

	if(!no_color)wxGetApp().glColorEnsuringContrast(m_color);

	gp_Dir xdir = gp_Dir(1, 0, 0).Transformed(m_trsf);
	gp_Dir ydir = gp_Dir(0, 1, 0).Transformed(m_trsf);
	gp_Dir zdir = gp_Dir(0, 0, 1).Transformed(m_trsf);
	if(m_mode == TwoPointsDimensionMode || m_mode == TwoPointsXYOnlyDimensionMode || m_mode == TwoPointsXOnlyDimensionMode)
	{
		xdir = make_vector(A->m_p, b);
		if(xdir.IsParallel(zdir,wxGetApp().m_geom_tol))
			zdir = xdir ^ ydir;
		else
			ydir = zdir ^ xdir;
	}

	wxString text = MakeText();

	float width, height;
	if(!wxGetApp().get_text_size(text, &width, &height))return;

	// draw arrow line
	draw_arrow_line(m_mode, A->m_p, b, GetC2(), xdir, ydir, width, m_scale);

	// draw text
	RenderText(text, GetC2(), xdir, ydir, m_scale);

	EndedObject::glCommands(select,marked,no_color);
}

void HDimension::RenderText(const wxString &text, const gp_Pnt& p, const gp_Dir& xdir, const gp_Dir& ydir, double scale)
{
	float width, height;
	if(!wxGetApp().get_text_size(text, &width, &height))return;

	// make a matrix at top left of text
	gp_Pnt text_top_left( p.XYZ() + ydir.XYZ() * (scale * height) );
	gp_Trsf text_matrix = make_matrix(text_top_left, xdir, ydir);

	glPushMatrix();
	double m[16];
	extract_transposed(text_matrix, m);
	glMultMatrixd(m);

	if(DrawFlat)
	{
		//Try and draw this ortho.  must find the origin point in screen coordinates
		double x, y, z;

		// arrays to hold matrix information

		double model_view[16];
		glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

		double projection[16];
		glGetDoublev(GL_PROJECTION_MATRIX, projection);

		int viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		// get 3D coordinates based on window coordinates

		gluProject(0,0,0,
			model_view, projection, viewport,
			&x, &y, &z);

		wxGetApp().render_screen_text_at(text, scale*8,x,y,atan2(xdir.Y(),xdir.X()) * 180 / M_PI);
	}
	else
	{
		wxGetApp().render_text(text);
	}

	glPopMatrix();

}

#ifdef MULTIPLE_OWNERS
void HDimension::LoadToDoubles()
{
	EndedObject::LoadToDoubles();
	m_p2->LoadToDoubles();
}

void HDimension::LoadFromDoubles()
{
	EndedObject::LoadFromDoubles();
	m_p2->LoadFromDoubles();
}
#endif

HDimension* dimension_for_tool = NULL;

void HDimension::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	dimension_for_tool = this;
}

void HDimension::GetBox(CBox &box)
{
	gp_Pnt vt(0, 0, 0);
	vt.Transform(m_trsf);
	double p[3];
	extract(vt, p);
	box.Insert(p);

	wxString text = MakeText();
	float width, height;
	if(!wxGetApp().get_text_size(text, &width, &height))return;

	gp_Pnt point[3];
	point[0] = gp_Pnt(width, 0, 0);
	point[1] = gp_Pnt(0, -height, 0);
	point[2] = gp_Pnt(width, -height, 0);

	for(int i = 0; i<3; i++)
	{
		point[i].Transform(m_trsf);
		extract(point[i], p);
		box.Insert(p);
	}
}

HeeksObj *HDimension::MakeACopy(void)const
{
	return new HDimension(*this);
}

void HDimension::ModifyByMatrix(const double *m)
{
	gp_Trsf mat = make_matrix(m);
	m_trsf = mat * m_trsf;
}

void HDimension::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	wxString text = MakeText();
	float width, height;
	if(!wxGetApp().get_text_size(text, &width, &height))return;

	gp_Pnt point[4];
	point[0] = gp_Pnt(0, 0, 0);
	point[1] = gp_Pnt(width, 0, 0);
	point[2] = gp_Pnt(0, -height, 0);
	point[3] = gp_Pnt(width, -height, 0);

	for(int i = 0; i<4; i++)point[i].Transform(m_trsf);

	list->push_back(GripData(GripperTypeTranslate,point[0].X(),point[0].Y(),point[0].Z(),NULL));
	list->push_back(GripData(GripperTypeRotateObject,point[1].X(),point[1].Y(),point[1].Z(),NULL));
	list->push_back(GripData(GripperTypeRotateObject,point[2].X(),point[2].Y(),point[2].Z(),NULL));
	list->push_back(GripData(GripperTypeScale,point[3].X(),point[3].Y(),point[3].Z(),NULL));

	EndedObject::GetGripperPositions(list,just_for_endof);
	list->push_back(GripData(GripperTypeStretch,m_p2->m_p.X(),m_p2->m_p.Y(),m_p2->m_p.Z(),&m_p2));
}

static void on_set_trsf(const gp_Trsf &trsf, HeeksObj* object){
	((HDimension*)object)->m_trsf = trsf;
	wxGetApp().Repaint();
}

static void on_set_mode(int value, HeeksObj* object)
{
	HDimension* dimension = (HDimension*)object;
	dimension->m_mode = (DimensionMode)value;
	wxGetApp().Repaint();
}

static void on_set_units(int value, HeeksObj* object)
{
	HDimension* dimension = (HDimension*)object;
	dimension->m_units = (DimensionUnits)value;
	wxGetApp().Repaint();
}

static void on_set_scale(double value, HeeksObj* object)
{
	HDimension* dimension = (HDimension*)object;
	dimension->m_scale = value;
	wxGetApp().Repaint();
}

void HDimension::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyTrsf(_("orientation"), m_trsf, this, on_set_trsf));

		std::list< wxString > choices;
	choices.push_back ( wxString ( _("between two points") ) );
	choices.push_back ( wxString ( _("between two points, XY only") ) );
	choices.push_back ( wxString ( _("orthogonal") ) );
	list->push_back ( new PropertyChoice ( _("mode"),  choices, m_mode, this, on_set_mode ) );

	list->push_back ( new PropertyDouble ( _("scale"),  m_scale, this, on_set_scale ) );

	{
		std::list< wxString > choices;
		choices.push_back ( wxString ( _("use view units") ) );
		choices.push_back ( wxString ( _("inches") ) );
		choices.push_back ( wxString ( _("mm") ) );
		list->push_back ( new PropertyChoice ( _("units"),  choices, m_units, this, on_set_units ) );
	}

	wxString text = MakeText();
	list->push_back(new PropertyString(_("dimension value"), text, this, NULL));

	EndedObject::GetProperties(list);
}

bool HDimension::Stretch(const double *p, const double* shift, void* data)
{
	EndedObject::Stretch(p,shift,data);
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	if(data == &m_p2){
		m_p2->m_p = vp.XYZ() + vshift.XYZ();
	}
	return false;
}

void HDimension::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Dimension" );
	root->LinkEndChild( element );

	double m[16];
	extract(m_trsf, m);

	element->SetAttribute("col", m_color.COLORREF_color());
	element->SetDoubleAttribute("m0", m[0] );
	element->SetDoubleAttribute("m1", m[1] );
	element->SetDoubleAttribute("m2", m[2] );
	element->SetDoubleAttribute("m3", m[3] );
	element->SetDoubleAttribute("m4", m[4] );
	element->SetDoubleAttribute("m5", m[5] );
	element->SetDoubleAttribute("m6", m[6] );
	element->SetDoubleAttribute("m7", m[7] );
	element->SetDoubleAttribute("m8", m[8] );
	element->SetDoubleAttribute("m9", m[9] );
	element->SetDoubleAttribute("ma", m[10]);
	element->SetDoubleAttribute("mb", m[11]);
	element->SetDoubleAttribute("scale",m_scale);
	element->SetAttribute("mode", m_mode);
	switch(m_units)
	{
	case DimensionUnitsMM:
		element->SetAttribute("units", "mm");
		break;
	case DimensionUnitsInches:
		element->SetAttribute("units", "inches");
		break;
	default:
		element->SetAttribute("units", "global");
		break;
	}

	WriteBaseXML(element);
}

// static
HeeksObj* HDimension::ReadFromXMLElement(TiXmlElement* pElem)
{
	double m[16];
	wxString text;
	HeeksColor c;
	double p0[3] = {0, 0, 0};
	double p1[3] = {0, 0, 0};
	double p2[3] = {0, 0, 0};
	double scale=1;

	DimensionMode mode = TwoPointsDimensionMode;
	DimensionUnits units = DimensionUnitsGlobal;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){c = HeeksColor((long)(a->IntValue()));}
		else if(name == "m0"){m[0] = a->DoubleValue();}
		else if(name == "m1"){m[1] = a->DoubleValue();}
		else if(name == "m2"){m[2] = a->DoubleValue();}
		else if(name == "m3"){m[3] = a->DoubleValue();}
		else if(name == "m4"){m[4] = a->DoubleValue();}
		else if(name == "m5"){m[5] = a->DoubleValue();}
		else if(name == "m6"){m[6] = a->DoubleValue();}
		else if(name == "m7"){m[7] = a->DoubleValue();}
		else if(name == "m8"){m[8] = a->DoubleValue();}
		else if(name == "m9"){m[9] = a->DoubleValue();}
		else if(name == "ma"){m[10]= a->DoubleValue();}
		else if(name == "mb"){m[11]= a->DoubleValue();}
		else if(name == "scale"){scale= a->DoubleValue();}
		else if(name == "mode"){mode = (DimensionMode)(a->IntValue());}
		else if(name == "units"){
			const char* str = a->Value();
			switch(str[0])
			{
			case 'm':
				units = DimensionUnitsMM;
				break;
			case 'i':
				units = DimensionUnitsInches;
				break;
			}
		}
	}

	HDimension* new_object = new HDimension(make_matrix(m), make_point(p0), make_point(p1), make_point(p2), mode, units, &c);
	new_object->ReadBaseXML(pElem);
	new_object->m_scale = scale;

	if(new_object->GetNumChildren()>3)
	{
		//This is a new style line, with children points
		new_object->Remove(new_object->A);
		new_object->Remove(new_object->B);
		new_object->Remove(new_object->m_p2);
		delete new_object->A;
		delete new_object->B;
		delete new_object->m_p2;
		new_object->A = (HPoint*)new_object->GetFirstChild();
		new_object->B = (HPoint*)new_object->GetNextChild();
		new_object->m_p2 = (HPoint*)new_object->GetNextChild();
		new_object->A->m_draw_unselected = false;
		new_object->B->m_draw_unselected = false;
		new_object->m_p2->m_draw_unselected = false;
		new_object->A->SetSkipForUndo(true);
		new_object->B->SetSkipForUndo(true);
		new_object->m_p2->SetSkipForUndo(true);
	}


	return new_object;
}

// static
void HDimension::draw_arrow_line(DimensionMode mode, const gp_Pnt &p0, const gp_Pnt &p1, const gp_Pnt &p2, const gp_Dir &xdir, const gp_Dir &ydir, double width, double scale)
{
	double short_line_length = 5.0 * scale;
	double long_line_extra = 2.0 * scale;

	double y0 = gp_Vec(p2.XYZ()) * gp_Vec(ydir.XYZ()) - gp_Vec(p0.XYZ()) * gp_Vec(ydir.XYZ());
	double y1 = gp_Vec(p2.XYZ()) * gp_Vec(ydir.XYZ()) - gp_Vec(p1.XYZ()) * gp_Vec(ydir.XYZ());

	gp_Pnt vt0( p0.XYZ() + ydir.XYZ() * y0);
	gp_Pnt vt1( p1.XYZ() + ydir.XYZ() * y1);
	gp_Pnt vt2 = p2;

	gp_Dir along_dir = make_vector(gp_Pnt(p0), gp_Pnt(p1));
	gp_Dir xdir_along = xdir;
	if(along_dir * xdir < 0)xdir_along = -xdir;

	gp_Pnt new_vt0 = vt0;
	gp_Pnt new_vt1 = vt1;

	gp_Pnt middle_text_point = p2.XYZ() + along_dir.XYZ() * (width/2 * scale);
	double x0 = gp_Vec(p0.XYZ()) * gp_Vec(xdir_along.XYZ());
	double x1 = gp_Vec(p1.XYZ()) * gp_Vec(xdir_along.XYZ());
	double xm = gp_Vec(middle_text_point.XYZ()) * gp_Vec(xdir_along.XYZ());

	double arrow_head_scale = scale;
	if(xm < x0 || xm > x1)
	{
		arrow_head_scale *= -1;
	}

	double distance = vt0.Distance(vt1);

	// draw arrow heads, if there's room
	if((distance > 2 * scale + wxGetApp().m_geom_tol) || (xm < x0) || (xm > x1))
	{
		gp_XYZ t[2][3];
		t[0][0] = vt0.XYZ();
		t[0][1] = vt0.XYZ() + xdir_along.XYZ() * arrow_head_scale + ydir.XYZ() * (arrow_head_scale * (-0.4));
		t[0][2] = vt0.XYZ() + xdir_along.XYZ() * arrow_head_scale + ydir.XYZ() * (arrow_head_scale * 0.4);
		t[1][0] = vt1.XYZ();
		t[1][1] = vt1.XYZ() + xdir_along.XYZ() * (-arrow_head_scale) + ydir.XYZ() * (arrow_head_scale * 0.4);
		t[1][2] = vt1.XYZ() + xdir_along.XYZ() * (-arrow_head_scale) + ydir.XYZ() * (arrow_head_scale * (-0.4));

		// adjust line vertices
		new_vt0 = gp_Pnt(vt0.XYZ() + xdir_along.XYZ() * arrow_head_scale);
		new_vt1 = gp_Pnt(vt1.XYZ() + xdir_along.XYZ() * (-arrow_head_scale));

		// draw two triangles
		for(int i = 0; i<2; i++)
		{
			glBegin(GL_LINE_STRIP);
			glVertex3d(t[i][0].X(), t[i][0].Y(), t[i][0].Z());
			glVertex3d(t[i][1].X(), t[i][1].Y(), t[i][1].Z());
			glVertex3d(t[i][2].X(), t[i][2].Y(), t[i][2].Z());
			glVertex3d(t[i][0].X(), t[i][0].Y(), t[i][0].Z());
			glEnd();
		}
	}

	// draw side lines
	glBegin(GL_LINES);
	glVertex3d(p0.X(), p0.Y(), p0.Z());
	glVertex3d(vt0.X(), vt0.Y(), vt0.Z());
	glVertex3d(p1.X(), p1.Y(), p1.Z());
	glVertex3d(vt1.X(), vt1.Y(), vt1.Z());
	glEnd();

	if(xm < x0)
	{
		// long line first
		gp_Pnt vt4 = vt2.XYZ() + xdir_along.XYZ() * (-long_line_extra);
		glBegin(GL_LINES);
		glVertex3d(vt2.X(), vt2.Y(), vt2.Z());
		glVertex3d(new_vt0.X(), new_vt0.Y(), new_vt0.Z());
		glEnd();

		// little line
		gp_Pnt vt3 = new_vt1.XYZ() + xdir_along.XYZ() * short_line_length;
		glBegin(GL_LINES);
		glVertex3d(new_vt1.X(), new_vt1.Y(), new_vt1.Z());
		glVertex3d(vt3.X(), vt3.Y(), vt3.Z());
		glEnd();
	}
	else if(xm > x1)
	{
		// little first
		gp_Pnt vt3 = new_vt0.XYZ() - xdir_along.XYZ() * short_line_length;
		glBegin(GL_LINES);
		glVertex3d(vt3.X(), vt3.Y(), vt3.Z());
		glVertex3d(new_vt0.X(), new_vt0.Y(), new_vt0.Z());
		glEnd();

		// long line
		glBegin(GL_LINES);
		gp_Pnt vt4 = vt2.XYZ() + xdir_along.XYZ() * (width * scale + long_line_extra);
		glVertex3d(vt1.X(), vt1.Y(), vt1.Z());
		glVertex3d(vt4.X(), vt4.Y(), vt4.Z());
		glEnd();
	}
	else
	{
		// draw the arrow line
		glBegin(GL_LINES);
		glVertex3d(new_vt0.X(), new_vt0.Y(), new_vt0.Z());
		glVertex3d(new_vt1.X(), new_vt1.Y(), new_vt1.Z());
		glEnd();
	}
}

// static
void HDimension::WriteToConfig(HeeksConfig& config)
{
	config.Write(_T("DimensionDrawFlat"), DrawFlat);
}

// static
void HDimension::ReadFromConfig(HeeksConfig& config)
{
	config.Read(_T("DimensionDrawFlat"), &DrawFlat, false);
}
