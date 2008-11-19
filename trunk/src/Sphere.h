// Sphere.h

#pragma once

#include "Solid.h"

class CSphere: public CSolid{
private:
	static wxIcon* m_icon;

public:
	gp_Pnt m_pos;
	double m_radius;

	CSphere(const gp_Pnt& pos, double radius, const wxChar* title, const HeeksColor& col);
	CSphere(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col);

	// HeeksObj's virtual functions
	const wxChar* GetTypeString(void)const{return _("Sphere");}
	wxString GetIcon(){return _T("sphere");}
	HeeksObj *MakeACopy(void)const;
	bool ModifyByMatrix(const double* m);
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	void OnApplyProperties();
	bool GetCentrePoint(double* pos);
	bool GetScaleAboutMatrix(double *m);

	// CShape's virtual functions
	void SetXMLElement(TiXmlElement* element);
	void SetFromXMLElement(TiXmlElement* pElem);

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_SPHERE;}
};
