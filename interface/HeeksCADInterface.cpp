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
#include "HILine.h"
#include "HArc.h"
#include "HCircle.h"
#include "HSpline.h"
#include "Group.h"
#include "Cylinder.h"
#include "Cuboid.h"
#include "Cone.h"
#include "Sphere.h"
#include "Wire.h"
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
#include "Vertex.h"
#include "../interface/ToolImage.h"
#include "StlSolid.h"
#include "RuledSurface.h"
#include "HeeksConfig.h"
#include <gp_Sphere.hxx>
#include <gp_Cone.hxx>
#include <gp_Sphere.hxx>
#include <TopoDS_Wire.hxx>
#include "Geom.h"
#include "wxImageLoader.h"

double CHeeksCADInterface::GetTolerance()
{
	return wxGetApp().m_geom_tol;
}

void CHeeksCADInterface::RefreshProperties()
{
	wxGetApp().m_frame->m_properties->RefreshByRemovingAndAddingAll();
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

static CFlyOutList* toolbar_flyout = NULL;

void CHeeksCADInterface::StartToolBarFlyout(const wxString& title_and_bitmap)
{
	if(toolbar_flyout)delete toolbar_flyout;
	toolbar_flyout = new CFlyOutList(title_and_bitmap);
}

void CHeeksCADInterface::AddFlyoutButton(const wxString& title, const wxBitmap& bitmap, const wxString& tooltip, void(*onButtonFunction)(wxCommandEvent&))
{
	if(toolbar_flyout)
	{
		toolbar_flyout->m_list.push_back(CFlyOutItem(title, bitmap, tooltip, onButtonFunction));
	}
}

void CHeeksCADInterface::EndToolBarFlyout(wxToolBar* toolbar)
{
	if(toolbar_flyout)
	{
		wxGetApp().m_frame->AddToolBarFlyout(toolbar, *toolbar_flyout);
		delete toolbar_flyout;
		toolbar_flyout = NULL;
	}
}

float CHeeksCADInterface::GetToolImageButtonScale()
{
	return ToolImage::m_button_scale;
}

int CHeeksCADInterface::GetToolImageBitmapSize()
{
	return ToolImage::GetBitmapSize();
}

int CHeeksCADInterface::AddMenuItem(wxMenu* menu, const wxString& title, const wxBitmap& bitmap, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&), wxMenu* submenu, bool check_item)
{
	return wxGetApp().m_frame->AddMenuItem(menu, title, bitmap, onButtonFunction, onUpdateButtonFunction, submenu, check_item);
}

wxString CHeeksCADInterface::GetExeFolder()
{
	return wxGetApp().GetExeFolder();
}

HeeksObj* CHeeksCADInterface::GetMainObject()
{
	return &(wxGetApp());
}

void CHeeksCADInterface::Remove(HeeksObj* object)
{
	wxGetApp().Remove(object);
}

void CHeeksCADInterface::Add(HeeksObj* object,HeeksObj* prev)
{
	wxGetApp().Add(object,prev);
}

void CHeeksCADInterface::CreateUndoPoint()
{
	wxGetApp().CreateUndoPoint();
}

void CHeeksCADInterface::WentTransient(HeeksObj* obj, TransientObject *tobj)
{
	wxGetApp().WentTransient(obj, tobj);
}

void CHeeksCADInterface::Changed()
{
	wxGetApp().Changed();
}

const std::list<HeeksObj*>& CHeeksCADInterface::GetMarkedList(void)
{
	return wxGetApp().m_marked_list->list();
}

bool CHeeksCADInterface::GetArcCentre(HeeksObj* object, double* c)
{
	switch (object->GetType())
	{
		case ArcType:
			extract(((HArc*)object)->C->m_p, c);
			return true;

		case CircleType:
			extract(((HCircle*)object)->C->m_p, c);
			return true;

	} // End switch

	return(false);
}

bool CHeeksCADInterface::GetArcAxis(HeeksObj* object, double* a)
{
	extract(((HArc*)object)->m_axis.Direction(), a);
	return true;
}

double CHeeksCADInterface::CircleGetRadius(HeeksObj* object)
{
	return ((HCircle*)object)->m_radius;
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

void CHeeksCADInterface::Unmark(HeeksObj* object)
{
	wxGetApp().m_marked_list->Remove(object, true);
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

bool CHeeksCADInterface::GetLastClickPosition(double *pos)
{
	return wxGetApp().m_select_mode->GetLastClickPosition(pos);
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

HeeksObj* CHeeksCADInterface::NewPoint(const double* p)
{
    // I needed this for HeeksPython- it's just a point
	return new HPoint(make_point(p),&wxGetApp().current_color);

}

HeeksObj* CHeeksCADInterface::NewLine(const double* s, const double* e)
{
	return new HLine(make_point(s), make_point(e), &wxGetApp().current_color);
}

HeeksObj* CHeeksCADInterface::NewILine(const double* s, const double* e)
{
	return new HILine(make_point(s), make_point(e), &wxGetApp().current_color);
}

HeeksObj* CHeeksCADInterface::NewCircle(const double* c, double r)
{
	gp_Dir up(0,0,1);
	return new HCircle(gp_Circ(gp_Ax2(make_point(c),up),r),&wxGetApp().current_color);
}

HeeksObj* CHeeksCADInterface::NewGroup()
{
	return new CGroup();
}

HeeksObj* CHeeksCADInterface::NewCuboid(const double* c, double x, double y, double z)
{
	gp_Dir up(0,0,1);
	return new CCuboid(gp_Ax2(make_point(c),up),x,y,z,_T("Cuboid"),wxGetApp().current_color);
}

HeeksObj* CHeeksCADInterface::NewCylinder(const double* c, double r, double h)
{
	gp_Dir up(0,0,1);
	return new CCylinder(gp_Ax2(make_point(c),up),r,h,_T("Cylinder"),wxGetApp().current_color);
}

HeeksObj* CHeeksCADInterface::NewCone(const double* c, double r1, double r2, double h)
{
	gp_Dir up(0,0,1);
	return new CCone(gp_Ax2(make_point(c),up),r1,r2,h,_T("Cone"),wxGetApp().current_color);
}

//added by DF for heekspython
HeeksObj* CHeeksCADInterface::NewSphere(const double* pos, double radius)
{
   return new CSphere(gp_Pnt(make_point(pos)), radius,_T("Sphere"), wxGetApp().current_color);
}

HeeksObj* CHeeksCADInterface::NewSolid(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col)
{
	return(new CSolid( solid, title, col ) );

} // End NewSolid() method


HeeksObj* CHeeksCADInterface::Fuse(std::list<HeeksObj*> objects)
{
	return CShape::FuseShapes(objects);
}

HeeksObj* CHeeksCADInterface::Cut(std::list<HeeksObj*> objects)
{
	return CShape::CutShapes(objects);
}

HeeksObj* CHeeksCADInterface::Common(std::list<HeeksObj*> objects)
{
	return CShape::CommonShapes(objects);
}

void CHeeksCADInterface::TranslateObject(HeeksObj* obj, const double*c)
{
	gp_Pnt zp(0,0,0);
	gp_Trsf t;
	t.SetTranslation(zp,make_point(c));
	double m[16];
	extract(t,m);
	obj->ModifyByMatrix(m);
}

void CHeeksCADInterface::RotateObject(HeeksObj* obj, const double*c,const double*u, double r)
{
	gp_Trsf t;
	t.SetRotation(gp_Ax1(make_point(c),gp_Dir(make_point(u).XYZ())),r);
	double m[16];
	extract(t,m);
	obj->ModifyByMatrix(m);
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

HeeksObj* CHeeksCADInterface::NewArc(const double* c, const double* u, double r, double s, double e)
{
	// arc
	gp_Pnt pc = make_point(c);
	gp_Dir up(make_point(u).XYZ());
	gp_Circ circle(gp_Ax2(pc, up), r);
	gp_Pnt p0 = pc.XYZ() + circle.XAxis().Direction().XYZ()*r;
	gp_Pnt p1 =pc.XYZ() + circle.XAxis().Direction().XYZ()*r;
	p0.Rotate(circle.Axis(),s);
	p1.Rotate(circle.Axis(),e);

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

void CHeeksCADInterface::SaveXMLFile(const std::list<HeeksObj*>& objects, const wxChar *filepath, bool for_clipboard)
{
	wxGetApp().SaveXMLFile( objects, filepath, for_clipboard );
}


HeeksObj* CHeeksCADInterface::ReadXMLElement(TiXmlElement* pElem)
{
	return wxGetApp().ReadXMLElement(pElem);
}

void CHeeksCADInterface::RegisterReadXMLfunction(const char* type_name, HeeksObj*(*read_xml_function)(TiXmlElement* pElem))
{
	wxGetApp().RegisterReadXMLfunction(type_name, read_xml_function);
}

void CHeeksCADInterface::OpenXMLFile(const wxChar *filepath,HeeksObj* paste_into)
{
	wxGetApp().OpenXMLFile(filepath, paste_into);
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

std::list<HeeksObj*> CHeeksCADInterface::GetIDObjects(int type, int id)
{
    return wxGetApp().GetIDObjects(type,id);
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

void CHeeksCADInterface::ViewExtents(bool rotate)
{
	wxGetApp().m_frame->m_graphics->OnMagExtents(rotate, false);
}

void CHeeksCADInterface::SaveSTLFile(const std::list<HeeksObj*>& objects, const wxChar *filepath, double facet_tolerance)
{
	wxGetApp().SaveSTLFile(objects, filepath, facet_tolerance);
}

SketchOrderType CHeeksCADInterface::GetSketchOrder(HeeksObj* sketch)
{
	return ((CSketch*)sketch)->GetSketchOrder();
}

HeeksObj* CHeeksCADInterface::LineArcsToWire(std::list<HeeksObj*> list)
{
	TopoDS_Wire wire;
	ConvertLineArcsToWire2(list,wire);
	return new CWire(wire,_T("Wire"));
}

bool CHeeksCADInterface::ConvertSketchToFaceOrWire(HeeksObj* object, std::list<TopoDS_Shape> &face_or_wire, bool face_not_wire)
{
	return(::ConvertSketchToFaceOrWire( object, face_or_wire, face_not_wire ));
}

bool CHeeksCADInterface::ConvertWireToSketch(const TopoDS_Wire& wire, HeeksObj* sketch, double deviation)
{
	return(::ConvertWireToSketch( wire, sketch, deviation ));
}



HeeksObj* CHeeksCADInterface::MakePipe(HeeksObj* spine, HeeksObj* profile)
{
	return CreatePipeFromProfile(spine,profile);
}

bool CHeeksCADInterface::ReOrderSketch(HeeksObj* sketch, SketchOrderType new_order)
{
	return ((CSketch*)sketch)->ReOrderSketch(new_order);
}

HeeksObj* CHeeksCADInterface::ExtrudeSketch(HeeksObj* sketch, double height)
{
	std::list<HeeksObj*>list;
	list.push_back(sketch);
	return CreateExtrusionOrRevolution(list,height, true, false);
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

void CHeeksCADInterface::FaceGetSurfaceUVPeriod(HeeksObj* face, double *uv, bool *isUPeriodic, bool *isVPeriodic)
{
	return ((CFace*)face)->GetSurfaceUVPeriod(uv, isUPeriodic, isVPeriodic);
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

void CHeeksCADInterface::FaceGetTorusParams(HeeksObj* face, double *pos, double *dir, double *majorRadius, double *minorRadius)
{
	gp_Torus t;
	((CFace*)face)->GetTorusParams(t);

	if(pos)extract(t.Location(), pos);
	if(dir)extract(t.Axis().Direction(), dir);
	if(majorRadius)*majorRadius = t.MajorRadius();
	if(minorRadius)*minorRadius = t.MinorRadius();
}

bool CHeeksCADInterface::FaceGetNurbSurfaceParams(HeeksObj* face, CNurbSurfaceParams* params)
{
	return ((CFace*)face)->GetNurbSurfaceParams(params);
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

HeeksObj* CHeeksCADInterface::FaceGetParentBody(HeeksObj* face)
{
	return ((CFace*)face)->GetParentBody();
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

long CHeeksCADInterface::BodyGetNumVertices(HeeksObj* body)
{
	return ((CShape*)body)->m_vertices->GetNumChildren();
}

HeeksObj* CHeeksCADInterface::BodyGetFirstVertex(HeeksObj* body)
{
	return ((CShape*)body)->m_vertices->GetFirstChild();
}

HeeksObj* CHeeksCADInterface::BodyGetNextVertex(HeeksObj* body)
{
	return ((CShape*)body)->m_vertices->GetNextChild();
}

bool CHeeksCADInterface::BodyGetExtents(HeeksObj* body, double* extents, const double* orig, const double* xdir, const double* ydir, const double* zdir)
{
	return ((CShape*)body)->GetExtents(extents, orig, xdir, ydir, zdir);
}

long CHeeksCADInterface::BodyGetColor(HeeksObj* body)
{
	// returns a COLORREF style long for the color of the body
	return ((CShape*)body)->m_color.COLORREF_color();
}

int CHeeksCADInterface::BodyGetShapeType(HeeksObj* body)
{
	// 0 - TopAbs_COMPOUND
	// 1 - TopAbs_COMPSOLID
	// 2 - TopAbs_SOLID
	// 3 - TopAbs_SHELL
	// 4 - TopAbs_FACE
	// 5 - TopAbs_WIRE
	// 6 - TopAbs_EDGE
	// 7 - TopAbs_VERTEX
	// 8 - TopAbs_SHAPE
	return ((CShape*)body)->Shape().ShapeType();
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

HeeksObj* CHeeksCADInterface::EdgeGetVertex0(HeeksObj* edge)
{
	return ((CEdge*)edge)->GetVertex0();
}

HeeksObj* CHeeksCADInterface::EdgeGetVertex1(HeeksObj* edge)
{
	return ((CEdge*)edge)->GetVertex1();
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

bool CHeeksCADInterface::EdgeGetEllipseParams(HeeksObj* edge, double* d11)
{
	return ((CEdge*)edge)->GetEllipseParams(d11);
}

void CHeeksCADInterface::EdgeSetTempAttribute(HeeksObj* edge, int attr)
{
	((CEdge*)edge)->m_temp_attr = attr;
}

int CHeeksCADInterface::EdgeGetTempAttribute(HeeksObj* edge)
{
	return ((CEdge*)edge)->m_temp_attr;
}

double CHeeksCADInterface::EdgeGetLength(HeeksObj* edge)
{
	return ((CEdge*)edge)->Length();
}

double CHeeksCADInterface::EdgeGetLength2(HeeksObj* edge, double uStart, double uEnd)
{
	return ((CEdge*)edge)->Length2(uStart, uEnd);
}

bool CHeeksCADInterface::EdgeGetClosestPoint(HeeksObj* edge, const double *pos, double *closest_pnt, double &u)
{
	gp_Pnt cp;
	if(((CEdge*)edge)->GetClosestPoint(make_point(pos), cp, u))
	{
		extract(cp, closest_pnt);
		return true;
	}
	return false;
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

void CHeeksCADInterface::VertexGetPoint(HeeksObj* vertex, double *d3)
{
	memcpy(d3, ((CVertex*)vertex)->m_point, 3*sizeof(double));
}

HeeksObj* CHeeksCADInterface::VertexGetFirstEdge(HeeksObj* vertex)
{
	return ((CVertex*)vertex)->GetFirstEdge();
}

HeeksObj* CHeeksCADInterface::VertexGetNextEdge(HeeksObj* vertex)
{
	return ((CVertex*)vertex)->GetNextEdge();
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

void CHeeksCADInterface::RegisterOnSaveFn( void(*callbackfunc)(bool) )
{
	wxGetApp().RegisterOnSaveFn(callbackfunc);
}

void CHeeksCADInterface::RegisterIsModifiedFn( bool(*callbackfunc)() )
{
	wxGetApp().RegisterIsModifiedFn(callbackfunc);
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

HeeksObj* CHeeksCADInterface::NewSTLSolid()
{
	HeeksColor col(220, 40, 40);
	return new CStlSolid(&col);
}

void CHeeksCADInterface::STLSolidAddTriangle(HeeksObj* stl_solid, float* t)
{
	((CStlSolid*)stl_solid)->AddTriangle(t);
}

const HeeksColor& CHeeksCADInterface::GetBackgroundColor()
{
	return wxGetApp().background_color[0];
}

void CHeeksCADInterface::SetColor(int r, int b, int g)
{
	HeeksColor color(r,b,g);
	wxGetApp().current_color = color;
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
		config.Write(_T("ViewUnits"), wxGetApp().m_view_units);
	}
}

void CHeeksCADInterface::SplineToBiarcs(HeeksObj* spline, std::list<HeeksObj*> &new_spans, double tolerance)
{
	((HSpline*)spline)->ToBiarcs(new_spans, tolerance);
}

bool CHeeksCADInterface::Intersect(const gp_Lin& lin, const gp_Lin& lin2, gp_Pnt &pnt)
{
	printf("CHeeksCADInterface::intersect(line,line) called\n");
	// Call the one in the Geom module.
	return( intersect( lin, lin2, pnt ) );
}

bool CHeeksCADInterface::Intersect(const gp_Pnt& pnt, const gp_Lin& lin)
{
	// Call the one in the Geom module.
	return( intersect( pnt, lin ) );
}

bool CHeeksCADInterface::Intersect(const gp_Pnt& pnt, const gp_Circ& cir)
{
	// Call the one in the Geom module.
	return( intersect( pnt, cir ) );
}

void CHeeksCADInterface::Intersect(const gp_Lin& line, const gp_Circ& circle, std::list<gp_Pnt> &list)
{
	// Call the one in the Geom module.
	intersect( line, circle, list );
}

void CHeeksCADInterface::Intersect(const gp_Circ& c1, const gp_Circ& c2, std::list<gp_Pnt> &list)
{
	// Call the one in the Geom module.
	intersect( c1, c2, list );
}

void CHeeksCADInterface::RegisterOnBuildTexture( void(*callbackfunc)() )
{
	wxGetApp().RegisterOnBuildTexture(callbackfunc);
}

int CHeeksCADInterface::LoadIconsTexture(const wxChar *filepath){
    int width, height, textureWidth, textureHeight;
	unsigned int* t = loadImage(filepath, &width, &height, &textureWidth, &textureHeight);
	if(t)return *t;
	return 0;
}


/**
    This method accepts a file pointer that will handle all the file open and import calls to
    a file whose extension is within the list provided.  If a handler is already registered for one
    of these extensions then the registration fails and a 'false' status is returned.
 */
bool CHeeksCADInterface::RegisterFileOpenHandler( const std::list<wxString> file_extensions, void (*fileopen_handler)(const wxChar *path) )
{
    return( wxGetApp().RegisterFileOpenHandler( file_extensions, fileopen_handler ));
}

bool CHeeksCADInterface::UnregisterFileOpenHandler( void (*fileopen_handler)(const wxChar *path) )
{
    return(wxGetApp().UnregisterFileOpenHandler( fileopen_handler ));
}
