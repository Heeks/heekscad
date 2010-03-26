// GraphicsCanvas.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "ViewPoint.h"
#include "../interface/Observer.h"

class CGraphicsCanvas: public wxGLCanvas, Observer
{
private:
	bool m_LButton;
	wxPoint m_CurrentPoint;
	bool m_render_on_front_done;
	int m_save_buffer_for_XOR;
	std::list<CViewPoint> m_view_points;
	bool m_frozen;
	bool m_refresh_wanted_on_thaw;

public:
	CViewPoint m_view_point;
	bool m_orthogonal;

	void OnEditColor();
	void OnSelectDetails();
	void OnCharEvent(wxKeyEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnMagExtents(bool rotate, bool recalculate_gl_lists);
	void OnMag(gp_Vec &unitY, gp_Vec &unitZ, bool recalculate_gl_lists);
	void OnMagXY (bool recalculate_gl_lists);
	void OnMagXYM(bool recalculate_gl_lists);
	void OnMagXZ (bool recalculate_gl_lists);
	void OnMagXZM(bool recalculate_gl_lists);
	void OnMagYZ (bool recalculate_gl_lists);
	void OnMagYZM(bool recalculate_gl_lists);
	void OnMagXYZ(bool recalculate_gl_lists);
	void OnMagPrevious();
	void SetViewPoint(void);
	void StoreViewPoint(void);
	void RestorePreviousViewPoint(void);
	void DrawObjectsOnFront(const std::list<HeeksObj*> &list, bool do_depth_testing = false);
	bool UsePreviousViewPoint(void);
	void WindowMag(wxRect &window_box);
	void FindMarkedObject(const wxPoint &point, MarkedObject* marked_object);
	void DrawFront(void);
	void EndDrawFront(void);
	void FrontRender(void);
	void SetIdentityProjection(void);
	void SetXOR(void);
	void EndXOR(void);
	void DrawWindow(wxRect &rect, bool allow_extra_bits); // extra bits are added to the corners when dragging from right to left

public:
    CGraphicsCanvas(wxWindow* parent, int *attribList = (int*) NULL);
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

private:
    DECLARE_EVENT_TABLE()
};
