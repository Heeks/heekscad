// dxf.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "dxf.h"
#include "HLine.h"
#include "HArc.h"
#include "HCircle.h"
#include "HEllipse.h"
#include "HSpline.h"
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
	m_ofs->imbue(std::locale("C"));

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

void CDxfWrite::WriteCircle(const double* c, double radius)
{
	(*m_ofs) << 0			<< endl;
	(*m_ofs) << "CIRCLE"		<< endl;
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
}

void CDxfWrite::WriteEllipse(const double* c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir)
{
	double m[3];
	m[2]=0;
	m[0] = major_radius * sin(rotation);
	m[1] = major_radius * cos(rotation);

	double ratio = minor_radius/major_radius;

	if(!dir){
		double temp = start_angle;
		start_angle = end_angle;
		end_angle = temp;
	}
	(*m_ofs) << 0			<< endl;
	(*m_ofs) << "ELLIPSE"		<< endl;
	(*m_ofs) << 8			<< endl;	// Group code for layer name
	(*m_ofs) << 0			<< endl;	// Layer number
	(*m_ofs) << 10			<< endl;	// Centre X
	(*m_ofs) << c[0]		<< endl;	// X in WCS coordinates
	(*m_ofs) << 20			<< endl;
	(*m_ofs) << c[1]		<< endl;	// Y in WCS coordinates
	(*m_ofs) << 30			<< endl;
	(*m_ofs) << c[2]		<< endl;	// Z in WCS coordinates
	(*m_ofs) << 40			<< endl;	// 
	(*m_ofs) << ratio		<< endl;	// Ratio
	(*m_ofs) << 11			<< endl;	// 
	(*m_ofs) << m[0]		<< endl;	// Major X 
	(*m_ofs) << 21			<< endl;
	(*m_ofs) << m[1]		<< endl;	// Major Y 
	(*m_ofs) << 31			<< endl;
	(*m_ofs) << m[2]		<< endl;	// Major Z 
	(*m_ofs) << 41		<< endl;
	(*m_ofs) << start_angle	<< endl;	// Start angle
	(*m_ofs) << 42		<< endl;
	(*m_ofs) << end_angle	<< endl;	// End angle
}

CDxfRead::CDxfRead(const wxChar* filepath)
{
	// start the file
	m_fail = false;
	m_eUnits = eMillimeters;

	m_ifs = new ifstream(Ttc(filepath));
	if(!(*m_ifs)){
		m_fail = true;
		return;
	}
	m_ifs->imbue(std::locale("C"));
}

CDxfRead::~CDxfRead()
{
	delete m_ifs;
}

double CDxfRead::mm( const double & value ) const
{
	switch(m_eUnits)
	{
		case eUnspecified:	return(value * 1.0);	// We don't know any better.
		case eInches:		return(value * 25.4);
		case eFeet:		return(value * 25.4 * 12);
		case eMiles:		return(value *  1609344.0);
		case eMillimeters:	return(value * 1.0);
		case eCentimeters:	return(value * 10.0);
		case eMeters:		return(value * 1000.0);
		case eKilometers:	return(value * 1000000.0);
		case eMicroinches:	return(value * 25.4 / 1000.0);
		case eMils:		return(value * 25.4 / 1000.0);
		case eYards:		return(value * 3 * 12 * 25.4);
		case eAngstroms:	return(value * 0.0000001);
		case eNanometers:	return(value * 0.000001);
		case eMicrons:		return(value * 0.001);
		case eDecimeters:	return(value * 100.0);
		case eDekameters:	return(value * 10000.0);
		case eHectometers:	return(value * 100000.0);
		case eGigameters:	return(value * 1000000000000.0);
		case eAstronomicalUnits: return(value * 149597870690000.0);
		case eLightYears:	return(value * 9454254955500000000.0);
		case eParsecs:		return(value * 30856774879000000000.0);
		default:		return(value * 1.0);	// We don't know any better.
	} // End switch
} // End mm() method


bool CDxfRead::ReadLine()
{
	double s[3] = {0, 0, 0};
	double e[3] = {0, 0, 0};

	while(!((*m_ifs).eof()))
	{
		get_line();
		int n;


		if(sscanf(m_str, "%d", &n) != 1)return false;
		std::istringstream ss;
		ss.imbue(std::locale("C"));
		switch(n){
			case 0:
				// next item found, so finish with line
				OnReadLine(s, e);
				return true;
			case 10:
				// start x
				get_line();
				ss.str(m_str); ss >> s[0]; s[0] = mm(s[0]); if(ss.fail()) return false;
				break;
			case 20:
				// start y
				get_line();
				ss.str(m_str); ss >> s[1]; s[1] = mm(s[1]); if(ss.fail()) return false;
				break;
			case 30:
				// start z
				get_line();
				ss.str(m_str); ss >> s[2]; s[2] = mm(s[2]); if(ss.fail()) return false;
				break;
			case 11:
				// end x
				get_line();
				ss.str(m_str); ss >> e[0]; e[0] = mm(e[0]); if(ss.fail()) return false;
				break;
			case 21:
				// end y
				get_line();
				ss.str(m_str); ss >> e[1]; e[1] = mm(e[1]); if(ss.fail()) return false;
				break;
			case 31:
				// end z
				get_line();
				ss.str(m_str); ss >> e[2]; e[2] = mm(e[2]); if(ss.fail()) return false;
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

	OnReadLine(s, e);
	return false;
}

bool CDxfRead::ReadArc()
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
		std::istringstream ss;
		ss.imbue(std::locale("C"));
		switch(n){
			case 0:
				// next item found, so finish with arc
				OnReadArc(start_angle, end_angle, radius, c);
				return true;
			case 10:
				// centre x
				get_line();
				ss.str(m_str); ss >> c[0]; c[0] = mm(c[0]); if(ss.fail()) return false;
				break;
			case 20:
				// centre y
				get_line();
				ss.str(m_str); ss >> c[1]; c[1] = mm(c[1]); if(ss.fail()) return false;
				break;
			case 30:
				// centre z
				get_line();
				ss.str(m_str); ss >> c[2]; c[2] = mm(c[2]); if(ss.fail()) return false;
				break;
			case 40:
				// radius
				get_line();
				ss.str(m_str); ss >> radius; radius = mm(radius); if(ss.fail()) return false;
				break;
			case 50:
				// start angle
				get_line();
				ss.str(m_str); ss >> start_angle; if(ss.fail()) return false;
				break;
			case 51:
				// end angle
				get_line();
				ss.str(m_str); ss >> end_angle; if(ss.fail()) return false;
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
	OnReadArc(start_angle, end_angle, radius, c);
	return false;
}

bool CDxfRead::ReadSpline()
{
	struct SplineData sd;
	sd.norm[0] = 0;
	sd.norm[1] = 0;
	sd.norm[2] = 1;
	sd.degree = 0;
	sd.knots = 0;
	sd.flag = 0;
	sd.control_points = 0;
	sd.fit_points = 0;

	double temp_double;
	while(!((*m_ifs).eof()))
	{
		get_line();
		int n;
		if(sscanf(m_str, "%d", &n) != 1)return false;
		std::istringstream ss;
		ss.imbue(std::locale("C"));
		switch(n){
			case 0:
				// next item found, so finish with Spline
				OnReadSpline(sd);
				return true;
			case 210:
				// normal x
				get_line();
				ss.str(m_str); ss >> sd.norm[0]; sd.norm[0] = mm(sd.norm[0]); if(ss.fail()) return false;
				break;
			case 220:
				// normal y
				get_line();
				ss.str(m_str); ss >> sd.norm[1]; sd.norm[1] = mm(sd.norm[1]); if(ss.fail()) return false;
				break;
			case 230:
				// normal z
				get_line();
				ss.str(m_str); ss >> sd.norm[2]; sd.norm[2] = mm(sd.norm[2]); if(ss.fail()) return false;
				break;
			case 70:
				// flag
				get_line();
				ss.str(m_str); ss >> sd.flag; if(ss.fail()) return false;
				break;
			case 71:
				// degree
				get_line();
				ss.str(m_str); ss >> sd.degree; if(ss.fail()) return false;
				break;
			case 72:
				// knots
				get_line();
				ss.str(m_str); ss >> sd.knots; if(ss.fail()) return false;
				break;
			case 73:
				// control points
				get_line();
				ss.str(m_str); ss >> sd.control_points; if(ss.fail()) return false;
				break;
			case 74:
				// fit points
				get_line();
				ss.str(m_str); ss >> sd.fit_points; if(ss.fail()) return false;
				break;
			case 12:
				// starttan x
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.starttanx.push_back(temp_double);
				break;
			case 22:
				// starttan y
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.starttany.push_back(temp_double);
				break;
			case 32:
				// starttan z
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.starttanz.push_back(temp_double);
				break;
			case 13:
				// endtan x
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.endtanx.push_back(temp_double);
				break;
			case 23:
				// endtan y
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.endtany.push_back(temp_double);
				break;
			case 33:
				// endtan z
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.endtanz.push_back(temp_double);
				break;
			case 40:
				// knot
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.knot.push_back(temp_double);
				break;
			case 41:
				// weight
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.weight.push_back(temp_double);
				break;
			case 10:
				// control x
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.controlx.push_back(temp_double);
				break;
			case 20:
				// control y
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.controly.push_back(temp_double);
				break;
			case 30:
				// control z
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.controlz.push_back(temp_double);
				break;
			case 11:
				// fit x
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.fitx.push_back(temp_double);
				break;
			case 21:
				// fit y
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.fity.push_back(temp_double);
				break;
			case 31:
				// fit z
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.fitz.push_back(temp_double);
				break;
			case 42:
			case 43:
			case 44:
				// skip the next line
				get_line();
				break;
			default:
				// skip the next line
				get_line();
				break;
		}
	}
	OnReadSpline(sd);
	return false;
}


bool CDxfRead::ReadCircle()
{
	double radius = 0.0;
	double c[3]; // centre

	while(!((*m_ifs).eof()))
	{
		get_line();
		int n;
		if(sscanf(m_str, "%d", &n) != 1)return false;
		std::istringstream ss;
		ss.imbue(std::locale("C"));
		switch(n){
			case 0:
				// next item found, so finish with Circle
				OnReadCircle(c, radius);
				return true;
			case 10:
				// centre x
				get_line();
				ss.str(m_str); ss >> c[0]; c[0] = mm(c[0]); if(ss.fail()) return false;
				break;
			case 20:
				// centre y
				get_line();
				ss.str(m_str); ss >> c[1]; c[1] = mm(c[1]); if(ss.fail()) return false;
				break;
			case 30:
				// centre z
				get_line();
				ss.str(m_str); ss >> c[2]; c[2] = mm(c[2]); if(ss.fail()) return false;
				break;
			case 40:
				// radius
				get_line();
				ss.str(m_str); ss >> radius; radius = mm(radius); if(ss.fail()) return false;
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
	OnReadCircle(c, radius);
	return false;
}

bool CDxfRead::ReadEllipse()
{
	double c[3]; // centre
	double m[3]; //major axis point
	double ratio=0; //ratio of major to minor axis
	double start=0; //start of arc
	double end=0;  // end of arc

	while(!((*m_ifs).eof()))
	{
		get_line();
		int n;
		if(sscanf(m_str, "%d", &n) != 1)return false;
		std::istringstream ss;
		ss.imbue(std::locale("C"));
		switch(n){
			case 0:
				// next item found, so finish with Ellipse
				OnReadEllipse(c, m, ratio, start, end);
				return true;
			case 10:
				// centre x
				get_line();
				ss.str(m_str); ss >> c[0]; c[0] = mm(c[0]); if(ss.fail()) return false;
				break;
			case 20:
				// centre y
				get_line();
				ss.str(m_str); ss >> c[1]; c[1] = mm(c[1]); if(ss.fail()) return false;
				break;
			case 30:
				// centre z
				get_line();
				ss.str(m_str); ss >> c[2]; c[2] = mm(c[2]); if(ss.fail()) return false;
				break;
			case 11:
				// major x
				get_line();
				ss.str(m_str); ss >> m[0]; m[0] = mm(m[0]); if(ss.fail()) return false;
				break;
			case 21:
				// major y
				get_line();
				ss.str(m_str); ss >> m[1]; m[1] = mm(m[1]); if(ss.fail()) return false;
				break;
			case 31:
				// major z
				get_line();
				ss.str(m_str); ss >> m[2]; m[2] = mm(m[2]); if(ss.fail()) return false;
				break;
			case 40:
				// ratio
				get_line();
				ss.str(m_str); ss >> ratio; if(ss.fail()) return false;
				break;
			case 41:
				// start
				get_line();
				ss.str(m_str); ss >> start; if(ss.fail()) return false;
				break;
			case 42:
				// end
				get_line();
				ss.str(m_str); ss >> end; if(ss.fail()) return false;
				break;	
			case 100:
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
	OnReadEllipse(c, m, ratio, start, end);
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

static void AddPolyLinePoint(CDxfRead* dxf_read, double x, double y, bool bulge_found, double bulge)
{
	if(poly_prev_found)
	{
		bool arc_done = false;
		if(poly_prev_bulge_found)
		{
				double dx = x - poly_prev_x;
				double dy = y - poly_prev_y;
				double c = sqrt(dx*dx + dy*dy);

				double a = atan(fabs(poly_prev_bulge))*4;

				//find radius of circle that for arc of angle a, has chord length c
				double r = (c/2) / cos((Pi-a)/2);

				double d = sqrt(r*r - (c/2)*(c/2));
				
				double ps[3] = {poly_prev_x, poly_prev_y, 0};
				double pe[3] = {x, y, 0};
				gp_Pnt pPnt = make_point(ps);
				gp_Pnt nPnt = make_point(pe);
				gp_Dir dir(nPnt.XYZ()-pPnt.XYZ());
				
				gp_Pnt mid = pPnt.XYZ() + dir.XYZ() * c / 2;
				
				dir.Rotate(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(0,0,1)),Pi/2);
				gp_Pnt off;
				if(poly_prev_bulge >= 0)
					off = mid.XYZ() + dir.XYZ() * (d); 
				else
					off = mid.XYZ() + dir.XYZ() * (-d); 
			
				double pc[3];
				extract(off,pc);
				
				dxf_read->OnReadArc(ps, pe, pc, poly_prev_bulge >= 0);
				arc_done = true;
			
		}

		if(!arc_done)
		{
			double s[3] = {poly_prev_x, poly_prev_y, 0};
			double e[3] = {x, y, 0};
			dxf_read->OnReadLine(s, e);
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

bool CDxfRead::ReadLwPolyLine()
{
	PolyLineStart();

	bool x_found = false;
	bool y_found = false;
	double x = 0.0;
	double y = 0.0;
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
		std::istringstream ss;
		ss.imbue(std::locale("C"));
		switch(n){
			case 0:
				// next item found
				if(x_found && y_found){
					// add point
					AddPolyLinePoint(this, x, y, bulge_found, bulge);
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
					AddPolyLinePoint(this, x, y, bulge_found, bulge);
					bulge_found = false;
					x_found = false;
					y_found = false;
				}
				ss.str(m_str); ss >> x; x = mm(x); if(ss.fail()) return false;
				x_found = true;
				break;
			case 20:
				// y
				get_line();
				ss.str(m_str); ss >> y; y = mm(y); if(ss.fail()) return false;
				y_found = true;
				break;
			case 42:
				// bulge
				get_line();
				ss.str(m_str); ss >> bulge; if(ss.fail()) return false;
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
			AddPolyLinePoint(this, poly_first_x, poly_first_y, false, 0.0);
		}
		return true;
	}

	return false;
}

void CDxfRead::OnReadArc(double start_angle, double end_angle, double radius, const double* c){
	double s[3], e[3];
	s[0] = c[0] + radius * cos(start_angle * Pi/180);
	s[1] = c[1] + radius * sin(start_angle * Pi/180);
	s[2] = c[2];
	e[0] = c[0] + radius * cos(end_angle * Pi/180);
	e[1] = c[1] + radius * sin(end_angle * Pi/180);
	e[2] = c[2];

	OnReadArc(s, e, c, true);
}

void CDxfRead::OnReadCircle(const double* c, double radius){
	double s[3];
    double start_angle = 0;
	s[0] = c[0] + radius * cos(start_angle * Pi/180);
	s[1] = c[1] + radius * sin(start_angle * Pi/180);
	s[2] = c[2];

	OnReadCircle(s, c, false); //false to change direction because otherwise the arc length is zero
}

void CDxfRead::OnReadEllipse(const double* c, const double* m, double ratio, double start_angle, double end_angle){
	double major_radius = sqrt(m[0]*m[0] + m[1]*m[1] + m[2]*m[2]);
	double minor_radius = major_radius * ratio;

	//Since we only support 2d stuff, we can calculate the rotation from the major axis x and y value only,
	//since z is zero, major_radius is the vector length
	
	double rotation = atan2(m[1]/major_radius,m[0]/major_radius);


	OnReadEllipse(c, major_radius, minor_radius, rotation, start_angle, end_angle, true); 
}

void CDxfRead::OnReadSpline(struct SplineData& sd)
{
	TColgp_Array1OfPnt control (1,sd.controlx.size());
	TColStd_Array1OfReal weight (1,sd.controlx.size());

	std::list<double> knoto;
	std::list<int> multo;

	std::list<double>::iterator ity = sd.controly.begin(); 
	std::list<double>::iterator itz = sd.controlz.begin(); 
	std::list<double>::iterator itw = sd.weight.begin();

	int i=1;
	for(std::list<double>::iterator itx = sd.controlx.begin(); itx!=sd.controlx.end(); ++itx)
	{
		gp_Pnt pnt(*itx,*ity,*itz);
		control.SetValue(i,pnt);
		if(sd.weight.empty())
			weight.SetValue(i,1);
		else
		{
			weight.SetValue(i,*itw);
			++itw;
		}
		++i;
		++ity;
		++itz;
	}

	i=1;
	double last_knot = -1;
	for(std::list<double>::iterator it = sd.knot.begin(); it!=sd.knot.end(); ++it)
	{	
		if(*it != last_knot)
		{
			knoto.push_back(*it);
			multo.push_back(1);
			i++;
		}
		else
		{
			int temp = multo.back();
			multo.pop_back();
			multo.push_back(temp+1);
		}
		last_knot = *it;
	}
	TColStd_Array1OfReal knot (1,knoto.size());
	TColStd_Array1OfInteger mult (1,knoto.size());

	std::list<int>::iterator itm = multo.begin();
	i = 1;
	for(std::list<double>::iterator it = knoto.begin(); it!=knoto.end(); ++it)
	{
		knot.SetValue(i,*it);
		mult.SetValue(i,*itm);
		++itm;
		++i;
	}

    OnReadSpline(control, weight, knot, mult, sd.degree, (sd.flag & 2) != 0, (sd.flag & 4) != 0);
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
#if wxUSE_UNICODE
			if(m_str[i] != '\r')
#endif
			{
				str[j] = m_str[i]; j++;
			}
			non_white_found = true;
		}
	}
	str[j] = 0;
	strcpy(m_str, str);
}

bool CDxfRead::ReadUnits()
{
	get_line();	// Skip to next line.
	get_line();	// Skip to next line.
	int n = 0;
	if(sscanf(m_str, "%d", &n) == 1)
	{
		m_eUnits = eDxfUnits_t( n );
		return(true);
	} // End if - then
	else
	{
		return(false);
	}
}

void CDxfRead::DoRead()
{
	if(m_fail)return;

	get_line();

	while(!((*m_ifs).eof()))
	{
		if (!strcmp( m_str, "$INSUNITS" )){
			if (!ReadUnits())return;
			continue;
		} // End if - then
		else if(!strcmp(m_str, "0"))
		{
			get_line();

			if(!strcmp(m_str, "LINE")){
				if(!ReadLine())return;
				continue;
			}
			else if(!strcmp(m_str, "ARC")){
				if(!ReadArc())return;
				continue;
			}
			else if(!strcmp(m_str, "CIRCLE")){
				if(!ReadCircle())return;
				continue;
			}
			else if(!strcmp(m_str, "ELLIPSE")){
				if(!ReadEllipse())return;
				continue;
			}
			else if(!strcmp(m_str, "SPLINE")){
				if(!ReadSpline())return;
				continue;
			}
			else if(!strcmp(m_str, "LWPOLYLINE")){
				if(!ReadLwPolyLine())return;
				continue;
			}
		}

		get_line();
	}
}

// static
bool HeeksDxfRead::m_make_as_sketch = false;

void HeeksDxfRead::OnReadLine(const double* s, const double* e)
{
	HLine* new_object = new HLine(make_point(s), make_point(e), &(wxGetApp().current_color));
	AddSketchIfNeeded();
	if(m_sketch)m_sketch->Add(new_object, NULL);
	else wxGetApp().Add(new_object, NULL);
}

void HeeksDxfRead::OnReadArc(const double* s, const double* e, const double* c, bool dir)
{
	gp_Pnt p0 = make_point(s);
	gp_Pnt p1 = make_point(e);
	gp_Dir up(0, 0, 1);
	if(!dir)up = -up;
	gp_Pnt pc = make_point(c);
	gp_Circ circle(gp_Ax2(pc, up), p1.Distance(pc));
	HArc* new_object = new HArc(p0, p1, circle, &wxGetApp().current_color);
	AddSketchIfNeeded();
	if(m_sketch)m_sketch->Add(new_object, NULL);
	else wxGetApp().Add(new_object, NULL);
}

void HeeksDxfRead::OnReadCircle(const double* s, const double* c, bool dir)
{
	gp_Pnt p0 = make_point(s);
	//gp_Pnt p1 = make_point(e);
	gp_Dir up(0, 0, 1);
	if(!dir)up = -up;
	gp_Pnt pc = make_point(c);
	gp_Circ circle(gp_Ax2(pc, up), p0.Distance(pc));
	HCircle* new_object = new HCircle(circle, &wxGetApp().current_color);
	if(m_sketch)m_sketch->Add(new_object, NULL);
	else wxGetApp().Add(new_object, NULL);
}

void HeeksDxfRead::OnReadSpline(TColgp_Array1OfPnt &control, TColStd_Array1OfReal &weight, TColStd_Array1OfReal &knot,TColStd_Array1OfInteger &mult, int degree, bool periodic, bool rational)
{
	Geom_BSplineCurve spline(control,weight,knot,mult,degree,periodic,rational);
	HSpline* new_object = new HSpline(spline, &wxGetApp().current_color);
	AddSketchIfNeeded();
	if(m_sketch)m_sketch->Add(new_object, NULL);
	else wxGetApp().Add(new_object, NULL);
}

void HeeksDxfRead::OnReadEllipse(const double* c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir)
{
	gp_Dir up(0, 0, 1);
	if(!dir)up = -up;
	gp_Pnt pc = make_point(c);
	gp_Elips ellipse(gp_Ax2(pc, up), major_radius, minor_radius);
	ellipse.Rotate(gp_Ax1(pc,up),rotation);
	HEllipse* new_object = new HEllipse(ellipse, &wxGetApp().current_color);
	AddSketchIfNeeded();
	if(m_sketch)m_sketch->Add(new_object, NULL);
	else wxGetApp().Add(new_object, NULL);
}

void HeeksDxfRead::AddSketchIfNeeded()
{
	if(m_make_as_sketch && m_sketch == NULL)
	{
		m_sketch = new CSketch();
		wxGetApp().Add(m_sketch, NULL);
	}
}
