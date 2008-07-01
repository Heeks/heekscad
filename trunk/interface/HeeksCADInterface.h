// HeeksCADInterface.h

// include this in your dynamic library to interface with HeeksCAD

#pragma once

class HeeksObj;
class wxFrame;
class wxAuiManager;

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
	virtual void AddUndoably(HeeksObj* object);
	virtual const std::list<HeeksObj*>& GetSelection();
	virtual bool GetArcDirection(HeeksObj* object);
	virtual bool GetArcCentre(HeeksObj* object, double* c);
	virtual bool GetArcAxis(HeeksObj* object, double* a);
	virtual void get_2d_arc_segments(double xs, double ys, double xe, double ye, double xc, double yc, bool dir, bool want_start, double pixels_per_mm, void(*callbackfunc)(const double* xy));
	virtual double GetPixelScale();
};
