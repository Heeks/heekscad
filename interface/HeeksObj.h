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
	SketchType,
	ImageType,
	CoordinateSystemType,
	TextType,
	DimensionType,
	RulerType,
	ObjectMaximumType
};

#define MARKING_FILTER_LINE					0x00000001
#define MARKING_FILTER_ARC					0x00000002
#define MARKING_FILTER_ILINE				0x00000004
#define MARKING_FILTER_CIRCLE				0x00000008
#define MARKING_FILTER_POINT				0x00000010
#define MARKING_FILTER_SOLID				0x00000020
#define MARKING_FILTER_STL_SOLID			0x00000040
#define MARKING_FILTER_WIRE					0x00000080
#define MARKING_FILTER_FACE					0x00000100
#define MARKING_FILTER_EDGE					0x00000200
#define MARKING_FILTER_SKETCH				0x00000400
#define MARKING_FILTER_IMAGE				0x00000800
#define MARKING_FILTER_COORDINATE_SYSTEM	0x00000800
#define MARKING_FILTER_TEXT					0x00001000
#define MARKING_FILTER_DIMENSION			0x00002000
#define MARKING_FILTER_RULER				0x00004000

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
	virtual long GetMarkingMask()const{return 0;}
	virtual int GetIDGroupType()const{return GetType();}
	virtual void glCommands(bool select, bool marked, bool no_color){};
	virtual void Draw(wxDC& dc){} // for printing
	virtual bool DrawAfterOthers(){return false;}
	virtual void GetBox(CBox &box){}
	virtual const wxChar* GetShortString(void)const{return NULL;}
	virtual const wxChar* GetTypeString(void)const{return _("Unknown");}
	const wxChar* GetShortStringOrTypeString(void)const{if(GetShortString())return GetShortString();return GetTypeString();}
	virtual bool CanEditString(void)const{return false;}
	virtual void OnEditString(const wxChar* str){}
	virtual void KillGLLists(void){};
	virtual HeeksObj *MakeACopy()const = 0;
	virtual void CopyFrom(const HeeksObj* object){}
	virtual void SetColor(const HeeksColor &col){}
	virtual const HeeksColor* GetColor()const{return NULL;}
	virtual bool ModifyByMatrix(const double *m){return false;} // transform the object, returns true if objects created new to move them
	virtual bool GetStartPoint(double* pos){return false;}
	virtual bool GetEndPoint(double* pos){return false;}
	virtual bool GetCentrePoint(double* pos){return false;}
	virtual bool GetMidPoint(double* pos){return false;}
	virtual bool GetScaleAboutMatrix(double *m);
	virtual void GetProperties(std::list<Property *> *list);
	virtual void OnApplyProperties(){}
	virtual wxString GetIcon(){return _T("");}
	virtual int Intersects(const HeeksObj *object, std::list< double > *rl)const{return 0;}
	virtual bool FindNearPoint(const double* ray_start, const double* ray_direction, double *point){return false;}
	virtual bool FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point){return false;}
	virtual void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	virtual void GetTools(std::list<Tool*>* t_list, const wxPoint* p){}
	virtual bool Stretch(const double *p, const double* shift){return false;} // return true, if undo stretch is done with Add and Delete
	virtual bool StretchTemporary(const double *p, const double* shift){return Stretch(p, shift);}
	virtual void SetClickMarkPoint(MarkedObject* marked_object, const double* ray_start, const double* ray_direction){}
	virtual bool CanAdd(HeeksObj* object){return false;}
	virtual bool Add(HeeksObj* object, HeeksObj* prev_object) {object->m_owner=this; object->OnAdd(); return true;}
	virtual void Remove(HeeksObj* object){object->OnRemove();}
	virtual void OnAdd(){}
	virtual void OnRemove();
	virtual bool CanBeRemoved(){return true;}
	virtual bool CanBeCopied(){return true;}
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
