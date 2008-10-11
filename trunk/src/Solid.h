// Solid.h

#pragma once

#include "Shape.h"
#include <TopoDS_Solid.hxx>

enum SolidTypeEnum{
	SOLID_TYPE_UNKNOWN, // probably not a primitive solid anymore
	SOLID_TYPE_SPHERE,
	SOLID_TYPE_CYLINDER,
	SOLID_TYPE_CUBOID,
	SOLID_TYPE_CONE
};

class CSolid:public CShape{
private:
	static wxIcon* m_icon;

public:
	SolidTypeEnum m_type; // so the solid can be stretched in specific ways, if it's still a primitive solid

	CSolid(const TopoDS_Solid &solid, const wxChar* title, bool use_one_gl_list = false);
	~CSolid();

	virtual const CSolid& operator=(const CSolid& s){ return *this;}

	int GetType()const{return SolidType;}
	long GetMarkingMask()const{return MARKING_FILTER_SOLID;}
	const wxChar* GetTypeString(void)const{return _T("Solid");}
	wxIcon* GetIcon();
	HeeksObj *MakeACopy(void)const;

	virtual SolidTypeEnum GetSolidType(){return SOLID_TYPE_UNKNOWN;}
};
