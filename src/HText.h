// HText.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"
#include "../interface/HeeksColor.h"

class HText: public HeeksObj{
private:
	HeeksColor m_color;
	static wxIcon* m_icon;

public:
	gp_Trsf m_trsf; // matrix defining position, orientation, scale, compared with default text size
	wxString m_text;

	HText(const gp_Trsf &trsf, const wxString &text, const HeeksColor* col);
	HText(const HText &b);
	~HText(void);

	const HText& operator=(const HText &b);

	// HeeksObj's virtual functions
	int GetType()const{return TextType;}
	long GetMarkingMask()const{return MARKING_FILTER_TEXT;}
	void glCommands(bool select, bool marked, bool no_color);
	bool DrawAfterOthers(){return true;}
	void GetBox(CBox &box);
	const wxChar* GetTypeString(void)const{return _("Text");}
	HeeksObj *MakeACopy(void)const;
	wxString GetIcon(){return wxGetApp().GetResFolder() + _T("/icons/text");}
	bool ModifyByMatrix(const double *mat);
	void SetColor(const HeeksColor &col){m_color = col;}
	const HeeksColor* GetColor()const{return &m_color;}
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool Stretch(const double *p, const double* shift);
	void CopyFrom(const HeeksObj* object){operator=(*((HText*)object));}
	void WriteXML(TiXmlNode *root);
	const wxChar* GetShortString(void)const{return m_text.c_str();}
	bool CanEditString(void)const{return true;}
	void OnEditString(const wxChar* str);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
};
