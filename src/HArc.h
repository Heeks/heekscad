// HArc.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "EndedObject.h"

class HArc: public EndedObject{
private:
	HeeksColor color;

public:
	gp_Ax1 m_axis;

	HPoint* C;
	double m_radius;

	~HArc(void);
	HArc(const gp_Pnt &a, const gp_Pnt &b, const gp_Circ &c, const HeeksColor* col);
	HArc(const HArc &arc);

	const HArc& operator=(const HArc &b);

	gp_Circ GetCircle() const;
	void SetCircle(gp_Circ c);
	bool IsIncluded(gp_Pnt pnt);

	// HeeksObj's virtual functions
	int GetType()const{return ArcType;}
	long GetMarkingMask()const{return MARKING_FILTER_ARC;}
	int GetIDGroupType()const{return LineType;}
	void glCommands(bool select, bool marked, bool no_color);
	void Draw(wxDC& dc);
	void GetBox(CBox &box);
	const wxChar* GetTypeString(void)const{return _("Arc");}
	HeeksObj *MakeACopy(void)const;
	const wxBitmap &GetIcon();
	void ModifyByMatrix(const double *mat);
	void SetColor(const HeeksColor &col){color = col;}
	const HeeksColor* GetColor()const{return &color;}
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool FindNearPoint(const double* ray_start, const double* ray_direction, double *point);
	bool FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point);
	bool Stretch(const double *p, const double* shift, void* data);
	void GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point = true)const;
	bool GetCentrePoint(double* pos);
	void WriteXML(TiXmlNode *root);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	bool DescendForUndo(){return false;}
	bool IsDifferent(HeeksObj* other);
	HeeksObj* MakeACopyWithID();
	void ReloadPointers();

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);

	bool Intersects(const gp_Pnt &pnt)const;
	gp_Vec GetSegmentVector(double fraction)const;
	gp_Pnt GetPointAtFraction(double fraction)const;
	static bool TangentialArc(const gp_Pnt &p0, const gp_Vec &v0, const gp_Pnt &p1, gp_Pnt &centre, gp_Dir &axis);
	bool UsesID(){return true;} 
	void Reverse();
	double IncludedAngle()const;

#ifdef MULTIPLE_OWNERS
	//Ended Objects virtaul functions;
	void LoadToDoubles();
	void LoadFromDoubles();
#endif
};
