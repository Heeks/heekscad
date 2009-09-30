// DynamicSolid.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/ObjList.h"

class DynamicSolid:public ObjList{
protected:
	static wxIcon* m_icon;
	std::list<TopoDS_Shape> m_shapes;

public:
	CFaceList* m_faces;
	CEdgeList* m_edges;
	CVertexList* m_vertices;

	DynamicSolid();
	~DynamicSolid();

	//virtual const DynamicSolid& operator=(const DynamicSolid& s);

	int GetType()const{return SolidType;}
	long GetMarkingMask()const{return MARKING_FILTER_SOLID;}
	const wxChar* GetTypeString(void)const{return _("Solid");}
	wxString GetIcon(){return wxGetApp().GetResFolder() + _T("/icons/solid");}
	//HeeksObj *MakeACopy(void)const;
	void ReloadPointers(); 

	virtual SolidTypeEnum GetSolidType(){return SOLID_TYPE_UNKNOWN;}

	void SetShapes(std::list<TopoDS_Shape>);
	void DrawShapes();
};
