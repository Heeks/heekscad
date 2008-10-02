// HeeksObj.h

#pragma once

#include "Box.h"

class HeeksColor;
class Property;
class Tool;
class MarkedObject;
class TiXmlElement;

enum{
	UnknownType,
	LineType,
	ArcType,
	ILineType,
	CircleType,
	PointType,
	GripperType,
	SolidType,
	StlSolidType,
	WireType,
	FaceType,
	EdgeType,
	TrianglesType,
	TriType,
	SubstituteTreeType,
	LineArcCollectionType,
	ImageType,
	ObjectMaximumType
};

class HeeksObj{
public:
	unsigned int m_id;
	unsigned int m_layer;
	bool m_visible; // only temporary, ( use layer to be more permanent )
	HeeksObj *m_owner;

	HeeksObj(void);
	HeeksObj(const HeeksObj& ho);
	virtual ~HeeksObj() {if (m_owner) m_owner->Remove(this);}

	virtual const HeeksObj& operator=(const HeeksObj &ho);

	// virtual functions
	virtual int GetType()const{return UnknownType;}
	virtual int GetIDGroupType()const{return GetType();}
	virtual void glCommands(bool select, bool marked, bool no_color){};
	virtual void GetBox(CBox &box){}
	virtual const wxChar* GetShortString(void)const{return NULL;}
	virtual const wxChar* GetTypeString(void)const{return _T("Unknown");}
	const wxChar* GetShortStringOrTypeString(void)const{if(GetShortString())return GetShortString();return GetTypeString();}
	virtual bool CanEditString(void)const{return false;}
	virtual void OnEditString(const wxChar* str){}
	virtual void KillGLLists(void){};
	virtual HeeksObj *MakeACopy()const = 0;
	virtual void CopyFrom(const HeeksObj* object){}
	virtual void SetColor(const HeeksColor &col){}
	virtual const HeeksColor* GetColor()const{return NULL;}
	virtual void ModifyByMatrix(const double *m){}
	virtual bool GetStartPoint(double* pos){return false;}
	virtual bool GetEndPoint(double* pos){return false;}
	virtual bool GetCentrePoint(double* pos){return false;}
	virtual bool GetMidPoint(double* pos){return false;}
	virtual void GetProperties(std::list<Property *> *list);
	virtual wxIcon* GetIcon(){return NULL;}
	virtual int Intersects(const HeeksObj *object, std::list< double > *rl)const{return 0;}
	virtual bool FindNearPoint(const double* ray_start, const double* ray_direction, double *point){return false;}
	virtual bool FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point){return false;}
	virtual void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	virtual void GetTools(std::list<Tool*>* t_list, const wxPoint* p){}
	virtual void Stretch(const double *p, const double* shift, double* new_position){}
	virtual void SetClickMarkPoint(MarkedObject* marked_object, const double* ray_start, const double* ray_direction){}
	virtual bool CanAdd(HeeksObj* object){return false;}
	virtual bool Add(HeeksObj* object, HeeksObj* prev_object) {object->m_owner=this; object->OnAdd(); return true;}
	virtual void Remove(HeeksObj* object){object->OnRemove();}
	virtual void OnAdd(){}
	virtual void OnRemove();
	virtual HeeksObj* GetFirstChild(){return NULL;}
	virtual HeeksObj* GetNextChild(){return NULL;}
	virtual HeeksObj* GetAtIndex(int index){return NULL;}
	virtual int GetNumChildren(){return 0;}
	virtual HeeksObj* GetFirstAutoExpandChild(){return NULL;}
	virtual HeeksObj* GetNextAutoExpandChild(){return NULL;}
	virtual void GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal = true){} // [nine doubles, three doubles],  or [nine doubles, nine doubles] if just_one_average_normal = false
	virtual double Area()const{return 0.0;}
	virtual void GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point = true)const{};
	virtual void WriteXML(TiXmlElement *root){}
	virtual void WriteBaseXML(TiXmlElement *element);
	virtual void ReadBaseXML(TiXmlElement* element);
	void SetID(int id);
	bool OnVisibleLayer();
};
