// InputMode.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "../interface/HeeksObj.h"
#include "../interface/Tool.h"

#pragma once

class HeeksObj;
class Tool;

class CInputMode{
public:
	virtual ~CInputMode() {}

	virtual const wxChar* GetTitle() = 0;
	virtual bool TitleHighlighted(){return true;}
	virtual const wxChar* GetHelpText(){return NULL;}
	virtual void OnMouse( wxMouseEvent& event ){}
	virtual void OnKeyDown(wxKeyEvent& event){}
	virtual void OnKeyUp(wxKeyEvent& event){}
	virtual bool OnModeChange(void){return true;}
	virtual void GetTools(std::list<Tool*> *t_list, const wxPoint *p){}
	virtual void OnFrontRender(){}
	virtual void OnRender(){}
	virtual void GetProperties(std::list<Property *> *list){}
};
