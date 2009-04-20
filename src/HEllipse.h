// HEllipse.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"
#include "../interface/HeeksColor.h"
#include <gp_Elips.hxx>

class HEllipse: public HeeksObj{
private:
	HeeksColor color;
	static wxIcon* m_icon;

public:
	gp_Elips m_ellipse;

	~HEllipse(void);
	HEllipse(const gp_Elips &c, const HeeksColor* col);
	HEllipse(const HEllipse &c);

	const HEllipse& operator=(const HEllipse &c);

	// HeeksObj's virtual functions
	int GetType()const{return EllipseType;}
	long GetMarkingMask()const{return MARKING_FILTER_CIRCLE;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wxChar* GetTypeString(void)const{return _("Ellipse");}
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
	bool GetCentrePoint(double* pos);
	void WriteXML(TiXmlNode *root);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	double GetRotation();
	void SetRotation(double rot);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);

};
