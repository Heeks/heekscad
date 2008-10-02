// MagDragWindow.h
#if !defined MagDragWindowHEADER
#define MagDragWindowHEADER

#include "stdafx.h"
#include "../interface/InputMode.h"

class MagDragWindow: public CInputMode{
private:
	wxRect window_box;
	bool window_box_exists;
	wxCursor* cursor;
	CInputMode *save_input_mode;

public:
	MagDragWindow(){save_input_mode = NULL; window_box_exists = false;}

	// virtual functions for InputMode
	const wxChar* GetTitle(){return _T("Magnifying by dragging a window");}
	void OnMouse( wxMouseEvent& event );
	bool OnModeChange(void);
	void OnFrontRender();
};

#endif
