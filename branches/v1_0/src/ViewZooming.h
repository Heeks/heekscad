// ViewZooming.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#pragma once

class ViewZooming: public CInputMode{
	wxPoint button_down_point;
	wxPoint CurrentPoint;

public:
	static bool m_reversed;

	// virtual functions for InputMode
	const wxChar* GetTitle(){return _("View Zoom");}
	const wxChar* GetHelpText();
	void OnMouse( wxMouseEvent& event );
	bool OnModeChange(void);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
};
