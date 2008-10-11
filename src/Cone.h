// Cone.h

#pragma once

#include "Solid.h"

class CCone: public CSolid{
private:
	static wxIcon* m_icon;

public:
	gp_Ax2 m_pos;
	double m_r1;
	double m_r2;
	double m_height;

	CCone(const gp_Ax2& pos, double r1, double r2, double height, const wxChar* title);

	// HeeksObj's virtual functions
	const wxChar* GetTypeString(void)const{return _T("Cone");}
	wxIcon* GetIcon();
	HeeksObj *MakeACopy(void)const;
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_CONE;}
};
