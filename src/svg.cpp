// svg.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "svg.h"
#include "HLine.h"
#include "HArc.h"
#include "HCircle.h"
#include "HEllipse.h"
#include "HSpline.h"
#include "Sketch.h"
#include "../tinyxml/tinyxml.h"

#include <GeomConvert_CompCurveToBSplineCurve.hxx>

#include <sstream>

CSvgRead::CSvgRead()
{
}

void CSvgRead::Read(const wxChar* filepath, bool undoably)
{
	// start the file
	m_fail = false;
	TiXmlDocument doc(Ttc(filepath));
	if (!doc.LoadFile())
	{
		if(doc.Error())
		{
			m_fail = true;
		}
		return;
	}

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	// block: name
	{
		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) return;
		std::string name(pElem->Value());

		if(name != "svg")
		{
			m_fail = true;
			return;
		}

		// save this for later
		hRoot=TiXmlHandle(pElem);
	}

	// loop through all the objects
	for(pElem = hRoot.FirstChildElement().Element(); pElem;	pElem = pElem->NextSiblingElement())
	{
		ReadSVGElement(pElem, undoably);
	}

}

CSvgRead::~CSvgRead()
{
}

void CSvgRead::ReadSVGElement(TiXmlElement* pElem, bool undoably)
{
	std::string name(pElem->Value());

	if(name == "g")
	{
		// loop through all the child elements, looking for path
		for(pElem = TiXmlHandle(pElem).FirstChildElement().Element(); pElem; pElem = pElem->NextSiblingElement())
		{
			ReadSVGElement(pElem, undoably);
		}
		return;
	}

	if(name == "path")
	{
		ReadPath(pElem,undoably);
	}

	if(name == "rect")
	{
		ReadRect(pElem,undoably);
	}

	if(name == "circle")
	{
		ReadCircle(pElem,undoably);
	}

	if(name == "ellipse")
	{
		ReadEllipse(pElem,undoably);
	}
	
	if(name == "line")
	{
		ReadLine(pElem,undoably);
	}

	if(name == "polyline")
	{
		ReadPolyline(pElem,false,undoably);
	}

	if(name == "polygon")
	{
		ReadPolyline(pElem,true,undoably);
	}
}

void CSvgRead::ReadRect(TiXmlElement *pElem, bool undoably)
{
	double x,y,width,height;
	double rx=0; 
	double ry=0;
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "x")x=a->DoubleValue();
		if(name == "y")y=a->DoubleValue();
		if(name == "width")width=a->DoubleValue();
		if(name == "height")height=a->DoubleValue();
		if(name == "rx")rx=a->DoubleValue();
		if(name == "ry")ry=a->DoubleValue();
	}

	y=-y;height=-height;

	gp_Pnt p1(x,y,0);
	gp_Pnt p2(x+width,y,0);
	gp_Pnt p3(x+width,y+height,0);
	gp_Pnt p4(x,y+height,0);

	//TODO: add rounded rectangle support

	OnReadLine(p1,p2,undoably);
	OnReadLine(p2,p3,undoably);
	OnReadLine(p3,p4,undoably);
	OnReadLine(p4,p1,undoably);
}

void CSvgRead::ReadEllipse(TiXmlElement *pElem, bool undoably)
{
	double x=0;
	double y=0;
	double rx=0; 
	double ry=0;
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "cx")x=a->DoubleValue();
		if(name == "cy")y=a->DoubleValue();
		if(name == "rx")rx=a->DoubleValue();
		if(name == "ry")ry=a->DoubleValue();
	}

	y=-y;

	double rot = 0;
	if(ry>rx)
	{
		double temp = rx;
		rx = ry;
		ry = temp;
		rot = Pi/2;
	}
	OnReadEllipse(gp_Pnt(x,y,0),rx,ry,rot,0,2*Pi,undoably);
}

void CSvgRead::ReadLine(TiXmlElement *pElem, bool undoably)
{
	double x1=0;
	double y1=0;
	double x2=0; 
	double y2=0;
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "x1")x1=a->DoubleValue();
		if(name == "y1")y1=a->DoubleValue();
		if(name == "x2")x2=a->DoubleValue();
		if(name == "y2")y2=a->DoubleValue();
	}

	y1=-y1; y2=-y2;

	OnReadLine(gp_Pnt(x1,y1,0),gp_Pnt(x2,y2,0),undoably);
}

void CSvgRead::ReadPolyline(TiXmlElement *pElem, bool close, bool undoably)
{
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "points")
		{
			const char* d = a->Value();
			gp_Pnt ppnt(0,0,0);
			gp_Pnt spnt(0,0,0);
			bool has_point = false;

			int pos = 0;
			while(1){
				if(d[pos] == 0){
					if(has_point && close)
						OnReadLine(ppnt,spnt,undoably);
					break;
				}
				if(isdigit(d[pos])){
					double x,y;
					int count=0;
					sscanf(&d[pos],"%lf,%lf%n",&x,&y,&count);
					y=-y;
					gp_Pnt cpnt(x,y,0);
					if(has_point)
						OnReadLine(ppnt,cpnt,undoably);
					else
					{
						has_point = true;
						spnt = cpnt;
					}
					ppnt = cpnt;
					pos+=count;
				}
				else{
					pos++;
				}
			}
		}
	}
}


void CSvgRead::ReadCircle(TiXmlElement *pElem, bool undoably)
{
	double x,y,r;
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "cx")x=a->DoubleValue();
		if(name == "cy")y=a->DoubleValue();
		if(name == "r")r=a->DoubleValue();
	}
	y=-y;

	gp_Pnt cp(x,y,0);
	OnReadCircle(cp,r,undoably);
}

gp_Pnt CSvgRead::ReadStart(const char *text,gp_Pnt ppnt,bool isupper,bool undoably)
{
	double x, y;
	sscanf(text, "%lf,%lf", &x, &y);
	y = -y;
	gp_Pnt npt(x,y,0);
	if(!isupper)
	{
		npt.SetX(x+ppnt.X());
		npt.SetY(y+ppnt.Y());
	}
	OnReadStart(undoably);
	return npt;
}

gp_Pnt CSvgRead::ReadLine(const char *text,gp_Pnt ppnt,bool isupper,bool undoably)
{
	double x, y;
	sscanf(text, "%lf,%lf", &x, &y);
	y = -y;
	gp_Pnt npt(x,y,0);
	if(!isupper)
	{
		npt.SetX(x+ppnt.X());
		npt.SetY(y+ppnt.Y());
	}
	OnReadLine(ppnt,npt,undoably);
	return npt;
}

void CSvgRead::ReadClose(gp_Pnt ppnt,gp_Pnt spnt,bool undoably)
{
	OnReadLine(ppnt,spnt,undoably);
}

gp_Pnt CSvgRead::ReadHorizontal(const char *text,gp_Pnt ppnt,bool isupper,bool undoably)
{
	double x;
	sscanf(text, "%lf", &x);
	gp_Pnt npt(x,ppnt.Y(),0);
	if(!isupper)
		npt.SetX(x+ppnt.X());
	OnReadLine(ppnt,npt,undoably);
	return npt;
}

gp_Pnt CSvgRead::ReadVertical(const char *text,gp_Pnt ppnt,bool isupper,bool undoably)
{
	double y;
	sscanf(text, "%lf", &y);
	y = -y;
	gp_Pnt npt(ppnt.X(),y,0);
	if(!isupper)
		npt.SetY(y+ppnt.Y());
	OnReadLine(ppnt,npt,undoably);
	return npt;
}


struct TwoPoints CSvgRead::ReadCubic(const char *text,gp_Pnt ppnt,bool isupper,bool undoably)
{
	struct TwoPoints retpts;
	double x1, y1, x2, y2, x3, y3;
	sscanf(text, "%lf,%lf %lf,%lf %lf,%lf", &x1, &y1, &x2, &y2, &x3, &y3);
	y1 = -y1; y2 = -y2; y3 = -y3;	

	if(!isupper)
	{
		x1+=ppnt.X(); y1+=ppnt.Y();
		x2+=x1;	y2+=y1;
		x3+=x2;	y3+=y2;
	}

	gp_Pnt pnt1(x1,y1,0);
	retpts.pcpnt = gp_Pnt(x2,y2,0);
	retpts.ppnt = gp_Pnt(x3,y3,0);

	OnReadCubic(ppnt,pnt1,retpts.pcpnt,retpts.ppnt,undoably);
	return retpts;
}

struct TwoPoints CSvgRead::ReadCubic(const char *text,gp_Pnt ppnt, gp_Pnt pcpnt, bool isupper,bool undoably)
{
	struct TwoPoints retpts;
	double x2, y2, x3, y3;
	sscanf(text, "%lf,%lf %lf,%lf", &x2, &y2, &x3, &y3);
	y2 = -y2; y3 = -y3;	

	if(!isupper)
	{
		x2+=ppnt.X(); y2+=ppnt.Y();
		x3+=x2;	y3+=y2;
	}

	gp_Dir dir=ppnt.XYZ()-pcpnt.XYZ();
	double d = ppnt.Distance(pcpnt);
	gp_Pnt pnt1(ppnt.XYZ() + dir.XYZ() * d);
	retpts.pcpnt = gp_Pnt(x2,y2,0);
	retpts.ppnt = gp_Pnt(x3,y3,0);

	OnReadCubic(ppnt,pnt1,retpts.pcpnt,retpts.ppnt,undoably);

	return retpts;
}

struct TwoPoints CSvgRead::ReadQuadratic(const char *text,gp_Pnt ppnt,bool isupper,bool undoably)
{
	struct TwoPoints retpts;
	double x1, y1, x2, y2;
	sscanf(text, "%lf,%lf %lf,%lf", &x1, &y1, &x2, &y2);
	y1 = -y1; y2 = -y2; 

	if(!isupper)
	{
		x1+=ppnt.X(); y1+=ppnt.Y();
		x2+=x1;	y2+=y1;
	}

	retpts.pcpnt = gp_Pnt(x1,y1,0);
	retpts.ppnt = gp_Pnt(x2,y2,0);

	OnReadQuadratic(ppnt,retpts.pcpnt,retpts.ppnt,undoably);
	return retpts;
}

struct TwoPoints CSvgRead::ReadQuadratic(const char *text,gp_Pnt ppnt, gp_Pnt pcpnt, bool isupper,bool undoably)
{
	struct TwoPoints retpts;
	double x2, y2;
	sscanf(text, "%lf,%lf", &x2, &y2);
	y2 = -y2;

	if(!isupper)
	{
		x2+=ppnt.X(); y2+=ppnt.Y();
	}

	gp_Dir dir=ppnt.XYZ()-pcpnt.XYZ();
	double d = ppnt.Distance(pcpnt);
	gp_Pnt pnt1(ppnt.XYZ() + dir.XYZ() * d);
	retpts.pcpnt = pnt1;
	retpts.ppnt = gp_Pnt(x2,y2,0);

	OnReadQuadratic(ppnt,retpts.pcpnt,retpts.ppnt,undoably);
	return retpts;
}

gp_Pnt CSvgRead::ReadEllipse(const char *text,gp_Pnt ppnt,bool isupper,bool undoably)
{
	int large_arc_flag, sweep_flag;
	double rx, ry, xrot, x, y;
	sscanf(text, "%lf,%lf %lf %d,%d %lf,%lf", &rx, &ry, &xrot, &large_arc_flag, &sweep_flag, &x, &y);
	y=-y;
	if(!isupper)
	{
		x+=ppnt.X(); y+=ppnt.Y();
	}

	xrot = -Pi*xrot/180.0;

	gp_Pnt ept(x,y,0);
	gp_Dir up(0,0,1);
	gp_Pnt zp(0,0,0);

	//from http://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes

	gp_Vec mid((ppnt.XYZ()-ept.XYZ())/2);
	mid.Rotate(gp_Ax1(zp,up),-xrot);
	
	double error = mid.X() * mid.X()/(rx*rx) + mid.Y()*mid.Y()/(ry*ry);
		if(error > 1-wxGetApp().m_geom_tol)
	{
		rx *= sqrt(error) + wxGetApp().m_geom_tol;
		ry *= sqrt(error) + wxGetApp().m_geom_tol;
	}

	double root = sqrt((rx*rx*ry*ry-rx*rx*mid.Y()*mid.Y()-ry*ry*mid.X()*mid.X())/(rx*rx*mid.Y()*mid.Y()+ry*ry*mid.X()*mid.X()));
	double cx = root * rx * mid.Y() / ry;
	double cy = root * -ry * mid.X() / rx;
	if(large_arc_flag != sweep_flag)
	{
		cx = -cx; cy = -cy;
	}
	gp_Vec cvec(cx,cy,0);
	cvec.Rotate(gp_Ax1(zp,up),xrot);
	gp_Pnt cpnt(cvec.XYZ() + (ept.XYZ()+ppnt.XYZ())/2);

	if(rx<ry)
	{
		double temp = ry;
		ry = rx;
		rx = temp;
		xrot += Pi/2;
	}

	//TODO: calculate the start and end angles

	OnReadEllipse(cpnt,rx,ry,xrot,0,2*Pi,undoably);
	return ept;
}

void CSvgRead::ReadPath(TiXmlElement* pElem, bool undoably)
{
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "d")
		{
			// add lines and arcs and bezier curves
			const char* d = a->Value();
			gp_Pnt spnt(0,0,0);
			gp_Pnt ppnt(0,0,0);
			gp_Pnt pcpnt(0,0,0);

			int pos = 0;
			while(1){
				if(toupper(d[pos]) == 'M'){
					// make a sketch
					spnt = ReadStart(&d[pos+1],ppnt,isupper(d[pos]),undoably);
					ppnt = spnt;
					pos++;
				}
				else if(toupper(d[pos]) == 'L'){
					// add a line
					ppnt = ReadLine(&d[pos+1],ppnt,isupper(d[pos]),undoably);
					pos++;
				}
				else if(toupper(d[pos]) == 'H'){
					//horizontal line
					ppnt = ReadHorizontal(&d[pos+1],ppnt,isupper(d[pos]),undoably);
					pos++;
				}
				else if(toupper(d[pos]) == 'V'){
					//vertical line
					ppnt = ReadVertical(&d[pos+1],ppnt,isupper(d[pos]),undoably);
					pos++;
				}
				else if(toupper(d[pos]) == 'C'){
					// add a cubic bezier curve ( just split into lines for now )
					struct TwoPoints ret = ReadCubic(&d[pos+1],ppnt,isupper(d[pos]),undoably);
					ppnt = ret.ppnt;
					pcpnt = ret.pcpnt;
					pos++;
				}
				else if(toupper(d[pos]) == 'S'){
                    // add a cubic bezier curve ( short hand)
					struct TwoPoints ret = ReadCubic(&d[pos+1],ppnt,pcpnt,isupper(d[pos]),undoably);
					ppnt = ret.ppnt;
					pcpnt = ret.pcpnt;
					pos++;
				}
				else if(toupper(d[pos]) == 'Q'){
					// add a quadratic bezier curve 
					struct TwoPoints ret = ReadQuadratic(&d[pos+1],ppnt,isupper(d[pos]),undoably);
					ppnt = ret.ppnt;
					pcpnt = ret.pcpnt;
					pos++;
				}
				else if(toupper(d[pos]) == 'T'){
               		// add a quadratic bezier curve 
					struct TwoPoints ret = ReadQuadratic(&d[pos+1],ppnt,pcpnt,isupper(d[pos]),undoably);
					ppnt = ret.ppnt;
					pcpnt = ret.pcpnt;
					pos++;
				}
				else if(toupper(d[pos]) == 'A'){
					// add an elliptic arc
					ppnt = ReadEllipse(&d[pos+1],ppnt,isupper(d[pos]),undoably);
					pos++;
				}
				else if(toupper(d[pos]) == 'Z'){
					// join to end
					ReadClose(ppnt,spnt,undoably);
					pos++;
					ppnt = spnt;
				}
				else if(d[pos] == 0){
					break;
				}
				else{
					pos++;
				}
			}
		}
	}
}

HeeksSvgRead::HeeksSvgRead(const wxChar* filepath, bool undoably, bool usehspline)
{
	m_usehspline=usehspline;
	m_undoably=undoably;
	m_sketch = 0;
	Read(filepath,undoably);
}

void HeeksSvgRead::OnReadStart(bool undoably)
{
		m_sketch = new CSketch();
		if(undoably)wxGetApp().AddUndoably(m_sketch, NULL, NULL);
		else wxGetApp().Add(m_sketch, NULL);
}

void HeeksSvgRead::OnReadCubic(gp_Pnt s, gp_Pnt c1, gp_Pnt c2, gp_Pnt e,bool undoably)
{
	TColgp_Array1OfPnt poles(1,4);
	poles.SetValue(1,s); poles.SetValue(2,c1); poles.SetValue(3,c2); poles.SetValue(4,e);
	Geom_BezierCurve curve(poles);
	GeomConvert_CompCurveToBSplineCurve convert(&curve);

	Handle_Geom_BSplineCurve spline = convert.BSplineCurve();
	Geom_BSplineCurve* pspline = (Geom_BSplineCurve*)spline.Access();
	HSpline* new_object = new HSpline(*pspline, &wxGetApp().current_color);
	AddSketchIfNeeded(undoably);
	m_sketch->Add(new_object, NULL);
}

void HeeksSvgRead::OnReadQuadratic(gp_Pnt s, gp_Pnt c, gp_Pnt e,bool undoably)
{
	TColgp_Array1OfPnt poles(1,3);
	poles.SetValue(1,s); poles.SetValue(2,c); poles.SetValue(3,e);
	Geom_BezierCurve curve(poles);
	GeomConvert_CompCurveToBSplineCurve convert(&curve);

	Handle_Geom_BSplineCurve spline = convert.BSplineCurve();
	Geom_BSplineCurve* pspline = (Geom_BSplineCurve*)spline.Access();
	HSpline* new_object = new HSpline(*pspline, &wxGetApp().current_color);
	AddSketchIfNeeded(undoably);
	m_sketch->Add(new_object, NULL);
}

void HeeksSvgRead::OnReadLine(gp_Pnt p1, gp_Pnt p2,bool undoably)
{
	HLine *line = new HLine(p1,p2,&wxGetApp().current_color);
	AddSketchIfNeeded(undoably);
	m_sketch->Add(line, NULL);
}

void HeeksSvgRead::OnReadEllipse(gp_Pnt c, double maj_r, double min_r, double rot, double start_a, double end_a,bool undoably)
{
	gp_Dir up(0,0,1);
	gp_Elips elip(gp_Ax2(c,gp_Dir(0,0,1)),maj_r,min_r);
	elip.Rotate(gp_Ax1(c,up),rot);
	HEllipse *new_object = new HEllipse(elip,&wxGetApp().current_color);
	AddSketchIfNeeded(undoably);
	m_sketch->Add(new_object, NULL);
}

void HeeksSvgRead::OnReadCircle(gp_Pnt c, double r, bool undoably)
{
	gp_Dir up(0,0,1);
	gp_Circ cir(gp_Ax2(c,up),r);
	HCircle *new_object = new HCircle(cir,&wxGetApp().current_color);
	AddSketchIfNeeded(undoably);
	m_sketch->Add(new_object, NULL);
}

void HeeksSvgRead::AddSketchIfNeeded(bool undoably)
{
	if(m_sketch == NULL)
	{
		m_sketch = new CSketch();
		if(undoably)wxGetApp().AddUndoably(m_sketch, NULL, NULL);
		else wxGetApp().Add(m_sketch, NULL);
	}
}
