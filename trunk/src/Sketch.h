// Sketch.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/ObjList.h"
#include "../interface/HeeksColor.h"
#include "../interface/SketchOrder.h"

class CoordinateSystem;

class CSketch:public ObjList
{
	HeeksColor color;
	static wxIcon* m_icon;
	wxString m_title;

	int GetClosedSketchTurningNumber(); //only use this for closed sketches
	double GetAngleAtJunction(HeeksObj* prev_object, HeeksObj* object);
	double GetAngleBetweenVectors(const gp_Vec& v0, const gp_Vec& v1, double prev_segment_curvature, double segment_curvature);
	int GetSegmentType(HeeksObj* object);
	double GetSegmentCurvature(HeeksObj* object);

public:
	static std::string m_sketch_order_str[MaxSketchOrderTypes];
	SketchOrderType m_order;
	bool m_solidify;
	bool m_draw_with_transform;
	CoordinateSystem* m_coordinate_system;


	CSketch();
	CSketch(const CSketch& c);
	virtual ~CSketch();

	const CSketch& operator=(const CSketch& c);

	std::vector<TopoDS_Face> GetFaces();

	int GetType()const{return SketchType;}
	long GetMarkingMask()const{return MARKING_FILTER_SKETCH;}
	const wxChar* GetTypeString(void)const{return _("Sketch");}
	void GetIcon(int& texture_number, int& x, int& y){GET_ICON(8, 1);}
	void GetProperties(std::list<Property *> *list);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	HeeksObj *MakeACopy(void)const;
	void CopyFrom(const HeeksObj* object){operator=(*((CSketch*)((ObjList*)object)));}
	void WriteXML(TiXmlNode *root);
	bool UsesID(){return true;}
	void SetColor(const HeeksColor &col);
	const HeeksColor* GetColor()const;
	const wxChar* GetShortString(void)const{return m_title.c_str();}
	bool CanEditString(void)const{return true;}
	void OnEditString(const wxChar* str);
	bool Add(HeeksObj* object, HeeksObj* prev_object);
	void Remove(HeeksObj* object);
	void glCommands(bool select, bool marked, bool no_color);
	void ReloadPointers();
	void ModifyByMatrix(const double *m);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);

	void CalculateSketchOrder();
	SketchOrderType GetSketchOrder();
	bool ReOrderSketch(SketchOrderType new_order); // returns true if done
	void ReLinkSketch();
	void ReverseSketch();
	void ExtractSeparateSketches(std::list<HeeksObj*> &new_separate_sketches);

	int Intersects(const HeeksObj *object, std::list< double > *rl) const;
	HeeksObj *Parallel( const double distance );
};

class CSketchRelinker{
	const std::list<HeeksObj*> &m_old_list;
	std::set<HeeksObj*> m_added_from_old_set;
	std::list<HeeksObj*>::const_iterator m_old_front;
	HeeksObj* m_new_front;
	bool AddNext();
	bool TryAdd(HeeksObj* object);

public:
	std::list< std::list<HeeksObj*> > m_new_lists;

	CSketchRelinker(const std::list<HeeksObj*>& old_list):m_old_list(old_list), m_new_front(NULL){}

	bool Do(); // makes m_new_lists
};
