// Cuboid.h

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

	CCuboid(const gp_Ax2& pos, double x, double y, double z, const wxChar* title);

	// HeeksObj's virtual functions
	const wxChar* GetTypeString(void)const{return _T("Cuboid");}
	wxIcon* GetIcon();
	HeeksObj *MakeACopy(void)const;
	bool ModifyByMatrix(const double* m);
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	void OnApplyProperties();
	bool GetScaleAboutMatrix(double *m);
	bool Stretch(const double *p, const double* shift);

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_CUBOID;}
};
