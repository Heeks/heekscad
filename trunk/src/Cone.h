// Cone.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#ifdef WIN32
#pragma once
#endif

#include "Solid.h"

class CCone: public CSolid{
private:
	static wxIcon* m_icon;
	bool m_render_without_OpenCASCADE;

public:
	gp_Ax2 m_pos;
	double m_r1;
	double m_r2;
	double m_height;
	double m_temp_r1;
	double m_temp_r2;

	CCone(const gp_Ax2& pos, double r1, double r2, double height, const wxChar* title, const HeeksColor& col);
	CCone(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col);

	// HeeksObj's virtual functions
	const wxChar* GetTypeString(void)const{return _("Cone");}
	wxString GetIcon(){return _T("cone");}
	void glCommands(bool select, bool marked, bool no_color);
	HeeksObj *MakeACopy(void)const;
	bool ModifyByMatrix(const double* m);
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	void OnApplyProperties();
	bool ValidateProperties();
	bool GetScaleAboutMatrix(double *m);
	bool Stretch(const double *p, const double* shift);
	bool StretchTemporary(const double *p, const double* shift);

	// CShape's virtual functions
	void SetXMLElement(TiXmlElement* element);
	void SetFromXMLElement(TiXmlElement* pElem);

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_CONE;}

	bool Stretch2(const double *p, const double* shift, gp_Ax2& new_pos, double& new_r1, double& new_r2, double& new_height);
};
