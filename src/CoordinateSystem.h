// CoordinateSystem.h

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
	const wxChar* GetTypeString(void)const{return _T("Coordinate System");}
	HeeksObj *MakeACopy(void)const;
	wxIcon* GetIcon();
	bool ModifyByMatrix(const double *mat);
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<double> *list, bool just_for_endof){}
	void WriteXML(TiXmlElement *root);

	gp_Trsf GetMatrix();

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
	static void RenderDatum(); // render a coordinate system at 0, 0, 0
	static void AxesToAngles(const gp_Dir &x, const gp_Dir &y, double &v_angle, double &h_angle, double &t_angle);
	static void AnglesToAxes(const double &v_angle, const double &h_angle, const double &t_angle, gp_Dir &x, gp_Dir &y);
};
