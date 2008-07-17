// HeeksCADInterface.h

// include this in your dynamic library to interface with HeeksCAD

#pragma once

class HeeksObj;
class wxFrame;
class wxAuiManager;
class Observer;

class CHeeksCADInterface{
public:
	CHeeksCADInterface(){}
	~CHeeksCADInterface(){}

	virtual double GetTolerance();
	virtual void RefreshProperties();
	virtual void Repaint();
	virtual wxFrame* GetMainFrame();
	virtual wxAuiManager* GetAuiManager();
	virtual void AddToolBarButton(wxToolBar* toolbar, const wxString& title, wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&));
	virtual wxString GetExeFolder();
	virtual void AddUndoably(HeeksObj* object, HeeksObj* owner);
	virtual HeeksObj* GetMainObject();
	virtual void DeleteUndoably(HeeksObj* object);
	virtual const std::list<HeeksObj*>& GetMarkedList();
	virtual bool GetArcDirection(HeeksObj* object);
	virtual bool GetArcCentre(HeeksObj* object, double* c);
	virtual bool GetArcAxis(HeeksObj* object, double* a);
	virtual void get_2d_arc_segments(double xs, double ys, double xe, double ye, double xc, double yc, bool dir, bool want_start, double pixels_per_mm, void(*callbackfunc)(const double* xy));
	virtual bool GetSegmentVector(HeeksObj* object, double fraction, double* v);
	virtual double GetPixelScale();
	virtual void Mark(HeeksObj* object);
	virtual bool ObjectMarked(HeeksObj* object);
	virtual void ClearMarkedList();
	virtual void WasModified(HeeksObj* object);
	virtual void WasAdded(HeeksObj* object);
	virtual int PickObjects(const char* str);
	virtual HeeksObj* GetFirstObject();
	virtual HeeksObj* GetNextObject();
	virtual void DrawObjectsOnFront(const std::list<HeeksObj*> &list);
	virtual HeeksObj* GetLineArcCollection(int id);
	virtual HeeksObj* NewLineArcCollection();
	virtual int GetLineArcCollectionID(HeeksObj* la);
	virtual void SetLineArcCollectionID(HeeksObj* la, int id); // check for existing id using GetLineArcCollection and call DeleteUndoably first
	virtual HeeksObj* NewLine(const double* s, const double* e);
	virtual HeeksObj* NewArc(const double* s, const double* e, const double* c, const double* up); // set up to (0, 0, -1) for a clockwise arc
	virtual void RegisterObserver(Observer* observer);
	virtual void RemoveObserver(Observer* observer);
	virtual bool TangentialArc(const double* p0, const double* v0, const double* p1, double *c, double *a); // given p0, v0, p1, returns true if an arc found and sets c and a ( centre and axis direction ), false for a straight line
};
