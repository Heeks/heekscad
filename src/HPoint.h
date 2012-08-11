// HPoint.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"
#include "../interface/HeeksColor.h"
#ifdef MULTIPLE_OWNERS
#include "../interface/ObjList.h"

class HPoint: public ObjList{
#else
class HPoint: public HeeksObj{
#endif
private:
	HeeksColor color;

public:
	gp_Pnt m_p;
	bool m_draw_unselected;
	double mx,my;

	~HPoint(void);
	HPoint(const gp_Pnt &p, const HeeksColor* col);
	HPoint(const HPoint &p);

	const HPoint& operator=(const HPoint &b);

	// HeeksObj's virtual functions
	int GetType()const{return PointType;}
	long GetMarkingMask()const{return MARKING_FILTER_POINT;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wxChar* GetTypeString(void)const{return _("Point");}
	HeeksObj *MakeACopy(void)const;
	const wxBitmap &GetIcon();
	void ModifyByMatrix(const double *mat);
	void SetColor(const HeeksColor &col){color = col;}
	const HeeksColor* GetColor()const{return &color;}
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool GetStartPoint(double* pos);
	bool GetEndPoint(double* pos);
	void CopyFrom(const HeeksObj* object){operator=(*((HPoint*)object));}
	void WriteXML(TiXmlNode *root);
	void LoadFromDoubles();
	void LoadToDoubles();
	bool IsDifferent(HeeksObj* other);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);

	void Draw(wxDC& dc);


	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
};
