// Cone.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Solid.h"

class CCone: public CSolid{
protected:
	bool m_render_without_OpenCASCADE;

	// CShape's virtual functions
	void MakeTransformedShape(const gp_Trsf &mat);
	wxString StretchedName();

public:
	gp_Ax2 m_pos;
	double m_r1;
	double m_r2;
	double m_height;
	double m_temp_r1;
	double m_temp_r2;

	CCone(const gp_Ax2& pos, double r1, double r2, double height, const wxChar* title, const HeeksColor& col, float opacity);
	CCone(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col, float opacity);

	// HeeksObj's virtual functions
	const wxChar* GetTypeString(void)const{return _("Cone");}
	const wxBitmap &GetIcon();
	void glCommands(bool select, bool marked, bool no_color);
	HeeksObj *MakeACopy(void)const;
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void OnApplyProperties();
	bool ValidateProperties();
	bool GetScaleAboutMatrix(double *m);
	bool Stretch(const double *p, const double* shift, void* data);
	bool StretchTemporary(const double *p, const double* shift, void* data);
	bool IsDifferent(HeeksObj*other);
	bool DescendForUndo(){return false;}

	// CShape's virtual functions
	void SetXMLElement(TiXmlElement* element);
	void SetFromXMLElement(TiXmlElement* pElem);

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_CONE;}

	bool Stretch2(const double *p, const double* shift, gp_Ax2& new_pos, double& new_r1, double& new_r2, double& new_height);
};
