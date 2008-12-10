// PointOrWindow.h

#pragma once

#include "../interface/InputMode.h"

class WindowDragging;
class CViewPoint;

class PointOrWindow: public CInputMode{
private:
	wxPoint mouse_down_point;
	bool visible;

public:
	bool use_window;
	wxRect box_chosen;
	WindowDragging *window;

	PointOrWindow(bool wd);
	~PointOrWindow(void);

	// virtual functions for InputMode
	const wxChar* GetTitle(){return _("Dragging a window or picking a point");}
	void OnMouse( wxMouseEvent& event );
	bool OnModeChange(void);
	void OnFrontRender();

	void reset(void);
	void SetWithPoint(const wxPoint &point);
};
