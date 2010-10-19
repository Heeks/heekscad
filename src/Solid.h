// Solid.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Shape.h"

class CSolid:public CShape{
protected:
	// CShape's virtual functions
	void MakeTransformedShape(const gp_Trsf &mat);

public:
	SolidTypeEnum m_type; // so the solid can be stretched in specific ways, if it's still a primitive solid

	CSolid(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col, float opacity);
	CSolid( const CSolid & rhs );

	~CSolid();

	virtual const CSolid& operator=(const CSolid& s){ CShape::operator =(s); return *this;}

	int GetType()const{return SolidType;}
	long GetMarkingMask()const{return MARKING_FILTER_SOLID;}
	const wxChar* GetTypeString(void)const{return _("Solid");}
	const wxBitmap &GetIcon();
	HeeksObj *MakeACopy(void)const;
	void SetColor(const HeeksColor &col){m_color = col;}
	const HeeksColor* GetColor()const{return &m_color;}
	void OnApplyProperties();

	// CShape's virtual functions
	void SetXMLElement(TiXmlElement* element);
	void SetFromXMLElement(TiXmlElement* pElem);

	virtual SolidTypeEnum GetSolidType(){return SOLID_TYPE_UNKNOWN;}
};
