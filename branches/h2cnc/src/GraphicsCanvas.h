// GraphicsCanvas.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "ViewPoint.h"
#include "../interface/Observer.h"

class CViewport
{
protected:
	bool m_LButton;
	wxPoint m_CurrentPoint;
	bool m_render_on_front_done;
	int m_save_buffer_for_XOR;
	std::list<CViewPoint> m_view_points;
	bool m_frozen;
	bool m_refresh_wanted_on_thaw;
	int m_w;
	int m_h;

public:
	CViewPoint m_view_point;
	bool m_orthogonal;
	bool m_need_update;
	bool m_need_refresh;

	CViewport();
	CViewport(int w, int h);

	void SetViewport();
	void glCommands();
	void SetViewPoint(void);
	void InsertViewBox(const CBox& box);
	void StoreViewPoint(void);
	void RestorePreviousViewPoint(void);
	void DrawObjectsOnFront(const std::list<HeeksObj*> &list, bool do_depth_testing = false);
	void FindMarkedObject(const wxPoint &point, MarkedObject* marked_object);
	void DrawFront(void);
	void EndDrawFront(void);
	void FrontRender(void);
	void SetIdentityProjection();
	void SetXOR(void);
	void EndXOR(void);
	void DrawWindow(wxRect &rect, bool allow_extra_bits); // extra bits are added to the corners when dragging from right to left
	void WidthAndHeightChanged(int w, int h){m_w = w; m_h = h;}
	wxSize GetViewportSize(){return wxSize(m_w, m_h);}
	void GetViewportSize(int *w, int *h){*w = m_w; *h = m_h;}
    void ViewportOnMouse( wxMouseEvent& event );
	void OnMagExtents(bool rotate);
};

class CGraphicsCanvas: public wxGLCanvas, public CViewport, Observer
{
public:

    CGraphicsCanvas(wxWindow* parent);
    virtual ~CGraphicsCanvas(){};

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
    void OnMouse( wxMouseEvent& event );
	void OnMenuEvent(wxCommandEvent& WXUNUSED(event));

	// Observer's virtual functions
	void OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified);
	void WhenMarkedListChanges(bool selection_cleared, const std::list<HeeksObj*>* added_list, const std::list<HeeksObj*>* removed_list);
	void Clear();
	void Freeze();
	void Thaw();
	void Refresh();
	void RefreshSoon(); // for dragging the view, for example
	void OnEditColor();
	void OnSelectDetails();
	void OnCharEvent(wxKeyEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnMagExtents(bool rotate, bool recalculate_gl_lists);
	void OnMag(const gp_Vec &unitY, const gp_Vec &unitZ, bool recalculate_gl_lists);
	void OnMagXY (bool recalculate_gl_lists);
	void OnMagXYM(bool recalculate_gl_lists);
	void OnMagXZ (bool recalculate_gl_lists);
	void OnMagXZM(bool recalculate_gl_lists);
	void OnMagYZ (bool recalculate_gl_lists);
	void OnMagYZM(bool recalculate_gl_lists);
	void OnMagXYZ(bool recalculate_gl_lists);
	void OnMagPrevious();
	bool UsePreviousViewPoint(void);
	void WindowMag(wxRect &window_box);

private:
    DECLARE_EVENT_TABLE()
};
