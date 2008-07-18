// HeeksFrame.cpp

#include "stdafx.h"
#include "HeeksFrame.h"
#include "GraphicsCanvas.h"
#include "LeftCanvas.h"
#include "ObjPropsCanvas.h"
#include "OptionsCanvas.h"
#include "InputModeCanvas.h"
#include "LineArcDrawing.h"
#include "Shape.h"
#include "MarkedList.h"
#include "MagDragWindow.h"
#include "HArc.h"
#include "../interface/Tool.h"

enum{
	ID_LINES = 1,
	ID_VIEWING,
	ID_SUBTRACT,
	ID_SPHERE,
	ID_CUBE,
	ID_CYL,
	ID_REDRAW,
	ID_FUSE,
	ID_COMMON,
	ID_VEWING,
	ID_MAG,
	ID_MAG_EXTENTS,
	ID_MAG_NO_ROT,
	ID_MAG_PREVIOUS,
	ID_UNDO,
	ID_REDO,
	ID_NEXT_ID
};

BEGIN_EVENT_TABLE( CHeeksFrame, wxFrame )
EVT_MENU( Menu_File_Quit, CHeeksFrame::OnQuit )
EVT_MENU( Menu_File_About, CHeeksFrame::OnAbout )
EVT_MENU( Menu_View_Objects, CHeeksFrame::OnViewObjects )
EVT_UPDATE_UI(Menu_View_Objects, CHeeksFrame::OnUpdateViewObjects)
EVT_MENU( Menu_View_Options, CHeeksFrame::OnViewOptions )
EVT_UPDATE_UI(Menu_View_Options, CHeeksFrame::OnUpdateViewOptions)
EVT_MENU( Menu_View_Input, CHeeksFrame::OnViewInput )
EVT_UPDATE_UI(Menu_View_Input, CHeeksFrame::OnUpdateViewInput)
EVT_MENU( Menu_View_Properties, CHeeksFrame::OnViewProperties )
EVT_UPDATE_UI(Menu_View_Properties, CHeeksFrame::OnUpdateViewProperties)
EVT_MENU( Menu_View_ToolBar, CHeeksFrame::OnViewToolBar )
EVT_UPDATE_UI(Menu_View_ToolBar, CHeeksFrame::OnUpdateViewToolBar)
EVT_MENU( Menu_View_SolidBar, CHeeksFrame::OnViewSolidBar )
EVT_UPDATE_UI(Menu_View_SolidBar, CHeeksFrame::OnUpdateViewSolidBar)
EVT_MENU( Menu_View_ViewingBar, CHeeksFrame::OnViewViewingBar )
EVT_UPDATE_UI(Menu_View_ViewingBar, CHeeksFrame::OnUpdateViewViewingBar)
EVT_MENU( Menu_View_StatusBar, CHeeksFrame::OnViewStatusBar )
EVT_UPDATE_UI(Menu_View_StatusBar, CHeeksFrame::OnUpdateViewStatusBar)
EVT_MENU(wxID_OPEN, CHeeksFrame::OnOpenButton)
EVT_MENU(wxID_SAVE, CHeeksFrame::OnSaveButton)
EVT_MENU(wxID_NEW, CHeeksFrame::OnNewButton)
EVT_MENU(ID_LINES, CHeeksFrame::OnLinesButton)
EVT_MENU(ID_VIEWING, CHeeksFrame::OnViewingButton)
EVT_MENU(ID_SPHERE, CHeeksFrame::OnSphereButton)
EVT_MENU(ID_CUBE, CHeeksFrame::OnCubeButton)
EVT_MENU(ID_CYL, CHeeksFrame::OnCylButton)
EVT_MENU(ID_SUBTRACT, CHeeksFrame::OnSubtractButton)
EVT_MENU(ID_FUSE, CHeeksFrame::OnFuseButton)
EVT_MENU(ID_COMMON, CHeeksFrame::OnCommonButton)
EVT_MENU(ID_REDRAW, CHeeksFrame::OnRedrawButton)
EVT_MENU(ID_MAG, CHeeksFrame::OnMagButton)
EVT_MENU(ID_UNDO, CHeeksFrame::OnUndoButton)
EVT_MENU(ID_REDO, CHeeksFrame::OnRedoButton)
EVT_MENU(ID_MAG_EXTENTS, CHeeksFrame::OnMagExtentsButton)
EVT_MENU(ID_MAG_NO_ROT, CHeeksFrame::OnMagNoRotButton)
EVT_MENU(ID_MAG_PREVIOUS, CHeeksFrame::OnMagPreviousButton)
EVT_MENU_RANGE(ID_NEXT_ID, ID_NEXT_ID + 1000, CHeeksFrame::OnExternalButton)
EVT_UPDATE_UI_RANGE(ID_NEXT_ID, ID_NEXT_ID + 1000, CHeeksFrame::OnUpdateExternalButton)
EVT_SIZE(CHeeksFrame::OnSize)
EVT_MOVE(CHeeksFrame::OnMove)
END_EVENT_TABLE()

CHeeksFrame::CHeeksFrame( const wxString& title, const wxPoint& pos, const wxSize& size )
	: wxFrame((wxWindow *)NULL, -1, title, pos, size)
{
	wxGetApp().m_frame = this;
	m_next_id_for_button = ID_NEXT_ID;

	// File Menu
	wxMenu *menuFile = new wxMenu;
	menuFile->Append( Menu_File_About, wxT( "&About..." ) );
	menuFile->AppendSeparator();
	menuFile->Append( Menu_File_Quit, wxT( "E&xit" ) );
	
	// View Menu
	m_menuView = new wxMenu;
	m_menuView->AppendCheckItem( Menu_View_Objects, wxT( "&Objects" ) );
	m_menuView->AppendCheckItem( Menu_View_Options, wxT( "Optio&ns" ) );
	m_menuView->AppendCheckItem( Menu_View_Input, wxT( "&Input" ) );
	m_menuView->AppendCheckItem( Menu_View_Properties, wxT( "&Properties" ) );
	m_menuView->AppendCheckItem( Menu_View_ToolBar, wxT( "&Tool Bar" ) );
	m_menuView->AppendCheckItem( Menu_View_SolidBar, wxT( "&Solids Tool Bar" ) );
	m_menuView->AppendCheckItem( Menu_View_ViewingBar, wxT( "&Viewing Tool Bar" ) );
	m_menuView->AppendCheckItem( Menu_View_StatusBar, wxT( "St&atus Bar" ) );
	
	// Add them to the main menu
	m_menuBar = new wxMenuBar;
	m_menuBar->Append( menuFile, wxT( "&File" ) );
	SetMenuBar( m_menuBar );
	m_menuBar->Append( m_menuView, wxT( "&View" ) );

	m_aui_manager = new wxAuiManager(this);

	int graphics_attrib_list[] = {
		WX_GL_RGBA,
		1,
		WX_GL_DOUBLEBUFFER,
		1,
		WX_GL_DEPTH_SIZE,
		1,
		WX_GL_MIN_RED,
		1,
		WX_GL_MIN_GREEN,
		1,
		WX_GL_MIN_BLUE,
		1,
		WX_GL_MIN_ALPHA,
		0,
		0
	};

    m_graphics = new CGraphicsCanvas(this, graphics_attrib_list);

    m_left = new CLeftCanvas(this);
    m_left->SetCursor(wxCursor(wxCURSOR_MAGNIFIER));

    m_options = new COptionsCanvas(this);
	m_input_canvas = new CInputModeCanvas(this);

    m_properties = new CObjPropsCanvas(this);

	m_statusBar = CreateStatusBar();
	SetStatusText( wxT( "" ) );

	m_toolBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
	m_toolBar->SetToolBitmapSize(wxSize(32, 32));

	m_solidBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
	m_solidBar->SetToolBitmapSize(wxSize(32, 32));

	m_viewingBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
	m_viewingBar->SetToolBitmapSize(wxSize(32, 32));

	wxString exe_folder = wxGetApp().GetExeFolder();

	// main tool bar
    m_toolBar->AddTool(wxID_NEW, _T("New"), wxBitmap(exe_folder + "/bitmaps/new.png", wxBITMAP_TYPE_PNG), _T("New file"));
    m_toolBar->AddTool(wxID_OPEN, _T("Open"), wxBitmap(exe_folder + "/bitmaps/open.png", wxBITMAP_TYPE_PNG), _T("Open file"));
    m_toolBar->AddTool(wxID_SAVE, _T("Save"), wxBitmap(exe_folder + "/bitmaps/save.png", wxBITMAP_TYPE_PNG), _T("Save file"));
    m_toolBar->AddTool(ID_UNDO, _T("Undo"), wxBitmap(exe_folder + "/bitmaps/undo.png", wxBITMAP_TYPE_PNG), _T("Undo the previous command"));
    m_toolBar->AddTool(ID_REDO, _T("Redo"), wxBitmap(exe_folder + "/bitmaps/redo.png", wxBITMAP_TYPE_PNG), _T("Redo the next command"));
    m_toolBar->AddTool(ID_VIEWING, _T("Select"), wxBitmap(exe_folder + "/bitmaps/select.png", wxBITMAP_TYPE_PNG), _T("Select Mode"));
    m_toolBar->AddTool(ID_LINES, _T("Lines"), wxBitmap(exe_folder + "/bitmaps/lines.png", wxBITMAP_TYPE_PNG), _T("Start Line Drawing"));
    m_toolBar->Realize();

	// Solids tool bar
    m_solidBar->AddTool(ID_SPHERE, _T("Sphere"), wxBitmap(exe_folder + "/bitmaps/sphere.png", wxBITMAP_TYPE_PNG), _T("Add a sphere"));
    m_solidBar->AddTool(ID_CUBE, _T("Cube"), wxBitmap(exe_folder + "/bitmaps/cube.png", wxBITMAP_TYPE_PNG), _T("Add a cube"));
    m_solidBar->AddTool(ID_CYL, _T("Cylinder"), wxBitmap(exe_folder + "/bitmaps/cyl.png", wxBITMAP_TYPE_PNG), _T("Add a cylinder"));
	m_solidBar->AddTool(ID_SUBTRACT, _T("Cut"), wxBitmap(exe_folder + "/bitmaps/cut.png", wxBITMAP_TYPE_PNG), _T("Cut one solid from another"));
	m_solidBar->AddTool(ID_FUSE, _T("Fuse"), wxBitmap(exe_folder + "/bitmaps/fuse.png", wxBITMAP_TYPE_PNG), _T("Fuse one solid to another"));
	m_solidBar->AddTool(ID_COMMON, _T("Common"), wxBitmap(exe_folder + "/bitmaps/common.png", wxBITMAP_TYPE_PNG), _T("Find common solid between two solids"));
    m_solidBar->AddTool(ID_REDRAW, _T("Redraw"), wxBitmap(exe_folder + "/bitmaps/redraw.png", wxBITMAP_TYPE_PNG), _T("Redraw"));
	m_solidBar->Realize();

	// viewing tool bar
	m_viewingBar->AddTool(ID_MAG_PREVIOUS, _T("View Back"), wxBitmap(exe_folder + "/bitmaps/magprev.png", wxBITMAP_TYPE_PNG), _T("Go back to previous view"));
	m_viewingBar->AddTool(ID_MAG, _T("Zoom Window"), wxBitmap(exe_folder + "/bitmaps/mag.png", wxBITMAP_TYPE_PNG), _T("Zoom in to a dragged window"));
	m_viewingBar->AddTool(ID_MAG_EXTENTS, _T("Mag Extents"), wxBitmap(exe_folder + "/bitmaps/magextents.png", wxBITMAP_TYPE_PNG), _T("Zoom in to fit the extents of the drawing into the graphics window"));
	m_viewingBar->AddTool(ID_MAG_NO_ROT, _T("Mag No Rotation"), wxBitmap(exe_folder + "/bitmaps/magnorot.png", wxBITMAP_TYPE_PNG), _T("Zoom in to fit the extents of the drawing into the graphics window, but without rotating the view"));
	m_viewingBar->Realize();

	bool objects_visible = true;
	bool options_visible = true;
	bool input_canvas_visible = true;
	bool properties_visible = true;
	bool toolbar_visible = true;
	bool solidbar_visible = true;
	bool viewingbar_visible = true;
	bool statusbar_visible = true;

	wxGetApp().m_config->Read("FrameObjectsVisible", &objects_visible);
	wxGetApp().m_config->Read("FrameOptionsVisible", &options_visible);
	wxGetApp().m_config->Read("FrameInputVisible", &input_canvas_visible);
	wxGetApp().m_config->Read("FramePropertiesVisible", &properties_visible);
	wxGetApp().m_config->Read("FrameToolBarVisible", &toolbar_visible);
	wxGetApp().m_config->Read("FrameSolidBarVisible", &solidbar_visible);
	wxGetApp().m_config->Read("FrameViewingBarVisible", &viewingbar_visible);
	wxGetApp().m_config->Read("FrameStatusBarVisible", &statusbar_visible);

	m_aui_manager->AddPane(m_graphics, wxAuiPaneInfo().Name("Graphics").Caption("Graphics").CentrePane().BestSize(wxSize(800, 600)));
	m_aui_manager->AddPane(m_left, wxAuiPaneInfo().Name("Objects").Caption("Objects").Left().BestSize(wxSize(300, 400)));
	m_aui_manager->AddPane(m_options, wxAuiPaneInfo().Name("Options").Caption("Options").Left().BestSize(wxSize(300, 200)));
	m_aui_manager->AddPane(m_input_canvas, wxAuiPaneInfo().Name("Input").Caption("Input").Left().BestSize(wxSize(300, 200)));
	m_aui_manager->AddPane(m_properties, wxAuiPaneInfo().Name("Properties").Caption("Properties").Left().BestSize(wxSize(300, 200)));
	m_aui_manager->AddPane(m_toolBar, wxAuiPaneInfo().Name("ToolBar").Caption("General Tools").ToolbarPane().Top());
	m_aui_manager->AddPane(m_solidBar, wxAuiPaneInfo().Name("SolidBar").Caption("Solid Tools").ToolbarPane().Top());
	m_aui_manager->AddPane(m_viewingBar, wxAuiPaneInfo().Name("ViewingBar").Caption("Viewing Tools").ToolbarPane().Top());

	m_aui_manager->GetPane(m_left).Show(objects_visible);
	m_aui_manager->GetPane(m_options).Show(options_visible);
	m_aui_manager->GetPane(m_input_canvas).Show(input_canvas_visible);
	m_aui_manager->GetPane(m_properties).Show(properties_visible);
	m_aui_manager->GetPane(m_toolBar).Show(toolbar_visible);
	m_aui_manager->GetPane(m_solidBar).Show(solidbar_visible);
	m_aui_manager->GetPane(m_viewingBar).Show(viewingbar_visible);
	m_statusBar->Show(statusbar_visible);

	// load up any other dlls and call OnStartUp on each of them
	{
		::wxSetWorkingDirectory(wxGetApp().GetExeFolder());

		ifstream ifs("AddIns.txt");
		char str[1024];
		if(!(!ifs)){
			while(!ifs.eof()){
				ifs.getline(str, 1024);

				// strip white space
				wxString wstr(str);
				wstr = wstr.Trim();
				wstr = wstr.Trim(false);

				if(wstr.Len() == 0)continue;

				if(wstr[0] == '#')continue;

				wxDynamicLibrary* shared_library = new wxDynamicLibrary(wstr);
				if(shared_library->IsLoaded()){
					wxGetApp().m_loaded_libraries.push_back(shared_library);
					void(*OnStartUp)() = (void (*)())(shared_library->GetSymbol("OnStartUp"));
					(*OnStartUp)();
				}
				else{
					delete shared_library;
				}
			}
		}
	}

	wxGetApp().OnNewOrOpen();

	m_aui_manager->Update();
}

CHeeksFrame::~CHeeksFrame()
{
	wxGetApp().m_config->Write("FrameObjectsVisible", m_aui_manager->GetPane(m_left).IsShown());
	wxGetApp().m_config->Write("FrameOptionsVisible", m_aui_manager->GetPane(m_options).IsShown());
	wxGetApp().m_config->Write("FrameInputVisible", m_aui_manager->GetPane(m_input_canvas).IsShown());
	wxGetApp().m_config->Write("FramePropertiesVisible", m_aui_manager->GetPane(m_properties).IsShown());
	wxGetApp().m_config->Write("FrameToolBarVisible", m_aui_manager->GetPane(m_toolBar).IsShown());
	wxGetApp().m_config->Write("FrameSolidBarVisible", m_aui_manager->GetPane(m_solidBar).IsShown());
	wxGetApp().m_config->Write("FrameViewingBarVisible", m_aui_manager->GetPane(m_viewingBar).IsShown());
	wxGetApp().m_config->Write("FrameStatusBarVisible", m_statusBar->IsShown());

	// call the shared libraries function OnFrameDelete, so they can write profiel strings while aui manager still exists
	for(std::list<wxDynamicLibrary*>::iterator It = wxGetApp().m_loaded_libraries.begin(); It != wxGetApp().m_loaded_libraries.end(); It++){
		wxDynamicLibrary* shared_library = *It;
		void(*OnFrameDelete)() = (void(*)())(shared_library->GetSymbol("OnFrameDelete"));
		if(OnFrameDelete)(*OnFrameDelete)();
	}

	delete m_aui_manager;
}

bool CHeeksFrame::ShowFullScreen(bool show, long style){
	static bool objects_visible = true;
	static bool options_visible = true;
	static bool input_visible = true;
	static bool properties_visible = true;
	static bool toolbar_visible = true;
	static bool solidbar_visible = true;
	static bool viewingbar_visible = true;
	static bool statusbar_visible = true;

	static std::map< wxWindow*, bool > windows_visible;

	if(show){
		SetMenuBar(NULL);
		objects_visible = m_aui_manager->GetPane(m_left).IsShown();
		options_visible = m_aui_manager->GetPane(m_options).IsShown();
		input_visible = m_aui_manager->GetPane(m_input_canvas).IsShown();
		properties_visible = m_aui_manager->GetPane(m_properties).IsShown();
		toolbar_visible = m_aui_manager->GetPane(m_toolBar).IsShown();
		solidbar_visible = m_aui_manager->GetPane(m_solidBar).IsShown();
		viewingbar_visible = m_aui_manager->GetPane(m_viewingBar).IsShown();
		statusbar_visible = m_statusBar->IsShown();
		m_aui_manager->GetPane(m_left).Show(false);
		m_aui_manager->GetPane(m_options).Show(false);
		m_aui_manager->GetPane(m_input_canvas).Show(false);
		m_aui_manager->GetPane(m_properties).Show(false);
		m_aui_manager->GetPane(m_toolBar).Show(false);
		m_aui_manager->GetPane(m_solidBar).Show(false);
		m_aui_manager->GetPane(m_viewingBar).Show(false);
		m_statusBar->Show(false);
		for(std::list<wxWindow*>::iterator It = wxGetApp().m_hideable_windows.begin(); It != wxGetApp().m_hideable_windows.end(); It++)
		{
			wxWindow* w = *It;
			windows_visible.insert(std::pair< wxWindow*, bool > (w, w->IsShown()));
			m_aui_manager->GetPane(w).Show(false);
		}
	}
	else{
		SetMenuBar(m_menuBar);
		m_aui_manager->GetPane(m_left).Show(objects_visible);
		m_aui_manager->GetPane(m_options).Show(options_visible);
		m_aui_manager->GetPane(m_input_canvas).Show(input_visible);
		m_aui_manager->GetPane(m_properties).Show(properties_visible);
		m_aui_manager->GetPane(m_toolBar).Show(toolbar_visible);
		m_aui_manager->GetPane(m_solidBar).Show(solidbar_visible);
		m_aui_manager->GetPane(m_viewingBar).Show(viewingbar_visible);
		m_statusBar->Show(statusbar_visible);
		for(std::list<wxWindow*>::iterator It = wxGetApp().m_hideable_windows.begin(); It != wxGetApp().m_hideable_windows.end(); It++)
		{
			wxWindow* w = *It;
			std::map< wxWindow*, bool >::iterator FindIt = windows_visible.find(w);
			if(FindIt != windows_visible.end()){
				bool visible = FindIt->second;
				m_aui_manager->GetPane(w).Show(visible);
			}
		}
	}

	bool res =  wxTopLevelWindow::ShowFullScreen(show, style);
	m_aui_manager->Update();
	return res;
}

void 
CHeeksFrame::OnQuit( wxCommandEvent& WXUNUSED( event ) )
{
	Close(TRUE);
}

void 
CHeeksFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
	wxMessageBox( wxT( "HeeksCAD" ),
			wxT( "Version 0.1 October 2007" ), wxOK | wxICON_INFORMATION, this );
}

void CHeeksFrame::OnViewObjects( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = m_aui_manager->GetPane(m_left);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		m_aui_manager->Update();
	}
}

void CHeeksFrame::OnUpdateViewObjects( wxUpdateUIEvent& event )
{
	event.Check(m_aui_manager->GetPane(m_left).IsShown());
}

void CHeeksFrame::OnViewOptions( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = m_aui_manager->GetPane(m_options);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		m_aui_manager->Update();
	}
}

void CHeeksFrame::OnUpdateViewOptions( wxUpdateUIEvent& event )
{
	event.Check(m_aui_manager->GetPane(m_options).IsShown());
}

void CHeeksFrame::OnViewInput( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = m_aui_manager->GetPane(m_input_canvas);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		m_aui_manager->Update();
	}
}

void CHeeksFrame::OnUpdateViewInput( wxUpdateUIEvent& event )
{
	event.Check(m_aui_manager->GetPane(m_input_canvas).IsShown());
}

void CHeeksFrame::OnViewToolBar( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = m_aui_manager->GetPane(m_toolBar);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		m_aui_manager->Update();
	}
}

void CHeeksFrame::OnUpdateViewToolBar( wxUpdateUIEvent& event )
{
	event.Check(m_aui_manager->GetPane(m_toolBar).IsShown());
}

void CHeeksFrame::OnViewSolidBar( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = m_aui_manager->GetPane(m_solidBar);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		m_aui_manager->Update();
	}
}

void CHeeksFrame::OnUpdateViewSolidBar( wxUpdateUIEvent& event )
{
	event.Check(m_aui_manager->GetPane(m_solidBar).IsShown());
}

void CHeeksFrame::OnViewViewingBar( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = m_aui_manager->GetPane(m_viewingBar);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		m_aui_manager->Update();
	}
}

void CHeeksFrame::OnUpdateViewViewingBar( wxUpdateUIEvent& event )
{
	event.Check(m_aui_manager->GetPane(m_viewingBar).IsShown());
}

void CHeeksFrame::OnViewStatusBar( wxCommandEvent& event )
{
	m_statusBar->Show(event.IsChecked());
}

void CHeeksFrame::OnUpdateViewStatusBar( wxUpdateUIEvent& event )
{
	event.Check(m_statusBar->IsShown());
}

void CHeeksFrame::OnViewProperties( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = m_aui_manager->GetPane(m_properties);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		m_aui_manager->Update();
	}
}

void CHeeksFrame::OnUpdateViewProperties( wxUpdateUIEvent& event )
{
	event.Check(m_aui_manager->GetPane(m_properties).IsShown());
}

void 
CHeeksFrame::OnViewingButton( wxCommandEvent& WXUNUSED( event ) )
{
	wxGetApp().SetInputMode((CInputMode*)(wxGetApp().m_select_mode));
}

void 
CHeeksFrame::OnLinesButton( wxCommandEvent& WXUNUSED( event ) )
{
	wxGetApp().SetInputMode(&line_strip);
}

void CHeeksFrame::OnOpenButton( wxCommandEvent& event )
{
    wxFileDialog dialog(this, _T("Open solid data file"), wxEmptyString, wxEmptyString,	_T(wxGetApp().GetKnownFilesWildCardString()));
    dialog.SetDirectory(wxGetHomeDir());
    dialog.CentreOnParent();

    if (dialog.ShowModal() == wxID_OK)
    {
		wxGetApp().Reset();
		wxGetApp().OpenFile( dialog.GetPath().c_str() );
		wxGetApp().OnNewOrOpen();
    }
}

void CHeeksFrame::OnSaveButton( wxCommandEvent& event )
{
	wxString default_file = wxString(wxGetApp().m_filepath).BeforeLast('.');
	if(default_file.size() == 0)default_file = wxString("Untitled");
	default_file = default_file + wxString(".step");

	wxFileDialog fd(this, _T("Save graphical data file"), wxEmptyString, default_file, wxGetApp().GetKnownFilesWildCardString(), wxSAVE|wxOVERWRITE_PROMPT);

	fd.SetFilterIndex(1);

    if (fd.ShowModal() == wxID_OK)
    {
        wxGetApp().SaveFile( fd.GetPath().c_str() );
    }
}

void CHeeksFrame::OnUndoButton( wxCommandEvent& event )
{
	wxGetApp().RollBack();
	wxGetApp().Repaint();
}

void CHeeksFrame::OnRedoButton( wxCommandEvent& event )
{
	wxGetApp().RollForward();
	wxGetApp().Repaint();
}

void CHeeksFrame::OnNewButton( wxCommandEvent& event )
{
	wxGetApp().Reset();
	wxGetApp().OnNewOrOpen();
	wxGetApp().Repaint();
}

void CHeeksFrame::OnSubtractButton( wxCommandEvent& event )
{
	CShape::CutShapes(wxGetApp().m_marked_list->list());
}

void CHeeksFrame::OnFuseButton( wxCommandEvent& event )
{
	CShape::FuseShapes(wxGetApp().m_marked_list->list());
}

void CHeeksFrame::OnCommonButton( wxCommandEvent& event )
{
	CShape::CommonShapes(wxGetApp().m_marked_list->list());
}

void CHeeksFrame::OnSphereButton( wxCommandEvent& event )
{
	CShape::AddASphere();
}

void CHeeksFrame::OnCubeButton( wxCommandEvent& event )
{
	CShape::AddACube();
}

void CHeeksFrame::OnCylButton( wxCommandEvent& event )
{
	CShape::AddACylinder();
}

void CHeeksFrame::OnRedrawButton( wxCommandEvent& event )
{
	wxGetApp().RecalculateGLLists();
	wxGetApp().Repaint();
}

void CHeeksFrame::OnMagButton( wxCommandEvent& event )
{
	wxGetApp().SetInputMode(wxGetApp().magnification);
}

void CHeeksFrame::OnMagExtentsButton( wxCommandEvent& event )
{
	wxGetApp().m_frame->m_graphics->OnMagExtents(true, true);
}

void CHeeksFrame::OnMagNoRotButton( wxCommandEvent& event )
{
	wxGetApp().m_frame->m_graphics->OnMagExtents(false, true);
}

void CHeeksFrame::OnMagPreviousButton( wxCommandEvent& event )
{
	wxGetApp().m_frame->m_graphics->OnMagPrevious();
}

void CHeeksFrame::OnExternalButton( wxCommandEvent& event )
{
	int id = event.GetId();

	std::map<int, SExternalButtonFunctions >::iterator FindIt = m_external_buttons.find(id);
	if(FindIt != m_external_buttons.end()){
		SExternalButtonFunctions& ebf = FindIt->second;
		(*(ebf.on_button))(event);
	}
}

void CHeeksFrame::OnUpdateExternalButton( wxUpdateUIEvent& event )
{
	int id = event.GetId();

	std::map<int, SExternalButtonFunctions >::iterator FindIt = m_external_buttons.find(id);
	if(FindIt != m_external_buttons.end()){
		SExternalButtonFunctions& ebf = FindIt->second;
		if(ebf.on_update_button)(*(ebf.on_update_button))(event);
	}
}

ofstream* ofs_for_sim = NULL;

void settrifunc(double *x, double *n){
	int type = 1; // add a triangle
	ofs_for_sim->write((char *)(&type), sizeof(int));
	ofs_for_sim->write((char *)(x), 9*sizeof(double));
}

CBox box_for_floodfill;

void floodfill(double area, double *x, double *n){
	if(area < 100)return;
	if(box_for_floodfill.m_valid){
		int type = 2;// flood fill point
		gp_Pnt p(x[0], x[1], x[2]);
		gp_Pnt c = gp_Pnt(p.XYZ() - 1.42 * gp_XYZ(n[0], n[1], n[2]));
		double x[3] = {c.X(), c.Y(), c.Z()};
		ofs_for_sim->write((char *)(&type), sizeof(int));
		ofs_for_sim->write((char *)(x), 3*sizeof(double));
		ofs_for_sim->write((char *)(box_for_floodfill.m_x), 6*sizeof(double));
	}
}

void CHeeksFrame::OnSize( wxSizeEvent& evt )
{
	wxSize size = evt.GetSize();
	int width = size.GetWidth();
	int height = size.GetHeight();
	wxGetApp().m_config->Write("MainFrameWidth", width);
	wxGetApp().m_config->Write("MainFrameHeight", height);
}

void CHeeksFrame::OnMove( wxMoveEvent& evt )
{
	wxPoint pos = GetPosition();
	int posx = pos.x;
	int posy = pos.y;
	wxGetApp().m_config->Write("MainFramePosX", posx);
	wxGetApp().m_config->Write("MainFramePosY", posy);
}

int CHeeksFrame::AddToolBarTool(wxToolBar* toolbar, const wxString& title, wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&))
{
	while(m_external_buttons.find(m_next_id_for_button) != m_external_buttons.end())
	{
		// already used
		m_next_id_for_button++;
	}

	if(m_next_id_for_button > ID_NEXT_ID + 1000)
	{
		// too many button IDs!
		wxMessageBox("too many button IDs!, see CHeeksFrame::AddToolBarTool");
	}

	int id_to_use = m_next_id_for_button;
	toolbar->AddTool(id_to_use, title, bitmap, caption);
	SExternalButtonFunctions ebf;
	ebf.on_button = onButtonFunction;
	ebf.on_update_button = onUpdateButtonFunction;
	m_external_buttons.insert(std::pair<int, SExternalButtonFunctions > ( id_to_use, ebf ));
	m_next_id_for_button++;
	return id_to_use;
}

int CHeeksFrame::AddMenuCheckItem(wxMenu* menu, const wxString& title, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&))
{
	while(m_external_buttons.find(m_next_id_for_button) != m_external_buttons.end())
	{
		// already used
		m_next_id_for_button++;
	}

	if(m_next_id_for_button > ID_NEXT_ID + 1000)
	{
		// too many button IDs!
		wxMessageBox("too many button IDs!, see CHeeksFrame::AddMenuCheckItem");
	}

	int id_to_use = m_next_id_for_button;
	menu->AppendCheckItem(id_to_use, title);
	SExternalButtonFunctions ebf;
	ebf.on_button = onButtonFunction;
	ebf.on_update_button = onUpdateButtonFunction;
	m_external_buttons.insert(std::pair<int, SExternalButtonFunctions > ( id_to_use, ebf ));
	m_next_id_for_button++;
	return id_to_use;
}

static std::map<int, Tool*> tool_map_for_OnTool;

static void OnTool(wxCommandEvent& event)
{
	std::map<int, Tool*>::iterator FindIt = tool_map_for_OnTool.find(event.GetId());
	if(FindIt != tool_map_for_OnTool.end())
	{
		Tool* tool = FindIt->second;
		tool->Run();
	}
}

void CHeeksFrame::AddToolBarTool(wxToolBar* toolbar, Tool* tool)
{
	wxBitmap* bitmap = tool->Bitmap();
	if(bitmap)
	{
		int id_used_for_button = wxGetApp().m_frame->AddToolBarTool(toolbar, _T(tool->GetTitle()), *bitmap, _T(tool->GetToolTip()), OnTool);
		tool_map_for_OnTool.insert( std::pair<int, Tool*> ( id_used_for_button, tool ) );
	}
}

// a class just so I can get at the protected m_tools of wxToolBar
class ToolBarForGettingToolsFrom: public wxToolBar
{
public:
	void GetToolsIdList(std::list<int> &list)
	{
		wxToolBarToolsList::Node *node;
		for ( node = m_tools.GetFirst(); node; node = node->GetNext() )
		{
			wxToolBarToolBase *tool = node->GetData();
			list.push_back(tool->GetId());
		}
	}
};

void CHeeksFrame::ClearToolBar(wxToolBar* m_toolBar)
{
	ToolBarForGettingToolsFrom* toolBar = (ToolBarForGettingToolsFrom*)m_toolBar;

	std::list<int> list;
	toolBar->GetToolsIdList(list);
	for(std::list<int>::iterator It = list.begin(); It != list.end(); It++)
	{
		int id = *It;
		m_external_buttons.erase(id);
		tool_map_for_OnTool.erase(id);
		if(id < m_next_id_for_button)m_next_id_for_button = id;
		m_toolBar->DeleteTool(id);
	}
}

