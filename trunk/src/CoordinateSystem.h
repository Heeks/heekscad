// CoordinateSystem.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"

class CoordinateSystem: public HeeksObj
{
private:
	static wxIcon* m_icon;

public:
	gp_Pnt m_o;
	gp_Dir m_x;
	gp_Dir m_y;
	wxString m_title;

	static double size;
	static bool size_is_pixels; // false for mm
	static bool rendering_current;

	// used for user properties, but only to create m_x and m_y
	double m_vertical_angle;
	double m_horizontal_angle;
	double m_twist_angle;

	CoordinateSystem(const wxString& str, const gp_Pnt &o, const gp_Dir &x, const gp_Dir &y);
	CoordinateSystem(const CoordinateSystem &c);
	~CoordinateSystem(void);

	const CoordinateSystem& operator=(const CoordinateSystem &c);

	// HeeksObj's virtual functions
	int GetType()const{return CoordinateSystemType;}
	long GetMarkingMask()const{return MARKING_FILTER_COORDINATE_SYSTEM;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wxChar* GetTypeString(void)const{return _("Coordinate System");}
	HeeksObj *MakeACopy(void)const;
	wxString GetIcon(){return wxGetApp().GetResFolder() + _T("/icons/coordsys");}
	bool ModifyByMatrix(const double *mat);
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<double> *list, bool just_for_endof){}
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	void WriteXML(TiXmlNode *root);

	gp_Trsf GetMatrix();
	void ApplyMatrix();

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
	static void RenderArrow();
	static void RenderDatum(bool bright, bool solid); // render a coordinate system at 0, 0, 0
	static void AxesToAngles(const gp_Dir &x, const gp_Dir &y, double &v_angle, double &h_angle, double &t_angle);
	static void AnglesToAxes(const double &v_angle, const double &h_angle, const double &t_angle, gp_Dir &x, gp_Dir &y);
	void AxesToAngles(){AxesToAngles(m_x, m_y, m_vertical_angle, m_horizontal_angle, m_twist_angle);}
	void AnglesToAxes(){AnglesToAxes(m_vertical_angle, m_horizontal_angle, m_twist_angle, m_x, m_y);}
	void PickFrom3Points();
};
