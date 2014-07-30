// HGear.h
// Copyright (c) 2011, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class HGear: public HeeksObj{
	void SetSegmentsVariables(void(*callbackfunc)(const double *p))const;

public:
	gp_Ax2 m_pos; // coordinate system defining position and orientation
	int m_num_teeth;
	double m_module;
	double m_addendum_offset;
	double m_addendum_multiplier;
	double m_dedendum_multiplier;
	double m_pressure_angle;
	double m_tip_relief;
	double m_depth;
	double m_cone_half_angle; // 0 for a cylinder ( spur gear ), 90 for a circular rack
	double m_angle; // draw the gear rotated anti-clockwise by this angle

	HGear();
	HGear(const HGear &o);
	~HGear(void);

	const HGear& operator=(const HGear &o);

	// HeeksObj's virtual functions
	int GetType()const{return GearType;}
	long GetMarkingMask()const{return MARKING_FILTER_GEAR;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wxChar* GetTypeString(void)const{return _("Gear");}
	HeeksObj *MakeACopy(void)const;
	const wxBitmap &GetIcon();
	void ModifyByMatrix(const double *mat);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	bool GetScaleAboutMatrix(double *m);
	void GetProperties(std::list<Property *> *list);
	void WriteXML(TiXmlNode *root);
	bool DescendForUndo(){return false;}
	bool IsDifferent(HeeksObj* other);
	void GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point = true)const;
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
	void GetOneToothSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point = true)const;
	HeeksObj* MakeSketch()const;
	double GetClearanceMM()const;
};
