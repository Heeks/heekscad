// GraphicsCanvas.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "ViewPoint.h"
#include "../interface/Observer.h"
#include <wx/glcanvas.h>

class CGraphicsCanvas: public wxGLCanvas, Observer
{
private:
	bool m_LButton;
	wxPoint m_CurrentPoint;
	bool m_render_on_front_done;
	int m_save_buffer_for_XOR;
	std::list<CViewPoint> m_view_points;

	void update_mode(void);

public:
	CViewPoint m_view_point;
	bool m_orthogonal;

	void OnEditColor();
	void OnSelectDetails();
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnMagExtents(bool rotate, bool recalculate_gl_lists);
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
	void WhenMarkedListChanges(bool all_added, bool all_removed, const std::list<HeeksObj*>* added_list, const std::list<HeeksObj*>* removed_list);
	void Clear();

private:
    DECLARE_EVENT_TABLE()
};
