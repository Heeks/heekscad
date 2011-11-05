// Ruler.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"
class HeeksConfig;

struct RulerMark{
	int pos; // distance from the 0 mark
	double length;
	double width;

	void glCommands(double units); // including text
};

class HRuler: public HeeksObj{
	//      ___________________________________________________________________  
	//     |   |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||   | 
	//     |   |    |    |    |    |    |    |    |    |    |    |    |    |   | 
	//     |   |cm      1|        2|        3|        4|         |         |   | 
	//     |                                                                   | 
	//     |                                                                   | 
	//     ¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬¬  
	//

	int m_gl_list;

	void GetFourCorners(gp_Pnt *point);
	void CalculateMarks(std::list<RulerMark> &marks);
	double GetUnits();

public:
	gp_Trsf m_trsf; // position and orientation ( no scaling allowed )
	bool m_use_view_units;
	double m_units; // used, if m_use_view_units == false; 1.0 for mm, 25.4 for inches
	double m_width; // in mm
	double m_length;// in mm
	double m_empty_length; // space at each end in mm

	HRuler();

	// HeeksObj's virtual functions
	int GetType()const{return RulerType;}
	long GetMarkingMask()const{return MARKING_FILTER_RULER;}
	void glCommands(bool select, bool marked, bool no_color);
	void KillGLLists(void);
	void GetBox(CBox &box);
	const wxChar* GetTypeString(void)const{return _("Ruler");}
	HeeksObj *MakeACopy(void)const;
	void ModifyByMatrix(const double *mat);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	bool GetScaleAboutMatrix(double *m);
	bool CanBeRemoved(){return false;}
	bool CanBeCopied(){return false;}

	void WriteToConfig(HeeksConfig& config);
	void ReadFromConfig(HeeksConfig& config);
};
