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
};