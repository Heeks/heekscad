// Sketch.h

#pragma once

#include "../interface/ObjList.h"
#include "../interface/HeeksColor.h"
#include "../interface/SketchOrder.h"

class CSketch:public ObjList
{
	HeeksColor color;
	static wxIcon* m_icon;

	int GetClosedSketchTurningNumber(); //only use this for closed sketches
	double GetAngleAtJunction(HeeksObj* prev_object, HeeksObj* object);
	double GetAngleBetweenVectors(const gp_Vec& v0, const gp_Vec& v1, double prev_segment_curvature, double segment_curvature);
	int GetSegmentType(HeeksObj* object);
	double GetSegmentCurvature(HeeksObj* object);

public:
	static std::string m_sketch_order_str[MaxSketchOrderTypes];

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
	void SetColor(const HeeksColor &col);
	const HeeksColor* GetColor()const;

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);

	SketchOrderType GetSketchOrder();
	bool ReOrderSketch(SketchOrderType new_order); // returns true if done
	void ReLinkSketch();
	void ReverseSketch();
};

class CSketchRelinker{
	const std::list<HeeksObj*> &m_old_list;
	std::set<HeeksObj*> m_added_from_old_set;
	std::list<HeeksObj*>::const_iterator m_old_front;
	std::list<HeeksObj*>::const_iterator m_old_back;
	HeeksObj* m_new_front;
	HeeksObj* m_new_back;
	bool AddNext();
	bool TryAdd(HeeksObj* object, bool front_not_back);

public:
	std::list<HeeksObj*> m_new_list;

	CSketchRelinker(const std::list<HeeksObj*>& old_list):m_old_list(old_list), m_new_front(NULL), m_new_back(NULL){}

	bool Do(); // makes m_new_list, returns false if there were some left over
};
