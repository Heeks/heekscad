// defines all the exported functions for HeeksCAD
#include "stdafx.h"

#include "Interface.h"
#include "HeeksFrame.h"
#include "PropertiesCanvas.h"

double HeeksGetTolerance(void)
{
	return wxGetApp().m_geom_tol;
}

void HeeksRefreshProperties(void)
{
	wxGetApp().m_frame->m_properties->RefreshByRemovingAndAddingAll();
}

void HeeksRepaint(void)
{
	wxGetApp().Repaint();
}

int HeeksGetMainFrame()
{
	return (int)(wxGetApp().m_frame);
}

int HeeksGetAuiManager()
{
	return (int)(wxGetApp().m_frame->m_aui_manager);
}

void HeeksAddToolBarTool(wxToolBar* toolbar, const wxString& title, wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&))
{
	wxGetApp().m_frame->AddToolBarTool(toolbar, title, bitmap, caption, onButtonFunction);
}
