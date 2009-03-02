// HeeksCADInterface.h
/*
 * Copyright (c) 2009, Dan Heeks
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */

// include this in your dynamic library to interface with HeeksCAD

#pragma once

class HeeksObj;
class wxFrame;
class wxAuiManager;
class Observer;
class wxPoint;
class CInputMode;
class TiXmlElement;

#include "SketchOrder.h"

class CHeeksCADInterface{
public:
	CHeeksCADInterface(){}
	~CHeeksCADInterface(){}

	virtual double GetTolerance();
	virtual void RefreshProperties();
	virtual void RefreshOptions();
	virtual void RefreshInput();
	virtual void Repaint(bool soon = false);
	virtual bool GetCamera(double* pos, double* target, double* up, bool& perspective, double& field_of_view, double& near_plane, double& far_plane);
	virtual wxFrame* GetMainFrame();
	virtual wxWindow* GetGraphicsCanvas();
#ifdef WIN32
	virtual HGLRC GetRC();
#endif
	virtual wxMenuBar* GetMenuBar();
	virtual wxMenu* GetWindowMenu();
	virtual wxAuiManager* GetAuiManager();
	virtual void AddToolBarButton(wxToolBar* toolbar, const wxString& title, const wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&) = NULL);
	virtual float GetToolImageButtonScale();
	virtual int GetToolImageBitmapSize();
	virtual int AddMenuCheckItem(wxMenu* menu, const wxString& title, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&) = NULL);
	virtual int AddMenuItem(wxMenu* menu, const wxString& title, void(*onButtonFunction)(wxCommandEvent&));
	virtual wxString GetExeFolder();
	virtual void AddUndoably(HeeksObj* object, HeeksObj* owner);
	virtual HeeksObj* GetMainObject();
	virtual void DeleteUndoably(HeeksObj* object);
	virtual const std::list<HeeksObj*>& GetMarkedList();
	virtual bool GetArcCentre(HeeksObj* object, double* c);
	virtual bool GetArcAxis(HeeksObj* object, double* a);
	virtual void get_2d_arc_segments(double xs, double ys, double xe, double ye, double xc, double yc, bool dir, bool want_start, double pixels_per_mm, void(*callbackfunc)(const double* xy));
	virtual bool GetSegmentVector(HeeksObj* object, double fraction, double* v);
	virtual double GetPixelScale();
	virtual void Mark(HeeksObj* object);
	virtual bool ObjectMarked(HeeksObj* object);
	virtual void SetMarkingFilter(long filter);
	virtual long GetMarkingFilter();
	virtual void ClearMarkedList();
	virtual CInputMode* GetSelectMode();
	virtual void SetInputMode(CInputMode* input_mode);
	virtual void WasModified(HeeksObj* object);
	virtual void WasAdded(HeeksObj* object);
	virtual void WasRemoved(HeeksObj* object);
	virtual void WereAdded(const std::list<HeeksObj*> &list);
	virtual void WereRemoved(const std::list<HeeksObj*> &list);
	virtual int PickObjects(const wxChar* str, long marking_filter = -1, bool m_just_one = false);
	virtual bool PickPosition(const wxChar* str, double* pos);
	virtual bool Digitize(const wxPoint &point, double* pos);
	virtual HeeksObj* GetFirstObject();
	virtual HeeksObj* GetNextObject();
	virtual void DrawObjectsOnFront(const std::list<HeeksObj*> &list);
	virtual HeeksObj* NewSketch();
	virtual HeeksObj* NewLine(const double* s, const double* e);
	virtual HeeksObj* NewArc(const double* s, const double* e, const double* c, const double* up); // set up to (0, 0, -1) for a clockwise arc
	virtual void RegisterObserver(Observer* observer);
	virtual void RemoveObserver(Observer* observer);
	virtual bool TangentialArc(const double* p0, const double* v0, const double* p1, double *c, double *a); // given p0, v0, p1, returns true if an arc found and sets c and a ( centre and axis direction ), false for a straight line
	virtual void RegisterHideableWindow(wxWindow* w);
	virtual HeeksObj* ReadXMLElement(TiXmlElement* pElem);
	virtual void RegisterReadXMLfunction(const char* type_name, HeeksObj*(*read_xml_function)(TiXmlElement* pElem));
	virtual void OpenXMLFile(const wxChar *filepath, bool undoably = false, HeeksObj* paste_into = NULL);
	virtual void ObjectWriteBaseXML(HeeksObj* object, TiXmlElement* element);
	virtual void ObjectReadBaseXML(HeeksObj* object, TiXmlElement* element);
	virtual HeeksObj* GetIDObject(int type, int id);
	virtual void SetObjectID(HeeksObj* object, int id); // check for existing id using GetIDObject and call DeleteUndoably first
	virtual int GetNextID(int type);
	virtual bool InOpenFile();
	virtual void RemoveID(HeeksObj* object); // only call this from ObjList::Remove()
	virtual const wxChar* GetFileFullPath(); // returns NULL for untitled
	virtual void SetViewBox(const double* b);
	virtual void SaveSTLFile(const std::list<HeeksObj*>& objects, const wxChar *filepath);

	// sketches
	virtual SketchOrderType GetSketchOrder(HeeksObj* sketch);
	virtual bool ReOrderSketch(HeeksObj* sketch, SketchOrderType new_order); // returns true if done

	// body functions
	virtual long BodyGetNumFaces(HeeksObj* body);
	virtual HeeksObj* BodyGetFirstFace(HeeksObj* body);
	virtual HeeksObj* BodyGetNextFace(HeeksObj* body);
	virtual HeeksObj* BodyGetPickedFace(HeeksObj* body);
	virtual long BodyGetNumEdges(HeeksObj* body);
	virtual HeeksObj* BodyGetFirstEdge(HeeksObj* body);
	virtual HeeksObj* BodyGetNextEdge(HeeksObj* body);

	// face functions
	virtual void FaceSetTempAttribute(HeeksObj* face, int attr);
	virtual int FaceGetTempAttribute(HeeksObj* face);
	virtual int FaceGetSurfaceType(HeeksObj* face);
	virtual void FaceGetUVBox(HeeksObj* face, double *uv_box);// 4 doubles
	virtual void FaceGetPointAndNormalAtUV(HeeksObj* face, double u, double v, double* p, double* norm);
	virtual bool FaceGetUVAtPoint(HeeksObj* face, const double *pos, double *u, double *v);
	virtual bool FaceGetClosestPoint(HeeksObj* face, const double *pos, double *closest_pnt);
	virtual void FaceGetPlaneParams(HeeksObj* face, double *d, double *norm);
	virtual void FaceGetCylinderParams(HeeksObj* face, double *pos, double *dir, double *radius);
	virtual void FaceGetSphereParams(HeeksObj* face, double *pos, double *radius);
	virtual void FaceGetConeParams(HeeksObj* face, double *pos, double *dir, double *radius, double* half_angle);
	virtual int FaceGetEdgeCount(HeeksObj* face);
	virtual HeeksObj* FaceGetFirstEdge(HeeksObj* face);
	virtual HeeksObj* FaceGetNextEdge(HeeksObj* face);
	virtual HeeksObj* FaceGetFirstLoop(HeeksObj* face);
	virtual HeeksObj* FaceGetNextLoop(HeeksObj* face);
	virtual bool FaceOrientation(HeeksObj* face);

	// edge functions
	virtual int EdgeGetCurveType(HeeksObj* edge);
	virtual int EdgeGetFaceCount(HeeksObj* edge);
	virtual HeeksObj* EdgeGetFirstFace(HeeksObj* edge);
	virtual HeeksObj* EdgeGetNextFace(HeeksObj* edge);
	virtual void EdgeGetCurveParams(HeeksObj* edge, double* start, double* end, double* uStart, double* uEnd, int* Reversed);
	virtual void EdgeGetCurveParams2(HeeksObj* edge, double *uStart, double *uEnd, int *isClosed, int *isPeriodic);
	virtual bool EdgeInFaceSense(HeeksObj* edge, HeeksObj* face);
	virtual void EdgeEvaluate(HeeksObj* edge, double u, double *p, double *tangent);
	virtual bool EdgeGetLineParams(HeeksObj* edge, double* d6);
	virtual bool EdgeGetCircleParams(HeeksObj* edge, double* d6);

	// loop functions
	virtual long LoopGetEdgeCount(HeeksObj* loop);
	virtual HeeksObj* LoopGetFirstEdge(HeeksObj* loop);
	virtual HeeksObj* LoopGetNextEdge(HeeksObj* loop);
	virtual HeeksObj* LoopGetEdge(HeeksObj* loop, int index);
	virtual bool LoopIsOuter(HeeksObj* loop);

	virtual const wxChar* GetRevisionNumber();
	virtual void RegisterOnGLCommands( void(*callbackfunc)() );
	virtual void RemoveOnGLCommands( void(*callbackfunc)() );
	virtual void RegisterOnGraphicsSize( void(*callbackfunc)(wxSizeEvent& evt) );
	virtual void RemoveOnGraphicsSize( void(*callbackfunc)(wxSizeEvent& evt) );
	virtual void RegisterOnMouseFn( void(*callbackfunc)(wxMouseEvent&) );
	virtual void RemoveOnMouseFn( void(*callbackfunc)(wxMouseEvent&) );
	virtual void RegisterToolBar( wxToolBarBase* );
	virtual void RegisterAddToolBars( void(*callbackfunc)() );
	virtual void PropertiesOnApply2();// don't need to press tick to make changes
	virtual void PropertiesApplyChanges();// don't need to press tick to make changes
};
