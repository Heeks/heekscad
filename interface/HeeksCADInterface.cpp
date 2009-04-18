// HeeksCADInterface.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

// included only in the executable

#include "stdafx.h"
#include "HeeksCADInterface.h"
#include "HeeksFrame.h"
#include "PropertiesCanvas.h"
#include "MarkedList.h"
#include "HLine.h"
#include "HArc.h"
#include "HCircle.h"
#include "ObjPropsCanvas.h"
#include "OptionsCanvas.h"
#include "TreeCanvas.h"
#include "InputModeCanvas.h"
#include "GraphicsCanvas.h"
#include "Sketch.h"
#include "DigitizeMode.h"
#include "SelectMode.h"
#include "Shape.h"
#include "Face.h"
#include "Edge.h"
#include "Loop.h"
#include "../interface/ToolImage.h"
#include "StlSolid.h"
#include "HeeksConfig.h"
#include <gp_Sphere.hxx>
#include <gp_Cone.hxx>

double CHeeksCADInterface::GetTolerance()
{
	return wxGetApp().m_geom_tol;
}

void CHeeksCADInterface::RefreshProperties()
{
	wxGetApp().m_frame->m_properties->RefreshByRemovingAndAddingAll(false);
}

void CHeeksCADInterface::RefreshOptions()
{
	wxGetApp().m_frame->m_options->RefreshByRemovingAndAddingAll();
}

void CHeeksCADInterface::RefreshInput()
{
	wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
}

void CHeeksCADInterface::Repaint(bool soon)
{
	wxGetApp().Repaint(soon);
}

bool CHeeksCADInterface::GetCamera(double* pos, double* target, double* up, bool& perspective, double& field_of_view, double& near_plane, double& far_plane)
{
	if(wxGetApp().m_frame == NULL)return false;
	if(wxGetApp().m_frame->m_graphics == NULL)return false;
	CViewPoint &v = wxGetApp().m_frame->m_graphics->m_view_point;
	extract(v.m_lens_point, pos);
	extract(v.m_target_point, target);
	extract(v.m_vertical, up);
	perspective = false;
	{
		int width = v.m_window_rect[2];
		int height = v.m_window_rect[3];
		// use smallest
		if(height < width)width = height;
		field_of_view = width / v.m_pixel_scale;
	}
	near_plane = v.m_near_plane;
	far_plane = v.m_far_plane;
	return true;
}

wxFrame* CHeeksCADInterface::GetMainFrame()
{
	return wxGetApp().m_frame;
}

wxWindow* CHeeksCADInterface::GetGraphicsCanvas()
{
	return wxGetApp().m_frame->m_graphics;
}

#ifdef WIN32
HGLRC CHeeksCADInterface::GetRC()
{
	return wxGetApp().m_frame->m_graphics->GetContext()->GetGLRC();
}
#endif

wxMenuBar* CHeeksCADInterface::GetMenuBar()
{
	return wxGetApp().m_frame->m_menuBar;
}

wxMenu* CHeeksCADInterface::GetWindowMenu()
{
	return wxGetApp().m_frame->m_menuWindow;
}

wxAuiManager* CHeeksCADInterface::GetAuiManager()
{
	return wxGetApp().m_frame->m_aui_manager;
}

void CHeeksCADInterface::AddToolBarButton(wxToolBar* toolbar, const wxString& title, const wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&))
{
	wxGetApp().m_frame->AddToolBarTool(toolbar, title, bitmap, caption, onButtonFunction, onUpdateButtonFunction);
}

float CHeeksCADInterface::GetToolImageButtonScale()
{
	return ToolImage::m_button_scale;
}

int CHeeksCADInterface::GetToolImageBitmapSize()
{
	return ToolImage::GetBitmapSize();
}

int CHeeksCADInterface::AddMenuCheckItem(wxMenu* menu, const wxString& title, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&))
{
	return wxGetApp().m_frame->AddMenuCheckItem(menu, title, onButtonFunction, onUpdateButtonFunction);
}
	
int CHeeksCADInterface::AddMenuItem(wxMenu* menu, const wxString& title, void(*onButtonFunction)(wxCommandEvent&))
{
	return wxGetApp().m_frame->AddMenuItem(menu, title, onButtonFunction);
}

wxString CHeeksCADInterface::GetExeFolder()
{
	return wxGetApp().GetExeFolder();
}

void CHeeksCADInterface::AddUndoably(HeeksObj* object, HeeksObj* owner)
{
	wxGetApp().AddUndoably(object, owner, NULL);
}

HeeksObj* CHeeksCADInterface::GetMainObject()
{
	return &(wxGetApp());
}

void CHeeksCADInterface::DeleteUndoably(HeeksObj* object)
{
	wxGetApp().DeleteUndoably(object);
}

const std::list<HeeksObj*>& CHeeksCADInterface::GetMarkedList(void)
{
	return wxGetApp().m_marked_list->list();
}

bool CHeeksCADInterface::GetArcCentre(HeeksObj* object, double* c)
{
	extract(((HArc*)object)->m_circle.Location(), c);
	return true;
}

bool CHeeksCADInterface::GetArcAxis(HeeksObj* object, double* a)
{
	extract(((HArc*)object)->m_circle.Axis().Direction(), a);
	return true;
}

double CHeeksCADInterface::CircleGetRadius(HeeksObj* object)
{
	return ((HCircle*)object)->m_circle.Radius();
}

void CHeeksCADInterface::get_2d_arc_segments(double xs, double ys, double xe, double ye, double xc, double yc, bool dir, bool want_start, double pixels_per_mm, void(*callbackfunc)(const double* xy))
{
	wxGetApp().get_2d_arc_segments(xs, ys, xe, ye, xc, yc, dir, want_start, pixels_per_mm, callbackfunc);
}

bool CHeeksCADInterface::GetSegmentVector(HeeksObj* object, double fraction, double* v)
{
	gp_Vec gv;

	switch(object->GetType())
	{
	case LineType:
		gv = ((HLine*)object)->GetSegmentVector(fraction);
		break;
	case ArcType:
		gv = ((HArc*)object)->GetSegmentVector(fraction);
		break;
	default:
		return false;
	}

	extract(gv, v);
	return true;
}

double CHeeksCADInterface::GetPixelScale()
{
	return wxGetApp().GetPixelScale();
}

void CHeeksCADInterface::Mark(HeeksObj* object)
{
	wxGetApp().m_marked_list->Add(object, true);
}

bool CHeeksCADInterface::ObjectMarked(HeeksObj* object)
{
	return wxGetApp().m_marked_list->ObjectMarked(object);
}

void CHeeksCADInterface::SetMarkingFilter(long filter)
{
	wxGetApp().m_marked_list->m_filter = filter;
}

long CHeeksCADInterface::GetMarkingFilter()
{
	return wxGetApp().m_marked_list->m_filter;
}

void CHeeksCADInterface::ClearMarkedList()
{
	wxGetApp().m_marked_list->Clear(true);
}

CInputMode* CHeeksCADInterface::GetSelectMode()
{
	return wxGetApp().m_select_mode;
}

void CHeeksCADInterface::SetInputMode(CInputMode* input_mode)
{
	wxGetApp().SetInputMode(input_mode);
}

void CHeeksCADInterface::WasModified(HeeksObj* object)
{
	wxGetApp().WasModified(object);
}

void CHeeksCADInterface::WasAdded(HeeksObj* object)
{
	wxGetApp().WasAdded(object);
}

void CHeeksCADInterface::WasRemoved(HeeksObj* object)
{
	wxGetApp().WasRemoved(object);
}

void CHeeksCADInterface::WereAdded(const std::list<HeeksObj*> &list)
{
	wxGetApp().WereAdded(list);
}

void CHeeksCADInterface::WereRemoved(const std::list<HeeksObj*> &list)
{
	wxGetApp().WereRemoved(list);
}

int CHeeksCADInterface::PickObjects(const wxChar* str, long marking_filter, bool m_just_one)
{
	return wxGetApp().PickObjects(str, marking_filter, m_just_one);
}

bool CHeeksCADInterface::PickPosition(const wxChar* str, double* pos)
{
	return wxGetApp().PickPosition(str, pos);
}

bool CHeeksCADInterface::Digitize(const wxPoint &point, double* pos)
{
	DigitizedPoint p = wxGetApp().m_digitizing->digitize(point);
	if(p.m_type == DigitizeNoItemType)
		return false;

	extract(p.m_point, pos);
	return true;
}

HeeksObj* CHeeksCADInterface::GetFirstObject()
{
	return wxGetApp().GetFirstChild();
}

HeeksObj* CHeeksCADInterface::GetNextObject()
{
	return wxGetApp().GetNextChild();
}

void CHeeksCADInterface::DrawObjectsOnFront(const std::list<HeeksObj*> &list)
{
	wxGetApp().m_frame->m_graphics->DrawObjectsOnFront(list);
}

HeeksObj* CHeeksCADInterface::NewSketch()
{
	return new CSketch;
}

HeeksObj* CHeeksCADInterface::NewLine(const double* s, const double* e)
{
	return new HLine(make_point(s), make_point(e), &wxGetApp().current_color);
}

HeeksObj* CHeeksCADInterface::NewArc(const double* s, const double* e, const double* c, const double* up)
{
	// arc
	gp_Pnt p0 = make_point(s);
	gp_Pnt p1 = make_point(e);
	gp_Dir dir(up[0], up[1], up[2]);
	gp_Pnt pc = make_point(c);
	gp_Circ circle(gp_Ax2(pc, dir), p1.Distance(pc));
	return new HArc(p0, p1, circle, &wxGetApp().current_color);
}

void CHeeksCADInterface::RegisterObserver(Observer* observer)
{
	wxGetApp().RegisterObserver(observer);
}

void CHeeksCADInterface::RemoveObserver(Observer* observer)
{
	wxGetApp().RemoveObserver(observer);
}

bool CHeeksCADInterface::TangentialArc(const double* p0, const double* v0, const double* p1, double *c, double *a)
{
	gp_Pnt centre;
	gp_Dir axis;
	bool arc_found = HArc::TangentialArc(make_point(p0), make_vector(v0), make_point(p1), centre, axis);
	if(arc_found)
	{
		extract(centre, c);
		extract(axis, a);
	}
	return arc_found;
}

void CHeeksCADInterface::RegisterHideableWindow(wxWindow* w)
{
	wxGetApp().RegisterHideableWindow(w);
}

HeeksObj* CHeeksCADInterface::ReadXMLElement(TiXmlElement* pElem)
{
	return wxGetApp().ReadXMLElement(pElem);
}

void CHeeksCADInterface::RegisterReadXMLfunction(const char* type_name, HeeksObj*(*read_xml_function)(TiXmlElement* pElem))
{
	wxGetApp().RegisterReadXMLfunction(type_name, read_xml_function);
}

void CHeeksCADInterface::OpenXMLFile(const wxChar *filepath, bool undoably, HeeksObj* paste_into)
{
	wxGetApp().OpenXMLFile(filepath, undoably, paste_into);
}

void CHeeksCADInterface::ObjectWriteBaseXML(HeeksObj* object, TiXmlElement* element)
{
	wxGetApp().ObjectWriteBaseXML(object, element);
}

void CHeeksCADInterface::ObjectReadBaseXML(HeeksObj* object, TiXmlElement* element)
{
	wxGetApp().ObjectReadBaseXML(object, element);
}

HeeksObj* CHeeksCADInterface::GetIDObject(int type, int id)
{
	return wxGetApp().GetIDObject(type, id);
}

void CHeeksCADInterface::SetObjectID(HeeksObj* object, int id)
{
	wxGetApp().SetObjectID(object, id);
}

int CHeeksCADInterface::GetNextID(int type)
{
	return wxGetApp().GetNextID(type);
}

bool CHeeksCADInterface::InOpenFile()
{
	return wxGetApp().m_in_OpenFile;
}

void CHeeksCADInterface::RemoveID(HeeksObj* object)
{
	wxGetApp().RemoveID(object);
}

const wxChar* CHeeksCADInterface::GetFileFullPath()
{
	return wxGetApp().m_filepath;
}

void CHeeksCADInterface::SetViewBox(const double* b)
{
	if(wxGetApp().m_frame == NULL)return;
	if(wxGetApp().m_frame->m_graphics == NULL)return;
	CViewPoint &v = wxGetApp().m_frame->m_graphics->m_view_point;
	v.m_extra_depth_box.Insert(b[0], b[1], b[2]);
	v.m_extra_depth_box.Insert(b[3], b[4], b[5]);
}

void CHeeksCADInterface::SaveSTLFile(const std::list<HeeksObj*>& objects, const wxChar *filepath)
{
	wxGetApp().SaveSTLFile(objects, filepath);
}

SketchOrderType CHeeksCADInterface::GetSketchOrder(HeeksObj* sketch)
{
	return ((CSketch*)sketch)->GetSketchOrder();
}

bool CHeeksCADInterface::ReOrderSketch(HeeksObj* sketch, SketchOrderType new_order)
{
	return ((CSketch*)sketch)->ReOrderSketch(new_order);
}

void CHeeksCADInterface::ExtractSeparateSketches(HeeksObj* sketch, std::list<HeeksObj*> &new_separate_sketches)
{
	((CSketch*)sketch)->ExtractSeparateSketches(new_separate_sketches);
}

long CHeeksCADInterface::BodyGetNumFaces(HeeksObj* body)
{
	return ((CShape*)body)->m_faces->GetNumChildren();
}

HeeksObj* CHeeksCADInterface::BodyGetFirstFace(HeeksObj* body)
{
	return ((CShape*)body)->m_faces->GetFirstChild();
}

HeeksObj* CHeeksCADInterface::BodyGetNextFace(HeeksObj* body)
{
	return ((CShape*)body)->m_faces->GetNextChild();
}

HeeksObj* CHeeksCADInterface::BodyGetPickedFace(HeeksObj* body)
{
	return ((CShape*)body)->m_picked_face;
}

void CHeeksCADInterface::FaceSetTempAttribute(HeeksObj* face, int attr)
{
	((CFace*)face)->m_temp_attr = attr;
}

int CHeeksCADInterface::FaceGetTempAttribute(HeeksObj* face)
{
	return ((CFace*)face)->m_temp_attr;
}

int CHeeksCADInterface::FaceGetSurfaceType(HeeksObj* face)
{
	return ((CFace*)face)->GetSurfaceType();
}

void CHeeksCADInterface::FaceGetUVBox(HeeksObj* face, double *uv_box)
{
	return ((CFace*)face)->GetUVBox(uv_box);
}

void CHeeksCADInterface::FaceGetPointAndNormalAtUV(HeeksObj* face, double u, double v, double* p, double* norm)
{
	gp_Pnt pos;
	gp_Dir n = ((CFace*)face)->GetNormalAtUV(u, v, p ? (&pos):NULL);
	if(p)extract(pos, p);
	if(norm)extract(n, norm);
}

bool CHeeksCADInterface::FaceGetUVAtPoint(HeeksObj* face, const double *pos, double *u, double *v)
{
	return ((CFace*)face)->GetUVAtPoint(make_point(pos), u, v);
}

bool CHeeksCADInterface::FaceGetClosestPoint(HeeksObj* face, const double *pos, double *closest_pnt)
{
	gp_Pnt cp;
	if(((CFace*)face)->GetClosestPoint(make_point(pos), cp))
	{
		extract(cp, closest_pnt);
		return true;
	}
	return false;
}

void CHeeksCADInterface::FaceGetPlaneParams(HeeksObj* face, double *d, double *norm)
{
	gp_Pln p;
	((CFace*)face)->GetPlaneParams(p);

	if(norm)extract(p.Axis().Direction(), norm);
	if(d)*d = -(gp_Vec(p.Axis().Direction().XYZ()) * gp_Vec(p.Axis().Location().XYZ()));
}

void CHeeksCADInterface::FaceGetCylinderParams(HeeksObj* face, double *pos, double *dir, double *radius)
{
	gp_Cylinder c;
	((CFace*)face)->GetCylinderParams(c);

	if(pos)extract(c.Location(), pos);
	if(dir)extract(c.Axis().Direction(), dir);
	if(radius)*radius = c.Radius();
}

void CHeeksCADInterface::FaceGetSphereParams(HeeksObj* face, double *pos, double *radius)
{
	gp_Sphere s;
	((CFace*)face)->GetSphereParams(s);

	if(pos)extract(s.Location(), pos);
	if(radius)*radius = s.Radius();
}

void CHeeksCADInterface::FaceGetConeParams(HeeksObj* face, double *pos, double *dir, double *radius, double* half_angle)
{
	gp_Cone c;
	((CFace*)face)->GetConeParams(c);

	if(pos)extract(c.Location(), pos);
	if(dir)extract(c.Axis().Direction(), dir);
	if(radius)*radius = c.RefRadius();
	if(half_angle)*half_angle = c.SemiAngle();
}

int CHeeksCADInterface::FaceGetEdgeCount(HeeksObj* face)
{
	return ((CFace*)face)->m_edges.size();
}

HeeksObj* CHeeksCADInterface::FaceGetFirstEdge(HeeksObj* face)
{
	return ((CFace*)face)->GetFirstEdge();
}

HeeksObj* CHeeksCADInterface::FaceGetNextEdge(HeeksObj* face)
{
	return ((CFace*)face)->GetNextEdge();
}

HeeksObj* CHeeksCADInterface::FaceGetFirstLoop(HeeksObj* face)
{
	return ((CFace*)face)->GetFirstLoop();
}

HeeksObj* CHeeksCADInterface::FaceGetNextLoop(HeeksObj* face)
{
	return ((CFace*)face)->GetNextLoop();
}

bool CHeeksCADInterface::FaceOrientation(HeeksObj* face)
{
	return ((CFace*)face)->Orientation();
}

long CHeeksCADInterface::BodyGetNumEdges(HeeksObj* body)
{
	return ((CShape*)body)->m_edges->GetNumChildren();
}

HeeksObj* CHeeksCADInterface::BodyGetFirstEdge(HeeksObj* body)
{
	return ((CShape*)body)->m_edges->GetFirstChild();
}

HeeksObj* CHeeksCADInterface::BodyGetNextEdge(HeeksObj* body)
{
	return ((CShape*)body)->m_edges->GetNextChild();
}

int CHeeksCADInterface::EdgeGetCurveType(HeeksObj* edge)
{
	return ((CEdge*)edge)->GetCurveType();
}

int CHeeksCADInterface::EdgeGetFaceCount(HeeksObj* edge)
{
	return ((CEdge*)edge)->m_faces.size();
}

HeeksObj* CHeeksCADInterface::EdgeGetFirstFace(HeeksObj* edge)
{
	return ((CEdge*)edge)->GetFirstFace();
}

HeeksObj* CHeeksCADInterface::EdgeGetNextFace(HeeksObj* edge)
{
	return ((CEdge*)edge)->GetNextFace();
}

void CHeeksCADInterface::EdgeGetCurveParams(HeeksObj* edge, double* start, double* end, double* uStart, double* uEnd, int* Reversed)
{
	((CEdge*)edge)->GetCurveParams(start, end, uStart, uEnd, Reversed);
}

void CHeeksCADInterface::EdgeGetCurveParams2(HeeksObj* edge, double *uStart, double *uEnd, int *isClosed, int *isPeriodic)
{
	((CEdge*)edge)->GetCurveParams2(uStart, uEnd, isClosed, isPeriodic);
}

bool CHeeksCADInterface::EdgeInFaceSense(HeeksObj* edge, HeeksObj* face)
{
	return ((CEdge*)edge)->InFaceSense((CFace*)face);
}

void CHeeksCADInterface::EdgeEvaluate(HeeksObj* edge, double u, double *p, double *tangent)
{
	((CEdge*)edge)->Evaluate(u, p, tangent);
}

bool CHeeksCADInterface::EdgeGetLineParams(HeeksObj* edge, double* d6)
{
	return ((CEdge*)edge)->GetLineParams(d6);
}

bool CHeeksCADInterface::EdgeGetCircleParams(HeeksObj* edge, double* d6)
{
	return ((CEdge*)edge)->GetCircleParams(d6);
}

long CHeeksCADInterface::LoopGetEdgeCount(HeeksObj* loop)
{
	return ((CLoop*)loop)->m_edges.size();
}

HeeksObj* CHeeksCADInterface::LoopGetFirstEdge(HeeksObj* loop)
{
	return ((CLoop*)loop)->GetFirstEdge();
}

HeeksObj* CHeeksCADInterface::LoopGetNextEdge(HeeksObj* loop)
{
	return ((CLoop*)loop)->GetNextEdge();
}

HeeksObj* CHeeksCADInterface::LoopGetEdge(HeeksObj* loop, int index)
{
	return ((CLoop*)loop)->GetEdge(index);
}

bool CHeeksCADInterface::LoopIsOuter(HeeksObj* loop)
{
	return ((CLoop*)loop)->m_is_outer;
}

const wxChar* CHeeksCADInterface::GetRevisionNumber()
{
	return wxGetApp().m_version_number;
}

void CHeeksCADInterface::RegisterOnGLCommands( void(*callbackfunc)() )
{
	wxGetApp().RegisterOnGLCommands(callbackfunc);
}

void CHeeksCADInterface::RemoveOnGLCommands( void(*callbackfunc)() )
{
	wxGetApp().RemoveOnGLCommands(callbackfunc);
}

void CHeeksCADInterface::RegisterOnGraphicsSize( void(*callbackfunc)(wxSizeEvent& evt) )
{
	wxGetApp().RegisterOnGraphicsSize(callbackfunc);
}

void CHeeksCADInterface::RemoveOnGraphicsSize( void(*callbackfunc)(wxSizeEvent& evt) )
{
	wxGetApp().RemoveOnGraphicsSize(callbackfunc);
}

void CHeeksCADInterface::RegisterOnMouseFn( void(*callbackfunc)(wxMouseEvent&) )
{
	wxGetApp().RegisterOnMouseFn(callbackfunc);
}

void CHeeksCADInterface::RemoveOnMouseFn( void(*callbackfunc)(wxMouseEvent&) )
{
	wxGetApp().RemoveOnMouseFn(callbackfunc);
}

void CHeeksCADInterface::RegisterToolBar( wxToolBarBase* toolbar )
{
	wxGetApp().m_external_toolbars.push_back(toolbar);
}

void CHeeksCADInterface::RegisterAddToolBars( void(*callbackfunc)() )
{
	wxGetApp().m_AddToolBars_list.push_back(callbackfunc);
}

void CHeeksCADInterface::PropertiesOnApply2()
{
	// don't need to press tick to make changes
	wxGetApp().m_frame->m_properties->OnApply2();
}

void CHeeksCADInterface::AddToAboutBox(const wxChar* str)
{
	wxGetApp().m_frame->m_extra_about_box_str.Append(str);
}

void CHeeksCADInterface::SetDefaultLayout(const wxString& str)
{
	wxGetApp().m_frame->SetDefaultLayout(str);
}

void CHeeksCADInterface::StartHistory()
{
	wxGetApp().StartHistory();
}

void CHeeksCADInterface::EndHistory(void)
{
	wxGetApp().EndHistory();
}

HeeksObj* CHeeksCADInterface::NewSTLSolid()
{
	return new CStlSolid(&HeeksColor(220, 40, 40));
}

void CHeeksCADInterface::STLSolidAddTriangle(HeeksObj* stl_solid, float* t)
{
	((CStlSolid*)stl_solid)->AddTriangle(t);
}

const HeeksColor& CHeeksCADInterface::GetBackgroundColor()
{
	return wxGetApp().background_color;
}

bool CHeeksCADInterface::InputDouble(const wxChar* prompt, const wxChar* value_name, double &value)
{
	return wxGetApp().InputDouble(prompt, value_name, value);
}

double CHeeksCADInterface::GetViewUnits()
{
	return wxGetApp().m_view_units;
}

void CHeeksCADInterface::SetViewUnits(double units, bool write_to_config)
{
	wxGetApp().m_view_units = units;
	if(write_to_config)
	{
		HeeksConfig config;
		config.Write(_T("ViewUnits"), &wxGetApp().m_view_units);
	}
}

