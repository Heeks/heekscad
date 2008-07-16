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
#include "GraphicsCanvas.h"
#include "LineArcCollection.h"

double CHeeksCADInterface::GetTolerance()
{
	return wxGetApp().m_geom_tol;
}

void CHeeksCADInterface::RefreshProperties()
{
	wxGetApp().m_frame->m_properties->RefreshByRemovingAndAddingAll();
}

void CHeeksCADInterface::Repaint()
{
	wxGetApp().Repaint();
}

wxFrame* CHeeksCADInterface::GetMainFrame()
{
	return wxGetApp().m_frame;
}

wxAuiManager* CHeeksCADInterface::GetAuiManager()
{
	return wxGetApp().m_frame->m_aui_manager;
}

void CHeeksCADInterface::AddToolBarButton(wxToolBar* toolbar, const wxString& title, wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&))
{
	wxGetApp().m_frame->AddToolBarTool(toolbar, title, bitmap, caption, onButtonFunction);
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

bool CHeeksCADInterface::GetArcDirection(HeeksObj* object)
{
	return ((HArc*)object)->m_dir;
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

void CHeeksCADInterface::WasModified(HeeksObj* object)
{
	wxGetApp().WasModified(object);
}

void CHeeksCADInterface::WasAdded(HeeksObj* object)
{
	wxGetApp().WasAdded(object);
}

int CHeeksCADInterface::PickObjects(const char* str)
{
	return wxGetApp().PickObjects(str);
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

HeeksObj* CHeeksCADInterface::GetLineArcCollection(int id)
{
	return CLineArcCollection::GetLineArcCollection(id);
}

HeeksObj* CHeeksCADInterface::NewLineArcCollection()
{
	return new CLineArcCollection;
}

int CHeeksCADInterface::GetLineArcCollectionID(HeeksObj* la)
{
	return ((CLineArcCollection*)la)->m_id;
}

void CHeeksCADInterface::SetLineArcCollectionID(HeeksObj* la, int id)
{
	CLineArcCollection::used_ids.erase(((CLineArcCollection*)la)->m_id);
	((CLineArcCollection*)la)->m_id = id;
	CLineArcCollection::used_ids.insert( std::pair<int, CLineArcCollection*>(id, (CLineArcCollection*)la) );
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
