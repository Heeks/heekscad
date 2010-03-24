// HAngularDimension.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "ConstrainedObject.h"
#include "../interface/HeeksColor.h"

enum AngularDimensionTextMode
{
	StringAngularDimensionTextMode,
	DegreesAngularDimensionTextMode,
	RadiansAngularDimensionTextMode
};



class HAngularDimension: public ConstrainedObject{
private:
	HeeksColor m_color;
	static wxIcon* m_icon;

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
	void GetIcon(int& texture_number, int& x, int& y){GET_ICON(0, 1);}
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
	void LoadToDoubles();
	void LoadFromDoubles();
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	bool IsDifferent(HeeksObj* other);
	void ReloadPointers();

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
};
