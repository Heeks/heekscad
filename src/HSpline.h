// HSpline.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"
#include "../interface/HeeksColor.h"
#include <Geom_BSplineCurve.hxx>
//#include <Handle_Geom_Geometry.hxx>

class HSpline: public HeeksObj{
private:
	HeeksColor color;
	static wxIcon* m_icon;

public:
	Handle(Geom_BSplineCurve) m_spline;

	~HSpline(void);
	HSpline(const Geom_BSplineCurve &s, const HeeksColor* col);
	HSpline(const HSpline &c);

	const HSpline& operator=(const HSpline &c);

	// HeeksObj's virtual functions
	int GetType()const{return SplineType;}
	long GetMarkingMask()const{return MARKING_FILTER_CIRCLE;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wxChar* GetTypeString(void)const{return _("Spline");}
	HeeksObj *MakeACopy(void)const;
	wxString GetIcon(){return wxGetApp().GetExeFolder() + _T("/icons/circle");}
	bool ModifyByMatrix(const double *mat);
	void SetColor(const HeeksColor &col){color = col;}
	const HeeksColor* GetColor()const{return &color;}
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool FindNearPoint(const double* ray_start, const double* ray_direction, double *point);
	bool FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point);
	bool Stretch(const double *p, const double* shift);
	void GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point = true)const;
	void WriteXML(TiXmlNode *root);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);

};
