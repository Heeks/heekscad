// HDimension.cpp
#include "stdafx.h"
#include "HDimension.h"
#include "../interface/PropertyDouble.h"
#include "PropertyTrsf.h"
#include "../tinyxml/tinyxml.h"
#include "Gripper.h"

wxIcon* HDimension::m_icon = NULL;


HDimension::HDimension(const gp_Trsf &trsf, const wxString &text, const gp_Pnt &p0, const gp_Pnt &p1, const gp_Pnt &p2, DimensionMode mode, const HeeksColor* col): m_trsf(trsf), m_text(text), m_p0(p0), m_p1(p1), m_p2(p2), m_mode(mode), m_color(*col), m_scale(1.0)
{
}

HDimension::HDimension(const HDimension &b)
{
	operator=(b);
}

HDimension::~HDimension(void)
{
}

const HDimension& HDimension::operator=(const HDimension &b)
{
	HeeksObj::operator=(b);
	m_trsf = b.m_trsf;
	m_text = b.m_text;
	m_color = b.m_color;
	m_p0 = b.m_p0;
	m_p1 = b.m_p1;
	m_p2 = b.m_p2;
	m_mode = b.m_mode;
	m_scale = b.m_scale;
	return *this;
}

void HDimension::glCommands(bool select, bool marked, bool no_color)
{
	if(m_p0.IsEqual(m_p1, wxGetApp().m_geom_tol))return;

	if(!no_color)wxGetApp().glColorEnsuringContrast(m_color);

	switch(m_mode)
	{
	case TwoPointsDimensionMode:
		{
			gp_Dir zdir = gp_Dir(0, 0, 1).Transformed(m_trsf);
			gp_Dir xdir(make_vector(m_p0, m_p1));
			gp_Dir ydir = zdir ^ xdir;

			draw_arrow_line(m_p0, m_p1, m_p2, xdir ,ydir, m_scale);

			// make a matrix at top left of text
			float width, height;
			if(!wxGetApp().get_text_size(m_text, &width, &height))return;
			gp_Pnt text_top_left( m_p2.XYZ() + ydir.XYZ() * (m_scale * height) );
			gp_Trsf text_matrix = make_matrix(text_top_left, xdir, ydir);

			glPushMatrix();
			double m[16];
			extract_transposed(text_matrix, m);
			glMultMatrixd(m);

			wxGetApp().render_text(m_text);

			glPopMatrix();
		}
		break;

	}
}

void HDimension::GetBox(CBox &box)
{
	gp_Pnt vt(0, 0, 0);
	vt.Transform(m_trsf);
	double p[3];
	extract(vt, p);
	box.Insert(p);

	float width, height;
	if(!wxGetApp().get_text_size(m_text, &width, &height))return;

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

wxIcon* HDimension::GetIcon()
{
	if(m_icon == NULL)m_icon = new wxIcon(wxGetApp().GetExeFolder() + _T("/icons/dimension.png"), wxBITMAP_TYPE_PNG);
	return m_icon;
}

bool HDimension::ModifyByMatrix(const double *m)
{
	gp_Trsf mat = make_matrix(m);
	m_trsf = mat * m_trsf;
	return false;
}

void HDimension::GetGripperPositions(std::list<double> *list, bool just_for_endof)
{
	float width, height;
	if(!wxGetApp().get_text_size(m_text, &width, &height))return;

	gp_Pnt point[4];
	point[0] = gp_Pnt(0, 0, 0);
	point[1] = gp_Pnt(width, 0, 0);
	point[2] = gp_Pnt(0, -height, 0);
	point[3] = gp_Pnt(width, -height, 0);

	for(int i = 0; i<4; i++)point[i].Transform(m_trsf);

	list->push_back(GripperTypeTranslate);
	list->push_back(point[0].X());
	list->push_back(point[0].Y());
	list->push_back(point[0].Z());
	list->push_back(GripperTypeRotateObject);
	list->push_back(point[1].X());
	list->push_back(point[1].Y());
	list->push_back(point[1].Z());
	list->push_back(GripperTypeRotateObject);
	list->push_back(point[2].X());
	list->push_back(point[2].Y());
	list->push_back(point[2].Z());
	list->push_back(GripperTypeScale);
	list->push_back(point[3].X());
	list->push_back(point[3].Y());
	list->push_back(point[3].Z());
}

static void on_set_trsf(const gp_Trsf &trsf, HeeksObj* object){
	((HDimension*)object)->m_trsf = trsf;
	wxGetApp().Repaint();
}

void HDimension::GetProperties(std::list<Property *> *list)
{
	__super::GetProperties(list);

	list->push_back(new PropertyTrsf(_T("orientation"), m_trsf, this, on_set_trsf));
}

bool HDimension::Stretch(const double *p, const double* shift)
{
	return false;
}

void HDimension::OnEditString(const wxChar* str){
	m_text.assign(str);
	wxGetApp().WasModified(this);
}

void HDimension::WriteXML(TiXmlElement *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Text" );
	root->LinkEndChild( element );  
	element->SetAttribute("text", Ttc(m_text.c_str()));

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
	element->SetDoubleAttribute("sx", m_p0.X());
	element->SetDoubleAttribute("sy", m_p0.Y());
	element->SetDoubleAttribute("sz", m_p0.Z());
	element->SetDoubleAttribute("ex", m_p1.X());
	element->SetDoubleAttribute("ey", m_p1.Y());
	element->SetDoubleAttribute("ez", m_p1.Z());
	element->SetDoubleAttribute("cx", m_p2.X());
	element->SetDoubleAttribute("cy", m_p2.Y());
	element->SetDoubleAttribute("cz", m_p2.Z());
	element->SetAttribute("mode", m_mode);

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

	DimensionMode mode = TwoPointsDimensionMode;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){c = HeeksColor(a->IntValue());}
		else if(name == "text"){text.assign(Ctt(a->Value()));}
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
		else if(name == "sx"){p0[0]= a->DoubleValue();}
		else if(name == "sy"){p0[1]= a->DoubleValue();}
		else if(name == "sz"){p0[2]= a->DoubleValue();}
		else if(name == "ex"){p1[0]= a->DoubleValue();}
		else if(name == "ey"){p1[1]= a->DoubleValue();}
		else if(name == "ez"){p1[2]= a->DoubleValue();}
		else if(name == "cx"){p2[0]= a->DoubleValue();}
		else if(name == "cy"){p2[1]= a->DoubleValue();}
		else if(name == "cz"){p2[2]= a->DoubleValue();}
		else if(name == "mode"){mode = (DimensionMode)(a->IntValue());}
	}

	HDimension* new_object = new HDimension(make_matrix(m), text, make_point(p0), make_point(p1), make_point(p2), mode, &c);
	new_object->ReadBaseXML(pElem);

	return new_object;
}

// static
void HDimension::draw_arrow_line(const gp_Pnt &p0, const gp_Pnt &p1, const gp_Pnt &p2, const gp_Dir &xdir, const gp_Dir &ydir, double scale)
{
	double y2 = gp_Vec(p2.XYZ()) * gp_Vec(ydir.XYZ()) - gp_Vec(p0.XYZ()) * gp_Vec(ydir.XYZ());

	gp_Pnt vt0( p0.XYZ() + ydir.XYZ() * y2);
	gp_Pnt vt1( p1.XYZ() + ydir.XYZ() * y2);
	gp_Pnt vt2 = p2;

	// to do - arrow heads too

	// just a line, for now
	glBegin(GL_LINES);
	glVertex3d(vt0.X(), vt0.Y(), vt0.Z());
	glVertex3d(vt1.X(), vt1.Y(), vt1.Z());
	glEnd();
}