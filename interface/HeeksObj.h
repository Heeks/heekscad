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
	HeeksObj *m_owner;

	HeeksObj(void): m_owner(NULL){}
	HeeksObj(const HeeksObj& ho): m_owner(NULL) {operator=(ho);}
	virtual ~HeeksObj() {if (m_owner) m_owner->Remove(this);}

	virtual const HeeksObj& operator=(const HeeksObj &ho) {return *this;}

	// virtual functions
	virtual int GetType()const{return UnknownType;}
	virtual void glCommands(bool select, bool marked, bool no_color){};
	virtual void GetBox(CBox &box){}
	virtual const char* GetShortString(void)const{return NULL;}
	virtual const char* GetTypeString(void)const{return "Unknown";}
	const char* GetShortStringOrTypeString(void)const{if(GetShortString())return GetShortString();return GetTypeString();}
	virtual bool CanEditString(void)const{return false;}
	virtual void OnEditString(const char* str){}
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
	virtual void GetGripperPositions(std::list<double> *list, bool just_for_endof){}
	virtual void GetTools(std::list<Tool*>* t_list, const wxPoint* p){}
	virtual void Stretch(const double *p, const double* shift, double* new_position){}
	virtual void SetClickMarkPoint(MarkedObject* marked_object, const double* ray_start, const double* ray_direction){}
	virtual bool CanAdd(HeeksObj* object){return false;}
	virtual bool Add(HeeksObj* object, HeeksObj* prev_object) {object->m_owner=this; object->OnAdd(); return true;}
	virtual void Remove(HeeksObj* object){object->OnRemove();}
	virtual void OnAdd(){}
	virtual void OnRemove(){}
	virtual HeeksObj* GetFirstChild(){return NULL;}
	virtual HeeksObj* GetNextChild(){return NULL;}
	virtual HeeksObj* GetAtIndex(int index){return NULL;}
	virtual int GetNumChildren(){return 0;}
	virtual HeeksObj* GetFirstAutoExpandChild(){return NULL;}
	virtual HeeksObj* GetNextAutoExpandChild(){return NULL;}
	void    SetCurrentChild(HeeksObj* child) {}
	virtual void GetTriangles(void(*callbackfunc)(double* x, double* n), double cusp){} // nine doubles, nine doubles
	virtual void GetCentreNormals(void(*callbackfunc)(double area, double *x, double *n)){} // three doubles, three doubles
	virtual void GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point = true)const{};
	virtual void WriteXML(TiXmlElement *root){}
};
