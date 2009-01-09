// dxf.cpp

#include "stdafx.h"
#include "dxf.h"

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

bool CDxfRead::ReadLine()
{
	double s[3] = {0, 0, 0};
	double e[3] = {0, 0, 0};

	while(!((*m_ifs).eof()))
	{
		m_ifs->getline(m_str, 1024);
		int n;
		if(sscanf(m_str, "%d", &n) != 1)return false;
		switch(n){
			case 0:
				// next item found, so finish with line
				OnReadLine(s, e);
				return true;
			case 10:
				// start x
				m_ifs->getline(m_str, 1024);
				if(sscanf(m_str, "%lf", &(s[0])) != 1)return false;
				break;
			case 20:
				// start y
				m_ifs->getline(m_str, 1024);
				if(sscanf(m_str, "%lf", &(s[1])) != 1)return false;
				break;
			case 30:
				// start z
				m_ifs->getline(m_str, 1024);
				if(sscanf(m_str, "%lf", &(s[2])) != 1)return false;
				break;
			case 11:
				// end x
				m_ifs->getline(m_str, 1024);
				if(sscanf(m_str, "%lf", &(e[0])) != 1)return false;
				break;
			case 21:
				// end y
				m_ifs->getline(m_str, 1024);
				if(sscanf(m_str, "%lf", &(e[1])) != 1)return false;
				break;
			case 31:
				// end z
				m_ifs->getline(m_str, 1024);
				if(sscanf(m_str, "%lf", &(e[2])) != 1)return false;
				break;
			case 100:
			case 39:
			case 210:
			case 220:
			case 230:
				// skip the next line
				m_ifs->getline(m_str, 1024);
				break;
			default:
				// skip the next line
				m_ifs->getline(m_str, 1024);
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
		m_ifs->getline(m_str, 1024);
		int n;
		if(sscanf(m_str, "%d", &n) != 1)return false;
		switch(n){
			case 0:
				// next item found, so finish with arc
				OnReadArc(start_angle, end_angle, radius, c);
				return true;
			case 10:
				// centre x
				m_ifs->getline(m_str, 1024);
				if(sscanf(m_str, "%lf", &(c[0])) != 1)return false;
				break;
			case 20:
				// centre y
				m_ifs->getline(m_str, 1024);
				if(sscanf(m_str, "%lf", &(c[1])) != 1)return false;
				break;
			case 30:
				// centre z
				m_ifs->getline(m_str, 1024);
				if(sscanf(m_str, "%lf", &(c[2])) != 1)return false;
				break;
			case 40:
				// radius
				m_ifs->getline(m_str, 1024);
				if(sscanf(m_str, "%lf", &radius) != 1)return false;
				break;
			case 50:
				// start angle
				m_ifs->getline(m_str, 1024);
				if(sscanf(m_str, "%lf", &start_angle) != 1)return false;
				break;
			case 51:
				// end angle
				m_ifs->getline(m_str, 1024);
				if(sscanf(m_str, "%lf", &end_angle) != 1)return false;
				break;
			case 100:
			case 39:
			case 210:
			case 220:
			case 230:
				// skip the next line
				m_ifs->getline(m_str, 1024);
				break;
			default:
				// skip the next line
				m_ifs->getline(m_str, 1024);
				break;
		}
	}

	OnReadArc(start_angle, end_angle, radius, c);
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

void CDxfRead::DoRead()
{
	if(m_fail)return;

	m_ifs->getline(m_str, 1024);

	while(!((*m_ifs).eof()))
	{
		if(!strcmp(m_str, "0"))
		{
			m_ifs->getline(m_str, 1024);
			if(!strcmp(m_str, "LINE")){
				if(!ReadLine())return;
				continue;
			}
			else if(!strcmp(m_str, "ARC")){
				if(!ReadArc())return;
				continue;
			}
		}

		m_ifs->getline(m_str, 1024);
	}
}
