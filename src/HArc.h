// HArc.h

#pragma once
#include "../interface/HeeksObj.h"
#include "../interface/HeeksColor.h"
#include <gp_Circ.hxx>
#include "HGeomObject.h"

class HArc: public HGeomObject{
private:
	HeeksColor color;

public:
	gp_Pnt A, B;
	gp_Circ m_circle;

	~HArc(void);
	HArc(const gp_Pnt &a, const gp_Pnt &b, const gp_Circ &c, const HeeksColor* col);
	HArc(const HArc &arc);

	const HArc& operator=(const HArc &b);

	// HeeksObj's virtual functions
	int GetType()const{return ArcType;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const char* GetTypeString(void)const{return "Arc";}
	HeeksObj *MakeACopy(void)const;
	void ModifyByMatrix(const double *mat);
	void SetColor(const HeeksColor &col){color = col;}
	const HeeksColor* GetColor()const{return &color;}
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool FindNearPoint(const double* ray_start, const double* ray_direction, double *point);
	void Stretch(const double *p, const double* shift, double* new_position);
	void GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point = true)const;
	bool GetStartPoint(double* pos);
	bool GetEndPoint(double* pos);
	bool GetCentrePoint(double* pos);

	gp_Vec GetSegmentVector(double fraction);
	gp_Pnt GetPointAtFraction(double fraction);
	static bool TangentialArc(const gp_Pnt &p0, const gp_Vec &v0, const gp_Pnt &p1, gp_Pnt &centre, gp_Vec &axis);
};
