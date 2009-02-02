// ViewZooming.h
#if !defined ViewZoomingHEADER
#define ViewZoomingHEADER

#include "stdafx.h"
#include "../interface/InputMode.h"

class ViewZooming: public CInputMode{
	wxPoint button_down_point;
	wxPoint CurrentPoint;

public:
	static bool m_reversed;

	// virtual functions for InputMode
	const wxChar* GetTitle(){return _("View Zoom");}
	const wxChar* GetHelpText();
	void OnMouse( wxMouseEvent& event );
};

#endif
