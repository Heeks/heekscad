// HAngularDimension.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#ifdef MULTIPLE_OWNERS
#include "../interface/ObjList.h"
class HPoint;
#include "../interface/HeeksColor.h"
#else
#include "HPoint.h"
#endif

enum AngularDimensionTextMode
{
	StringAngularDimensionTextMode,
	DegreesAngularDimensionTextMode,
	RadiansAngularDimensionTextMode
};

#ifdef MULTIPLE_OWNERS
class HAngularDimension: public ObjList{
#else
class HAngularDimension: public HeeksObj{
#endif
private:
	HeeksColor m_color;

public:
	wxString m_text;
	HPoint* m_p0;
	HPoint* m_p1;
	HPoint* m_p2;
	HPoint* m_p3;
	HPoint* m_p4;
	AngularDimensionTextMode m_text_mode;
	double m_scale; // to do - text, gaps, and arrow heads will be scaled by this factor

	HAngularDimension(const wxString &text, const gp_Pnt &p0, const gp_Pnt &p1, const gp_Pnt &p2, const gp_Pnt &p3, const gp_Pnt &p4, AngularDimensionTextMode text_mode, const HeeksColor* col);
	HAngularDimension(const wxString &text, AngularDimensionTextMode text_mode, const HeeksColor* col);
	HAngularDimension(const HAngularDimension &b);
	~HAngularDimension(void);

	const HAngularDimension& operator=(const HAngularDimension &b);
	void DrawLine(gp_Pnt p1, gp_Pnt p2);
	void DrawArc(gp_Pnt center, double radius, double a1, double a2);

	// HeeksObj's virtual functions
	int GetType()const{return AngularDimensionType;}
	long GetMarkingMask()const{return MARKING_FILTER_DIMENSION;}
	void glCommands(bool select, bool marked, bool no_color);
	bool DrawAfterOthers(){return true;}
	void GetBox(CBox &box);
	const wxChar* GetTypeString(void)const{return _("Dimension");}
	HeeksObj *MakeACopy(void)const;
	const wxBitmap &GetIcon();
	void ModifyByMatrix(const double *mat);
	void SetColor(const HeeksColor &col){m_color = col;}
	const HeeksColor* GetColor()const{return &m_color;}
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool Stretch(const double *p, const double* shift, void* data);
	void CopyFrom(const HeeksObj* object){operator=(*((HAngularDimension*)object));}
	void WriteXML(TiXmlNode *root);
	//const wxChar* GetShortString(void)const{return m_text.c_str();}
	bool CanEditString(void)const{return true;}
	void OnEditString(const wxChar* str);
#ifdef MULTIPLE_OWNERS
	void LoadToDoubles();
	void LoadFromDoubles();
#endif
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	bool IsDifferent(HeeksObj* other);
	void ReloadPointers();

	wxString MakeText(double angle);
	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
};
