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
#include <boost/python/return_value_policy.hpp>
#include <boost/python/return_internal_reference.hpp>



#include "Cuboid.h"
#include "SelectMode.h"
#include "GraphicsCanvas.h"
#include "../interface/MarkedObject.h"
#include "../interface/ToolList.h"
#include "StlSolid.h"
#include "HeeksFrame.h"
#include "GraphicsCanvas.h"
#include "Shape.h"
#include "Face.h"

namespace bp = boost::python;

static void Redraw(void)
{
	wxGetApp().Repaint();
}

static void Init(void)
{
	wxGetApp().OnInit();
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

static boost::python::list GetDropDownTools(int x, int y, bool dont_use_point_for_functions, bool control_pressed)
{
	boost::python::list tool_list;

	MarkedObjectOneOfEach marked_object;
	wxGetApp().FindMarkedObject(wxPoint(x, y), &marked_object);

	std::list<Tool*> f_list;
	wxGetApp().GetDropDownTools(f_list, wxPoint(x, y), &marked_object, dont_use_point_for_functions, control_pressed);

	BOOST_FOREACH(Tool* tool, f_list) {
		tool_list.append(new PyTool(tool));
    }
	return tool_list;
}

static const char* GetResFolder()
{
	return Ttc(wxGetApp().GetResFolder().c_str());
}

static int GetSketch()
{
	//Allows the user to pick a single sketch 
	// it returns the sketch id, or 0 if none was picked
	int result = wxGetApp().PickObjects(_("Select a Sketch"), MARKING_FILTER_SKETCH,true);

	for(std::list<HeeksObj*>::iterator It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
	{
		HeeksObj* object = *It;
		if(object->GetType() == SketchType){
			return object->GetID();
		}
	}

	return 0;
}

static boost::python::list GetSketches()
{
	// Allows the user to pick multiple sketches
	// returns a python list of sketch ids

	boost::python::list sketch_list;

	int result = wxGetApp().PickObjects(_("Select Sketches"), MARKING_FILTER_SKETCH);

	PyObject* pList = PyList_New(0);
	for(std::list<HeeksObj*>::iterator It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
	{
		HeeksObj* object = *It;
		if(object->GetType() == SketchType){
			sketch_list.append(object->GetID());
		}
	}

	return sketch_list;
}

static boost::python::list GetSelectedSketches()
{
	// returns a python list of sketch ids
	boost::python::list sketch_list;
	for(std::list<HeeksObj*>::iterator It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
	{
		HeeksObj* object = *It;
		if(object->GetType() == SketchType){
			sketch_list.append(object->GetID());
		}
	}

	return sketch_list;
}

static double GetViewUnits()
{
	return wxGetApp().m_view_units;
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

class ToolpathPoint
{
public:
	double x[3];
	bool iv;

	ToolpathPoint(double X, double Y, double Z, bool Iv){x[0] = X; x[1] = Y; x[2] = Z; iv = Iv;}
};

class CLineDraw
{
	std::list<ToolpathPoint> pts;
	int display_list;
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	CBox m_box;
	void delete_list()
	{
		if(display_list)
		{
			glDeleteLists(display_list, 1);
			display_list = 0;
		}
	}

public:
	CLineDraw(){red = 0; green = 0; blue = 0; display_list = 0;}
	CLineDraw(unsigned char r, unsigned char g, unsigned char b){
		display_list = 0;
		red = r;
		green = g;
		blue = b;
	}
	~CLineDraw(){delete_list();}

	void add_point(double x, double y, double z, bool iv)
	{
		delete_list();
		pts.push_back(ToolpathPoint(x, y, z, iv));
		m_box.Insert(x, y, z);
	}

	void glCommands()
	{
		if(display_list)glCallList(display_list);
		else
		{
			display_list = glGenLists(1);
			glNewList(display_list, GL_COMPILE_AND_EXECUTE);
			glColor3ub(red, green, blue);
			bool strip_begun = false;
			for(std::list<ToolpathPoint>::iterator It = pts.begin(); It != pts.end(); It++)
			{
				ToolpathPoint &pt = *It;
				if(pt.iv)
				{
					if(!strip_begun)
					{
						glBegin(GL_LINE_STRIP);
						strip_begun = true;
					}
				}
				else
				{
					if(strip_begun)
					{
						glEnd();
						strip_begun = false;
					}
				}
				glVertex3dv(pt.x);
			}
			if(strip_begun)
			{
				glEnd();
				strip_begun = false;
			}

			glEndList();
		}
	}

	void GetBox(CBox &box)
	{
		box.Insert(m_box);
	}

};

static void GLBeginTriangles()
{
	glBegin(GL_TRIANGLES);
}

static void GLBeginLineStrip()
{
	glBegin(GL_LINE_STRIP);
}

static void GLEnd()
{
	glEnd();
}

static void GLVertex3d(double x, double y, double z)
{
	glVertex3d(x, y, z);
}

static void GLColor3ub(unsigned char r, unsigned char g, unsigned char b)
{
	glColor3ub(r, g, b);
}

static CShape* GetFirstBody(void)
{
	HeeksObj* object = wxGetApp().GetFirstChild();
	while(object)
	{
		if(CShape::IsTypeAShape(object->GetType()))return (CShape*)object;
		object = wxGetApp().GetNextChild();
	}

	return NULL;
}

static CShape* GetNextBody(void)
{
	HeeksObj* object = wxGetApp().GetNextChild();
	while(object)
	{
		if(CShape::IsTypeAShape(object->GetType()))return (CShape*)object;
		object = wxGetApp().GetNextChild();
	}

	return NULL;
}

static void CStlSolidglCommands(CStlSolid& stl_solid)
{
	stl_solid.glCommands(false, false, false);
}

static CFace* BodyGetFirstFace(CShape* body)
{
	return (CFace*)(body->m_faces->GetFirstChild());
}

static int PickObjects(const char* str, long marking_filter, bool m_just_one)
{
	return wxGetApp().PickObjects(Ctt(str), marking_filter, m_just_one);
}

static bool PyOpenFile(const char* str)
{
	return wxGetApp().OpenFile(Ctt(str));
}

BOOST_PYTHON_MODULE(HeeksCAD)
{
	bp::class_<CViewport>("Viewport") 
        .def(bp::init<int, int>())
		.def("glCommands", &CViewport::glCommands)
		.def("DrawFront", &CViewport::DrawFront)
		.def("WidthAndHeightChanged", &CViewport::WidthAndHeightChanged)
		.def("OnMouseEvent", &ViewportOnMouseEvent)
		.def("SetViewPoint", &CViewport::SetViewPoint)
		.def("InsertViewBox", &CViewport::InsertViewBox)
		.def("OnMagExtents", &CViewport::OnMagExtents)
        .def_readwrite("m_need_update", &CViewport::m_need_update)
        .def_readwrite("m_need_refresh", &CViewport::m_need_refresh)
        .def_readwrite("m_orthogonal", &CViewport::m_orthogonal)
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

	bp::class_<CLineDraw>("LineDraw") 
        .def(bp::init<unsigned char, unsigned char, unsigned char>())
		.def("add_point", &CLineDraw::add_point)
		.def("glCommands", &CLineDraw::glCommands)
	;

	bp::class_<HeeksColor>("HeeksColor") 
        .def(bp::init<unsigned char, unsigned char, unsigned char>())
	;

	bp::class_<CStlSolid>("StlSolid") 
        .def(bp::init<const std::wstring&>())
		.def("glCommands", &CStlSolidglCommands)
		.def("GetBox", &CStlSolid::GetBox)
	;

	bp::class_<CFace>("Face")
        .def(bp::init<CFace>())
	;

	bp::class_<CShape>("Body")
        .def(bp::init<CShape>())
		.def("GetFirstFace", &BodyGetFirstFace, bp::return_value_policy<bp::reference_existing_object>(), "returns the first face")
	;

	bp::class_<CBox>("Box") 
        .def(bp::init<CBox>())
        .def(bp::init<double, double, double, double, double, double>())
        .def(bp::self == bp::other<CBox>())
        .def("Insert",static_cast< void (CBox::*)(double, double, double)>(&CBox::Insert))
		.def("MinX", &CBox::MinX)
		.def("MaxX", &CBox::MaxX)
		.def("MinY", &CBox::MinY)
		.def("MaxY", &CBox::MaxY)
		.def("MinZ", &CBox::MinZ)
		.def("MaxZ", &CBox::MaxZ)
        .def_readonly("valid", &CBox::m_valid)		
	;

	bp::def("redraw", Redraw);
    bp::def("init", Init);
    bp::def("OnCubeButton", OnCubeButton);
    bp::def("GetDropDownTools", GetDropDownTools);
    bp::def("GetResFolder", GetResFolder);
    bp::def("getsketch", GetSketch);
    bp::def("getsketches", GetSketches);
    bp::def("get_selected_sketches", GetSelectedSketches);
    bp::def("get_view_units", GetViewUnits);
    bp::def("glBeginTriangles", GLBeginTriangles);
    bp::def("glBeginLineStrip", GLBeginLineStrip);
    bp::def("glEnd", GLEnd);
    bp::def("glVertex3d", GLVertex3d);
    bp::def("glColor3ub", GLColor3ub);
    bp::def("GetFirstBody", GetFirstBody, bp::return_value_policy<bp::reference_existing_object>());
    bp::def("GetNextBody", GetNextBody, bp::return_value_policy<bp::reference_existing_object>());
	bp::def("PickObjects", PickObjects);
	bp::def("OpenFile", PyOpenFile);
}
