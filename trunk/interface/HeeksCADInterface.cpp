// HeeksCADInterface.cpp

// included only in the executable

#include "stdafx.h"
#include "HeeksCADInterface.h"
#include "HeeksFrame.h"
#include "PropertiesCanvas.h"
#include "MarkedList.h"
#include "HLine.h"
#include "HArc.h"
#include "ObjPropsCanvas.h"
#include "OptionsCanvas.h"
#include "LeftCanvas.h"
#include "InputModeCanvas.h"
#include "GraphicsCanvas.h"
#include "LineArcCollection.h"
#include "DigitizeMode.h"
#include "SelectMode.h"
#include "Shape.h"
#include "Face.h"
#include "Edge.h"

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

wxFrame* CHeeksCADInterface::GetMainFrame()
{
	return wxGetApp().m_frame;
}

wxWindow* CHeeksCADInterface::GetGraphicsCanvas()
{
	return wxGetApp().m_frame->m_graphics;
}

wxMenuBar* CHeeksCADInterface::GetMenuBar()
{
	return wxGetApp().m_frame->m_menuBar;
}

wxMenu* CHeeksCADInterface::GetViewMenu()
{
	return wxGetApp().m_frame->m_menuView;
}

wxWindow* CHeeksCADInterface::GetObjectsCanvas()
{
	return wxGetApp().m_frame->m_left;
}

wxAuiManager* CHeeksCADInterface::GetAuiManager()
{
	return wxGetApp().m_frame->m_aui_manager;
}

void CHeeksCADInterface::AddToolBarButton(wxToolBar* toolbar, const wxString& title, wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&))
{
	wxGetApp().m_frame->AddToolBarTool(toolbar, title, bitmap, caption, onButtonFunction, onUpdateButtonFunction);
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
	wxGetApp().m_marked_list->Add(object);
}

bool CHeeksCADInterface::ObjectMarked(HeeksObj* object)
{
	return wxGetApp().m_marked_list->ObjectMarked(object);
}

void CHeeksCADInterface::ClearMarkedList()
{
	wxGetApp().m_marked_list->Clear();
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

int CHeeksCADInterface::PickObjects(const wxChar* str)
{
	return wxGetApp().PickObjects(str);
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

HeeksObj* CHeeksCADInterface::NewLineArcCollection()
{
	return new CLineArcCollection;
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

HeeksObj* CHeeksCADInterface::GetIDObject(int type, int id)
{
	return wxGetApp().GetIDObject(type, id);
}

void CHeeksCADInterface::SetObjectID(HeeksObj* object, int id)
{
	wxGetApp().SetObjectID(object, id);
}

void CHeeksCADInterface::WriteIDToXML(HeeksObj* object, TiXmlElement *element)
{
	wxGetApp().WriteIDToXML(object, element);
}

void CHeeksCADInterface::ReadIDFromXML(HeeksObj* object, TiXmlElement *element)
{
	wxGetApp().ReadIDFromXML(object, element);
}

int CHeeksCADInterface::GetNextID(int type)
{
	return wxGetApp().GetNextID(type);
}

bool CHeeksCADInterface::GetDisableSetObjectIDOnAdd()
{
	return wxGetApp().m_disable_SetObjectID_on_Add;
}

void CHeeksCADInterface::RemoveID(HeeksObj* object)
{
	wxGetApp().RemoveID(object);
}

const wxChar* CHeeksCADInterface::GetFileFullPath()
{
	return wxGetApp().m_filepath;
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

void CHeeksCADInterface::FaceGetNormalAtUV(HeeksObj* face, double u, double v, double* norm)
{
	gp_Dir n = ((CFace*)face)->GetNormalAtUV(u, v);
	extract(n, norm);
}

bool CHeeksCADInterface::FaceGetUVAtPoint(HeeksObj* face, const double *pos, double *u, double *v)
{
	return ((CFace*)face)->GetUVAtPoint(make_point(pos), u, v);
}

void CHeeksCADInterface::FaceGetPlaneParams(HeeksObj* face, double *d, double *norm)
{
	gp_Pln p;
	((CFace*)face)->GetPlaneParams(p);

	extract(p.Axis().Direction(), norm);
	*d = -(gp_Vec(p.Axis().Direction().XYZ()) * gp_Vec(p.Axis().Location().XYZ()));
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
