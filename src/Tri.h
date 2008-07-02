// Tri.h

#pragma once

#include "../interface/HeeksObj.h"

class CTri : public HeeksObj 
{
private:
	CBox m_box;
	bool m_norm_exists;
	gp_Pnt m_p[3];
	gp_Vec m_norm;

public:
	CTri(void);
	CTri(const gp_Pnt& p0, const gp_Pnt& p1, const gp_Pnt& p2);
	CTri(const CTri& tri);
	virtual ~CTri();

	const CTri& operator=(const CTri& tri);

	// HeeksObj's virtual functions
	void glCommands(bool select, bool marked, bool no_color);
	int GetType()const{return TriType;}
	void GetBox(CBox &box);
	HeeksObj *MakeACopy(void)const;
	const char* GetTypeString(void)const{return "Triangle";}
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	void ModifyByMatrix(const double *m);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	void Stretch(const double *p, const double* shift, double* new_position);
	wxIcon* GetIcon();
	bool GetStartPoint(double* pos);

	// member functions
	void ResetNormExists(){m_norm_exists = false;}
};

