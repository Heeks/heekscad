// InputMode.h
#pragma once

class HeeksObj;
class Tool;

class CInputMode{
public:
	virtual ~CInputMode() {}

	virtual void OnMouse( wxMouseEvent& event ){}
	virtual void OnKeyDown(wxKeyEvent& event){}
	virtual void OnKeyUp(wxKeyEvent& event){}
	virtual bool OnModeChange(void){return true;}
	virtual void GetObjectGetTools(std::list<Tool*> *f_list, HeeksObj *o, const wxPoint *p){}
	virtual void GetTools(std::list<Tool*> *f_list, const wxPoint *p){}
	virtual void OnFrontRender(){}
	virtual void OnRender(){}
	virtual void GetProperties(std::list<Property *> *list){}
	virtual void GetSharedProperties(std::list<Property *> *list){}
};
