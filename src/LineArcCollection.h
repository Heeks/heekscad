// LineArcCollection.h

#pragma once

#include "../interface/ObjList.h"

class CLineArcCollection:public ObjList
{
	static wxIcon* m_icon;

public:
	static std::map<int, CLineArcCollection*> used_ids;
	static int next_id;
	int m_id;

	CLineArcCollection();
	virtual ~CLineArcCollection();

	int GetType()const{return LineArcCollectionType;}
	const char* GetTypeString(void)const{return "Line Drawing";}
	wxIcon* GetIcon();
	void GetProperties(std::list<Property *> *list);

	static HeeksObj* GetLineArcCollection(int id);
};

