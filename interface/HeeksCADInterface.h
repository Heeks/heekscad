// HeeksCADInterface.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

// include this in your dynamic library to interface with HeeksCAD

#pragma once

class HeeksObj;
class wxFrame;
class wxAuiManager;
class Observer;
class wxPoint;
class CInputMode;
class TiXmlElement;
class HeeksColor;
class wxMenuBar;
class wxMenu;
class wxToolBar;
class wxString;
class wxCommandEvent;
class wxUpdateUIEvent;
class wxSizeEvent;
class wxMouseEvent;
class wxToolBarBase;
class CNurbSurfaceParams;
class TransientObject;

#include "SketchOrder.h"

class TopoDS_Shape;
class TopoDS_Edge;
class TopoDS_Wire;
class TopoDS_Solid;
class gp_Lin;
class gp_Pnt;
class gp_Circ;

#include <list>

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
	virtual void StartToolBarFlyout(const wxString& title_and_bitmap);
	virtual void AddFlyoutButton(const wxString& title, const wxBitmap& bitmap, const wxString& tooltip, void(*onButtonFunction)(wxCommandEvent&));
	virtual void EndToolBarFlyout(wxToolBar* toolbar);
	virtual float GetToolImageButtonScale();
	virtual int GetToolImageBitmapSize();
	virtual int AddMenuItem(wxMenu* menu, const wxString& title, const wxBitmap& bitmap, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&) = NULL, wxMenu* submenu = NULL, bool check_item = false);
	virtual wxString GetExeFolder();
	virtual wxString GetResFolder();
	virtual HeeksObj* GetMainObject();
	virtual const std::list<HeeksObj*>& GetMarkedList();
	virtual unsigned long GetMarkedListSize();
	virtual HeeksObj* GetFirstMarkedObject();
	virtual HeeksObj* GetNextMarkedObject();
	virtual bool GetArcCentre(HeeksObj* object, double* c);
	virtual bool GetArcAxis(HeeksObj* object, double* a);
	virtual double CircleGetRadius(HeeksObj* object);
	virtual void get_2d_arc_segments(double xs, double ys, double xe, double ye, double xc, double yc, bool dir, bool want_start, double pixels_per_mm, void(*callbackfunc)(const double* xy));
	virtual bool GetSegmentVector(HeeksObj* object, double fraction, double* v);
	virtual double GetPixelScale();
	virtual void Mark(HeeksObj* object);
	virtual void Unmark(HeeksObj* object);
	virtual bool ObjectMarked(HeeksObj* object);
	virtual void SetMarkingFilter(long filter);
	virtual long GetMarkingFilter();
	virtual void ClearMarkedList();
	virtual CInputMode* GetSelectMode();
	virtual void SetInputMode(CInputMode* input_mode);
	virtual int PickObjects(const wxChar* str, long marking_filter = -1, bool m_just_one = false);
	virtual bool PickPosition(const wxChar* str, double* pos);
	virtual bool Digitize(const wxPoint &point, double* pos);
	virtual bool GetLastClickPosition(double *pos);
	virtual HeeksObj* GetFirstObject();
	virtual HeeksObj* GetNextObject();
	virtual void DrawObjectsOnFront(const std::list<HeeksObj*> &list);
	virtual HeeksObj* NewSketch();
	virtual HeeksObj* NewLine(const double* s, const double* e);
	virtual HeeksObj* NewILine(const double* s, const double* e);
	virtual HeeksObj* NewPoint(const double* p);
	virtual HeeksObj* NewArc(const double* s, const double* e, const double* c, const double* up); // set up to (0, 0, -1) for a clockwise arc
	virtual HeeksObj* NewArc(const double* c, const double* u, double r, double s, double e); // set up to (0, 0, -1) for a clockwise arc
	virtual HeeksObj* NewCircle(const double *c, double r);
	virtual HeeksObj* NewCuboid(const double *c, double x, double y, double z);
	virtual HeeksObj* NewCylinder(const double*c, double r, double h);
	virtual HeeksObj* NewCone(const double*c, double r1, double r2, double h);
	virtual HeeksObj* NewSphere(const double*pos, double radius);
	virtual HeeksObj* NewGroup();
	virtual HeeksObj* NewSolid(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col);
	virtual HeeksObj* Fuse(std::list<HeeksObj*> objects);
	virtual HeeksObj* Cut(std::list<HeeksObj*> objects);
	virtual HeeksObj* Common(std::list<HeeksObj*> objects);
	virtual void RotateObject(HeeksObj*, const double*p,const double*u,double r);
	virtual void TranslateObject(HeeksObj*,const double* c);
	virtual void RegisterObserver(Observer* observer);
	virtual void RemoveObserver(Observer* observer);
	virtual bool TangentialArc(const double* p0, const double* v0, const double* p1, double *c, double *a); // given p0, v0, p1, returns true if an arc found and sets c and a ( centre and axis direction ), false for a straight line
	virtual void RegisterHideableWindow(wxWindow* w);
	virtual HeeksObj* ReadXMLElement(TiXmlElement* pElem);
	virtual void RegisterReadXMLfunction(const char* type_name, HeeksObj*(*read_xml_function)(TiXmlElement* pElem));
	virtual void OpenXMLFile(const wxChar *filepath, HeeksObj* paste_into = NULL);
	virtual void ObjectWriteBaseXML(HeeksObj* object, TiXmlElement* element);
	virtual void ObjectReadBaseXML(HeeksObj* object, TiXmlElement* element);
	virtual std::list<HeeksObj*> GetIDObjects(int type, int id);
	virtual HeeksObj* GetIDObject(int type, int id);
	virtual void SetObjectID(HeeksObj* object, int id); // check for existing id using GetIDObject and call DeleteUndoably first
	virtual void SaveXMLFile(const std::list<HeeksObj*>& objects, const wxChar *filepath, bool for_clipboard);
	virtual void Changed();
	virtual void Remove(HeeksObj* obj);
	virtual void Add(HeeksObj* object, HeeksObj* other);
	virtual void CreateUndoPoint();
	virtual void WentTransient(HeeksObj* obj, TransientObject *tobj);

	virtual int GetNextID(int type);
	virtual bool InOpenFile();
	virtual void RemoveID(HeeksObj* object); // only call this from ObjList::Remove()
	virtual const wxChar* GetFileFullPath(); // returns NULL for untitled
	virtual void SetViewBox(const double* b);
	virtual void ViewExtents(bool rotate);
	virtual void SaveSTLFile(const std::list<HeeksObj*>& objects, const wxChar *filepath, double facet_tolerance = -1, double* scale = NULL);

	// sketches
	virtual SketchOrderType GetSketchOrder(HeeksObj* sketch);
	virtual bool ReOrderSketch(HeeksObj* sketch, SketchOrderType new_order); // returns true if done
	virtual void ExtractSeparateSketches(HeeksObj* sketch, std::list<HeeksObj*> &new_separate_sketches, const bool allow_individual_objects);
	virtual HeeksObj* ExtrudeSketch(HeeksObj* sketch, double height);
	virtual HeeksObj* LineArcsToWire(std::list<HeeksObj*> list);
	virtual HeeksObj* MakePipe(HeeksObj* spine, HeeksObj* profile);
	virtual HeeksObj* MakeRuled(std::list<HeeksObj*> list, bool make_solid);
	virtual bool ConvertSketchToFaceOrWire(HeeksObj* object, std::list<TopoDS_Shape> &face_or_wire, bool face_not_wire);
	virtual bool ConvertWireToSketch(const TopoDS_Wire& wire, HeeksObj* sketch, double deviation);
	virtual HeeksObj* GetSketchFromName(const wxChar* name);
	virtual bool ConvertSketchToEdges(HeeksObj *object, std::vector<TopoDS_Edge> &edges);

	// body functions
	virtual long BodyGetNumFaces(HeeksObj* body);
	virtual HeeksObj* BodyGetFirstFace(HeeksObj* body);
	virtual HeeksObj* BodyGetNextFace(HeeksObj* body);
	virtual HeeksObj* BodyGetPickedFace(HeeksObj* body);
	virtual long BodyGetNumEdges(HeeksObj* body);
	virtual HeeksObj* BodyGetFirstEdge(HeeksObj* body);
	virtual HeeksObj* BodyGetNextEdge(HeeksObj* body);
	virtual long BodyGetNumVertices(HeeksObj* body);
	virtual HeeksObj* BodyGetFirstVertex(HeeksObj* body);
	virtual HeeksObj* BodyGetNextVertex(HeeksObj* body);
	virtual bool BodyGetExtents(HeeksObj* body, double* extents, const double* orig = NULL, const double* xdir = NULL, const double* ydir = NULL, const double* zdir = NULL);
	virtual long BodyGetColor(HeeksObj* body);
	virtual int BodyGetShapeType(HeeksObj* body);

	// face functions
	virtual void FaceSetTempAttribute(HeeksObj* face, int attr);
	virtual int FaceGetTempAttribute(HeeksObj* face);
	virtual int FaceGetSurfaceType(HeeksObj* face);
	virtual void FaceGetUVBox(HeeksObj* face, double *uv_box);// 4 doubles
	virtual void FaceGetSurfaceUVPeriod(HeeksObj* face, double *uv, bool *isUPeriodic, bool *isVPeriodic);// 2 doubles
	virtual void FaceGetPointAndNormalAtUV(HeeksObj* face, double u, double v, double* p, double* norm);
	virtual bool FaceGetUVAtPoint(HeeksObj* face, const double *pos, double *u, double *v);
	virtual bool FaceGetClosestPoint(HeeksObj* face, const double *pos, double *closest_pnt);
	virtual bool FaceGetSurfaceClosestPoint(HeeksObj* face, const double *pos, double *closest_pnt);
	virtual void FaceGetPlaneParams(HeeksObj* face, double *d, double *norm);
	virtual void FaceGetCylinderParams(HeeksObj* face, double *pos, double *dir, double *radius);
	virtual void FaceGetSphereParams(HeeksObj* face, double *pos, double *radius);
	virtual void FaceGetConeParams(HeeksObj* face, double *pos, double *dir, double *radius, double* half_angle);
	virtual void FaceGetTorusParams(HeeksObj* face, double *pos, double *dir, double *majorRadius, double *minorRadius);
	virtual bool FaceGetNurbSurfaceParams(HeeksObj* face, CNurbSurfaceParams* params);
	virtual int FaceGetEdgeCount(HeeksObj* face);
	virtual HeeksObj* FaceGetFirstEdge(HeeksObj* face);
	virtual HeeksObj* FaceGetNextEdge(HeeksObj* face);
	virtual HeeksObj* FaceGetFirstLoop(HeeksObj* face);
	virtual HeeksObj* FaceGetNextLoop(HeeksObj* face);
	virtual bool FaceOrientation(HeeksObj* face);
	virtual HeeksObj* FaceGetParentBody(HeeksObj* face);

	// edge functions
	virtual int EdgeGetCurveType(HeeksObj* edge);
	virtual int EdgeGetFaceCount(HeeksObj* edge);
	virtual HeeksObj* EdgeGetFirstFace(HeeksObj* edge);
	virtual HeeksObj* EdgeGetNextFace(HeeksObj* edge);
	virtual HeeksObj* EdgeGetVertex0(HeeksObj* edge);
	virtual HeeksObj* EdgeGetVertex1(HeeksObj* edge);
	virtual void EdgeGetCurveParams(HeeksObj* edge, double* start, double* end, double* uStart, double* uEnd, int* Reversed);
	virtual void EdgeGetCurveParams2(HeeksObj* edge, double *uStart, double *uEnd, int *isClosed, int *isPeriodic);
	virtual bool EdgeInFaceSense(HeeksObj* edge, HeeksObj* face);
	virtual void EdgeEvaluate(HeeksObj* edge, double u, double *p, double *tangent);
	virtual bool EdgeGetLineParams(HeeksObj* edge, double* d6);
	virtual bool EdgeGetCircleParams(HeeksObj* edge, double* d6);
	virtual bool EdgeGetEllipseParams(HeeksObj* edge, double* d11);
	virtual void EdgeSetTempAttribute(HeeksObj* edge, int attr);
	virtual int EdgeGetTempAttribute(HeeksObj* edge);
	virtual double EdgeGetLength(HeeksObj* edge);
	virtual double EdgeGetLength2(HeeksObj* edge, double uStart, double uEnd);
	virtual bool EdgeGetClosestPoint(HeeksObj* edge, const double *pos, double *closest_pnt, double &u);

	// loop functions
	virtual long LoopGetEdgeCount(HeeksObj* loop);
	virtual HeeksObj* LoopGetFirstEdge(HeeksObj* loop);
	virtual HeeksObj* LoopGetNextEdge(HeeksObj* loop);
	virtual HeeksObj* LoopGetEdge(HeeksObj* loop, int index);
	virtual HeeksObj* LoopGetFace(HeeksObj* loop);
	virtual bool LoopIsOuter(HeeksObj* loop);

	// vertex functions
	virtual void VertexGetPoint(HeeksObj* vertex, double *d3);
	virtual HeeksObj* VertexGetFirstEdge(HeeksObj* vertex);
	virtual HeeksObj* VertexGetNextEdge(HeeksObj* vertex);

	virtual const wxChar* GetRevisionNumber();
	virtual void RegisterOnGLCommands( void(*callbackfunc)() );
	virtual void RemoveOnGLCommands( void(*callbackfunc)() );
	virtual void RegisterOnGraphicsSize( void(*callbackfunc)(wxSizeEvent& evt) );
	virtual void RemoveOnGraphicsSize( void(*callbackfunc)(wxSizeEvent& evt) );
	virtual void RegisterOnMouseFn( void(*callbackfunc)(wxMouseEvent&) );
	virtual void RemoveOnMouseFn( void(*callbackfunc)(wxMouseEvent&) );
	virtual void RegisterOnSaveFn( void(*callbackfunc)(bool from_changed_prompt) );
	virtual void RegisterIsModifiedFn( bool(*callbackfunc)() );
	virtual void RegisterToolBar( wxToolBarBase* );
	virtual void RegisterAddToolBars( void(*callbackfunc)() );
	virtual void PropertiesOnApply2();// don't need to press tick to make changes
	virtual void AddToAboutBox(const wxChar* str);
	virtual void SetDefaultLayout(const wxString& str);
	virtual HeeksObj* NewSTLSolid();
	virtual void STLSolidAddTriangle(HeeksObj* stl_solid, float* t);
	virtual const HeeksColor& GetBackgroundColor();
	virtual void SetColor(int r, int b, int g);
	virtual bool InputDouble(const wxChar* prompt, const wxChar* value_name, double &value);
	virtual bool InputLength(const wxChar* prompt, const wxChar* value_name, double &value);
	virtual double GetViewUnits();
	virtual void SetViewUnits(double units, bool write_to_config);
	virtual void SplineToBiarcs(HeeksObj* spline, std::list<HeeksObj*> &new_spans, double tolerance);
	virtual HeeksObj* NewSplineFromPoints(unsigned int num_points, const double* d3); // list of 3 doubles

	// Geometry functions
	virtual bool Intersect(const gp_Lin& lin, const gp_Lin& lin2, gp_Pnt &pnt);
	virtual bool Intersect(const gp_Pnt& pnt, const gp_Lin& lin);
	virtual bool Intersect(const gp_Pnt& pnt, const gp_Circ& cir);
	virtual void Intersect(const gp_Lin& line, const gp_Circ& circle, std::list<gp_Pnt> &list);
	virtual void Intersect(const gp_Circ& c1, const gp_Circ& c2, std::list<gp_Pnt> &list);

	virtual void RegisterOnBuildTexture( void(*callbackfunc)() );
	virtual int LoadIconsTexture(const wxChar *filepath);

	virtual bool RegisterFileOpenHandler( const std::list<wxString> file_extensions, void (*fileopen_handler)(const wxChar *path) );
	virtual bool UnregisterFileOpenHandler( void (*fileopen_handler)(const wxChar *path) );

	virtual void RegisterUnitsChangeHandler( void (*units_changed_handler)(const double value) );
	virtual void UnregisterUnitsChangeHandler( void (*units_changed_handler)(const double value) );
};
