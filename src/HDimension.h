// HDimension.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "EndedObject.h"
#include "../interface/HeeksColor.h"

enum DimensionMode
{
	TwoPointsDimensionMode,
	OrthogonalDimensionMode,
};

enum DimensionTextMode
{
	StringDimensionTextMode,
	PythagoreanDimensionTextMode,
	HorizontalDimensionTextMode,
	VerticalDimensionTextMode
};

class HeeksConfig;

class HDimension: public EndedObject{
private:
	HeeksColor m_color;
	static wxIcon* m_icon;

public:
	gp_Trsf m_trsf; // draw matrix at time of creation
	wxString m_text;
	HPoint* m_p2;
	DimensionMode m_mode;
	DimensionTextMode m_text_mode;
	double m_scale; // to do - text, gaps, and arrow heads will be scaled by this factor
	static bool DrawFlat;

	HDimension(const gp_Trsf &trsf, const wxString &text, const gp_Pnt &p0, const gp_Pnt &p1, const gp_Pnt &p2, DimensionMode mode, DimensionTextMode text_mode, const HeeksColor* col);
	HDimension(const HDimension &b);
	~HDimension(void);

	const HDimension& operator=(const HDimension &b);

	bool IsConstrained();

	// HeeksObj's virtual functions
	int GetType()const{return DimensionType;}
	long GetMarkingMask()const{return MARKING_FILTER_DIMENSION;}
	void glCommands(bool select, bool marked, bool no_color);
	bool DrawAfterOthers(){return true;}
	void GetBox(CBox &box);
	const wxChar* GetTypeString(void)const{return _("Dimension");}
	HeeksObj *MakeACopy(void)const;
	void GetIcon(int& texture_number, int& x, int& y){GET_ICON(0, 1);}
	bool ModifyByMatrix(const double *mat);
	void SetColor(const HeeksColor &col){m_color = col;}
	const HeeksColor* GetColor()const{return &m_color;}
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool Stretch(const double *p, const double* shift, void* data);
	void CopyFrom(const HeeksObj* object){operator=(*((HDimension*)object));}
	void WriteXML(TiXmlNode *root);
	//const wxChar* GetShortString(void)const{return m_text.c_str();}
	bool CanEditString(void)const{return true;}
	void OnEditString(const wxChar* str);
	void LoadToDoubles();
	void LoadFromDoubles();
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	bool IsDifferent(HeeksObj* other);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);

	static void draw_arrow_line(DimensionMode mode, const gp_Pnt &p0, const gp_Pnt &p1, const gp_Pnt &p2, const gp_Dir &xdir, const gp_Dir &ydir, double width, double scale);
	static void WriteToConfig(HeeksConfig& config);
	static void ReadFromConfig(HeeksConfig& config);
};