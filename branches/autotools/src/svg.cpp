// svg.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

//**********************************************************************************
// Status and my 2 cents:
//		Most of the SVG spec is completely implemented. A bnf parser would be better,
//		but most of the attribute strings should be parsed correctly.
//		
//		SVG tags and attribute names should not be case sensitive, but they are.
//		they should also be able to be inside of namespaces, not sure if this works
//
//		Matrices are fundamentally broken, there is a note about that below
//
//		blocks and links don't work. not sure how these should be handled
//		autocad style or should they be exploded?
//
//		there was going to be a setting to explode curves to lines upon deserialization
//		but, it seems like a better idea to implement such a thing as a command in heekscad
//
//		Rounded rectangles aren't handled
//
//		Elliptic arcs need there start and end angles calculated, but there wasn't an elliptic
//		arc element at time of writing
//
//		Line widths and colors are not imported
//	
//		there are fundamentally different ways of deserializing the stream, ie.
//			exploding curves
//			using rects instead of lines	
//			grouping all objects in <g> tags with sketches
//			creating lines with linewidths, or faces of the right width
//		maybe the import command should give a dialog that populates a CDeserializationProfile?
//***********************************************************************************

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

void CSvgRead::Read(const wxChar* filepath)
{
	// start the file
	m_fail = false;
	m_transform = gp_Trsf();

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
		ReadSVGElement(pElem);
	}

}

CSvgRead::~CSvgRead()
{
}

std::string CSvgRead::RemoveCommas(std::string input)
{
	//SVG allows for arbitrary whitespace and commas everywhere
	//sscanf ignores whitespace, so the only problem is commas
	for(unsigned int i=0; i < input.length(); i++)
		if(input[i] == ',')
			input[i] = ' ';
	return input;
}

void CSvgRead::ReadSVGElement(TiXmlElement* pElem)
{
	std::string name(pElem->Value());
	m_transform_stack.push_back(m_transform);

	ReadTransform(pElem);

	if(name == "g")
	{
		// loop through all the child elements, looking for path
		for(pElem = TiXmlHandle(pElem).FirstChildElement().Element(); pElem; pElem = pElem->NextSiblingElement())
		{
			ReadSVGElement(pElem);
		}
	}

	if(name == "path")
	{
		ReadPath(pElem);
	}

	if(name == "rect")
	{
		ReadRect(pElem);
	}

	if(name == "circle")
	{
		ReadCircle(pElem);
	}

	if(name == "ellipse")
	{
		ReadEllipse(pElem);
	}
	
	if(name == "line")
	{
		ReadLine(pElem);
	}

	if(name == "polyline")
	{
		ReadPolyline(pElem,false);
	}

	if(name == "polygon")
	{
		ReadPolyline(pElem,true);
	}

	m_transform = m_transform_stack.back();
	m_transform_stack.pop_back();
	

}

void CSvgRead::ReadTransform(TiXmlElement *pElem)
{
	// TODO: there are lots of default parameters used in transforms
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "transform")
		{
			std::string s = a->Value();
			s = RemoveCommas(s);
			const char* d = s.c_str();
			int pos=0;
			int count=0;
			gp_Trsf ntrsf;
			if(d[pos] == 0)
				break;
			if(strncmp(&d[pos],"translate",9)==0)
			{
				double x,y;
				sscanf(&d[pos],"translate(%lf %lf)%n",&x,&y,&count);
				y=-y;
				ntrsf.SetTranslationPart(gp_Vec(x,y,0));
				m_transform.Multiply(ntrsf);
				pos+=count;
			}
			if(strncmp(&d[pos],"matrix",6)==0)
			{
				double m[16];
				sscanf(&d[pos],"matrix(%lf %lf %lf %lf %lf %lf)%n",&m[0],&m[4],&m[1],&m[5],&m[3],&m[7],&count);
				m[2]=0;
				m[6]=0;
				m[8]=0;
				m[9]=0;
				m[10]=1;
				m[11]=0;
				m[12]=0;
				m[13]=0;
				m[14]=0;
				m[15]=1;

				double d = m[0]*m[5]-m[4]*m[1];
				m[10] = sqrt(d); //The Z component must be of the same magnitude as the rest of
				//the matrix. It really makes no difference what it is, since all z's are 0
				
				//TODO: Uncomment the following lines for matrix support. 
				//Opencascade doesn't support assymetric transforms(non uniform matrices)
				//unforunately most matrix transforms are non uniform, so this usually just 
				//throws exceptions

				//In all probability we will have to transform all shapes by the assymetric matrix
				//this is tricky for ellipses and such
				//probably a v3 feature

				//:JonPry
				
				// ntrsf = make_matrix(m);
				// m_transform.Multiply(ntrsf);	
				pos+=count;
			}
			if(strncmp(&d[pos],"skewX",5)==0)
			{
				//TODO: see above, these are assymetric transforms
				double skew=0;
				sscanf(&d[pos],"skewX(%lf)%n",&skew,&count);
				pos+=count;
			}
			if(strncmp(&d[pos],"skewY",5)==0)
			{
				//TODO: see above, these are assymetric transforms
				double skew=0;
				sscanf(&d[pos],"skewY(%lf)%n",&skew,&count);
				pos+=count;
			}
			if(strncmp(&d[pos],"scale",5)==0)
			{
				double x=0;
				double y=0;
				sscanf(&d[pos],"scale(%lf %lf)%n",&x,&y,&count);
				if(y==0)
					y=x;
				//TODO: assymetric scaling
				ntrsf.SetScale(gp_Pnt(0,0,0),x);
				m_transform.Multiply(ntrsf);
				pos+=count;
			}
			if(strncmp(&d[pos],"rotate",6)==0)
			{
				double rot;
				sscanf(&d[pos],"rotate(%lf)%n",&rot,&count);
				ntrsf.SetRotation(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(0,0,1)),3*Pi/2-rot);
				m_transform.Multiply(ntrsf);
				pos+=count;

			}
		}
	}

}

void CSvgRead::ReadRect(TiXmlElement *pElem)
{
	double x = 0,y = 0,width = 0,height = 0;
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

	OnReadLine(p1,p2);
	OnReadLine(p2,p3);
	OnReadLine(p3,p4);
	OnReadLine(p4,p1);
}

void CSvgRead::ReadEllipse(TiXmlElement *pElem)
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
	OnReadEllipse(gp_Pnt(x,y,0),rx,ry,rot,0,2*Pi);
}

void CSvgRead::ReadLine(TiXmlElement *pElem)
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

	OnReadLine(gp_Pnt(x1,y1,0),gp_Pnt(x2,y2,0));
}

void CSvgRead::ReadPolyline(TiXmlElement *pElem, bool close)
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
						OnReadLine(ppnt,spnt);
					break;
				}
				if(isdigit(d[pos])){
					double x,y;
					int count=0;
					sscanf(&d[pos],"%lf,%lf%n",&x,&y,&count);
					y=-y;
					gp_Pnt cpnt(x,y,0);
					if(has_point)
						OnReadLine(ppnt,cpnt);
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


void CSvgRead::ReadCircle(TiXmlElement *pElem)
{
	double x = 0,y = 0,r = 0;
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
	OnReadCircle(cp,r);
}

gp_Pnt CSvgRead::ReadStart(const char *text,gp_Pnt ppnt,bool isupper)
{
	double x, y;
	sscanf(text, "%lf%lf", &x, &y);
	y = -y;
	gp_Pnt npt(x,y,0);
	if(!isupper)
	{
		npt.SetX(x+ppnt.X());
		npt.SetY(y+ppnt.Y());
	}
	OnReadStart();
	return npt;
}

gp_Pnt CSvgRead::ReadLine(const char *text,gp_Pnt ppnt,bool isupper)
{
	double x, y;
	sscanf(text, "%lf%lf", &x, &y);
	y = -y;
	gp_Pnt npt(x,y,0);
	if(!isupper)
	{
		npt.SetX(x+ppnt.X());
		npt.SetY(y+ppnt.Y());
	}
	OnReadLine(ppnt,npt);
	return npt;
}

void CSvgRead::ReadClose(gp_Pnt ppnt,gp_Pnt spnt)
{
	OnReadLine(ppnt,spnt);
}

gp_Pnt CSvgRead::ReadHorizontal(const char *text,gp_Pnt ppnt,bool isupper)
{
	double x;
	sscanf(text, "%lf", &x);
	gp_Pnt npt(x,ppnt.Y(),0);
	if(!isupper)
		npt.SetX(x+ppnt.X());
	OnReadLine(ppnt,npt);
	return npt;
}

gp_Pnt CSvgRead::ReadVertical(const char *text,gp_Pnt ppnt,bool isupper)
{
	double y;
	sscanf(text, "%lf", &y);
	y = -y;
	gp_Pnt npt(ppnt.X(),y,0);
	if(!isupper)
		npt.SetY(y+ppnt.Y());
	OnReadLine(ppnt,npt);
	return npt;
}


struct TwoPoints CSvgRead::ReadCubic(const char *text,gp_Pnt ppnt,bool isupper)
{
	struct TwoPoints retpts;
	double x1, y1, x2, y2, x3, y3;
	sscanf(text, "%lf%lf%lf%lf%lf%lf", &x1, &y1, &x2, &y2, &x3, &y3);
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

	OnReadCubic(ppnt,pnt1,retpts.pcpnt,retpts.ppnt);
	return retpts;
}

struct TwoPoints CSvgRead::ReadCubic(const char *text,gp_Pnt ppnt, gp_Pnt pcpnt, bool isupper)
{
	struct TwoPoints retpts;
	double x2, y2, x3, y3;
	sscanf(text, "%lf%lf%lf%lf", &x2, &y2, &x3, &y3);
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

	OnReadCubic(ppnt,pnt1,retpts.pcpnt,retpts.ppnt);

	return retpts;
}

struct TwoPoints CSvgRead::ReadQuadratic(const char *text,gp_Pnt ppnt,bool isupper)
{
	struct TwoPoints retpts;
	double x1, y1, x2, y2;
	sscanf(text, "%lf%lf%lf%lf", &x1, &y1, &x2, &y2);
	y1 = -y1; y2 = -y2; 

	if(!isupper)
	{
		x1+=ppnt.X(); y1+=ppnt.Y();
		x2+=x1;	y2+=y1;
	}

	retpts.pcpnt = gp_Pnt(x1,y1,0);
	retpts.ppnt = gp_Pnt(x2,y2,0);

	OnReadQuadratic(ppnt,retpts.pcpnt,retpts.ppnt);
	return retpts;
}

struct TwoPoints CSvgRead::ReadQuadratic(const char *text,gp_Pnt ppnt, gp_Pnt pcpnt, bool isupper)
{
	struct TwoPoints retpts;
	double x2, y2;
	sscanf(text, "%lf%lf", &x2, &y2);
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

	OnReadQuadratic(ppnt,retpts.pcpnt,retpts.ppnt);
	return retpts;
}

gp_Pnt CSvgRead::ReadEllipse(const char *text,gp_Pnt ppnt,bool isupper)
{
	int large_arc_flag, sweep_flag;
	double rx, ry, xrot, x, y;
	sscanf(text, "%lf%lf%lf%d%d%lf%lf", &rx, &ry, &xrot, &large_arc_flag, &sweep_flag, &x, &y);
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

	gp_Pnt start(ppnt.XYZ() - cpnt.XYZ());
	gp_Pnt end(ept.XYZ()-cpnt.XYZ());
	start.Rotate(gp_Ax1(zp,up),-xrot);
	end.Rotate(gp_Ax1(zp,up),-xrot);

	double start_angle = atan2(start.Y()/ry,start.X()/rx);
	double end_angle = atan2(end.Y()/ry,end.X()/rx);

	if(start_angle<0)
		start_angle+=2*Pi;
	if(end_angle<0)
		end_angle+=2*Pi;

	double d_angle = end_angle - start_angle;
	
	if(d_angle < 0)
		d_angle += 2*Pi;

	if((large_arc_flag && (d_angle < Pi)) || (!large_arc_flag && (d_angle > Pi)))
	{
		double temp = start_angle;
		start_angle = end_angle;
		end_angle = temp;
	}

	OnReadEllipse(cpnt,rx,ry,xrot,start_angle,end_angle);
	return ept;
}

void CSvgRead::ReadPath(TiXmlElement* pElem)
{
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "d")
		{
			// add lines and arcs and bezier curves
			std::string in(a->Value());
			in = RemoveCommas(in);
			const char* d = in.c_str();
			gp_Pnt spnt(0,0,0);
			gp_Pnt ppnt(0,0,0);
			gp_Pnt pcpnt(0,0,0);

			int pos = 0;
			while(1){
				if(toupper(d[pos]) == 'M'){
					// make a sketch
					spnt = ReadStart(&d[pos+1],ppnt,isupper(d[pos])!=0);
					ppnt = spnt;
					pos++;
				}
				else if(toupper(d[pos]) == 'L'){
					// add a line
					ppnt = ReadLine(&d[pos+1],ppnt,isupper(d[pos])!=0);
					pos++;
				}
				else if(toupper(d[pos]) == 'H'){
					//horizontal line
					ppnt = ReadHorizontal(&d[pos+1],ppnt,isupper(d[pos])!=0);
					pos++;
				}
				else if(toupper(d[pos]) == 'V'){
					//vertical line
					ppnt = ReadVertical(&d[pos+1],ppnt,isupper(d[pos])!=0);
					pos++;
				}
				else if(toupper(d[pos]) == 'C'){
					// add a cubic bezier curve ( just split into lines for now )
					struct TwoPoints ret = ReadCubic(&d[pos+1],ppnt,isupper(d[pos])!=0);
					ppnt = ret.ppnt;
					pcpnt = ret.pcpnt;
					pos++;
				}
				else if(toupper(d[pos]) == 'S'){
                    // add a cubic bezier curve ( short hand)
					struct TwoPoints ret = ReadCubic(&d[pos+1],ppnt,pcpnt,isupper(d[pos])!=0);
					ppnt = ret.ppnt;
					pcpnt = ret.pcpnt;
					pos++;
				}
				else if(toupper(d[pos]) == 'Q'){
					// add a quadratic bezier curve 
					struct TwoPoints ret = ReadQuadratic(&d[pos+1],ppnt,isupper(d[pos])!=0);
					ppnt = ret.ppnt;
					pcpnt = ret.pcpnt;
					pos++;
				}
				else if(toupper(d[pos]) == 'T'){
               		// add a quadratic bezier curve 
					struct TwoPoints ret = ReadQuadratic(&d[pos+1],ppnt,pcpnt,isupper(d[pos])!=0);
					ppnt = ret.ppnt;
					pcpnt = ret.pcpnt;
					pos++;
				}
				else if(toupper(d[pos]) == 'A'){
					// add an elliptic arc
					ppnt = ReadEllipse(&d[pos+1],ppnt,isupper(d[pos])!=0);
					pos++;
				}
				else if(toupper(d[pos]) == 'Z'){
					// join to end
					ReadClose(ppnt,spnt);
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

HeeksSvgRead::HeeksSvgRead(const wxChar* filepath, bool usehspline)
{
	m_usehspline=usehspline;
	m_sketch = 0;
	Read(filepath);
}

void HeeksSvgRead::ModifyByMatrix(HeeksObj* object)
{
	double m[16];
	extract(m_transform,m);
	object->ModifyByMatrix(m);
}

void HeeksSvgRead::OnReadStart()
{
		m_sketch = new CSketch();
		wxGetApp().Add(m_sketch, NULL);
}

void HeeksSvgRead::OnReadCubic(gp_Pnt s, gp_Pnt c1, gp_Pnt c2, gp_Pnt e)
{
	TColgp_Array1OfPnt poles(1,4);
	poles.SetValue(1,s); poles.SetValue(2,c1); poles.SetValue(3,c2); poles.SetValue(4,e);
	Handle(Geom_BezierCurve) curve = new Geom_BezierCurve(poles);
	GeomConvert_CompCurveToBSplineCurve convert(curve);

	Handle_Geom_BSplineCurve spline = convert.BSplineCurve();
//	Geom_BSplineCurve pspline = *((Geom_BSplineCurve*)spline.Access());
	HSpline* new_object = new HSpline(spline, &wxGetApp().current_color);
	ModifyByMatrix(new_object);
	AddSketchIfNeeded();
	m_sketch->Add(new_object, NULL);
}

void HeeksSvgRead::OnReadQuadratic(gp_Pnt s, gp_Pnt c, gp_Pnt e)
{
	TColgp_Array1OfPnt poles(1,3);
	poles.SetValue(1,s); poles.SetValue(2,c); poles.SetValue(3,e);
	Handle(Geom_BezierCurve) curve = new Geom_BezierCurve(poles);
	GeomConvert_CompCurveToBSplineCurve convert(curve);

	Handle_Geom_BSplineCurve spline = convert.BSplineCurve();
//	Geom_BSplineCurve pspline = *((Geom_BSplineCurve*)spline.Access());
	HSpline* new_object = new HSpline(spline, &wxGetApp().current_color);
	ModifyByMatrix(new_object);
	AddSketchIfNeeded();
	m_sketch->Add(new_object, NULL);}

void HeeksSvgRead::OnReadLine(gp_Pnt p1, gp_Pnt p2)
{
	HLine *line = new HLine(p1,p2,&wxGetApp().current_color);
	ModifyByMatrix(line);
	AddSketchIfNeeded();
	m_sketch->Add(line, NULL);
}

void HeeksSvgRead::OnReadEllipse(gp_Pnt c, double maj_r, double min_r, double rot, double start, double end)
{
	gp_Dir up(0,0,1);
	gp_Elips elip(gp_Ax2(c,gp_Dir(0,0,1)),maj_r,min_r);
	elip.Rotate(gp_Ax1(c,up),rot);
	HEllipse *new_object = new HEllipse(elip,start,end,&wxGetApp().current_color);
	ModifyByMatrix(new_object);
	AddSketchIfNeeded();
	m_sketch->Add(new_object, NULL);
}

void HeeksSvgRead::OnReadCircle(gp_Pnt c, double r)
{
	gp_Dir up(0,0,1);
	gp_Circ cir(gp_Ax2(c,up),r);
	HCircle *new_object = new HCircle(cir,&wxGetApp().current_color);
	ModifyByMatrix(new_object);
	AddSketchIfNeeded();
	m_sketch->Add(new_object, NULL);
}

void HeeksSvgRead::AddSketchIfNeeded()
{
	if(m_sketch == NULL)
	{
		m_sketch = new CSketch();
		wxGetApp().Add(m_sketch, NULL);
	}
}
