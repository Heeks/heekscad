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
	virtual void GetTools(std::list<Tool*> *t_list, const wxPoint *p){}
	virtual void OnFrontRender(){}
	virtual void OnRender(){}
	virtual void GetProperties(std::list<Property *> *list){}
	virtual void GetOptions(std::list<Property *> *list){}
};
