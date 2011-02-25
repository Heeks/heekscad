// PythonInterface.cpp
#include "stdafx.h"

#ifdef WIN32
#include "windows.h"
#endif

#include "PythonInterface.h"


#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#include <wx/wxPython/wxPython.h>
#define _DEBUG
#else
#include <Python.h>
#include <wx/wxPython/wxPython.h>
#endif

#include <boost/progress.hpp>
#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/python/call.hpp>

#include "Cuboid.h"
#include "SelectMode.h"
#include "GraphicsCanvas.h"
#include "../interface/MarkedObject.h"
#include "../interface/ToolList.h"

namespace bp = boost::python;

static void Redraw(void)
{
	wxGetApp().Repaint();
	printf("Repaint()\n");
}

static void Init(void)
{
	wxGetApp().OnInit();
	printf("Init()\n");
}

static void AddObjectFromButton(HeeksObj* new_object)
{
	wxGetApp().CreateUndoPoint();
	wxGetApp().Add(new_object,NULL);
	wxGetApp().Changed();
	wxGetApp().m_marked_list->Clear(true);
	wxGetApp().m_marked_list->Add(new_object, true);
	wxGetApp().SetInputMode(wxGetApp().m_select_mode);
	wxGetApp().Repaint();
}

static void OnCubeButton(void)
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(false);
	CCuboid* new_object = new CCuboid(gp_Ax2(gp_Pnt(0, 0, 0).Transformed(mat), gp_Dir(0, 0, 1).Transformed(mat), gp_Dir(1, 0, 0).Transformed(mat)), 10, 10, 10, _("Cuboid"), HeeksColor(191, 240, 191), 1.0f);
	AddObjectFromButton(new_object);
}

class PyTool{
public:
	Tool* tool;
	PyTool(){tool = NULL;}
	PyTool(Tool* t){tool = t;}

	void Run(){if(tool)tool->Run();}
	const char* GetTitle(){return tool ? Ttc(tool->GetTitle()): "";}
	const char* GetToolTip(){return tool ? Ttc(tool->GetToolTip()): "";}
	bool Disabled(){return tool ? tool->Disabled() : false;}
	bool Checked(){return tool ? tool->Checked() : false;}
	bool IsSeparator(){return tool == NULL;}
	const char* BitmapPath(){return tool ? Ttc(tool->BitmapPath().c_str()): "";}
	bool IsAToolList(){return tool ? tool->IsAToolList() : false;}
	boost::python::list GetChildTools(){
		boost::python::list tool_list;
		for(std::list<Tool*>::iterator It = ((ToolList*)tool)->m_tool_list.begin(); It != ((ToolList*)tool)->m_tool_list.end(); It++)
		{
			tool_list.append(new PyTool(*It));
		}
		return tool_list;
	}
};

static boost::python::list GetDropDownTools(int x, int y, bool dont_use_point_for_functions, bool from_graphics_canvas, bool control_pressed)
{
	boost::python::list tool_list;

	MarkedObjectOneOfEach marked_object;
	wxGetApp().FindMarkedObject(wxPoint(x, y), &marked_object);

	std::list<Tool*> f_list;
	wxGetApp().GetDropDownTools(f_list, wxPoint(x, y), &marked_object, dont_use_point_for_functions, from_graphics_canvas, control_pressed);

	BOOST_FOREACH(Tool* tool, f_list) {
		tool_list.append(new PyTool(tool));
    }
	return tool_list;
}


class MouseEvent
{
public:
	int m_event_type;
    int m_x, m_y;

    bool          m_leftDown;
    bool          m_middleDown;
    bool          m_rightDown;

    bool          m_controlDown;
    bool          m_shiftDown;
    bool          m_altDown;
    bool          m_metaDown;

    int           m_wheelRotation;
    int           m_wheelDelta;
    int           m_linesPerAction;

	wxMouseEvent GetWxMouseEvent()
	{
		wxMouseEvent e;
		switch(m_event_type)
		{
		case 1:
			e = wxMouseEvent(wxEVT_LEFT_DOWN);
			break;
		case 2:
			e = wxMouseEvent(wxEVT_LEFT_UP);
			break;
		case 3:
			e = wxMouseEvent(wxEVT_LEFT_DCLICK);
			break;
		case 4:
			e = wxMouseEvent(wxEVT_RIGHT_DOWN);
			break;
		case 5:
			e = wxMouseEvent(wxEVT_RIGHT_UP);
			break;
		case 6:
			e = wxMouseEvent(wxEVT_MIDDLE_DOWN);
			break;
		case 7:
			e = wxMouseEvent(wxEVT_MIDDLE_UP);
			break;
		case 8:
			e = wxMouseEvent(wxEVT_MOTION);
			break;
		case 9:
			e = wxMouseEvent(wxEVT_MOUSEWHEEL);
			break;

		}
		e.m_x = m_x;
		e.m_y = m_y;
		e.m_leftDown = m_leftDown;
		e.m_middleDown = m_middleDown;
		e.m_rightDown = m_rightDown;
		e.m_controlDown = m_controlDown;
		e.m_shiftDown = m_shiftDown;
		e.m_altDown = m_altDown;
		e.m_metaDown = m_metaDown;
		e.m_wheelRotation = m_wheelRotation;
		e.m_wheelDelta = m_wheelDelta;
		e.m_linesPerAction = m_linesPerAction;

		return e;
	}
};

static void ViewportOnMouseEvent(CViewport &v, MouseEvent &e)
{
	v.ViewportOnMouse(e.GetWxMouseEvent());
}

BOOST_PYTHON_MODULE(HeeksCAD)
{
	bp::class_<CViewport>("Viewport") 
        .def(bp::init<int, int>())
		.def("glCommands", &CViewport::glCommands)
		.def("DrawFront", &CViewport::DrawFront)
		.def("WidthAndHeightChanged", &CViewport::WidthAndHeightChanged)
		.def("OnMouseEvent", &ViewportOnMouseEvent)
        .def_readwrite("m_need_update", &CViewport::m_need_update)
        .def_readwrite("m_need_refresh", &CViewport::m_need_refresh)
    ;

	bp::class_<MouseEvent>("MouseEvent") 
        .def(bp::init<MouseEvent>())
        .def_readwrite("m_event_type", &MouseEvent::m_event_type)
        .def_readwrite("m_x", &MouseEvent::m_x)
        .def_readwrite("m_y", &MouseEvent::m_y)
        .def_readwrite("m_leftDown", &MouseEvent::m_leftDown)
        .def_readwrite("m_middleDown", &MouseEvent::m_middleDown)
        .def_readwrite("m_rightDown", &MouseEvent::m_rightDown)
        .def_readwrite("m_controlDown", &MouseEvent::m_controlDown)
        .def_readwrite("m_shiftDown", &MouseEvent::m_shiftDown)
        .def_readwrite("m_altDown", &MouseEvent::m_altDown)
        .def_readwrite("m_metaDown", &MouseEvent::m_metaDown)
        .def_readwrite("m_wheelRotation", &MouseEvent::m_wheelRotation)
        .def_readwrite("m_wheelDelta", &MouseEvent::m_wheelDelta)
        .def_readwrite("m_linesPerAction", &MouseEvent::m_linesPerAction)
    ;

	bp::class_<PyTool>("Tool") 
        .def(bp::init<PyTool>())
		.def("Run", &PyTool::Run)
		.def("GetTitle", &PyTool::GetTitle)
		.def("GetToolTip", &PyTool::GetToolTip)
		.def("Disabled", &PyTool::Disabled)
		.def("Checked", &PyTool::Checked)
		.def("IsSeparator", &PyTool::IsSeparator)
		.def("BitmapPath", &PyTool::BitmapPath)
		.def("IsAToolList", &PyTool::IsAToolList)
		.def("GetChildTools", &PyTool::GetChildTools)
    ;

	bp::def("redraw", Redraw);
    bp::def("init", Init);
    bp::def("OnCubeButton", OnCubeButton);
    bp::def("GetDropDownTools", GetDropDownTools);
}
