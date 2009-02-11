// dxf.cpp

#include "stdafx.h"
#include "dxf.h"
#include "HLine.h"
#include "HArc.h"
#include "Sketch.h"

CDxfWrite::CDxfWrite(const wxChar* filepath)
{
	// start the file
	m_fail = false;
#ifdef __WXMSW__
	m_ofs = new ofstream(filepath, ios::out);
#else
	m_ofs = new ofstream(Ttc(filepath), ios::out);
#endif
	if(!(*m_ofs)){
		m_fail = true;
		return;
	}

	// start
	(*m_ofs) << 0          << endl;
	(*m_ofs) << "SECTION"  << endl;
	(*m_ofs) << 2          << endl;
	(*m_ofs) << "ENTITIES" << endl;
}

CDxfWrite::~CDxfWrite()
{
	// end
	(*m_ofs) << 0          << endl;
	(*m_ofs) << "ENDSEC"   << endl;
	(*m_ofs) << 0          << endl;
	(*m_ofs) << "EOF";

	delete m_ofs;
}

void CDxfWrite::WriteLine(const double* s, const double* e)
{
	(*m_ofs) << 0			<< endl;
	(*m_ofs) << "LINE"		<< endl;
	(*m_ofs) << 8			<< endl;	// Group code for layer name
	(*m_ofs) << 0			<< endl;	// Layer number
	(*m_ofs) << 10			<< endl;	// Start point of line
	(*m_ofs) << s[0]		<< endl;	// X in WCS coordinates
	(*m_ofs) << 20			<< endl;
	(*m_ofs) << s[1]		<< endl;	// Y in WCS coordinates
	(*m_ofs) << 30			<< endl;
	(*m_ofs) << s[2]		<< endl;	// Z in WCS coordinates
	(*m_ofs) << 11			<< endl;	// End point of line
	(*m_ofs) << e[0]		<< endl;	// X in WCS coordinates
	(*m_ofs) << 21			<< endl;
	(*m_ofs) << e[1]		<< endl;	// Y in WCS coordinates
	(*m_ofs) << 31			<< endl;
	(*m_ofs) << e[2]		<< endl;	// Z in WCS coordinates
}

void CDxfWrite::WriteArc(const double* s, const double* e, const double* c, bool dir)
{
	double ax = s[0] - c[0];
	double ay = s[1] - c[1];
	double bx = e[0] - c[0];
	double by = e[1] - c[1];

	double start_angle = atan2(ay, ax) * 180/Pi;
	double end_angle = atan2(by, bx) * 180/Pi;
	double radius = sqrt(ax*ax + ay*ay);
	if(!dir){
		double temp = start_angle;
		start_angle = end_angle;
		end_angle = temp;
	}
	(*m_ofs) << 0			<< endl;
	(*m_ofs) << "ARC"		<< endl;
	(*m_ofs) << 8			<< endl;	// Group code for layer name
	(*m_ofs) << 0			<< endl;	// Layer number
	(*m_ofs) << 10			<< endl;	// Centre X
	(*m_ofs) << c[0]		<< endl;	// X in WCS coordinates
	(*m_ofs) << 20			<< endl;
	(*m_ofs) << c[1]		<< endl;	// Y in WCS coordinates
	(*m_ofs) << 30			<< endl;
	(*m_ofs) << c[2]		<< endl;	// Z in WCS coordinates
	(*m_ofs) << 40			<< endl;	// 
	(*m_ofs) << radius		<< endl;	// Radius
	(*m_ofs) << 50			<< endl;
	(*m_ofs) << start_angle	<< endl;	// Start angle
	(*m_ofs) << 51			<< endl;
	(*m_ofs) << end_angle	<< endl;	// End angle
}

CDxfRead::CDxfRead(const wxChar* filepath)
{
	// start the file
	m_fail = false;
#ifdef __WXMSW__
	m_ifs = new ifstream(filepath);
#else
	m_ifs = new ifstream(Ttc(filepath));
#endif
	if(!(*m_ifs)){
		m_fail = true;
		return;
	}
}

CDxfRead::~CDxfRead()
{
	delete m_ifs;
}

bool CDxfRead::ReadLine(bool undoably)
{
	double s[3] = {0, 0, 0};
	double e[3] = {0, 0, 0};

	while(!((*m_ifs).eof()))
	{
		get_line();
		int n;
		if(sscanf(m_str, "%d", &n) != 1)return false;
		switch(n){
			case 0:
				// next item found, so finish with line
				OnReadLine(s, e, undoably);
				return true;
			case 10:
				// start x
				get_line();
				if(sscanf(m_str, "%lf", &(s[0])) != 1)return false;
				break;
			case 20:
				// start y
				get_line();
				if(sscanf(m_str, "%lf", &(s[1])) != 1)return false;
				break;
			case 30:
				// start z
				get_line();
				if(sscanf(m_str, "%lf", &(s[2])) != 1)return false;
				break;
			case 11:
				// end x
				get_line();
				if(sscanf(m_str, "%lf", &(e[0])) != 1)return false;
				break;
			case 21:
				// end y
				get_line();
				if(sscanf(m_str, "%lf", &(e[1])) != 1)return false;
				break;
			case 31:
				// end z
				get_line();
				if(sscanf(m_str, "%lf", &(e[2])) != 1)return false;
				break;
			case 100:
			case 39:
			case 210:
			case 220:
			case 230:
				// skip the next line
				get_line();
				break;
			default:
				// skip the next line
				get_line();
				break;
		}
	}

	OnReadLine(s, e, undoably);
	return false;
}

bool CDxfRead::ReadArc(bool undoably)
{
	double start_angle = 0.0;// in degrees
	double end_angle = 0.0;
	double radius = 0.0;
	double c[3]; // centre

	while(!((*m_ifs).eof()))
	{
		get_line();
		int n;
		if(sscanf(m_str, "%d", &n) != 1)return false;
		switch(n){
			case 0:
				// next item found, so finish with arc
				OnReadArc(start_angle, end_angle, radius, c, undoably);
				return true;
			case 10:
				// centre x
				get_line();
				if(sscanf(m_str, "%lf", &(c[0])) != 1)return false;
				break;
			case 20:
				// centre y
				get_line();
				if(sscanf(m_str, "%lf", &(c[1])) != 1)return false;
				break;
			case 30:
				// centre z
				get_line();
				if(sscanf(m_str, "%lf", &(c[2])) != 1)return false;
				break;
			case 40:
				// radius
				get_line();
				if(sscanf(m_str, "%lf", &radius) != 1)return false;
				break;
			case 50:
				// start angle
				get_line();
				if(sscanf(m_str, "%lf", &start_angle) != 1)return false;
				break;
			case 51:
				// end angle
				get_line();
				if(sscanf(m_str, "%lf", &end_angle) != 1)return false;
				break;
			case 100:
			case 39:
			case 210:
			case 220:
			case 230:
				// skip the next line
				get_line();
				break;
			default:
				// skip the next line
				get_line();
				break;
		}
	}

	OnReadArc(start_angle, end_angle, radius, c, undoably);
	return false;
}

static bool poly_prev_found = false;
static double poly_prev_x;
static double poly_prev_y;
static double poly_prev_bulge_found;
static double poly_prev_bulge;
static bool poly_first_found = false;
static double poly_first_x;
static double poly_first_y;

static void AddPolyLinePoint(CDxfRead* dxf_read, double x, double y, bool bulge_found, double bulge, bool undoably)
{
	if(poly_prev_found)
	{
		bool arc_done = false;
		if(poly_prev_bulge_found)
		{
			// from here: http://www.afralisp.net/lisp/Bulges1.htm
			if(fabs(poly_prev_bulge)> 0.0000000000001)
			{
				double dx = x - poly_prev_x;
				double dy = y - poly_prev_y;
				double c = sqrt(dx*dx + dy*dy);
				double chord = c/2;
				double s = chord * poly_prev_bulge;
				double r = (chord*chord+s*s)/(2*s);
				double sx = -dy;
				double sy = dx;
				double mags = sqrt(sx*sx+sy*sy);
				if(mags>0.0000000000000001)
				{
					sx = sx/mags;
					sy = sy/mags;
					if(poly_prev_bulge<0)
					{
						sx = -sx;
						sy = -sy;
					}

					double d = r - s;
					double mx = poly_prev_x + dx/2;
					double my = poly_prev_y + dy/2;
					double cx = mx + sx * d;
					double cy = my + sy * d;

					double ps[3] = {poly_prev_x, poly_prev_y, 0};
					double pe[3] = {x, y, 0};
					double pc[3] = {cx, cy, 0};
					dxf_read->OnReadArc(ps, pe, pc, poly_prev_bulge >= 0, undoably);
					arc_done = true;
				}
			}
		}

		if(!arc_done)
		{
			double s[3] = {poly_prev_x, poly_prev_y, 0};
			double e[3] = {x, y, 0};
			dxf_read->OnReadLine(s, e, undoably);
		}
	}

	poly_prev_found = true;
	poly_prev_x = x;
	poly_prev_y = y;
	if(!poly_first_found)
	{
		poly_first_x = x;
		poly_first_y = y;
		poly_first_found = true;
	}
	poly_prev_bulge_found = bulge_found;
	poly_prev_bulge = bulge;
}

static void PolyLineStart()
{
	poly_prev_found = false;
	poly_first_found = false;
}

bool CDxfRead::ReadLwPolyLine(bool undoably)
{
	PolyLineStart();

	bool x_found = false;
	bool y_found = false;
	double x;
	double y;
	bool bulge_found = false;
	double bulge = 0.0;
	bool closed = false;
	int flags;
	bool next_item_found = false;

	while(!((*m_ifs).eof()) && !next_item_found)
	{
		get_line();
		int n;
		if(sscanf(m_str, "%d", &n) != 1)return false;
		switch(n){
			case 0:
				// next item found
				if(x_found && y_found){
					// add point
					AddPolyLinePoint(this, x, y, bulge_found, bulge, undoably);
					bulge_found = false;
					x_found = false;
					y_found = false;
				}
				next_item_found = true;
				break;
			case 10:
				// x
				get_line();
				if(x_found && y_found){
					// add point
					AddPolyLinePoint(this, x, y, bulge_found, bulge, undoably);
					bulge_found = false;
					x_found = false;
					y_found = false;
				}
				if(sscanf(m_str, "%lf", &x) != 1)return false;
				x_found = true;
				break;
			case 20:
				// y
				get_line();
				if(sscanf(m_str, "%lf", &y) != 1)return false;
				y_found = true;
				break;
			case 42:
				// bulge
				get_line();
				if(sscanf(m_str, "%lf", &bulge) != 1)return false;
				bulge_found = true;
				break;
			case 70:
				// flags
				get_line();
				if(sscanf(m_str, "%d", &flags) != 1)return false;
				closed = ((flags & 1) != 0);
				break;
			default:
				// skip the next line
				get_line();
				break;
		}
	}

	if(next_item_found)
	{
		if(closed && poly_first_found)
		{
			// repeat the first point
			AddPolyLinePoint(this, poly_first_x, poly_first_y, false, 0.0, undoably);
		}
		return true;
	}

	return false;
}

void CDxfRead::OnReadArc(double start_angle, double end_angle, double radius, const double* c, bool undoably){
	double s[3], e[3];
	s[0] = c[0] + radius * cos(start_angle * Pi/180);
	s[1] = c[1] + radius * sin(start_angle * Pi/180);
	s[2] = c[2];
	e[0] = c[0] + radius * cos(end_angle * Pi/180);
	e[1] = c[1] + radius * sin(end_angle * Pi/180);
	e[2] = c[2];

	OnReadArc(s, e, c, true, undoably);
}

void CDxfRead::get_line()
{
	m_ifs->getline(m_str, 1024);

	char str[1024];
	int len = strlen(m_str);
	int j = 0;
	bool non_white_found = false;
	for(int i = 0; i<len; i++){
		if(non_white_found || (m_str[i] != ' ' && m_str[i] != '\t')){
			str[j] = m_str[i]; j++;
			non_white_found = true;
		}
	}
	str[j] = 0;
	strcpy(m_str, str);
}

void CDxfRead::DoRead(bool undoably)
{
	if(m_fail)return;

	get_line();

	ofstream f("dxflog.txt");

	while(!((*m_ifs).eof()))
	{
		
		if(!strcmp(m_str, "0"))
		{
			get_line();

			if(!strcmp(m_str, "LINE")){
				if(!ReadLine(undoably))return;
				continue;
			}
			else if(!strcmp(m_str, "ARC")){
				if(!ReadArc(undoably))return;
				continue;
			}
			else if(!strcmp(m_str, "LWPOLYLINE")){
				if(!ReadLwPolyLine(undoably))return;
				continue;
			}
		}

		get_line();
	}
}

// static
bool HeeksDxfRead::m_make_as_sketch = false;

void HeeksDxfRead::OnReadLine(const double* s, const double* e, bool undoably)
{
	HLine* new_object = new HLine(make_point(s), make_point(e), &(wxGetApp().current_color));
	AddSketchIfNeeded(undoably);
	if(undoably)wxGetApp().AddUndoably(new_object, m_sketch, NULL);
	else if(m_sketch)m_sketch->Add(new_object, NULL);
	else wxGetApp().Add(new_object, NULL);
}

void HeeksDxfRead::OnReadArc(const double* s, const double* e, const double* c, bool dir, bool undoably)
{
	gp_Pnt p0 = make_point(s);
	gp_Pnt p1 = make_point(e);
	gp_Dir up(0, 0, 1);
	if(!dir)up = -up;
	gp_Pnt pc = make_point(c);
	gp_Circ circle(gp_Ax2(pc, up), p1.Distance(pc));
	HArc* new_object = new HArc(p0, p1, circle, &wxGetApp().current_color);
	AddSketchIfNeeded(undoably);
	if(undoably)wxGetApp().AddUndoably(new_object, m_sketch, NULL);
	else if(m_sketch)m_sketch->Add(new_object, NULL);
	else wxGetApp().Add(new_object, NULL);
}

void HeeksDxfRead::AddSketchIfNeeded(bool undoably)
{
	if(m_make_as_sketch && m_sketch == NULL)
	{
		m_sketch = new CSketch();
		if(undoably)wxGetApp().AddUndoably(m_sketch, NULL, NULL);
		else wxGetApp().Add(m_sketch, NULL);
	}
}