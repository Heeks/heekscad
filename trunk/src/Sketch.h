// Sketch.h

#pragma once

#include "../interface/ObjList.h"

class CSketch:public ObjList
{
	static wxIcon* m_icon;

public:
	CSketch();
	CSketch(const CSketch& c);
	virtual ~CSketch();

	const CSketch& operator=(const CSketch& c);

	int GetType()const{return SketchType;}
	long GetMarkingMask()const{return MARKING_FILTER_SKETCH;}
	const wxChar* GetTypeString(void)const{return _("Sketch");}
	wxString GetIcon(){return _T("linedrawing");}
	void GetProperties(std::list<Property *> *list);
	HeeksObj *MakeACopy(void)const;
	void CopyFrom(const HeeksObj* object){operator=(*((CSketch*)((ObjList*)object)));}
	void WriteXML(TiXmlNode *root);
	bool UsesID(){return true;}

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
};

