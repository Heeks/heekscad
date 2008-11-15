// CordinateSystem.cpp

#include "stdafx.h"
#include "CoordinateSystem.h"
#include "PropertyVertex.h"
#include "../interface/PropertyDouble.h"
#include "../interface/Tool.h"
#include "HeeksFrame.h"
#include "ObjPropsCanvas.h"
#include "../tinyxml/tinyxml.h"
#include "ToolImage.h"

wxIcon* CoordinateSystem::m_icon = NULL;
double CoordinateSystem::size = 30;
bool CoordinateSystem::size_is_pixels = true;

wxIcon* CoordinateSystem::GetIcon()
{
	if(m_icon == NULL)
	{
		wxString exe_folder = wxGetApp().GetExeFolder();
		m_icon = new wxIcon(exe_folder + _T("/icons/coordsys.png"), wxBITMAP_TYPE_PNG);
	}
	return m_icon;
}

CoordinateSystem::CoordinateSystem(const wxString& str, const gp_Pnt &o, const gp_Dir &x, const gp_Dir &y)
{
	m_title = str;
	m_o = o;
	m_x = x;
	m_y = y;
}

CoordinateSystem::CoordinateSystem(const CoordinateSystem &c)
{
	operator=(c);
}

const CoordinateSystem& CoordinateSystem::operator=(const CoordinateSystem &c)
{
	HeeksObj::operator=(c);
	m_o = c.m_o;
	m_x = c.m_x;
	m_y = c.m_y;
	return *this;
}

CoordinateSystem::~CoordinateSystem(void)
{
}

// static
void CoordinateSystem::RenderDatum(bool bright)
{
	// red, green, blue for x, y, z, like I saw on Open Arena
	double s = size;
	if(size_is_pixels)s /= wxGetApp().GetPixelScale();
	glBegin(GL_LINES);
	if(bright)glColor3ub(255, 0, 0);
	else glColor3ub(128, 64, 64);
	glVertex3d(0, 0, 0);
	glVertex3d(s, 0, 0);
	if(bright)glColor3ub(0, 255, 0);
	else glColor3ub(64, 128, 64);
	glVertex3d(0, 0, 0);
	glVertex3d(0, s, 0);
	if(bright)glColor3ub(0, 0, 255);
	else glColor3ub(64, 64, 128);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, s);
	glEnd();
}

void CoordinateSystem::glCommands(bool select, bool marked, bool no_color)
{
	if(marked)glLineWidth(2);
	glPushMatrix();
	double m[16];
	extract_transposed(GetMatrix(), m);
	glMultMatrixd(m);
	bool bright = false;
	GLfloat save_depth_range[2];
	
	if(this == wxGetApp().m_current_coordinate_system)
	{
		bright = true;
		// set depth range, so that it appears in front of everything
		glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
		glDepthRange(0, 0);
	}

	RenderDatum(bright);

	if(bright)
	{
		// restore depth range
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}

	glPopMatrix();
	glLineWidth(1);
}

void CoordinateSystem::GetBox(CBox &box)
{
	gp_Pnt vt(0, 0, 0);
	vt.Transform(GetMatrix());
	double p[3];
	extract(vt, p);
	box.Insert(p);
}

HeeksObj *CoordinateSystem::MakeACopy(void)const
{
	return new CoordinateSystem(*this);
}

bool CoordinateSystem::ModifyByMatrix(const double *m)
{
	gp_Trsf mat = make_matrix(m);
	m_o.Transform(mat);
	m_x.Transform(mat);
	m_y.Transform(mat);
	return false;
}

static void on_set_pos(const gp_Pnt& pos, HeeksObj* object)
{
	((CoordinateSystem*)object)->m_o = pos;
	wxGetApp().Repaint();
}

static void on_set_vertical_angle(double value, HeeksObj* object)
{
	((CoordinateSystem*)object)->m_vertical_angle = value * Pi/180;
	((CoordinateSystem*)object)->AnglesToAxes();
	wxGetApp().Repaint();
}

static void on_set_horizontal_angle(double value, HeeksObj* object)
{
	((CoordinateSystem*)object)->m_horizontal_angle = value * Pi/180;
	((CoordinateSystem*)object)->AnglesToAxes();
	wxGetApp().Repaint();
}

static void on_set_twist_angle(double value, HeeksObj* object)
{
	((CoordinateSystem*)object)->m_twist_angle = value * Pi/180;
	((CoordinateSystem*)object)->AnglesToAxes();
	wxGetApp().Repaint();
}

static CoordinateSystem* coord_system_for_Tool = NULL;

class CoordSystem3Points:public Tool{
	// to set the position and orientation of a CoordinateSystem, by picking 1 - datum position, 2 - x axis position, 3 - y axis position ( somewhere where y > 0 )
private:
	static wxBitmap* m_bitmap;

public:
	void Run(){
		coord_system_for_Tool->PickFrom3Points();
	}
	const wxChar* GetTitle(){return _T("CoordSystem3Points");}
	wxBitmap* Bitmap(){if(m_bitmap == NULL){wxString exe_folder = wxGetApp().GetExeFolder();m_bitmap = new wxBitmap(exe_folder + _T("/bitmaps/coords3.png"), wxBITMAP_TYPE_PNG);}return m_bitmap;}
	const wxChar* GetToolTip(){return _("Set position and orientation by picking 3 points");}
};
wxBitmap* CoordSystem3Points::m_bitmap = NULL;

static CoordSystem3Points coord_system_3_points;

class SetCoordSystemActive:public Tool{
	// setthe coordinate system as the current one
private:
	static wxBitmap* m_bitmap;

public:
	void Run(){
		wxGetApp().m_current_coordinate_system = coord_system_for_Tool;
		wxGetApp().m_frame->m_properties->RefreshByRemovingAndAddingAll();
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("SetCoordSystemActive");}
	wxBitmap* Bitmap(){if(m_bitmap == NULL){wxString exe_folder = wxGetApp().GetExeFolder();m_bitmap = new wxBitmap(ToolImage(_T("setcoordsys")));}return m_bitmap;}
	const wxChar* GetToolTip(){return _("Set this coordinate system as the active one");}
};
wxBitmap* SetCoordSystemActive::m_bitmap = NULL;

static SetCoordSystemActive coord_system_set;

class UnsetCoordSystemActive:public Tool{
	// set world coordinate system active again
private:
	static wxBitmap* m_bitmap;

public:
	void Run(){
		wxGetApp().m_current_coordinate_system = NULL;
		wxGetApp().m_frame->m_properties->RefreshByRemovingAndAddingAll();
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("UnsetCoordSystemActive");}
	wxBitmap* Bitmap(){if(m_bitmap == NULL){wxString exe_folder = wxGetApp().GetExeFolder();m_bitmap = new wxBitmap(ToolImage(_T("unsetcoordsys")));}return m_bitmap;}
	const wxChar* GetToolTip(){return _("Set world coordinate system active");}
};
wxBitmap* UnsetCoordSystemActive::m_bitmap = NULL;

static UnsetCoordSystemActive coord_system_unset;

void CoordinateSystem::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	coord_system_for_Tool = this;
	t_list->push_back(&coord_system_3_points);
	t_list->push_back((this == wxGetApp().m_current_coordinate_system) ? ((Tool*)(&coord_system_unset)) : ((Tool*)(&coord_system_set)));
}

void CoordinateSystem::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyVertex(_("position"), m_o, this, on_set_pos));
	list->push_back(new PropertyVertex(_("x axis"), gp_Pnt(m_x.XYZ()), NULL));
	list->push_back(new PropertyVertex(_("y axis"), gp_Pnt(m_y.XYZ()), NULL));
	AxesToAngles(m_x, m_y, m_vertical_angle, m_horizontal_angle, m_twist_angle);
	list->push_back(new PropertyDouble(_("Vertical Angle"), m_vertical_angle * 180/Pi, this, on_set_vertical_angle));
	list->push_back(new PropertyDouble(_("Horizontal Angle"), m_horizontal_angle * 180/Pi, this, on_set_horizontal_angle));
	list->push_back(new PropertyDouble(_("Twist Angle"), m_twist_angle * 180/Pi, this, on_set_twist_angle));
}

void CoordinateSystem::WriteXML(TiXmlElement *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "CoordinateSystem" );
	root->LinkEndChild( element );  
	element->SetAttribute("title", Ttc(m_title.c_str()));
	element->SetDoubleAttribute("ox", m_o.X());
	element->SetDoubleAttribute("oy", m_o.Y());
	element->SetDoubleAttribute("oz", m_o.Z());
	element->SetDoubleAttribute("xx", m_x.X());
	element->SetDoubleAttribute("xy", m_x.Y());
	element->SetDoubleAttribute("xz", m_x.Z());
	element->SetDoubleAttribute("yx", m_y.X());
	element->SetDoubleAttribute("yy", m_y.Y());
	element->SetDoubleAttribute("yz", m_y.Z());
	WriteBaseXML(element);
}

gp_Trsf CoordinateSystem::GetMatrix()
{
	return make_matrix(m_o, m_x, m_y);
}

// static
HeeksObj* CoordinateSystem::ReadFromXMLElement(TiXmlElement* pElem)
{
	gp_Pnt o;
	gp_Dir x, y;
	wxString title;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "title"){title.assign(Ctt(a->Value()));}
		else if(name == "ox"){o.SetX(a->DoubleValue());}
		else if(name == "oy"){o.SetY(a->DoubleValue());}
		else if(name == "oz"){o.SetZ(a->DoubleValue());}
		else if(name == "xx"){x.SetX(a->DoubleValue());}
		else if(name == "xy"){x.SetY(a->DoubleValue());}
		else if(name == "xz"){x.SetZ(a->DoubleValue());}
		else if(name == "yx"){y.SetX(a->DoubleValue());}
		else if(name == "yy"){y.SetY(a->DoubleValue());}
		else if(name == "yz"){y.SetZ(a->DoubleValue());}
	}

	CoordinateSystem* new_object = new CoordinateSystem(title, o, x, y);
	new_object->ReadBaseXML(pElem);

	return new_object;
}

// code for AxesToAngles copied from http://tog.acm.org/GraphicsGems/gemsiv/euler_angle/EulerAngles.c

#define EulSafe	     "\000\001\002\000"
#define EulNext	     "\001\002\000\001"
#define EulGetOrd(ord,i,j,k,h,n,s,f) {unsigned o=ord;f=o&1;o>>=1;s=o&1;o>>=1;n=o&1;o>>=1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];h=s?k:i;}
#define EulOrd(i,p,r,f)	   (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))
#define EulOrdZXZs    EulOrd(2,0,1,0)

//static
void CoordinateSystem::AxesToAngles(const gp_Dir &x, const gp_Dir &y, double &v_angle, double &h_angle, double &t_angle)
{
	double M[4][4];
	extract(make_matrix(gp_Pnt(0, 0, 0), x, y), M[0]);
	int order = EulOrdZXZs;

    int i,j,k,h,n,s,f;
    EulGetOrd(order,i,j,k,h,n,s,f);
    if (s==1) {
	double sy = sqrt(M[i][j]*M[i][j] + M[i][k]*M[i][k]);
	if (sy > 16*FLT_EPSILON) {
	    t_angle = atan2(M[i][j], M[i][k]);
	    v_angle = atan2(sy, M[i][i]);
	    h_angle = atan2(M[j][i], -M[k][i]);
	} else {
	    t_angle = atan2(-M[j][k], M[j][j]);
	    v_angle = atan2(sy, M[i][i]);
	    h_angle = 0;
	}
    } else {
	double cy = sqrt(M[i][i]*M[i][i] + M[j][i]*M[j][i]);
	if (cy > 16*DBL_EPSILON) {
	    t_angle = atan2(M[k][j], M[k][k]);
	    v_angle = atan2(-M[k][i], cy);
	    h_angle = atan2(M[j][i], M[i][i]);
	} else {
	    t_angle = atan2(-M[j][k], M[j][j]);
	    v_angle = atan2(-M[k][i], cy);
	    h_angle = 0;
	}
    }
    if (n==1) {t_angle = -t_angle; v_angle = - v_angle; h_angle = -h_angle;}
    if (f==1) {double t = t_angle; t_angle = h_angle; h_angle = t;}
}

//static
void CoordinateSystem::AnglesToAxes(const double &v_angle, const double &h_angle, const double &t_angle, gp_Dir &x, gp_Dir &y)
{
	gp_Trsf zmat1;
	zmat1.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), t_angle);

	gp_Trsf xmat;
	xmat.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0)), v_angle);

	gp_Trsf zmat2;
	zmat2.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), h_angle);

	gp_Trsf mat = zmat2 * xmat * zmat1;

	x = gp_Dir(1, 0, 0).Transformed(mat);
	y = gp_Dir(0, 1, 0).Transformed(mat);
}

static double origin[3];
static double x_axis_pos[3];
static double y_axis_pos[3];

static void on_set_origin(const double* pos){ memcpy(origin, pos, 3*sizeof(double));}
static void on_set_x(const double* pos){ memcpy(x_axis_pos, pos, 3*sizeof(double));}
static void on_set_y(const double* pos){ memcpy(y_axis_pos, pos, 3*sizeof(double));}

void CoordinateSystem::PickFrom3Points()
{
	if(!wxGetApp().PickPosition(_("Pick the location"), origin, on_set_origin))return;
	if(!wxGetApp().PickPosition(_("Pick a point on the x-axis"), x_axis_pos, on_set_x))return;
	if(!wxGetApp().PickPosition(_("Pick a point where y > 0"), y_axis_pos, on_set_y))return;

	coord_system_for_Tool->m_o = make_point(origin);
	coord_system_for_Tool->m_x = make_vector(coord_system_for_Tool->m_o, make_point(x_axis_pos));
	coord_system_for_Tool->m_y = make_vector(coord_system_for_Tool->m_o, make_point(y_axis_pos));

	wxGetApp().m_frame->m_properties->ApplyChanges();
	wxGetApp().Repaint();
}
