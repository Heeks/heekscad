// HILine.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "EndedObject.h"

class HILine: public EndedObject{
private:
	HeeksColor color;

public:
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
	const wxBitmap &GetIcon();
	void SetColor(const HeeksColor &col){color = col;}
	const HeeksColor* GetColor()const{return &color;}
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool FindNearPoint(const double* ray_start, const double* ray_direction, double *point);
	bool FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	void CopyFrom(const HeeksObj* object){operator=(*((HILine*)object));}
	void WriteXML(TiXmlNode *root);
	bool GetEndPoint(double* pos);
	bool GetStartPoint(double* pos);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);

	gp_Lin GetLine()const;
};
