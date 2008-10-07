// HeeksCADInterface.h

// include this in your dynamic library to interface with HeeksCAD

#pragma once

class HeeksObj;
class wxFrame;
class wxAuiManager;
class Observer;
class wxPoint;
class CInputMode;
class TiXmlElement;

class CHeeksCADInterface{
public:
	CHeeksCADInterface(){}
	~CHeeksCADInterface(){}

	virtual double GetTolerance();
	virtual void RefreshProperties();
	virtual void RefreshOptions();
	virtual void RefreshInput();
	virtual void Repaint(bool soon = false);
	virtual bool GetCamera(double* pos, double* target, double* up, bool& perspective, double& field_of_view);
	virtual wxFrame* GetMainFrame();
	virtual wxWindow* GetGraphicsCanvas();
	virtual wxMenuBar* GetMenuBar();
	virtual wxMenu* GetViewMenu();
	virtual wxAuiManager* GetAuiManager();
	virtual void AddToolBarButton(wxToolBar* toolbar, const wxString& title, wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&) = NULL);
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
	virtual void ClearMarkedList();
	virtual CInputMode* GetSelectMode();
	virtual void SetInputMode(CInputMode* input_mode);
	virtual void WasModified(HeeksObj* object);
	virtual void WasAdded(HeeksObj* object);
	virtual void WasRemoved(HeeksObj* object);
	virtual void WereAdded(const std::list<HeeksObj*> &list);
	virtual void WereRemoved(const std::list<HeeksObj*> &list);
	virtual int PickObjects(const wxChar* str);
	virtual bool PickPosition(const wxChar* str, double* pos);
	virtual bool Digitize(const wxPoint &point, double* pos);
	virtual HeeksObj* GetFirstObject();
	virtual HeeksObj* GetNextObject();
	virtual void DrawObjectsOnFront(const std::list<HeeksObj*> &list);
	virtual HeeksObj* NewLineArcCollection();
	virtual HeeksObj* NewLine(const double* s, const double* e);
	virtual HeeksObj* NewArc(const double* s, const double* e, const double* c, const double* up); // set up to (0, 0, -1) for a clockwise arc
	virtual void RegisterObserver(Observer* observer);
	virtual void RemoveObserver(Observer* observer);
	virtual bool TangentialArc(const double* p0, const double* v0, const double* p1, double *c, double *a); // given p0, v0, p1, returns true if an arc found and sets c and a ( centre and axis direction ), false for a straight line
	virtual void RegisterHideableWindow(wxWindow* w);
	virtual HeeksObj* ReadXMLElement(TiXmlElement* pElem);
	virtual void RegisterReadXMLfunction(const char* type_name, HeeksObj*(*read_xml_function)(TiXmlElement* pElem));
	virtual HeeksObj* GetIDObject(int type, int id);
	virtual void SetObjectID(HeeksObj* object, int id); // check for existing id using GetIDObject and call DeleteUndoably first
	virtual int GetNextID(int type);
	virtual bool GetDisableSetObjectIDOnAdd();
	virtual void RemoveID(HeeksObj* object); // only call this from ObjList::Remove()
	virtual void WriteIDToXML(HeeksObj* object, TiXmlElement *element);
	virtual void ReadIDFromXML(HeeksObj* object, TiXmlElement *element);
	virtual const wxChar* GetFileFullPath(); // returns NULL for untitled

	// body functions
	virtual long BodyGetNumFaces(HeeksObj* body);
	virtual HeeksObj* BodyGetFirstFace(HeeksObj* body);
	virtual HeeksObj* BodyGetNextFace(HeeksObj* body);
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
	virtual void FaceGetPlaneParams(HeeksObj* face, double *d, double *norm);
	virtual void FaceGetCylinderParams(HeeksObj* face, double *pos, double *dir, double *radius);
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

	// loop functions
	virtual HeeksObj* LoopGetFirstEdge(HeeksObj* loop);
	virtual HeeksObj* LoopGetNextEdge(HeeksObj* loop);

	virtual const wxChar* GetRevisionNumber();
	virtual void RegisterOnGLCommands( void(*callbackfunc)() );
	virtual void RemoveOnGLCommands( void(*callbackfunc)() );
};
