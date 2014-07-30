// ViewPanning.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#pragma once

class ViewPanning: public CInputMode{
	wxPoint button_down_point;
	wxPoint CurrentPoint;

public:
	// virtual functions for InputMode
	const wxChar* GetTitle(){return _("View Pan");}
	const wxChar* GetHelpText();
	void OnMouse( wxMouseEvent& event );
	bool OnModeChange(void);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
};
