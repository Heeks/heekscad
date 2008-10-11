// LineArcCollection.h

#pragma once

#include "../interface/ObjList.h"

class CLineArcCollection:public ObjList
{
	static wxIcon* m_icon;

public:
	CLineArcCollection();
	CLineArcCollection(const CLineArcCollection& c);
	virtual ~CLineArcCollection();

	const CLineArcCollection& operator=(const CLineArcCollection& c);

	int GetType()const{return LineArcCollectionType;}
	long GetMarkingMask()const{return MARKING_FILTER_LINE_ARC_COLLECTION;}
	const wxChar* GetTypeString(void)const{return _T("Line Drawing");}
	wxIcon* GetIcon();
	void GetProperties(std::list<Property *> *list);
	HeeksObj *MakeACopy(void)const;
	void CopyFrom(const HeeksObj* object){operator=(*((CLineArcCollection*)((ObjList*)object)));}
	void WriteXML(TiXmlElement *root);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
};

