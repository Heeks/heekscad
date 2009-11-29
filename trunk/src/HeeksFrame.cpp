// HeeksFrame.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "HeeksFrame.h"
#include "../interface/Tool.h"
#include "../interface/ToolList.h"
#include "GraphicsCanvas.h"
#include "TreeCanvas.h"
#include "TreeCanvas.h"
#include "ObjPropsCanvas.h"
#include "OptionsCanvas.h"
#include "InputModeCanvas.h"
#include "LineArcDrawing.h"
#include "PointDrawing.h"
#include "RegularShapesDrawing.h"
#include "DimensionDrawing.h"
#include "Shape.h"
#include "MarkedList.h"
#include "MagDragWindow.h"
#include "ViewRotating.h"
#include "ViewZooming.h"
#include "HArc.h"
#include "RuledSurface.h"
#include "Sphere.h"
#include "Cuboid.h"
#include "Cylinder.h"
#include "Cone.h"
#include "HText.h"
#include "TransformTools.h"
#include "SelectMode.h"
#include "CoordinateSystem.h"
#include "HeeksPrintout.h"
#include "../interface/HeeksCADInterface.h"
#include "Plugins.h"
#include "HeeksConfig.h"
#include "AboutBox.h"

using namespace std;

BEGIN_EVENT_TABLE( CHeeksFrame, wxFrame )
EVT_CLOSE(CHeeksFrame::OnClose)
EVT_MENU( Menu_View_ResetLayout, CHeeksFrame::OnResetLayout )
EVT_MENU( Menu_View_SetToolBarsToLeft, CHeeksFrame::OnSetToolBarsToLeft )
EVT_MENU_RANGE(	ID_RECENT_FIRST, ID_RECENT_FIRST + MAX_RECENT_FILES, CHeeksFrame::OnRecentFile)
EVT_MENU_RANGE(ID_FIRST_EXTERNAL_BUTTON, ID_FIRST_POP_UP_MENU_TOOL + 1000, CHeeksFrame::OnExternalButton)
//wx__DECLARE_EVT2(wxEVT_COMMAND_BUTTON_CLICKED, ID_FIRST_EXTERNAL_BUTTON, ID_FIRST_POP_UP_MENU_TOOL + 1000, wxCommandEventHandler(CHeeksFrame::OnExternalButton))
EVT_UPDATE_UI_RANGE(ID_FIRST_EXTERNAL_BUTTON, ID_FIRST_POP_UP_MENU_TOOL + 1000, CHeeksFrame::OnUpdateExternalButton)
EVT_SIZE(CHeeksFrame::OnSize)
EVT_MOVE(CHeeksFrame::OnMove)
EVT_KEY_DOWN(CHeeksFrame::OnKeyDown)
EVT_KEY_UP(CHeeksFrame::OnKeyUp)
END_EVENT_TABLE()

class DnDFile : public wxFileDropTarget
{
public:
    DnDFile(wxFrame *pOwner) { m_pOwner = pOwner; }

    virtual bool OnDropFiles(wxCoord x, wxCoord y,
                             const wxArrayString& filenames);

private:
    wxFrame *m_pOwner;
};

bool DnDFile::OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames)
{
    size_t nFiles = filenames.GetCount();
    for ( size_t n = 0; n < nFiles; n++ )
    {
		wxGetApp().OpenFile(filenames[n]);
    }

    return true;
}

#ifdef WIN32
static wxString default_layout_string = _T("layout2|name=ToolBar;caption=General Tools;state=2108156;dir=1;layer=10;row=0;pos=0;prop=100000;bestw=279;besth=31;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=580;floaty=59;floatw=306;floath=71|name=GeomBar;caption=Geometry Tools;state=2108156;dir=1;layer=10;row=0;pos=290;prop=100000;bestw=254;besth=31;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=299;floaty=128;floatw=281;floath=71|name=SolidBar;caption=Solid Tools;state=2108156;dir=1;layer=10;row=0;pos=814;prop=100000;bestw=254;besth=31;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=290;floaty=152;floatw=281;floath=71|name=ViewingBar;caption=Viewing Tools;state=2108156;dir=1;layer=10;row=0;pos=555;prop=100000;bestw=248;besth=31;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=308;floaty=190;floatw=275;floath=71|name=TransformBar;caption=Transformation Tools;state=2108159;dir=1;layer=10;row=0;pos=812;prop=100000;bestw=217;besth=31;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=1114;floaty=171;floatw=244;floath=71|name=Graphics;caption=Graphics;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=800;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Objects;caption=Objects;state=2099196;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=300;besth=400;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Options;caption=Options;state=2099196;dir=4;layer=1;row=0;pos=1;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Input;caption=Input;state=2099196;dir=4;layer=1;row=0;pos=2;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Properties;caption=Properties;state=2099196;dir=4;layer=1;row=0;pos=3;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|dock_size(5,0,0)=504|dock_size(4,1,0)=234|dock_size(1,10,0)=33|");
#else
static wxString default_layout_string = _T("layout2|name=ToolBar;caption=General Tools;state=2108156;dir=1;layer=10;row=0;pos=0;prop=100000;bestw=326;besth=38;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=GeomBar;caption=Geometry Tools;state=2108156;dir=1;layer=10;row=0;pos=337;prop=100000;bestw=296;besth=38;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=530;floaty=129;floatw=313;floath=62|name=SolidBar;caption=Solid Tools;state=2108156;dir=1;layer=10;row=1;pos=0;prop=100000;bestw=296;besth=38;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=235;floaty=513;floatw=313;floath=62|name=ViewingBar;caption=Viewing Tools;state=2108156;dir=1;layer=10;row=1;pos=307;prop=100000;bestw=290;besth=38;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=628;floaty=117;floatw=307;floath=62|name=TransformBar;caption=Transformation Tools;state=2108159;dir=4;layer=10;row=0;pos=348;prop=100000;bestw=254;besth=38;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=626;floaty=210;floatw=271;floath=62|name=Graphics;caption=Graphics;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=800;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Objects;caption=Objects;state=2099196;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=300;besth=400;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Options;caption=Options;state=2099196;dir=4;layer=1;row=0;pos=1;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Input;caption=Input;state=2099196;dir=4;layer=1;row=0;pos=2;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Properties;caption=Properties;state=2099196;dir=4;layer=1;row=0;pos=3;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|dock_size(5,0,0)=504|dock_size(4,1,0)=234|dock_size(1,10,0)=40|dock_size(1,10,1)=40|");
#endif

CHeeksCADInterface heekscad_interface;

CHeeksFrame::CHeeksFrame( const wxString& title, const wxPoint& pos, const wxSize& size )
	: wxFrame((wxWindow *)NULL, -1, title, pos, size)
{ 
	wxGetApp().m_frame = this;

	m_logger = new wxLogWindow(NULL,_("Trace Log"),false,true);
	wxLog::SetActiveTarget(m_logger);

	m_next_id_for_button = ID_FIRST_EXTERNAL_BUTTON;
	m_printout = NULL;

	MakeMenus();

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

	int bitmap_size = ToolImage::default_bitmap_size;
	HeeksConfig config;
	config.Read(_T("ToolImageSize"), &bitmap_size);
	ToolImage::SetBitmapSize(bitmap_size);

    m_graphics = new CGraphicsCanvas(this, graphics_attrib_list);

	bool perspective = false;
	config.Read(_T("Perspective"), &perspective);
	m_graphics->m_view_point.SetPerspective(perspective);

	m_tree_canvas = new CTreeCanvas(this, graphics_attrib_list);

    m_options = new COptionsCanvas(this);
	m_input_canvas = new CInputModeCanvas(this);

    m_properties = new CObjPropsCanvas(this);

	m_statusBar = CreateStatusBar();

	wxString exe_folder = wxGetApp().GetExeFolder();

	AddToolBars();

	m_aui_manager->AddPane(m_graphics, wxAuiPaneInfo().Name(_T("Graphics")).Caption(_("Graphics")).CentrePane().BestSize(wxSize(800, 600)));
	m_aui_manager->AddPane(m_tree_canvas, wxAuiPaneInfo().Name(_T("Objects")).Caption(_("Objects")).Left().Layer(1).BestSize(wxSize(300, 400)));
	m_aui_manager->AddPane(m_options, wxAuiPaneInfo().Name(_T("Options")).Caption(_("Options")).Left().Layer(1).BestSize(wxSize(300, 200)));
	m_aui_manager->AddPane(m_input_canvas, wxAuiPaneInfo().Name(_T("Input")).Caption(_("Input")).Left().Layer(1).BestSize(wxSize(300, 200)));
	m_aui_manager->AddPane(m_properties, wxAuiPaneInfo().Name(_T("Properties")).Caption(_("Properties")).Left().Layer(1).BestSize(wxSize(300, 200)));

	// add to hiding list for full screen mode
	wxGetApp().RegisterHideableWindow(m_tree_canvas);
	wxGetApp().RegisterHideableWindow(m_options);
	wxGetApp().RegisterHideableWindow(m_input_canvas);
	wxGetApp().RegisterHideableWindow(m_properties);

	// set xml reading functions
	wxGetApp().InitializeXMLFunctions();

#if 0
	{
		::wxSetWorkingDirectory(wxGetApp().GetExeFolder());

		wxConfig plugins_config(_T("HeeksCAD"));
		plugins_config.SetPath(_T("/plugins"));
		plugins_config.Write(_T("HeeksCNC"), _T("HeeksCNC/libheekscnc.so.0.5.1"));	
	}
#endif

	// load up any other dlls and call OnStartUp on each of them
	{
		std::list<PluginData> plugins;
		ReadPluginsList(plugins);

		for(std::list<PluginData>::iterator It = plugins.begin(); It != plugins.end(); It++)
		{
			PluginData &pd = *It;
			if(pd.enabled)
			{
				wxFileName fn(pd.path);
				fn.Normalize();
				wxString path = fn.GetPath();

				::wxSetWorkingDirectory(path);

				wxDynamicLibrary* shared_library = new wxDynamicLibrary(fn.GetFullPath(),wxDL_NOW|wxDL_GLOBAL );
				if(shared_library->IsLoaded()){
					bool success;
					void(*OnStartUp)(CHeeksCADInterface*, const wxString&) = (void (*)(CHeeksCADInterface*, const wxString&))(shared_library->GetSymbol(_T("OnStartUp"), &success));
					if(OnStartUp)
					{
						(*OnStartUp)(&heekscad_interface, path);
						wxGetApp().m_loaded_libraries.push_back(shared_library);
					}
				}
				else{
					delete shared_library;
				}
			}
		}
	}

	SetDropTarget(new DnDFile(this));

	m_menuWindow->Append( Menu_View_ResetLayout, _( "Reset Layout" ) );
	m_menuWindow->Append( Menu_View_SetToolBarsToLeft, _( "Set toolbars to left" ) );

	//Read layout
	wxString str;
	config.Read(_T("AuiPerspective"), &str, default_layout_string);
	LoadPerspective(str);

	m_aui_manager->Update();
}

CHeeksFrame::~CHeeksFrame()
{
	wxGetApp().Clear(); // delete all the objects, from the dlls

	// call the shared libraries function OnFrameDelete, so they can write profile strings while aui manager still exists
	for(std::list<wxDynamicLibrary*>::iterator It = wxGetApp().m_loaded_libraries.begin(); It != wxGetApp().m_loaded_libraries.end(); It++){
		wxDynamicLibrary* shared_library = *It;
		bool success;
		void(*OnFrameDelete)() = (void(*)())(shared_library->GetSymbol(_T("OnFrameDelete"), &success));
		if(OnFrameDelete)(*OnFrameDelete)();
	}

	//Save the application layout
	wxString str = m_aui_manager->SavePerspective();
#if 0
	// save the layout string in a file
	{
#if wxUSE_UNICODE
		wofstream ofs("layout.txt");
#else
		ofstream ofs("layout.txt");
#endif
		ofs<<str.c_str();
	}
#endif

	HeeksConfig config;
	config.Write(_T("AuiPerspective"), str);
	config.Write(_T("ToolImageSize"), ToolImage::GetBitmapSize());
	config.Write(_T("Perspective"), m_graphics->m_view_point.GetPerspective());	

	delete m_aui_manager;
	delete wxLog::SetActiveTarget(new wxLogStderr(NULL));
}

void CHeeksFrame::OnKeyDown(wxKeyEvent& event)
{
	if(event.GetKeyCode() == WXK_ESCAPE && wxGetApp().EndSketchMode())
	{
	}
	else
		wxGetApp().input_mode_object->OnKeyDown(event);
	event.Skip();
}

void CHeeksFrame::OnKeyUp(wxKeyEvent& event)
{
	wxGetApp().input_mode_object->OnKeyUp(event);
	event.Skip();
}

bool CHeeksFrame::ShowFullScreen(bool show, long style){
	static bool statusbar_visible = true;

	static std::map< wxWindow*, bool > windows_visible;

	if(show){
		SetMenuBar(NULL);
		windows_visible.clear();
		statusbar_visible = m_statusBar->IsShown();
		m_statusBar->Show(false);
		for(std::list<wxWindow*>::iterator It = wxGetApp().m_hideable_windows.begin(); It != wxGetApp().m_hideable_windows.end(); It++)
		{
			wxWindow* w = *It;
			windows_visible.insert(std::pair< wxWindow*, bool > (w, m_aui_manager->GetPane(w).IsShown() && w->IsShown()));
			m_aui_manager->GetPane(w).Show(false);
		}
	}
	else{
		SetMenuBar(m_menuBar);
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

void CHeeksFrame::OnClose( wxCloseEvent& event )
{
	if ( event.CanVeto() && !wxGetApp().CheckForModifiedDoc() )
	{
		event.Veto();
		return;
	}

	event.Skip();
}

static void OnQuit( wxCommandEvent& WXUNUSED( event ) )
{
	if(!wxGetApp().CheckForModifiedDoc())
		return;
	wxGetApp().m_frame->Close(TRUE);
}

static void OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
	CAboutBox dlg(wxGetApp().m_frame);
	dlg.ShowModal();
}

static void OnPlugins( wxCommandEvent& WXUNUSED( event ) )
{
	CPluginsDialog dlg(wxGetApp().m_frame);
	dlg.ShowModal();
}

static void OnViewObjects( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_tree_canvas);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		wxGetApp().m_frame->m_aui_manager->Update();
	}
}

static void OnUpdateViewObjects( wxUpdateUIEvent& event )
{
	event.Check(wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_tree_canvas).IsShown());
}

static void OnLog( wxCommandEvent& event )
{
	if(wxGetApp().m_frame->m_logger){
		wxGetApp().m_frame->m_logger->Show(true);
	}
}

static void OnViewOptions( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_options);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		wxGetApp().m_frame->m_aui_manager->Update();
	}
}

static void OnUpdateViewOptions( wxUpdateUIEvent& event )
{
	event.Check(wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_options).IsShown());
}

static void OnViewInput( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_input_canvas);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		wxGetApp().m_frame->m_aui_manager->Update();
	}
}

static void OnUpdateViewInput( wxUpdateUIEvent& event )
{
	event.Check(wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_input_canvas).IsShown());
}

static void OnViewToolBar( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_toolBar);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		wxGetApp().m_frame->m_aui_manager->Update();
	}
}

static void OnUpdateViewToolBar( wxUpdateUIEvent& event )
{
	event.Check(wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_toolBar).IsShown());
}

static void OnViewGeometryBar( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_geometryBar);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		wxGetApp().m_frame->m_aui_manager->Update();
	}
}

static void OnUpdateViewGeometryBar( wxUpdateUIEvent& event )
{
	event.Check(wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_geometryBar).IsShown());
}

static void OnViewSolidBar( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_solidBar);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		wxGetApp().m_frame->m_aui_manager->Update();
	}
}

static void OnUpdateViewSolidBar( wxUpdateUIEvent& event )
{
	event.Check(wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_solidBar).IsShown());
}

static void OnViewViewingBar( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_viewingBar);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		wxGetApp().m_frame->m_aui_manager->Update();
	}
}

static void OnUpdateViewViewingBar( wxUpdateUIEvent& event )
{
	event.Check(wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_viewingBar).IsShown());
}

static void OnViewTransformBar( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_transformBar);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		wxGetApp().m_frame->m_aui_manager->Update();
	}
}

static void OnUpdateViewTransformBar( wxUpdateUIEvent& event )
{
	event.Check(wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_transformBar).IsShown());
}

static void OnViewStatusBar( wxCommandEvent& event )
{
	wxGetApp().m_frame->m_statusBar->Show(event.IsChecked());
}

static void OnUpdateViewStatusBar( wxUpdateUIEvent& event )
{
	event.Check(wxGetApp().m_frame->m_statusBar->IsShown());
}

void CHeeksFrame::OnResetLayout( wxCommandEvent& event )
{
	ToolImage::SetBitmapSize(ToolImage::default_bitmap_size);
	OnChangeBitmapSize();
	LoadPerspective(default_layout_string);
	m_aui_manager->Update();
}

void CHeeksFrame::OnSetToolBarsToLeft( wxCommandEvent& event )
{
	OnChangeBitmapSize();
	SetToolBarsToLeft();
	m_aui_manager->Update();
}

static void OnViewProperties( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_properties);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		wxGetApp().m_frame->m_aui_manager->Update();
	}
}

static void OnUpdateOpenRecent( wxUpdateUIEvent& event )
{
	size_t size = wxGetApp().m_frame->m_recent_files_menu->GetMenuItemCount();
	std::list<wxMenuItem*> menu_items;
	for(size_t i = 0; i< size; i++)menu_items.push_back(wxGetApp().m_frame->m_recent_files_menu->FindItemByPosition(i));
	for(std::list<wxMenuItem*>::iterator It = menu_items.begin(); It != menu_items.end(); It++)
	{
		wxMenuItem* menu_item = *It;
		wxGetApp().m_frame->m_recent_files_menu->Delete(menu_item);
	}

	int recent_id = ID_RECENT_FIRST;
	for(std::list< wxString >::iterator It = wxGetApp().m_recent_files.begin(); It != wxGetApp().m_recent_files.end() && recent_id < ID_RECENT_FIRST + MAX_RECENT_FILES; It++, recent_id++)
	{
		wxString& filepath = *It;
		wxGetApp().m_frame->m_recent_files_menu->Append(recent_id, filepath, filepath);
	}
}

static void OnUpdateViewProperties( wxUpdateUIEvent& event )
{
	event.Check(wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_properties).IsShown());
}

static void OnSelectModeButton( wxCommandEvent& WXUNUSED( event ) )
{
	wxGetApp().m_marked_list->m_filter = -1;
	wxGetApp().SetInputMode((CInputMode*)(wxGetApp().m_select_mode));
}

static void OnLinesButton( wxCommandEvent& WXUNUSED( event ) )
{
	wxGetApp().CreateUndoPoint();
	line_strip.drawing_mode = LineDrawingMode;
	wxGetApp().SetInputMode(&line_strip);
	wxGetApp().Changed();
}


static void OnEllipseButton( wxCommandEvent& WXUNUSED( event ) )
{
	wxGetApp().CreateUndoPoint();
	line_strip.drawing_mode = EllipseDrawingMode;
	wxGetApp().SetInputMode(&line_strip);
	wxGetApp().Changed();
}
static void OnPointsButton( wxCommandEvent& WXUNUSED( event ) )
{
	wxGetApp().CreateUndoPoint();
	wxGetApp().SetInputMode(&point_drawing);
	wxGetApp().Changed();
}

static void OnRegularShapesButton( wxCommandEvent& WXUNUSED( event ) )
{
	wxGetApp().CreateUndoPoint();
	wxGetApp().SetInputMode(&regular_shapes_drawing);
	wxGetApp().Changed();
}

static void OnTextButton( wxCommandEvent& WXUNUSED( event ) )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(true);
	HText* new_object = new HText(mat, _T("text"), &(wxGetApp().current_color), wxGetApp().m_pCxfFont );
	wxGetApp().CreateUndoPoint();
	wxGetApp().Add(new_object, NULL);
	wxGetApp().m_marked_list->Clear(true);
	wxGetApp().m_marked_list->Add(new_object, true);
	wxGetApp().SetInputMode(wxGetApp().m_select_mode);
	wxGetApp().Changed();
	wxGetApp().Repaint();
}

static void OnDimensioningButton( wxCommandEvent& WXUNUSED( event ) )
{
	wxGetApp().CreateUndoPoint();
	wxGetApp().SetInputMode(&dimension_drawing);
}

static void OnCircles3pButton( wxCommandEvent& WXUNUSED( event ) )
{
	wxGetApp().CreateUndoPoint();
	line_strip.drawing_mode = CircleDrawingMode;
	line_strip.circle_mode = ThreePointsCircleMode;
	wxGetApp().SetInputMode(&line_strip);
}

static void OnCircles2pButton( wxCommandEvent& WXUNUSED( event ) )
{
	line_strip.drawing_mode = CircleDrawingMode;
	line_strip.circle_mode = CentreAndPointCircleMode;
	wxGetApp().SetInputMode(&line_strip);
}

static void OnCirclesprButton( wxCommandEvent& WXUNUSED( event ) )
{
	line_strip.drawing_mode = CircleDrawingMode;
	line_strip.circle_mode = CentreAndRadiusCircleMode;
	wxGetApp().SetInputMode(&line_strip);
}

static void OnILineButton( wxCommandEvent& WXUNUSED( event ) )
{
	line_strip.drawing_mode = ILineDrawingMode;
	wxGetApp().SetInputMode(&line_strip);
}

static void OnCoordinateSystem( wxCommandEvent& WXUNUSED( event ) )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(false);
	gp_Pnt o = gp_Pnt(0, 0, 0).Transformed(mat);
	gp_Dir x = gp_Dir(1, 0, 0).Transformed(mat);
	gp_Dir y = gp_Dir(0, 1, 0).Transformed(mat);
	CoordinateSystem* new_object = new CoordinateSystem(_("Coordinate System"), o, x, y);
	wxGetApp().Add(new_object, NULL);
	wxGetApp().m_marked_list->Clear(true);
	wxGetApp().m_marked_list->Add(new_object, true);
	wxGetApp().SetInputMode(wxGetApp().m_select_mode);
	wxGetApp().Repaint();

	// and pick from three points
	new_object->PickFrom3Points();
}

static void OnOpenButton( wxCommandEvent& event )
{
	wxString default_directory = wxEmptyString;

	if (wxGetApp().m_recent_files.size() > 0)
	{
		#ifdef WIN32
			wxString delimiter(_T("\\"));
		#else
			wxString delimiter(_T("/"));
		#endif // WIN32

		default_directory = *(wxGetApp().m_recent_files.begin());
		int last_directory_delimiter = default_directory.Find(delimiter[0],true);
		if (last_directory_delimiter > 0)
		{
			default_directory.Remove(last_directory_delimiter);
		}
	}

    wxFileDialog dialog(wxGetApp().m_frame, _("Open file"), default_directory, wxEmptyString, wxGetApp().GetKnownFilesWildCardString());
    dialog.CentreOnParent();

    if (dialog.ShowModal() == wxID_OK)
    {
		if(wxGetApp().CheckForModifiedDoc())
		{
			wxGetApp().Reset();
			if(!wxGetApp().OpenFile(dialog.GetPath().c_str()))
			{
				wxString str = wxString(_("Invalid file type chosen")) + _T("  ") + _("expecting") + _T(" ") + wxGetApp().GetKnownFilesCommaSeparatedList();
				wxMessageBox(str);
			}
			wxGetApp().OnNewOrOpen(true);
			wxGetApp().SetLikeNewFile();
		}
    }
}

static void OnImportButton( wxCommandEvent& event )
{

	wxString default_directory = wxEmptyString;

	if (wxGetApp().m_recent_files.size() > 0)
	{
		#ifdef WIN32
			wxString delimiter(_T("\\"));
		#else
			wxString delimiter(_T("/"));
		#endif // WIN32

		default_directory = *(wxGetApp().m_recent_files.begin());
		int last_directory_delimiter = default_directory.Find(delimiter[0],true);
		if (last_directory_delimiter > 0)
		{
			default_directory.Remove(last_directory_delimiter);
		}
	}

    wxFileDialog dialog(wxGetApp().m_frame, _("Import file"), default_directory, wxEmptyString, wxGetApp().GetKnownFilesWildCardString());
    dialog.CentreOnParent();

    if (dialog.ShowModal() == wxID_OK)
    {
		wxGetApp().OpenFile(dialog.GetPath().c_str(), true);
    }
}

static void OnSaveButton( wxCommandEvent& event )
{
    wxGetApp().SaveFile( wxGetApp().m_filepath.c_str(), true );
}

static void OnUndoButton( wxCommandEvent& event )
{
	wxGetApp().Undo();
	wxGetApp().Repaint();
}

static void OnRedoButton( wxCommandEvent& event )
{
	wxGetApp().Redo();
	wxGetApp().Repaint();
}

static void OnNewButton( wxCommandEvent& event )
{
	if(wxGetApp().CheckForModifiedDoc())
	{
		wxGetApp().Reset();
		wxGetApp().OnNewOrOpen(false);
		wxGetApp().SetLikeNewFile();
		wxGetApp().SetFrameTitle();
		wxGetApp().Repaint();
	}
}

static void OnCutButton( wxCommandEvent& event )
{
	wxGetApp().m_marked_list->CutSelectedItems();
}

static void OnUpdateCut( wxUpdateUIEvent& event )
{
	event.Enable(wxGetApp().m_marked_list->size() > 0);
}

static void OnCopyButton( wxCommandEvent& event )
{
	wxGetApp().m_marked_list->CopySelectedItems();
}

static void OnUpdateCopy( wxUpdateUIEvent& event )
{
	event.Enable(wxGetApp().m_marked_list->size() > 0);
}

static void OnPasteButton( wxCommandEvent& event )
{
	wxGetApp().Paste(NULL);
}

static void OnDeleteButton( wxCommandEvent& event )
{
	wxGetApp().CreateUndoPoint();
	wxGetApp().Remove(wxGetApp().m_marked_list->list());
	wxGetApp().m_marked_list->Clear(true);
	wxGetApp().Changed();
}

static void OnUpdateDelete( wxUpdateUIEvent& event )
{
	event.Enable(wxGetApp().m_marked_list->size() > 0);
}

static void OnUpdatePaste( wxUpdateUIEvent& event )
{
	event.Enable(wxGetApp().IsPasteReady());
}

static void OnSubtractButton( wxCommandEvent& event )
{
	if(!wxGetApp().CheckForNOrMore(wxGetApp().m_marked_list->list(), 2, SolidType, FaceType, _("Pick two or more faces or solids, the first one will be cut by the others"), _("Subtract Solids")))return;
	wxGetApp().CreateUndoPoint();
	CShape::CutShapes(wxGetApp().m_marked_list->list());
	wxGetApp().Changed();
}

static void OnFuseButton( wxCommandEvent& event )
{
	if(!wxGetApp().CheckForNOrMore(wxGetApp().m_marked_list->list(), 2, SolidType, _("Pick two or more solids to be fused together"), _("Fuse Solids")))return;
	wxGetApp().CreateUndoPoint();
	CShape::FuseShapes(wxGetApp().m_marked_list->list());
	wxGetApp().Changed();
}

static void OnCommonButton( wxCommandEvent& event )
{
	if(!wxGetApp().CheckForNOrMore(wxGetApp().m_marked_list->list(), 2, SolidType, _("Pick two or more solids, only the shape that is contained by all of them will remain"), _("Intersection of Solids")))return;
	wxGetApp().CreateUndoPoint();
	CShape::CommonShapes(wxGetApp().m_marked_list->list());
	wxGetApp().m_marked_list->Clear(true);
	wxGetApp().Changed();
}

static void OnFilletButton( wxCommandEvent& event )
{
	if(!wxGetApp().CheckForNOrMore(wxGetApp().m_marked_list->list(), 1, EdgeType, _("Pick one or more edges to add a fillet to"), _("Edge Fillet")))return;
	double rad = 2.0;
	HeeksConfig config;
	config.Read(_T("EdgeBlendRadius"), &rad);
	if(wxGetApp().InputDouble(_("Enter Blend Radius"), _("Radius"), rad))
	{
		wxGetApp().CreateUndoPoint();
		CShape::FilletOrChamferEdges(wxGetApp().m_marked_list->list(), rad);
		config.Write(_T("EdgeBlendRadius"), rad);
		wxGetApp().m_marked_list->Clear(true);
		wxGetApp().Changed();
	}
}

static void OnChamferButton( wxCommandEvent& event )
{
	if(!wxGetApp().CheckForNOrMore(wxGetApp().m_marked_list->list(), 1, EdgeType, _("Pick one or more edges to add a chamfer to"), _("Edge Chamfer")))return;
	double rad = 2.0;
	HeeksConfig config;
	config.Read(_T("EdgeChamferDist"), &rad);
	if(wxGetApp().InputDouble(_("Enter chamfer distance"), _("Distance"), rad))
	{
		wxGetApp().CreateUndoPoint();
		CShape::FilletOrChamferEdges(wxGetApp().m_marked_list->list(), rad, true);
		config.Write(_T("EdgeChamferDist"), rad);
		wxGetApp().m_marked_list->Clear(true);
		wxGetApp().Changed();
	}
}

static void OnRuledSurfaceButton( wxCommandEvent& event )
{
	if(!wxGetApp().CheckForNOrMore(wxGetApp().m_marked_list->list(), 2, SketchType, _("Pick two or more sketches, to create a lofted solid between\n( hold down Ctrl key to select more than one solid )"), _("Lofted Body")))return;
	wxGetApp().CreateUndoPoint();
	PickCreateRuledSurface();
	wxGetApp().Changed();
}

static void OnExtrudeButton( wxCommandEvent& event )
{
	if(!wxGetApp().CheckForNOrMore(wxGetApp().m_marked_list->list(), 1, SketchType, FaceType, _("Pick one or more sketches or faces, to create extruded body from\n( hold down Ctrl key to select more than one solid )"), _("Extrude")))return;
	wxGetApp().CreateUndoPoint();
	PickCreateExtrusion();
	wxGetApp().Changed();
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

static void OnSphereButton( wxCommandEvent& event )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(true);
	CSphere* new_object = new CSphere(gp_Pnt(0, 0, 0).Transformed(mat), 5, _("Sphere"), HeeksColor(240, 191, 191));
	AddObjectFromButton(new_object);
}

static void OnCubeButton( wxCommandEvent& event )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(false);
	CCuboid* new_object = new CCuboid(gp_Ax2(gp_Pnt(0, 0, 0).Transformed(mat), gp_Dir(0, 0, 1).Transformed(mat), gp_Dir(1, 0, 0).Transformed(mat)), 10, 10, 10, _("Cuboid"), HeeksColor(191, 240, 191));
	AddObjectFromButton(new_object);
}

static void OnCylButton( wxCommandEvent& event )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(true);
	CCylinder* new_object = new CCylinder(gp_Ax2(gp_Pnt(0, 0, 0).Transformed(mat), gp_Dir(0, 0, 1).Transformed(mat), gp_Dir(1, 0, 0).Transformed(mat)), 5, 10, _("Cylinder"), HeeksColor(191, 191, 240));
	AddObjectFromButton(new_object);
}

static void OnConeButton( wxCommandEvent& event )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(true);
	CCone* new_object = new CCone(gp_Ax2(gp_Pnt(0, 0, 0).Transformed(mat), gp_Dir(0, 0, 1).Transformed(mat), gp_Dir(1, 0, 0).Transformed(mat)), 10, 5, 20, _("Cone"), HeeksColor(240, 240, 191));
	AddObjectFromButton(new_object);
}

static void OnRedrawButton( wxCommandEvent& event )
{
	wxGetApp().RecalculateGLLists();
	wxGetApp().Repaint();
}

static void OnMagButton( wxCommandEvent& event )
{
	wxGetApp().SetInputMode(wxGetApp().magnification);
}

static void OnMagExtentsButton( wxCommandEvent& event )
{
	wxGetApp().m_frame->m_graphics->OnMagExtents(true, true);
}

static void OnMagNoRotButton( wxCommandEvent& event )
{
	wxGetApp().m_frame->m_graphics->OnMagExtents(false, true);
}

static void OnMagPreviousButton( wxCommandEvent& event )
{
	wxGetApp().m_frame->m_graphics->OnMagPrevious();
}

static void OnViewRotateButton( wxCommandEvent& event )
{
	wxGetApp().SetInputMode(wxGetApp().viewrotating);
}

static void OnViewZoomButton( wxCommandEvent& event )
{
	wxGetApp().SetInputMode(wxGetApp().viewzooming);
}

static void OnFullScreenButton( wxCommandEvent& event )
{
	wxGetApp().m_frame->ShowFullScreen(true);
}

static void OnMoveTranslateButton( wxCommandEvent& event )
{
	TransformTools::Translate(false);
}

static void OnCopyTranslateButton( wxCommandEvent& event )
{
	TransformTools::Translate(true);
}

static void OnMoveRotateButton( wxCommandEvent& event )
{
	TransformTools::Rotate(false);
}

static void OnCopyRotateButton( wxCommandEvent& event )
{
	TransformTools::Rotate(true);
}

static void OnMoveMirrorButton( wxCommandEvent& event )
{
	TransformTools::Mirror(false);
}

static void OnCopyMirrorButton( wxCommandEvent& event )
{
	TransformTools::Mirror(true);
}

static void OnMoveScaleButton( wxCommandEvent& event )
{
	TransformTools::Scale(false);
}

void CHeeksFrame::OnExternalButton( wxCommandEvent& event )
{
	wxGetApp().CreateUndoPoint();
	int id = event.GetId();

	std::map<int, SExternalButtonFunctions >::iterator FindIt = m_external_buttons.find(id);
	if(FindIt != m_external_buttons.end()){
		SExternalButtonFunctions& ebf = FindIt->second;
		(*(ebf.on_button))(event);
	}

	wxGetApp().Changed();
}

void CHeeksFrame::OnRecentFile( wxCommandEvent& event )
{
	int id = event.GetId();

	int recent_id = ID_RECENT_FIRST;
	for(std::list< wxString >::iterator It = wxGetApp().m_recent_files.begin(); It != wxGetApp().m_recent_files.end() && recent_id < ID_RECENT_FIRST + MAX_RECENT_FILES; It++, recent_id++)
	{
		if(recent_id != id)continue;
		wxString& filepath = *It;

		if(wxGetApp().CheckForModifiedDoc())
		{
			wxGetApp().Reset();
			wxGetApp().OpenFile(filepath.c_str());
			wxGetApp().OnNewOrOpen(true);
			wxGetApp().SetLikeNewFile();
		}
		break;
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
	HeeksConfig config;
	config.Write(_T("MainFrameWidth"), width);
	config.Write(_T("MainFrameHeight"), height);

	// call add-ins OnSize functions
	for(std::list< void(*)(wxSizeEvent&) >::iterator It = wxGetApp().m_on_graphics_size_list.begin(); It != wxGetApp().m_on_graphics_size_list.end(); It++)
	{
		void(*callback)(wxSizeEvent&) = *It;
		(*callback)(evt);
	}
}

void CHeeksFrame::OnMove( wxMoveEvent& evt )
{
	wxPoint pos = GetPosition();
	int posx = pos.x;
	int posy = pos.y;
	HeeksConfig config;
	config.Write(_T("MainFramePosX"), posx);
	config.Write(_T("MainFramePosY"), posy);
}

int CHeeksFrame::MakeNextIDForTool(void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&))
{
	while(m_external_buttons.find(m_next_id_for_button) != m_external_buttons.end())
	{
		// already used
		m_next_id_for_button++;
	}

	if(m_next_id_for_button >= ID_FIRST_POP_UP_MENU_TOOL)
	{
		// too many button IDs!
		wxMessageBox(_T("too many button IDs!, see CHeeksFrame::GetNextIDForTool"));
	}

	int id_to_use = m_next_id_for_button;


	SExternalButtonFunctions ebf;
	ebf.on_button = onButtonFunction;
	ebf.on_update_button = onUpdateButtonFunction;
	m_external_buttons.insert(std::pair<int, SExternalButtonFunctions > ( id_to_use, ebf ));
	m_next_id_for_button++;
	return id_to_use;
}

void CHeeksFrame::SetToolFunctions(int Id, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&))
{
	SExternalButtonFunctions ebf;
	ebf.on_button = onButtonFunction;
	ebf.on_update_button = onUpdateButtonFunction;
	m_external_buttons.erase(Id);
	m_external_buttons.insert(std::pair<int, SExternalButtonFunctions > ( Id, ebf ));
}

int CHeeksFrame::AddMenuItem(wxMenu* menu, const wxString& text, const wxBitmap& bitmap, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&), wxMenu* submenu, bool check_item)
{
	int id_to_use = MakeNextIDForTool(onButtonFunction, onUpdateButtonFunction);

	wxMenuItem *menuItem = new wxMenuItem(menu, id_to_use, text, wxString(_T("")), check_item ? wxITEM_CHECK : wxITEM_NORMAL, submenu);
	if(!check_item)menuItem->SetBitmap(bitmap);
	menu->Append(menuItem);

	return id_to_use;
}

wxToolBarToolBase* CHeeksFrame::AddToolBarTool(wxToolBar* toolbar, const wxString& title, const wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&))
{
	int id_to_use = MakeNextIDForTool(onButtonFunction, onUpdateButtonFunction);

	return toolbar->AddTool(id_to_use, title, bitmap, caption);
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
		wxToolBarToolBase* button_added = wxGetApp().m_frame->AddToolBarTool(toolbar, tool->GetTitle(), *bitmap, tool->GetToolTip(), OnTool);
		if(button_added)tool_map_for_OnTool.insert( std::pair<int, Tool*> ( button_added->GetId(), tool ) );
	}
}

class ToolBarPopup: public wxPopupTransientWindow
{
public:
	wxToolBar *m_toolBar;

public:
	ToolBarPopup( wxWindow *parent, const CFlyOutList &flyout_list):wxPopupTransientWindow( parent )
	{
		m_panel = new wxScrolledWindow( this, wxID_ANY );
		m_panel->SetBackgroundColour( *wxLIGHT_GREY );

 		m_toolBar = new wxToolBar(m_panel, -1, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_NODIVIDER | wxTB_FLAT);
		m_toolBar->SetToolBitmapSize(wxSize(ToolImage::GetBitmapSize(), ToolImage::GetBitmapSize()));

		int i = 0;
		for(std::list<CFlyOutItem>::const_iterator It = flyout_list.m_list.begin(); It != flyout_list.m_list.end(); It++, i++)
		{
			const CFlyOutItem &fo = *It;
			int id_to_use = i+ID_FIRST_POP_UP_MENU_TOOL;
			m_toolBar->AddTool(id_to_use, fo.m_title_and_bitmap, ToolImage(fo.m_title_and_bitmap), fo.m_tooltip);
		}
		m_toolBar->Realize();

		wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );
		topSizer->Add( m_toolBar, 0, wxALL, 5 );

		m_panel->SetAutoLayout( true );
		m_panel->SetSizer( topSizer );
		topSizer->Fit(m_panel);
		topSizer->Fit(this);
	}

private:
    wxScrolledWindow *m_panel;
    wxButton *m_button;
    wxStaticText *m_mouseText;

	void OnMouse( wxMouseEvent &event )
	{
		if(event.Moving())
		{
			wxPoint pos = event.GetPosition();
			wxToolBarToolBase* tool = m_toolBar->FindToolForPosition(event.GetPosition().x, event.GetPosition().y);
			if(tool)
			{
				wxToolTip::Enable(true);
			}
		}
		event.Skip();
	}

private:
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(ToolBarPopup, wxPopupTransientWindow)
    EVT_MOUSE_EVENTS( ToolBarPopup::OnMouse )
END_EVENT_TABLE()

class CFlyOutButton: public wxBitmapButton
{
	CFlyOutList m_flyout_list;
	wxToolBarToolBase* m_toolbar_tool;
	wxToolBar *m_toolBar;
	ToolBarPopup* m_toolbarPopup;

public:
    CFlyOutButton(const CFlyOutList &flyout_list,
				wxToolBarToolBase* toolbar_tool,
				wxWindow *parent,
				wxWindowID id,
				const wxBitmap& bitmap,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize)
		:wxBitmapButton(parent, id, bitmap, pos, size, wxBU_AUTODRAW | wxBU_EXACTFIT)
		,m_flyout_list(flyout_list)
		,m_toolbar_tool(toolbar_tool)
		,m_toolBar(NULL)
		,m_toolbarPopup(NULL)
	{
	}
    void OnMouse( wxMouseEvent& event )
	{
		if(event.LeftDown())
		{
			// delete previous popup
			delete m_toolbarPopup;

			// make a new popup toolbar
			m_toolbarPopup = new ToolBarPopup( this, m_flyout_list );
			wxWindow *btn = (wxWindow*) event.GetEventObject();
			wxPoint pos = btn->ClientToScreen( wxPoint(0,0) );
			wxSize sz = btn->GetSize();
			m_toolbarPopup->Position( wxPoint(pos.x - sz.GetWidth() - m_toolbar_tool->GetToolBar()->GetToolBitmapSize().GetWidth() - m_toolbar_tool->GetToolBar()->GetMargins().GetWidth(), pos.y), sz );
			m_toolbarPopup->Popup();
		}
	}
	void OnMenuEvent(wxCommandEvent& event)
	{
		int i = 0;
		for(std::list<CFlyOutItem>::const_iterator It = m_flyout_list.m_list.begin(); It != m_flyout_list.m_list.end(); It++, i++)
		{
			if( i+ID_FIRST_POP_UP_MENU_TOOL == event.GetId())
			{
				// hide the popup
				m_toolbarPopup->Hide();

				// call the OnButtonFunction
				const CFlyOutItem &fo = *It;
				(*fo.m_onButtonFunction)(event);

				// change the toolbar tool
				// I don't seem to be able to set the tool's bitmap after it is created!
#if 0
				wxGetApp().m_frame->SetToolFunctions(m_toolbar_tool->GetId(), fo.m_onButtonFunction, NULL);
				wxMenuItem* menu_item = *MIt;
				const wxBitmap& bitmap = menu_item->GetBitmap();
				m_toolbar_tool->SetNormalBitmap(bitmap);
				m_toolbar_tool->SetDisabledBitmap(bitmap);
				m_toolbar_tool->GetToolBar()->Refresh();
#endif
				break;
			}
		}
	}

private:
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CFlyOutButton, wxBitmapButton)
    EVT_MOUSE_EVENTS(CFlyOutButton::OnMouse)
    EVT_MENU_RANGE(ID_FIRST_POP_UP_MENU_TOOL, ID_FIRST_POP_UP_MENU_TOOL + 1000, CFlyOutButton::OnMenuEvent)
END_EVENT_TABLE()

void CHeeksFrame::AddToolBarFlyout(wxToolBar* toolbar, const wxString& title, const CFlyOutList& flyout_list)
{
	if(flyout_list.m_list.size() == 0)return;

	const CFlyOutItem* main_fo = flyout_list.GetMainItem();

	// add the main tool
	wxToolBarToolBase* toolbar_tool = AddToolBarTool(toolbar, main_fo->m_title_and_bitmap, ToolImage(main_fo->m_title_and_bitmap), main_fo->m_tooltip, main_fo->m_onButtonFunction);

	// add the down arrow button
	int id_to_use = MakeNextIDForTool(main_fo->m_onButtonFunction, NULL);
	wxBitmapButton* button = new CFlyOutButton(flyout_list, toolbar_tool, toolbar, id_to_use, ToolImage(_T("downarrow")), wxDefaultPosition, wxSize(ToolImage::GetBitmapSize() / 4, ToolImage::GetBitmapSize()) );
	toolbar->AddControl(button);
}

// a class just so I can get at the protected m_tools of wxToolBar
class ToolBarForGettingToolsFrom: public wxToolBar
{
public:
	void GetToolsIdList(std::list<int> &list)
	{
		wxToolBarToolsList::compatibility_iterator node;
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

//static
void CHeeksFrame::AddToolToListAndMenu(Tool *t, std::vector<ToolIndex> &tool_index_list, wxMenu *menu)
{
	if (t == NULL)
		menu->AppendSeparator();
	else if (t->IsAToolList())
	{
		wxMenu *menu2 = new wxMenu;
		std::list<Tool*>& tool_list = ((ToolList*)t)->m_tool_list;
		std::list<Tool*>::iterator It;
		for (It=tool_list.begin();It!=tool_list.end();It++)
		{
			AddToolToListAndMenu(*It, tool_index_list, menu2);
		}
		wxMenuItem* menu_item = menu->Append(0, t->GetTitle(), menu2);
		wxBitmap* bitmap = t->Bitmap();
		if(bitmap)menu_item->SetBitmap(*bitmap);
	}
	else
	{
		ToolIndex ti;
		ti.m_tool = t;
		ti.m_index = tool_index_list.size();
		tool_index_list.push_back(ti);
		wxMenuItem* menu_item = menu->Append(ti.m_index+ID_FIRST_POP_UP_MENU_TOOL, t->GetTitle());
		wxBitmap* bitmap = t->Bitmap();
		if(bitmap)menu_item->SetBitmap(*bitmap);
		if(t->Disabled())menu->Enable(ti.m_index+1, false);
		if(t->Checked ())menu->Check(ti.m_index+1, true);
	}
}

void CHeeksFrame::Draw(wxDC& dc)
{
	wxGetApp().Draw(dc);
}

static void OnPrint(wxCommandEvent& WXUNUSED(event))
{
    wxPrintDialogData printDialogData(* wxGetApp().m_printData);

    wxPrinter printer(& printDialogData);
	wxGetApp().m_frame->m_printout = new HeeksPrintout(_T("Heeks printout"));
    if (!printer.Print(wxGetApp().m_frame, wxGetApp().m_frame->m_printout, true /*prompt*/))
    {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
            wxMessageBox(_("There was a problem printing.\nPerhaps your current printer is not set correctly?"), _("Printing"), wxOK);
        else
            wxMessageBox(_("You canceled printing"), _("Printing"), wxOK);
    }
    else
    {
        (*wxGetApp().m_printData) = printer.GetPrintDialogData().GetPrintData();
    }

	delete wxGetApp().m_frame->m_printout;
	wxGetApp().m_frame->m_printout = NULL;
}

static void OnPrintPreview(wxCommandEvent& WXUNUSED(event))
{
    // Pass two printout objects: for preview, and possible printing.
    wxPrintDialogData printDialogData(* wxGetApp().m_printData);
    wxPrintPreview *preview = new wxPrintPreview(new HeeksPrintout, new HeeksPrintout, & printDialogData);
    if (!preview->Ok())
    {
        delete preview;
        wxMessageBox(_("There was a problem previewing.\nPerhaps your current printer is not set correctly?"), _("Previewing"), wxOK);
        return;
    }

    wxPreviewFrame *frame = new wxPreviewFrame(preview, wxGetApp().m_frame, _("Demo Print Preview"), wxPoint(100, 100), wxSize(600, 650));
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show();
}

static void OnPageSetup(wxCommandEvent& WXUNUSED(event))
{
    (*wxGetApp().m_pageSetupData) = *(wxGetApp().m_printData);

	wxPageSetupDialog pageSetupDialog(wxGetApp().m_frame, wxGetApp().m_pageSetupData);
    pageSetupDialog.ShowModal();

    (*wxGetApp().m_printData) = pageSetupDialog.GetPageSetupDialogData().GetPrintData();
    (*wxGetApp().m_pageSetupData) = pageSetupDialog.GetPageSetupDialogData();
}

void CHeeksFrame::OnChangeBitmapSize()
{
	m_aui_manager->DetachPane(m_toolBar);
	m_aui_manager->DetachPane(m_geometryBar);
	m_aui_manager->DetachPane(m_solidBar);
	m_aui_manager->DetachPane(m_viewingBar);
	m_aui_manager->DetachPane(m_transformBar);
	for(std::list< wxToolBarBase* >::iterator It = wxGetApp().m_external_toolbars.begin(); It != wxGetApp().m_external_toolbars.end(); It++)
	{
		wxToolBarBase* toolbar = *It;
		m_aui_manager->DetachPane(toolbar);
	}

	wxGetApp().RemoveHideableWindow(m_toolBar);
	wxGetApp().RemoveHideableWindow(m_geometryBar);
	wxGetApp().RemoveHideableWindow(m_solidBar);
	wxGetApp().RemoveHideableWindow(m_viewingBar);
	wxGetApp().RemoveHideableWindow(m_transformBar);
	for(std::list< wxToolBarBase* >::iterator It = wxGetApp().m_external_toolbars.begin(); It != wxGetApp().m_external_toolbars.end(); It++)
	{
		wxToolBarBase* toolbar = *It;
		wxGetApp().RemoveHideableWindow(toolbar);
	}

	delete m_toolBar;
	delete m_geometryBar;
	delete m_solidBar;
	delete m_viewingBar;
	delete m_transformBar;

	wxGetApp().m_external_toolbars.clear();

	AddToolBars();

	m_input_canvas->AddToolBar();
	m_input_canvas->RefreshByRemovingAndAddingAll();
	m_properties->AddToolBar();
	m_properties->RefreshByRemovingAndAddingAll();
}

void CHeeksFrame::SetToolBarsSize()
{
	m_toolBar->SetToolBitmapSize(wxSize(ToolImage::GetBitmapSize(), ToolImage::GetBitmapSize()));
	m_geometryBar->SetToolBitmapSize(wxSize(ToolImage::GetBitmapSize(), ToolImage::GetBitmapSize()));
	m_solidBar->SetToolBitmapSize(wxSize(ToolImage::GetBitmapSize(), ToolImage::GetBitmapSize()));
	m_viewingBar->SetToolBitmapSize(wxSize(ToolImage::GetBitmapSize(), ToolImage::GetBitmapSize()));
	m_transformBar->SetToolBitmapSize(wxSize(ToolImage::GetBitmapSize(), ToolImage::GetBitmapSize()));

	for(std::list< wxToolBarBase* >::iterator It = wxGetApp().m_external_toolbars.begin(); It != wxGetApp().m_external_toolbars.end(); It++)
	{
		wxToolBarBase* toolbar = *It;
		toolbar->SetToolBitmapSize(wxSize(ToolImage::GetBitmapSize(), ToolImage::GetBitmapSize()));
	}
}

void CHeeksFrame::MakeMenus()
{
	// File Menu
	wxMenu *file_menu = new wxMenu;
	AddMenuItem(file_menu, _("New\tCtrl+N"), ToolImage(_T("new")), OnNewButton);
	AddMenuItem(file_menu, _("Open\tCtrl+O"), ToolImage(_T("open")), OnOpenButton);
	AddMenuItem(file_menu, _("Save\tCtrl+S"), ToolImage(_T("save")), OnSaveButton);
	file_menu->AppendSeparator();
	AddMenuItem(file_menu, _("Print\tCtrl+P"), ToolImage(_T("print")), OnPrint);
 	AddMenuItem(file_menu, _("Page Setup"), ToolImage(_T("psetup")), OnPageSetup);
	AddMenuItem(file_menu, _("Print Preview"), ToolImage(_T("ppreview")), OnPrintPreview);
    m_recent_files_menu = new wxMenu;
    m_recent_files_menu->Append(-1, _T("test"));
	AddMenuItem(file_menu, _("Open Recent"), ToolImage(_T("recent")), NULL, OnUpdateOpenRecent, m_recent_files_menu);
	AddMenuItem(file_menu, _("Import"), ToolImage(_T("import")), OnImportButton);
	AddMenuItem(file_menu, _("About"), ToolImage(_T("about")), OnAbout);
	file_menu->AppendSeparator();
	AddMenuItem(file_menu, _("Plugins"), ToolImage(_T("plugin")), OnPlugins);
	file_menu->AppendSeparator();
	AddMenuItem(file_menu, _("Exit\tCtrl+Q"), ToolImage(_T("exit")), OnQuit);

	// Edit Menu
	wxMenu *edit_menu = new wxMenu;
	AddMenuItem(edit_menu, _("Undo\tCtrl+Z"), ToolImage(_T("undo")), OnUndoButton);
	AddMenuItem(edit_menu, _("Redo\tCtrl+Shift+Z"), ToolImage(_T("redo")), OnRedoButton);
	edit_menu->AppendSeparator();
	AddMenuItem(edit_menu, _("Cut\tCtrl+X"), ToolImage(_T("cut")), OnCutButton, OnUpdateCut);
	AddMenuItem(edit_menu, _("Copy\tCtrl+C"), ToolImage(_T("copy")), OnCopyButton, OnUpdateCopy);
	AddMenuItem(edit_menu, _("Paste\tCtrl+V"), ToolImage(_T("paste")), OnPasteButton, OnUpdatePaste);
	AddMenuItem(edit_menu, _("Delete\tDel"), ToolImage(_T("delete")), OnDeleteButton, OnUpdateDelete);
	edit_menu->AppendSeparator();
	AddMenuItem(edit_menu, _("Select Mode"), ToolImage(_T("select")), OnSelectModeButton);

	// Geometry Menu
	wxMenu *geometry_menu = new wxMenu;
	AddMenuItem(geometry_menu, _("Draw a sketch"), ToolImage(_T("lines")), OnLinesButton);
	//AddMenuItem(geometry_menu, _("Draw Circles"), ToolImage(_T("circles")), OnCirclesButton);
	AddMenuItem(geometry_menu, _("Draw Ellipses"), ToolImage(_T("circles")), OnEllipseButton);
	AddMenuItem(geometry_menu, _("Draw Infinite Lines"), ToolImage(_T("iline")), OnILineButton);
	AddMenuItem(geometry_menu, _("Draw Points"), ToolImage(_T("point")), OnPointsButton);
	AddMenuItem(geometry_menu, _("Draw Regular Shapes"), ToolImage(_T("regshapes")), OnRegularShapesButton);
	geometry_menu->AppendSeparator();
	AddMenuItem(geometry_menu, _("Add Text"), ToolImage(_T("text")), OnTextButton);
	AddMenuItem(geometry_menu, _("Add Dimension"), ToolImage(_T("dimension")), OnDimensioningButton);
	geometry_menu->AppendSeparator();
	AddMenuItem(geometry_menu, _("Add Coordinate System"), ToolImage(_T("coordsys")), OnCoordinateSystem);
	
	// View Menu
	wxMenu *view_menu = new wxMenu;
	AddMenuItem(view_menu, _("Previous view"), ToolImage(_T("magprev")), OnMagPreviousButton);
	view_menu->AppendSeparator();
	AddMenuItem(view_menu, _("Zoom window"), ToolImage(_T("mag")), OnMagButton);
	AddMenuItem(view_menu, _("Fit view to extents"), ToolImage(_T("magextents")), OnMagExtentsButton);
	AddMenuItem(view_menu, _("Fit view to extents, but no rotation"), ToolImage(_T("magnorot")), OnMagNoRotButton);
	view_menu->AppendSeparator();
	AddMenuItem(view_menu, _("View rotate"), ToolImage(_T("viewrot")), OnViewRotateButton);
	AddMenuItem(view_menu, _("View zoom"), ToolImage(_T("zoom")), OnViewZoomButton);
	AddMenuItem(view_menu, _("Full screen"), ToolImage(_T("fullscreen")), OnFullScreenButton);
	view_menu->AppendSeparator();
	AddMenuItem(view_menu, _("Redraw"), ToolImage(_T("redraw")), OnRedrawButton);
	
	// Solids Menu
	wxMenu *solids_menu = new wxMenu;
	AddMenuItem(solids_menu, _("Add a sphere"), ToolImage(_T("sphere")), OnSphereButton);
	AddMenuItem(solids_menu, _("Add a cube"), ToolImage(_T("cube")), OnCubeButton);
	AddMenuItem(solids_menu, _("Add a cylinder"), ToolImage(_T("cyl")), OnCylButton);
	AddMenuItem(solids_menu, _("Add a cone"), ToolImage(_T("cone")), OnConeButton);
	solids_menu->AppendSeparator();
	AddMenuItem(solids_menu, _("Loft two sketches"), ToolImage(_T("ruled")), OnRuledSurfaceButton);
	AddMenuItem(solids_menu, _("Extrude a sketch"), ToolImage(_T("extrude")), OnExtrudeButton);
	solids_menu->AppendSeparator();
	AddMenuItem(solids_menu, _("Cut"), ToolImage(_T("subtract")), OnSubtractButton);
	AddMenuItem(solids_menu, _("Fuse"), ToolImage(_T("fuse")), OnFuseButton);
	AddMenuItem(solids_menu, _("Common"), ToolImage(_T("common")), OnCommonButton);
	AddMenuItem(solids_menu, _("Fillet"), ToolImage(_T("fillet")), OnFilletButton);
	AddMenuItem(solids_menu, _("Chamfer"), ToolImage(_T("chamfer")), OnChamferButton);

	// Transformations Menu
	wxMenu *transform_menu = new wxMenu;
	AddMenuItem(transform_menu, _("Move Translate"), ToolImage(_T("movet")), OnMoveTranslateButton);
	AddMenuItem(transform_menu, _("Copy Translate"), ToolImage(_T("copyt")), OnCopyTranslateButton);
	transform_menu->AppendSeparator();
	AddMenuItem(transform_menu, _("Move Rotate"), ToolImage(_T("mover")), OnMoveRotateButton);
	AddMenuItem(transform_menu, _("Copy Rotate"), ToolImage(_T("copyr")), OnCopyRotateButton);
	transform_menu->AppendSeparator();
	AddMenuItem(transform_menu, _("Move Mirror"), ToolImage(_T("movem")), OnMoveMirrorButton);
	AddMenuItem(transform_menu, _("Copy Mirror"), ToolImage(_T("copym")), OnCopyMirrorButton);
	transform_menu->AppendSeparator();
	AddMenuItem(transform_menu, _("Move Scale"), ToolImage(_T("moves")), OnMoveScaleButton);

	// Window Menu
	m_menuWindow = new wxMenu;
	AddMenuItem(m_menuWindow, _("Objects"), wxBitmap(), OnViewObjects, OnUpdateViewObjects, NULL, true);
	AddMenuItem(m_menuWindow, _("Log"), wxBitmap(), OnLog);
	AddMenuItem(m_menuWindow, _("Options"), wxBitmap(), OnViewOptions, OnUpdateViewOptions, NULL, true);
	AddMenuItem(m_menuWindow, _("Input"), wxBitmap(), OnViewInput, OnUpdateViewInput, NULL, true);
	AddMenuItem(m_menuWindow, _("Properties"), wxBitmap(), OnViewProperties, OnUpdateViewProperties, NULL, true);
	AddMenuItem(m_menuWindow, _("Tool Bar"), wxBitmap(), OnViewToolBar, OnUpdateViewToolBar, NULL, true);
	AddMenuItem(m_menuWindow, _("Solids Tool Bar"), wxBitmap(), OnViewSolidBar, OnUpdateViewSolidBar, NULL, true);
	AddMenuItem(m_menuWindow, _("Geometry Tool Bar"), wxBitmap(), OnViewGeometryBar, OnUpdateViewGeometryBar, NULL, true);
	AddMenuItem(m_menuWindow, _("Viewing Tool Bar"), wxBitmap(), OnViewViewingBar, OnUpdateViewViewingBar, NULL, true);
	AddMenuItem(m_menuWindow, _("Transformations Tool Bar"), wxBitmap(), OnViewTransformBar, OnUpdateViewTransformBar, NULL, true);
	AddMenuItem(m_menuWindow, _("Status Bar"), wxBitmap(), OnViewStatusBar, OnUpdateViewStatusBar, NULL, true);

	// Add them to the main menu
	m_menuBar = new wxMenuBar;
	m_menuBar->Append( file_menu, _( "&File" ) );
	m_menuBar->Append( edit_menu, _( "&Edit" ) );
	m_menuBar->Append( geometry_menu, _( "&Geometry" ) );
	m_menuBar->Append( view_menu, _( "&View" ) );
	m_menuBar->Append( solids_menu, _( "&Solid" ) );
	m_menuBar->Append( transform_menu, _( "&Transform" ) );
	m_menuBar->Append( m_menuWindow, _( "&Window" ) );
	SetMenuBar( m_menuBar );
}

void CHeeksFrame::AddToolBars()
{
	m_toolBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
	m_geometryBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
	m_solidBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
	m_viewingBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
	m_transformBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);

	for(std::list< void(*)() >::iterator It = wxGetApp().m_AddToolBars_list.begin(); It != wxGetApp().m_AddToolBars_list.end(); It++)
	{
		void(*callbackfunc)() = *It;
		(*callbackfunc)();
	}

	SetToolBarsSize();

	AddToolBarTool(m_toolBar, _T("New"), ToolImage(_T("new")), _("New file"), OnNewButton);
	AddToolBarTool(m_toolBar, _T("Open"), ToolImage(_T("open")), _("Open file"), OnOpenButton);
	AddToolBarTool(m_toolBar, _T("Save"), ToolImage(_T("save")), _("Save file"), OnSaveButton);
	AddToolBarTool(m_toolBar, _T("Cut"), ToolImage(_T("cut")), _("Cut selected items to the clipboard"), OnCutButton, OnUpdateCut);
	AddToolBarTool(m_toolBar, _T("Copy"), ToolImage(_T("copy")), _("Copy selected items to the clipboard"), OnCopyButton, OnUpdateCopy);
	AddToolBarTool(m_toolBar, _T("Paste"), ToolImage(_T("paste")), _("Paste items from the clipboard"), OnPasteButton, OnUpdatePaste);
	AddToolBarTool(m_toolBar, _T("Undo"), ToolImage(_T("undo")), _("Undo the previous command"), OnUndoButton);
	AddToolBarTool(m_toolBar, _T("Redo"), ToolImage(_T("redo")), _("Redo the next command"), OnRedoButton);
	AddToolBarTool(m_toolBar, _T("Select"), ToolImage(_T("select")), _("Select Mode"), OnSelectModeButton);
	AddToolBarTool(m_geometryBar, _T("Lines"), ToolImage(_T("lines")), _("Draw a sketch"), OnLinesButton);

	{
		CFlyOutList flyout_list(_T(""), _T(""));
		flyout_list.m_list.push_back(CFlyOutItem(_T("circ3p"), _("Draw circles through 3 points"), OnCircles3pButton));
		flyout_list.m_list.push_back(CFlyOutItem(_T("circ2p"), _("Draw circles, centre and point"), OnCircles2pButton));
		flyout_list.m_list.push_back(CFlyOutItem(_T("circpr"), _("Draw circles, centre and radius"), OnCirclesprButton));
		AddToolBarFlyout(m_geometryBar, _T("circles"), flyout_list);
	}

	AddToolBarTool(m_geometryBar, _T("Ellipses"), ToolImage(_T("circles")), _("Draw ellipses"), OnEllipseButton);
	AddToolBarTool(m_geometryBar, _T("ILine"), ToolImage(_T("iline")), _("Start Drawing Infinite Lines"), OnILineButton);
	AddToolBarTool(m_geometryBar, _T("Points"), ToolImage(_T("point")), _("Start Drawing Points"), OnPointsButton);
	AddToolBarTool(m_geometryBar, _T("Regular Shapes"), ToolImage(_T("regshapes")), _("Draw regular shapes; rectangles, polygons, obrounds"), OnRegularShapesButton);
	AddToolBarTool(m_geometryBar, _T("Text"), ToolImage(_T("text")), _("Add a text object"), OnTextButton);
	AddToolBarTool(m_geometryBar, _T("Dimensioning"), ToolImage(_T("dimension")), _("Add a dimension"), OnDimensioningButton);
	AddToolBarTool(m_geometryBar, _T("CoordSys"), ToolImage(_T("coordsys")), _("Create a Coordinate System"), OnCoordinateSystem);
	{
		CFlyOutList flyout_list(_T(""), _T(""));
		flyout_list.m_list.push_back(CFlyOutItem(_T("sphere"), _("Add a sphere"), OnSphereButton));
		flyout_list.m_list.push_back(CFlyOutItem(_T("cube"), _("Add a cube"), OnCubeButton));
		flyout_list.m_list.push_back(CFlyOutItem(_T("cyl"), _("Add a cylinder"), OnCylButton));
		flyout_list.m_list.push_back(CFlyOutItem(_T("cone"), _("Add a cone"), OnConeButton));
		AddToolBarFlyout(m_solidBar, _T("Solid Primitives"), flyout_list);
	}
	AddToolBarTool(m_solidBar, _T("Ruled Surface"), ToolImage(_T("ruled")), _("Create a lofted face"), OnRuledSurfaceButton);
	AddToolBarTool(m_solidBar, _T("Extrude"), ToolImage(_T("extrude")), _("Extrude a wire or face"), OnExtrudeButton);
	AddToolBarTool(m_solidBar, _T("Cut"), ToolImage(_T("subtract")), _("Cut one solid from another"), OnSubtractButton);
	AddToolBarTool(m_solidBar, _T("Fuse"), ToolImage(_T("fuse")), _("Fuse one solid to another"), OnFuseButton);
	AddToolBarTool(m_solidBar, _T("Common"), ToolImage(_T("common")), _("Find common solid between two solids"), OnCommonButton);
	AddToolBarTool(m_solidBar, _T("Fillet"), ToolImage(_T("fillet")), _("Make a fillet on selected edges"), OnFilletButton);
	AddToolBarTool(m_solidBar, _T("Chamfer"), ToolImage(_T("chamfer")), _("Make a chamfer on selected edges"), OnChamferButton);
	AddToolBarTool(m_viewingBar, _T("View Back"), ToolImage(_T("magprev")), _("Go back to previous view"), OnMagPreviousButton);
	AddToolBarTool(m_viewingBar, _T("Zoom Window"), ToolImage(_T("mag")), _("Zoom in to a dragged window"), OnMagButton);
	AddToolBarTool(m_viewingBar, _T("Mag Extents"), ToolImage(_T("magextents")), _("Zoom in to fit the extents of the drawing into the graphics window"), OnMagExtentsButton);
	AddToolBarTool(m_viewingBar, _T("Mag No Rotation"), ToolImage(_T("magnorot")), _("Zoom in to fit the extents of the drawing into the graphics window, but without rotating the view"), OnMagNoRotButton);
	AddToolBarTool(m_viewingBar, _T("View Rotate"), ToolImage(_T("viewrot")), _("Enter view rotating mode"), OnViewRotateButton);
	AddToolBarTool(m_viewingBar, _T("View Zoom"), ToolImage(_T("zoom")), _("Drag to zoom in and out"), OnViewZoomButton);
	AddToolBarTool(m_viewingBar, _T("FullScreen"), ToolImage(_T("fullscreen")), _("Switch to full screen view ( press escape to return )"), OnFullScreenButton);
	AddToolBarTool(m_viewingBar, _T("Redraw"), ToolImage(_T("redraw")), _("Redraw"), OnRedrawButton);
	AddToolBarTool(m_transformBar, _T("Move Translate"), ToolImage(_T("movet")), _("Translate selected items"), OnMoveTranslateButton);
	AddToolBarTool(m_transformBar, _T("Copy Translate"), ToolImage(_T("copyt")), _("Copy and translate selected items"), OnCopyTranslateButton);
	AddToolBarTool(m_transformBar, _T("Move Rotate"), ToolImage(_T("mover")), _("Rotate selected items"), OnMoveRotateButton);
	AddToolBarTool(m_transformBar, _T("Copy Rotate"), ToolImage(_T("copyr")), _("Copy and rotate selected items"), OnCopyRotateButton);
	AddToolBarTool(m_transformBar, _T("Move Mirror"), ToolImage(_T("movem")), _("Mirror selected items"), OnMoveMirrorButton);
	AddToolBarTool(m_transformBar, _T("Copy Mirror"), ToolImage(_T("copym")), _("Copy and mirror selected items"), OnCopyMirrorButton);
	AddToolBarTool(m_transformBar, _T("Move Scale"), ToolImage(_T("moves")), _("Scale selected items"), OnMoveScaleButton);
	m_toolBar->Realize();
	m_geometryBar->Realize();
	m_solidBar->Realize();
	m_viewingBar->Realize();
	m_transformBar->Realize();
	m_aui_manager->AddPane(m_toolBar, wxAuiPaneInfo().Name(_T("ToolBar")).Caption(_("General Tools")).ToolbarPane().Top());
	m_aui_manager->AddPane(m_geometryBar, wxAuiPaneInfo().Name(_T("GeomBar")).Caption(_("Geometry Tools")).ToolbarPane().Top());
	m_aui_manager->AddPane(m_solidBar, wxAuiPaneInfo().Name(_T("SolidBar")).Caption(_("Solid Tools")).ToolbarPane().Top());
	m_aui_manager->AddPane(m_viewingBar, wxAuiPaneInfo().Name(_T("ViewingBar")).Caption(_("Viewing Tools")).ToolbarPane().Top());
	m_aui_manager->AddPane(m_transformBar, wxAuiPaneInfo().Name(_T("TransformBar")).Caption(_("Transformation Tools")).ToolbarPane().Top());
	wxGetApp().RegisterHideableWindow(m_toolBar);
	wxGetApp().RegisterHideableWindow(m_geometryBar);
	wxGetApp().RegisterHideableWindow(m_solidBar);
	wxGetApp().RegisterHideableWindow(m_viewingBar);
	wxGetApp().RegisterHideableWindow(m_transformBar);
}

void CHeeksFrame::LoadPerspective(const wxString& str)
{
	m_aui_manager->LoadPerspective(str);

	// translate the window captions
	m_aui_manager->GetPane(m_graphics).Caption(_("Graphics"));
	m_aui_manager->GetPane(m_tree_canvas).Caption(_("Objects"));
	m_aui_manager->GetPane(m_options).Caption(_("Options"));
	m_aui_manager->GetPane(m_input_canvas).Caption(_("Input"));
	m_aui_manager->GetPane(m_properties).Caption(_("Properties"));
	m_aui_manager->GetPane(m_toolBar).Caption(_("General Tools"));
	m_aui_manager->GetPane(m_geometryBar).Caption(_("Geometry Tools"));
	m_aui_manager->GetPane(m_solidBar).Caption(_("Solid Tools"));
	m_aui_manager->GetPane(m_viewingBar).Caption(_("Viewing Tools"));
	m_aui_manager->GetPane(m_transformBar).Caption(_("Transformation Tools"));

	SetToolBarsSize();
}

void CHeeksFrame::SetDefaultLayout(const wxString& str)
{
	default_layout_string = str;
}

void CHeeksFrame::SetToolBarsToLeft()
{
	m_aui_manager->GetPane(m_toolBar).Left();
	m_aui_manager->GetPane(m_geometryBar).Left();
	m_aui_manager->GetPane(m_solidBar).Left();
	m_aui_manager->GetPane(m_viewingBar).Left();
	m_aui_manager->GetPane(m_transformBar).Left();

	for(std::list< wxToolBarBase* >::iterator It = wxGetApp().m_external_toolbars.begin(); It != wxGetApp().m_external_toolbars.end(); It++)
	{
		wxToolBarBase* toolbar = *It;
		m_aui_manager->GetPane(toolbar).Left();
	}
}

CFlyOutItem::CFlyOutItem(const wxString& title_and_bitmap, const wxString& tooltip, void(*onButtonFunction)(wxCommandEvent&)):m_title_and_bitmap(title_and_bitmap), m_tooltip(tooltip), m_onButtonFunction(onButtonFunction)
{
}

CFlyOutList::CFlyOutList(const wxString& title_and_bitmap, const wxString& tooltip):CFlyOutItem(title_and_bitmap, tooltip, NULL)
{
}

static wxString GetFlyoutConfigString(const wxString& title)
{
	wxString config_string = wxString(_T("ToolBar")) + title;
	return config_string;
}

const CFlyOutItem* CFlyOutList::GetMainItem()const
{
	// get the item to show on toolbar
	const CFlyOutItem &first_fo = m_list.front();

	wxString config_string = GetFlyoutConfigString(m_title_and_bitmap) + _T("ActiveTool");
	HeeksConfig config;
	wxString active_tool_str;
	config.Read(config_string, &active_tool_str, first_fo.m_title_and_bitmap);

	const CFlyOutItem *main_fo = &first_fo;
	for(std::list<CFlyOutItem>::const_iterator It = m_list.begin(); It != m_list.end(); It++)
	{
		const CFlyOutItem &fo = *It;
		if(fo.m_title_and_bitmap == active_tool_str)
		{
			main_fo = &fo;
			break;
		}
	}

	return main_fo;
}
