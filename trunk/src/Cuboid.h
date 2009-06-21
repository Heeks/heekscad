// Cuboid.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Solid.h"

class CCuboid: public CSolid{
private:
	static wxIcon* m_icon;

public:
	gp_Ax2 m_pos; // coordinate system defining position and orientation
	double m_x; // width
	double m_y; // height
	double m_z; // depth

	CCuboid(const gp_Ax2& pos, double x, double y, double z, const wxChar* title, const HeeksColor& col);
	CCuboid(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col);

	// HeeksObj's virtual functions
	const wxChar* GetTypeString(void)const{return _("Cuboid");}
	wxString GetIcon(){return wxGetApp().GetResFolder() + _T("/icons/cube");}
	HeeksObj *MakeACopy(void)const;
	bool ModifyByMatrix(const double* m);
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void OnApplyProperties();
	bool GetScaleAboutMatrix(double *m);
	bool Stretch(const double *p, const double* shift, void* data);

	// CShape's virtual functions
	void SetXMLElement(TiXmlElement* element);
	void SetFromXMLElement(TiXmlElement* pElem);

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_CUBOID;}
};
