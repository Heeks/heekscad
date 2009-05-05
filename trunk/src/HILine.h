// HILine.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"
#include "../interface/HeeksColor.h"

class HILine: public HeeksObj{
private:
	HeeksColor color;
	static wxIcon* m_icon;

public:
	gp_Pnt A, B;

	~HILine(void);
	HILine(const gp_Pnt &a, const gp_Pnt &b, const HeeksColor* col);
	HILine(const HILine &line);

	const HILine& operator=(const HILine &b);

	// HeeksObj's virtual functions
	int GetType()const{return ILineType;}
	long GetMarkingMask()const{return MARKING_FILTER_ILINE;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wxChar* GetTypeString(void)const{return _("Infinite Line");}
	HeeksObj *MakeACopy(void)const;
#ifdef WIN32
	wxString GetIcon(){return wxGetApp().GetExeFolder() + _T("/icons/iline");}
#else
	wxString GetIcon(){return wxGetApp().GetExeFolder() + _T("/../share/heekscad/icons/iline");}
#endif
	bool ModifyByMatrix(const double *mat);
	void SetColor(const HeeksColor &col){color = col;}
	const HeeksColor* GetColor()const{return &color;}
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool FindNearPoint(const double* ray_start, const double* ray_direction, double *point);
	bool FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point);
	bool Stretch(const double *p, const double* shift);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	bool GetStartPoint(double* pos);
	bool GetEndPoint(double* pos);
	void CopyFrom(const HeeksObj* object){operator=(*((HILine*)object));}
	void WriteXML(TiXmlNode *root);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);

	gp_Lin GetLine()const;
};
