// Sphere.h

#pragma once

#include "Solid.h"

class CSphere: public CSolid{
private:
	static wxIcon* m_icon;

public:
	gp_Pnt m_pos;
	double m_radius;

	CSphere(const gp_Pnt& pos, double radius, const wxChar* title);

	// HeeksObj's virtual functions
	const wxChar* GetTypeString(void)const{return _T("Sphere");}
	wxIcon* GetIcon();
	HeeksObj *MakeACopy(void)const;
	void ModifyByMatrix(const double* m, bool for_undo);
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	void OnApplyProperties();
	bool GetCentrePoint(double* pos);
	bool GetScaleAboutMatrix(double *m);

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_SPHERE;}
};
