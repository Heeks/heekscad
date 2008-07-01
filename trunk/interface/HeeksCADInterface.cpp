// HeeksCADInterface.cpp

// include this in your dynamic library to interface with HeeksCAD

#include "stdafx.h"

#include <wx/dynlib.h>

HeeksCADInterface::HeeksCADInterface(const char* full_path)
{
	m_executable = new wxDynamicLibrary(full_path);
}

HeeksCADInterface::~HeeksCADInterface()
{
	delete m_executable;
}

static double(*HeeksGetTolerance)() = NULL;
static bool HeeksGetTolerance_find = false;

double HeeksCADInterface::GetTolerance()
{
	// get the geometry tolerance from HeeksCAD

	if(!HeeksGetTolerance_find){
		HeeksGetTolerance = (double (*)(void))(m_executable->GetSymbol("HeeksGetTolerance"));
		HeeksGetTolerance_find = true;
	}

	if(HeeksGetTolerance){
		return (*HeeksGetTolerance)();
	}

	return 0.001;
}

static void(*HeeksRefreshProperties)() = NULL;
static bool HeeksRefreshProperties_find = false;

void HeeksCADInterface::RefreshProperties()
{
	// Refresh the properties window in HeeksCAD

	if(!HeeksRefreshProperties_find){
		HeeksRefreshProperties = (void (*)())(m_executable->GetSymbol("HeeksRefreshProperties"));
		HeeksRefreshProperties_find = true;
	}

	if(HeeksRefreshProperties){
		(*HeeksRefreshProperties)();
	}
}

static void(*HeeksRepaint)() = NULL;
static bool HeeksRepaint_find = false;

void HeeksCADInterface::Repaint()
{
	// Refresh the properties window in HeeksCAD

	if(!HeeksRepaint_find){
		HeeksRepaint = (void (*)())(m_executable->GetSymbol("HeeksRepaint"));
		HeeksRepaint_find = true;
	}

	if(HeeksRepaint){
		(*HeeksRepaint)();
	}
}

static wxFrame* (*HeeksGetMainFrame)() = NULL;
static bool HeeksGetMainFrame_find = false;

wxFrame* HeeksCADInterface::GetMainFrame()
{
	// Refresh the properties window in HeeksCAD

	if(!HeeksGetMainFrame_find){
		HeeksGetMainFrame = (wxFrame* (*)())(m_executable->GetSymbol("HeeksGetMainFrame"));
		HeeksGetMainFrame_find = true;
	}

	if(HeeksGetMainFrame){
		return (*HeeksGetMainFrame)();
	}

	return NULL;
}

static wxAuiManager* (*HeeksGetAuiManager)() = NULL;
static bool HeeksGetAuiManager_find = false;

wxAuiManager* HeeksCADInterface::GetAuiManager()
{
	// Refresh the properties window in HeeksCAD

	if(!HeeksGetAuiManager_find){
		HeeksGetAuiManager = (wxAuiManager* (*)())(m_executable->GetSymbol("HeeksGetAuiManager"));
		HeeksGetAuiManager_find = true;
	}

	if(HeeksGetAuiManager){
		return (*HeeksGetAuiManager)();
	}

	return NULL;
}

static void(*HeeksAddToolBarTool)(wxToolBar*, const wxString&, wxBitmap&, const wxString&, void(*)(wxCommandEvent&)) = NULL;
static bool HeeksAddToolBarTool_find = false;

void HeeksCADInterface::Bastart(wxToolBar* toolbar, const wxString& title, wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&))
{
	// Refresh the properties window in HeeksCAD

	if(!HeeksAddToolBarTool_find){
		HeeksAddToolBarTool = (void (*)(wxToolBar*, const wxString&, wxBitmap&, const wxString&, void(*)(wxCommandEvent&)))(m_executable->GetSymbol("HeeksAddToolBarTool"));
		HeeksAddToolBarTool_find = true;
	}

	if(HeeksAddToolBarTool){
		(*HeeksAddToolBarTool)(toolbar, title, bitmap, caption, onButtonFunction);
	}
}