// HEllipse.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#ifdef MULTIPLE_OWNERS
#include "../interface/ObjList.h"
class HPoint;
class HEllipse: public ObjList{
#else
#include "HPoint.h"
class HEllipse: public HeeksObj{
#endif
private:
	HeeksColor color;

public:
	HPoint* C;
	double m_rot;
	double m_start; double m_end;
	double m_majr; double m_minr;
	gp_Dir m_zdir;
	gp_Dir m_xdir;

	~HEllipse(void);
	HEllipse(const gp_Elips &c, const HeeksColor* col);
	HEllipse(const gp_Elips &c, double start, double end,const HeeksColor* col);
	HEllipse(const HEllipse &c);

	const HEllipse& operator=(const HEllipse &c);

	double GetRotation()const;
	void SetRotation(double rot);
	void SetEllipse(gp_Elips e);
	gp_Elips GetEllipse() const;


	// HeeksObj's virtual functions
	int GetType()const{return EllipseType;}
	long GetMarkingMask()const{return MARKING_FILTER_CIRCLE;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wxChar* GetTypeString(void)const{return _("Ellipse");}
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
	void ReloadPointers();
	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
	void LoadToDoubles();
	void LoadFromDoubles();

};

