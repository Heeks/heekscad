// LineArcCollection.h

#pragma once

#include "../interface/ObjList.h"

class CLineArcCollection:public ObjList
{
	static wxIcon* m_icon;
	void set_initial(); // only call from constructor

public:
	static std::map<int, CLineArcCollection*> used_ids;
	static int next_id;
	int m_id;

	CLineArcCollection();
	CLineArcCollection(const CLineArcCollection& c);
	virtual ~CLineArcCollection();

	const CLineArcCollection& operator=(const CLineArcCollection& c);

	int GetType()const{return LineArcCollectionType;}
	const char* GetTypeString(void)const{return "Line Drawing";}
	wxIcon* GetIcon();
	void GetProperties(std::list<Property *> *list);
	HeeksObj *MakeACopy(void)const;
	void CopyFrom(const HeeksObj* object){operator=(*((CLineArcCollection*)object));}
	void WriteXML(TiXmlElement *root);
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
	static HeeksObj* GetLineArcCollection(int id);
	static void SetID(CLineArcCollection* la, int id);
};

