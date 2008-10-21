// ViewRotating.h
#if !defined ViewRotatingHEADER
#define ViewRotatingHEADER

#include "stdafx.h"
#include "../interface/InputMode.h"

class ViewRotating: public CInputMode{
	wxPoint button_down_point;
	wxPoint CurrentPoint;
public:
	// virtual functions for InputMode
	const wxChar* GetTitle(){return _T("View Rotate");}
	void OnMouse( wxMouseEvent& event );
};

#endif
