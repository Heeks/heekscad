// Cylinder.h

#pragma once

#include "Solid.h"

class CCylinder: public CSolid{
private:
	static wxIcon* m_icon;

public:
	gp_Ax2 m_pos;
	double m_radius;
	double m_height;

	CCylinder(const gp_Ax2& pos, double radius, double height, const wxChar* title);

	// HeeksObj's virtual functions
	const wxChar* GetTypeString(void)const{return _T("Cylinder");}
	wxIcon* GetIcon();
	HeeksObj *MakeACopy(void)const;
	void ModifyByMatrix(const double* m, bool for_undo);
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	void OnApplyProperties();
	bool GetScaleAboutMatrix(double *m);
	bool Stretch(const double *p, const double* shift, double* new_position);

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_CYLINDER;}
};
