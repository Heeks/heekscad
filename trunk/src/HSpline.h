// HSpline.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"
#include "../interface/HeeksColor.h"
//#include <Handle_Geom_Geometry.hxx>

// CTangentialArc is used to calculate an arc given desired start ( p0 ), end ( p1 ) and start direction ( v0 )
class CTangentialArc
{
public:
	gp_Pnt m_p0; // start point
	gp_Vec m_v0; // start direction
	gp_Pnt m_p1; // end point
	gp_Pnt m_c; // centre point
	gp_Dir m_a; // axis
	bool m_is_a_line;
	CTangentialArc(const gp_Pnt &p0, const gp_Vec &v0, const gp_Pnt &p1);
	bool radius_equal(const gp_Pnt &p, double tolerance)const;
	double radius()const;
	HeeksObj* MakeHArc()const;
};

class HSpline: public HeeksObj{
private:
	HeeksColor color;
	static wxIcon* m_icon;

	void CreateArcs(const gp_Pnt &p_start, const gp_Vec &v_start, double t_start, double t_end, gp_Pnt &p_end, gp_Vec &v_end);

public:
	Handle(Geom_BSplineCurve) m_spline;

	~HSpline(void);
	HSpline(const Geom_BSplineCurve &s, const HeeksColor* col);
	HSpline(const Handle_Geom_BSplineCurve s, const HeeksColor* col);
	HSpline(const HSpline &c);

	const HSpline& operator=(const HSpline &c);

	// HeeksObj's virtual functions
	int GetType()const{return SplineType;}
	long GetMarkingMask()const{return MARKING_FILTER_CIRCLE;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wxChar* GetTypeString(void)const{return _("Spline");}
	HeeksObj *MakeACopy(void)const;
	void GetIcon(int& texture_number, int& x, int& y){GET_ICON(11, 0);}
	bool ModifyByMatrix(const double *mat);
	void SetColor(const HeeksColor &col){color = col;}
	const HeeksColor* GetColor()const{return &color;}
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool FindNearPoint(const double* ray_start, const double* ray_direction, double *point);
	bool FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point);
	bool Stretch(const double *p, const double* shift, void* data);
	void GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point = true)const;
	void WriteXML(TiXmlNode *root);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
	bool IsDifferent(HeeksObj* other);

	void ToBiarcs(std::list<HeeksObj*> &new_spans, double tolerance);
};
