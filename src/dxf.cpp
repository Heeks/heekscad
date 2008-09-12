// dxf.cpp

#include "stdafx.h"
#include "dxf.h"

CDXF::CDXF(const char* filepath)
{
	// start the file
	m_fail = false;
	m_ofs = new ofstream(filepath, ios::out);
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

CDXF::~CDXF()
{
	// end
	(*m_ofs) << 0          << endl;
	(*m_ofs) << "ENDSEC"   << endl;
	(*m_ofs) << 0          << endl;
	(*m_ofs) << "EOF";

	delete m_ofs;
}

void CDXF::WriteLine(const double* s, const double* e)
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

void CDXF::WriteArc(const double* s, const double* e, const double* c, bool dir)
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

