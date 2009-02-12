// HeeksFrame.cpp

#include "stdafx.h"
#include "HeeksFrame.h"
#include "../interface/Tool.h"
#include "../interface/ToolList.h"
#include "GraphicsCanvas.h"
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
#include "wx/dnd.h"
#include "wx/filename.h"
#include <fstream>
#include "wx/print.h"
#include "wx/printdlg.h"
#include <wx/aui/aui.h>
#include "HeeksPrintout.h"
#include "../interface/HeeksCADInterface.h"
#include "Plugins.h"
#include "HeeksConfig.h"

using namespace std;

BEGIN_EVENT_TABLE( CHeeksFrame, wxFrame )
EVT_CLOSE(CHeeksFrame::OnClose)
EVT_MENU( Menu_File_Quit, CHeeksFrame::OnQuit )
EVT_MENU( Menu_File_About, CHeeksFrame::OnAbout )
EVT_MENU( Menu_File_Plugins, CHeeksFrame::OnPlugins )
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
EVT_MENU( Menu_View_GeometryBar, CHeeksFrame::OnViewGeometryBar )
EVT_UPDATE_UI(Menu_View_GeometryBar, CHeeksFrame::OnUpdateViewGeometryBar)
EVT_MENU( Menu_View_SolidBar, CHeeksFrame::OnViewSolidBar )
EVT_UPDATE_UI(Menu_View_SolidBar, CHeeksFrame::OnUpdateViewSolidBar)
EVT_MENU( Menu_View_ViewingBar, CHeeksFrame::OnViewViewingBar )
EVT_UPDATE_UI(Menu_View_ViewingBar, CHeeksFrame::OnUpdateViewViewingBar)
EVT_MENU( Menu_View_TransformBar, CHeeksFrame::OnViewTransformBar )
EVT_UPDATE_UI(Menu_View_TransformBar, CHeeksFrame::OnUpdateViewTransformBar)
EVT_MENU( Menu_View_StatusBar, CHeeksFrame::OnViewStatusBar )
EVT_UPDATE_UI(Menu_View_StatusBar, CHeeksFrame::OnUpdateViewStatusBar)
EVT_MENU( Menu_View_ResetLayout, CHeeksFrame::OnResetLayout )
EVT_MENU( Menu_View_SetToolBarsToLeft, CHeeksFrame::OnSetToolBarsToLeft )
EVT_MENU(wxID_OPEN, CHeeksFrame::OnOpenButton)
EVT_MENU(wxID_SAVE, CHeeksFrame::OnSaveButton)
EVT_MENU(wxID_NEW, CHeeksFrame::OnNewButton)
EVT_MENU(wxID_CUT, CHeeksFrame::OnCutButton)
EVT_UPDATE_UI(wxID_CUT, CHeeksFrame::OnUpdateCut)
EVT_MENU(wxID_COPY, CHeeksFrame::OnCopyButton)
EVT_UPDATE_UI(wxID_COPY, CHeeksFrame::OnUpdateCopy)
EVT_MENU(wxID_PASTE, CHeeksFrame::OnPasteButton)
EVT_MENU(wxID_DELETE, CHeeksFrame::OnDeleteButton)
EVT_UPDATE_UI(wxID_DELETE, CHeeksFrame::OnUpdateDelete)
EVT_UPDATE_UI(wxID_PASTE, CHeeksFrame::OnUpdatePaste)
EVT_MENU(WXPRINT_PRINT, CHeeksFrame::OnPrint)
EVT_MENU(WXPRINT_PREVIEW, CHeeksFrame::OnPrintPreview)
EVT_MENU(WXPRINT_PAGE_SETUP, CHeeksFrame::OnPageSetup)
EVT_UPDATE_UI(ID_OPEN_RECENT, CHeeksFrame::OnUpdateOpenRecent)
EVT_MENU(ID_IMPORT, CHeeksFrame::OnImportButton)
EVT_MENU_RANGE(	ID_RECENT_FIRST, ID_RECENT_FIRST + MAX_RECENT_FILES, CHeeksFrame::OnRecentFile)
EVT_MENU(ID_LINES, CHeeksFrame::OnLinesButton)
EVT_MENU(ID_CIRCLES, CHeeksFrame::OnCirclesButton)
EVT_MENU(ID_ILINE, CHeeksFrame::OnILineButton)
EVT_MENU(ID_POINTS, CHeeksFrame::OnPointsButton)
EVT_MENU(ID_REGSHAPES, CHeeksFrame::OnRegularShapesButton)
EVT_MENU(ID_TEXT, CHeeksFrame::OnTextButton)
EVT_MENU(ID_DIMENSIONING, CHeeksFrame::OnDimensioningButton)
EVT_MENU(ID_COORDINATE_SYSTEM, CHeeksFrame::OnCoordinateSystem)
EVT_MENU(ID_SELECT_MODE, CHeeksFrame::OnSelectModeButton)
EVT_MENU(ID_SPHERE, CHeeksFrame::OnSphereButton)
EVT_MENU(ID_CUBE, CHeeksFrame::OnCubeButton)
EVT_MENU(ID_CYL, CHeeksFrame::OnCylButton)
EVT_MENU(ID_CONE, CHeeksFrame::OnConeButton)
EVT_MENU(ID_SUBTRACT, CHeeksFrame::OnSubtractButton)
EVT_MENU(ID_FUSE, CHeeksFrame::OnFuseButton)
EVT_MENU(ID_COMMON, CHeeksFrame::OnCommonButton)
EVT_MENU(ID_FILLET, CHeeksFrame::OnFilletButton)
EVT_MENU(ID_CHAMFER, CHeeksFrame::OnChamferButton)
EVT_MENU(ID_REDRAW, CHeeksFrame::OnRedrawButton)
EVT_MENU(ID_RULED_SURFACE, CHeeksFrame::OnRuledSurfaceButton)
EVT_MENU(ID_EXTRUDE, CHeeksFrame::OnExtrudeButton)
EVT_MENU(ID_MAG, CHeeksFrame::OnMagButton)
EVT_MENU(wxID_UNDO, CHeeksFrame::OnUndoButton)
EVT_MENU(wxID_REDO, CHeeksFrame::OnRedoButton)
EVT_MENU(ID_MAG_EXTENTS, CHeeksFrame::OnMagExtentsButton)
EVT_MENU(ID_MAG_NO_ROT, CHeeksFrame::OnMagNoRotButton)
EVT_MENU(ID_MAG_PREVIOUS, CHeeksFrame::OnMagPreviousButton)
EVT_MENU(ID_VIEW_ROT, CHeeksFrame::OnViewRotateButton)
EVT_MENU(ID_VIEW_ZOOM, CHeeksFrame::OnViewZoomButton)
EVT_MENU(ID_FULL_SCREEN, CHeeksFrame::OnFullScreenButton)
EVT_MENU(ID_MOVE_TRANSLATE, CHeeksFrame::OnMoveTranslateButton)
EVT_MENU(ID_COPY_TRANSLATE, CHeeksFrame::OnCopyTranslateButton)
EVT_MENU(ID_MOVE_ROTATE, CHeeksFrame::OnMoveRotateButton)
EVT_MENU(ID_COPY_ROTATE, CHeeksFrame::OnCopyRotateButton)
EVT_MENU(ID_MOVE_MIRROR, CHeeksFrame::OnMoveMirrorButton)
EVT_MENU(ID_COPY_MIRROR, CHeeksFrame::OnCopyMirrorButton)
EVT_MENU(ID_MOVE_SCALE, CHeeksFrame::OnMoveScaleButton)
EVT_MENU_RANGE(ID_FIRST_EXTERNAL_BUTTON, ID_FIRST_POP_UP_MENU_TOOL + 1000, CHeeksFrame::OnExternalButton)
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
static wxString default_layout_string = _T("layout2|name=ToolBar;caption=General Tools;state=2108156;dir=1;layer=10;row=0;pos=0;prop=100000;bestw=279;besth=31;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=GeomBar;caption=Geometry Tools;state=2108156;dir=1;layer=10;row=0;pos=290;prop=100000;bestw=248;besth=31;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=SolidBar;caption=Solid Tools;state=2108156;dir=1;layer=10;row=1;pos=0;prop=100000;bestw=341;besth=31;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=970;floaty=297;floatw=296;floath=57|name=ViewingBar;caption=Viewing Tools;state=2108156;dir=1;layer=10;row=1;pos=290;prop=100000;bestw=248;besth=31;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=TransformBar;caption=Transformation Tools;state=2108156;dir=1;layer=10;row=0;pos=1098;prop=100000;bestw=217;besth=31;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Graphics;caption=Graphics;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=800;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Objects;caption=Objects;state=2099196;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=300;besth=400;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Options;caption=Options;state=2099196;dir=4;layer=1;row=0;pos=1;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Input;caption=Input;state=2099196;dir=4;layer=1;row=0;pos=2;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Properties;caption=Properties;state=2099196;dir=4;layer=1;row=0;pos=3;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|dock_size(5,0,0)=504|dock_size(4,1,0)=234|dock_size(1,10,0)=33|dock_size(1,10,1)=33|");
#else
static wxString default_layout_string = _T("layout2|name=Graphics;caption=Graphics;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=800;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Objects;caption=Objects;state=2099196;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=300;besth=400;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Options;caption=Options;state=2099196;dir=4;layer=1;row=0;pos=1;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Input;caption=Input;state=2099196;dir=4;layer=1;row=0;pos=2;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Properties;caption=Properties;state=2099196;dir=4;layer=1;row=0;pos=3;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=ToolBar;caption=General Tools;state=2108156;dir=1;layer=10;row=0;pos=0;prop=100000;bestw=328;besth=40;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=GeomBar;caption=Geometry Tools;state=2108156;dir=1;layer=10;row=0;pos=339;prop=100000;bestw=292;besth=40;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=520;floaty=288;floatw=309;floath=64|name=SolidBar;caption=Solid Tools;state=2108156;dir=1;layer=10;row=1;pos=1;prop=100000;bestw=392;besth=40;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=736;floaty=259;floatw=345;floath=64|name=ViewingBar;caption=Viewing Tools;state=2108156;dir=1;layer=10;row=1;pos=340;prop=100000;bestw=292;besth=40;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=679;floaty=334;floatw=309;floath=64|name=TransformBar;caption=Transformation Tools;state=2108159;dir=1;layer=10;row=0;pos=52;prop=100000;bestw=256;besth=40;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=618;floaty=411;floatw=273;floath=64|dock_size(5,0,0)=504|dock_size(4,1,0)=234|dock_size(1,10,0)=42|dock_size(1,10,1)=42|");
#endif

CHeeksCADInterface heekscad_interface;

CHeeksFrame::CHeeksFrame( const wxString& title, const wxPoint& pos, const wxSize& size )
	: wxFrame((wxWindow *)NULL, -1, title, pos, size)
{
	wxGetApp().m_frame = this;
	m_next_id_for_button = ID_FIRST_EXTERNAL_BUTTON;
	m_printout = NULL;

	// File Menu
	wxMenu *file_menu = new wxMenu;
	file_menu->Append( wxID_NEW, _( "New" ) );
	file_menu->Append( wxID_OPEN, _( "Open" ) );
	file_menu->Append( wxID_SAVE, _( "Save" ) );
	file_menu->AppendSeparator();
    file_menu->Append(WXPRINT_PRINT, _("Print"));
    file_menu->Append(WXPRINT_PAGE_SETUP, _("Page Setup"));
    file_menu->Append(WXPRINT_PREVIEW, _("Print Preview"));
    m_recent_files_menu = new wxMenu;
    m_recent_files_menu->Append(-1, _T("test"));
    file_menu->Append(ID_OPEN_RECENT, _("Open Recent"), m_recent_files_menu);
	file_menu->Append( ID_IMPORT, _( "Import" ) );
	file_menu->Append( Menu_File_About, _( "About" ) );
	file_menu->AppendSeparator();
	file_menu->Append( Menu_File_Plugins, _( "Plugins" ) );
	file_menu->AppendSeparator();
	file_menu->Append( Menu_File_Quit, _( "Exit" ) );

	// Edit Menu
	wxMenu *edit_menu = new wxMenu;
	edit_menu->Append( wxID_UNDO, _( "Undo" ) );
	edit_menu->Append( wxID_REDO, _( "Redo" ) );
	edit_menu->AppendSeparator();
    edit_menu->Append(wxID_CUT, _("Cut"));
    edit_menu->Append(wxID_COPY, _("Copy"));
    edit_menu->Append(wxID_PASTE, _("Paste"));
	edit_menu->Append( wxID_DELETE, _( "Delete" ) );
	edit_menu->AppendSeparator();
	edit_menu->Append( ID_SELECT_MODE, _( "Select Mode" ) );

	// Geometry Menu
	wxMenu *geometry_menu = new wxMenu;
	geometry_menu->Append( ID_LINES, _( "Draw a sketch" ) );
	geometry_menu->Append( ID_CIRCLES, _( "Draw Circles" ) );
	geometry_menu->Append( ID_ILINE, _( "Draw Infinite Lines" ) );
	geometry_menu->Append( ID_POINTS, _( "Draw Points" ) );
	geometry_menu->Append( ID_REGSHAPES, _( "Draw Regular Shapes" ) );
	geometry_menu->AppendSeparator();
    geometry_menu->Append(ID_TEXT, _("Add Text"));
    geometry_menu->Append(ID_DIMENSIONING, _("Add Dimension"));
	geometry_menu->AppendSeparator();
    geometry_menu->Append(ID_COORDINATE_SYSTEM, _("Add Coordinate System"));
	
	// View Menu
	wxMenu *view_menu = new wxMenu;
	view_menu->Append( ID_MAG_PREVIOUS, _( "Previous view" ) );
	view_menu->AppendSeparator();
	view_menu->Append( ID_MAG, _( "Zoom window" ) );
	view_menu->Append( ID_MAG_EXTENTS, _( "Fit view to extents" ) );
	view_menu->Append( ID_MAG_NO_ROT, _( "Fit view to extents, but no rotation" ) );
	view_menu->AppendSeparator();
	view_menu->Append( ID_VIEW_ROT, _( "View rotate" ) );
    view_menu->Append(ID_VIEW_ZOOM, _("View zoom"));
    view_menu->Append(ID_FULL_SCREEN, _("Full screen"));
	view_menu->AppendSeparator();
	view_menu->Append( ID_REDRAW, _( "Redraw" ) );
	
	// Solids Menu
	wxMenu *solids_menu = new wxMenu;
	solids_menu->Append( ID_SPHERE, _( "Add a sphere" ) );
	solids_menu->Append( ID_CUBE, _( "Add a cube" ) );
	solids_menu->Append( ID_CYL, _( "Add a cylinder" ) );
	solids_menu->Append( ID_CONE, _( "Add a cone" ) );
	solids_menu->AppendSeparator();
	solids_menu->Append( ID_RULED_SURFACE, _( "Loft two sketches" ) );
	solids_menu->Append( ID_EXTRUDE, _( "Extrude a sketch" ) );
	solids_menu->AppendSeparator();
	solids_menu->Append( ID_SUBTRACT, _( "Cut" ) );
	solids_menu->Append( ID_FUSE, _( "Fuse" ) );
	solids_menu->Append( ID_COMMON, _( "Common" ) );
	solids_menu->Append( ID_FILLET, _( "Fillet" ) );
	solids_menu->Append( ID_CHAMFER, _( "Chamfer" ) );

	// Transformations Menu
	wxMenu *transform_menu = new wxMenu;
	transform_menu->Append( ID_MOVE_TRANSLATE, _( "Move Translate" ) );
	transform_menu->Append( ID_COPY_TRANSLATE, _( "Copy Translate" ) );
	transform_menu->AppendSeparator();
	transform_menu->Append( ID_MOVE_ROTATE, _( "Move Rotate" ) );
	transform_menu->Append( ID_COPY_ROTATE, _( "Copy Rotate" ) );
	transform_menu->AppendSeparator();
	transform_menu->Append( ID_MOVE_MIRROR, _( "Move Mirror" ) );
	transform_menu->Append( ID_COPY_MIRROR, _( "Copy Mirror" ) );
	transform_menu->AppendSeparator();
	transform_menu->Append( ID_MOVE_SCALE, _( "Move Scale" ) );

	// Window Menu
	m_menuWindow = new wxMenu;
	m_menuWindow->AppendCheckItem( Menu_View_Objects, _( "Objects" ) );
	m_menuWindow->AppendCheckItem( Menu_View_Options, _( "Options" ) );
	m_menuWindow->AppendCheckItem( Menu_View_Input, _( "Input" ) );
	m_menuWindow->AppendCheckItem( Menu_View_Properties, _( "Properties" ) );
	m_menuWindow->AppendCheckItem( Menu_View_ToolBar, _( "Tool Bar" ) );
	m_menuWindow->AppendCheckItem( Menu_View_SolidBar, _( "Solids Tool Bar" ) );
	m_menuWindow->AppendCheckItem( Menu_View_GeometryBar, _( "Geometry Tool Bar" ) );
	m_menuWindow->AppendCheckItem( Menu_View_ViewingBar, _( "Viewing Tool Bar" ) );
	m_menuWindow->AppendCheckItem( Menu_View_TransformBar, _( "Transformations Tool Bar" ) );
	m_menuWindow->AppendCheckItem( Menu_View_StatusBar, _( "Status Bar" ) );

	// Add them to the main menu
	m_menuBar = new wxMenuBar;
	m_menuBar->Append( file_menu, _( "File" ) );
	m_menuBar->Append( edit_menu, _( "Edit" ) );
	m_menuBar->Append( geometry_menu, _( "Geometry" ) );
	m_menuBar->Append( view_menu, _( "View" ) );
	m_menuBar->Append( solids_menu, _( "Solid" ) );
	m_menuBar->Append( transform_menu, _( "Transform" ) );
	m_menuBar->Append( m_menuWindow, _( "Window" ) );
	SetMenuBar( m_menuBar );

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

	m_tree_canvas = new CTreeCanvas(this);
    m_tree_canvas->SetCursor(wxCursor(wxCURSOR_MAGNIFIER));

    m_options = new COptionsCanvas(this);
	m_input_canvas = new CInputModeCanvas(this);

    m_properties = new CObjPropsCanvas(this);

	m_statusBar = CreateStatusBar();
	SetStatusText( _T( "" ) );

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

				wxDynamicLibrary* shared_library = new wxDynamicLibrary(fn.GetFullPath());
				if(shared_library->IsLoaded()){
					void(*OnStartUp)(CHeeksCADInterface*) = (void (*)(CHeeksCADInterface*))(shared_library->GetSymbol(_T("OnStartUp")));
					if(OnStartUp)
					{
						(*OnStartUp)(&heekscad_interface);
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
		void(*OnFrameDelete)() = (void(*)())(shared_library->GetSymbol(_T("OnFrameDelete")));
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
}

void CHeeksFrame::OnKeyDown(wxKeyEvent& event)
{
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

void CHeeksFrame::OnQuit( wxCommandEvent& WXUNUSED( event ) )
{
	if(!wxGetApp().CheckForModifiedDoc())
		return;
	Close(TRUE);
}

void CHeeksFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
	wxString str = wxString(_T("HeeksCAD, written by Dan Heeks danheeks@gmail.com\n\nusing Open CASCADE solid modeller - http://www.opencascade.org"))
		+ wxString(_T("\n\nwindows made with wxWidgets - http://wxwidgets.org"))
		+ wxString(_T("\n\ntext uses glFont Copyright (c) 1998 Brad Fish E-mail: bhf5@email.byu.edu Web: http://students.cs.byu.edu/~bfish/"))
		+ wxString(_T("\n\nWith contributions from:\n Hirutso Enni"))
		+ wxString(_T("\n\nThis is free, open source software."));

	wxString version_str = wxGetApp().m_version_number;
	version_str.Replace(_T(" "), _T("."));

	wxMessageBox( str, version_str, wxOK | wxICON_INFORMATION, this );
}

void CHeeksFrame::OnPlugins( wxCommandEvent& WXUNUSED( event ) )
{
	CPluginsDialog dlg(this);
	dlg.ShowModal();
}

void CHeeksFrame::OnViewObjects( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = m_aui_manager->GetPane(m_tree_canvas);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		m_aui_manager->Update();
	}
}

void CHeeksFrame::OnUpdateViewObjects( wxUpdateUIEvent& event )
{
	event.Check(m_aui_manager->GetPane(m_tree_canvas).IsShown());
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

void CHeeksFrame::OnViewGeometryBar( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = m_aui_manager->GetPane(m_geometryBar);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		m_aui_manager->Update();
	}
}

void CHeeksFrame::OnUpdateViewGeometryBar( wxUpdateUIEvent& event )
{
	event.Check(m_aui_manager->GetPane(m_geometryBar).IsShown());
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

void CHeeksFrame::OnViewTransformBar( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = m_aui_manager->GetPane(m_transformBar);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		m_aui_manager->Update();
	}
}

void CHeeksFrame::OnUpdateViewTransformBar( wxUpdateUIEvent& event )
{
	event.Check(m_aui_manager->GetPane(m_transformBar).IsShown());
}

void CHeeksFrame::OnViewStatusBar( wxCommandEvent& event )
{
	m_statusBar->Show(event.IsChecked());
}

void CHeeksFrame::OnUpdateViewStatusBar( wxUpdateUIEvent& event )
{
	event.Check(m_statusBar->IsShown());
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

void CHeeksFrame::OnViewProperties( wxCommandEvent& event )
{
	wxAuiPaneInfo& pane_info = m_aui_manager->GetPane(m_properties);
	if(pane_info.IsOk()){
		pane_info.Show(event.IsChecked());
		m_aui_manager->Update();
	}
}

void CHeeksFrame::OnUpdateOpenRecent( wxUpdateUIEvent& event )
{
	size_t size = m_recent_files_menu->GetMenuItemCount();
	std::list<wxMenuItem*> menu_items;
	for(size_t i = 0; i< size; i++)menu_items.push_back(m_recent_files_menu->FindItemByPosition(i));
	for(std::list<wxMenuItem*>::iterator It = menu_items.begin(); It != menu_items.end(); It++)
	{
		wxMenuItem* menu_item = *It;
		m_recent_files_menu->Delete(menu_item);
	}

	int recent_id = ID_RECENT_FIRST;
	for(std::list< wxString >::iterator It = wxGetApp().m_recent_files.begin(); It != wxGetApp().m_recent_files.end() && recent_id < ID_RECENT_FIRST + MAX_RECENT_FILES; It++, recent_id++)
	{
		wxString& filepath = *It;
		m_recent_files_menu->Append(recent_id, filepath, filepath);
	}
}

void CHeeksFrame::OnUpdateViewProperties( wxUpdateUIEvent& event )
{
	event.Check(m_aui_manager->GetPane(m_properties).IsShown());
}

void CHeeksFrame::OnSelectModeButton( wxCommandEvent& WXUNUSED( event ) )
{
	wxGetApp().SetInputMode((CInputMode*)(wxGetApp().m_select_mode));
}

void CHeeksFrame::OnLinesButton( wxCommandEvent& WXUNUSED( event ) )
{
	line_strip.drawing_mode = LineDrawingMode;
	wxGetApp().SetInputMode(&line_strip);
}

void CHeeksFrame::OnPointsButton( wxCommandEvent& WXUNUSED( event ) )
{
	wxGetApp().SetInputMode(&point_drawing);
}

void CHeeksFrame::OnRegularShapesButton( wxCommandEvent& WXUNUSED( event ) )
{
	wxGetApp().SetInputMode(&regular_shapes_drawing);
}

void CHeeksFrame::OnTextButton( wxCommandEvent& WXUNUSED( event ) )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(true);
	HText* new_object = new HText(mat, _T("text"), &(wxGetApp().current_color));
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().m_marked_list->Clear(true);
	wxGetApp().m_marked_list->Add(new_object, true);
	wxGetApp().SetInputMode(wxGetApp().m_select_mode);
	wxGetApp().Repaint();
}

void CHeeksFrame::OnDimensioningButton( wxCommandEvent& WXUNUSED( event ) )
{
	wxGetApp().SetInputMode(&dimension_drawing);
}

void CHeeksFrame::OnCirclesButton( wxCommandEvent& WXUNUSED( event ) )
{
	line_strip.drawing_mode = CircleDrawingMode;
	wxGetApp().SetInputMode(&line_strip);
}

void CHeeksFrame::OnILineButton( wxCommandEvent& WXUNUSED( event ) )
{
	line_strip.drawing_mode = ILineDrawingMode;
	wxGetApp().SetInputMode(&line_strip);
}

void CHeeksFrame::OnCoordinateSystem( wxCommandEvent& WXUNUSED( event ) )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(false);
	gp_Pnt o = gp_Pnt(0, 0, 0).Transformed(mat);
	gp_Dir x = gp_Dir(1, 0, 0).Transformed(mat);
	gp_Dir y = gp_Dir(0, 1, 0).Transformed(mat);
	CoordinateSystem* new_object = new CoordinateSystem(_("Coordinate System"), o, x, y);
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().m_marked_list->Clear(true);
	wxGetApp().m_marked_list->Add(new_object, true);
	wxGetApp().SetInputMode(wxGetApp().m_select_mode);
	wxGetApp().Repaint();

	// and pick from three points
	new_object->PickFrom3Points();
}

void CHeeksFrame::OnOpenButton( wxCommandEvent& event )
{
    wxFileDialog dialog(this, _("Open file"), wxEmptyString, wxEmptyString, wxGetApp().GetKnownFilesWildCardString());
    dialog.CentreOnParent();

    if (dialog.ShowModal() == wxID_OK)
    {
		if(wxGetApp().CheckForModifiedDoc())
		{
			wxGetApp().Reset();
			wxGetApp().OpenFile(dialog.GetPath().c_str());
			wxGetApp().OnNewOrOpen(true);
			wxGetApp().SetLikeNewFile();
		}
    }
}

void CHeeksFrame::OnImportButton( wxCommandEvent& event )
{
    wxFileDialog dialog(this, _("Import file"), wxEmptyString, wxEmptyString, wxGetApp().GetKnownFilesWildCardString());
    dialog.CentreOnParent();

    if (dialog.ShowModal() == wxID_OK)
    {
		wxGetApp().OpenFile(dialog.GetPath().c_str(), true);
    }
}

void CHeeksFrame::OnSaveButton( wxCommandEvent& event )
{
    wxGetApp().SaveFile( wxGetApp().m_filepath.c_str(), true );
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
	if(wxGetApp().CheckForModifiedDoc())
	{
		wxGetApp().Reset();
		wxGetApp().OnNewOrOpen(false);
		wxGetApp().SetLikeNewFile();
		wxGetApp().SetFrameTitle();
		wxGetApp().Repaint();
	}
}

void CHeeksFrame::OnCutButton( wxCommandEvent& event )
{
	wxGetApp().m_marked_list->CutSelectedItems();
}

void CHeeksFrame::OnUpdateCut( wxUpdateUIEvent& event )
{
	event.Enable(wxGetApp().m_marked_list->size() > 0);
}

void CHeeksFrame::OnCopyButton( wxCommandEvent& event )
{
	wxGetApp().m_marked_list->CopySelectedItems();
}

void CHeeksFrame::OnUpdateCopy( wxUpdateUIEvent& event )
{
	event.Enable(wxGetApp().m_marked_list->size() > 0);
}

void CHeeksFrame::OnPasteButton( wxCommandEvent& event )
{
	wxGetApp().Paste(NULL);
}

void CHeeksFrame::OnDeleteButton( wxCommandEvent& event )
{
	wxGetApp().DeleteUndoably(wxGetApp().m_marked_list->list());
}

void CHeeksFrame::OnUpdateDelete( wxUpdateUIEvent& event )
{
	event.Enable(wxGetApp().m_marked_list->size() > 0);
}

void CHeeksFrame::OnUpdatePaste( wxUpdateUIEvent& event )
{
	event.Enable(wxGetApp().IsPasteReady());
}

void CHeeksFrame::OnSubtractButton( wxCommandEvent& event )
{
	if(!wxGetApp().CheckForNOrMore(wxGetApp().m_marked_list->list(), 2, SolidType, _("Pick two or more solids, the first one will be cut by the others"), _("Subtract Solids")))return;
	CShape::CutShapes(wxGetApp().m_marked_list->list());
}

void CHeeksFrame::OnFuseButton( wxCommandEvent& event )
{
	if(!wxGetApp().CheckForNOrMore(wxGetApp().m_marked_list->list(), 2, SolidType, _("Pick two or more solids to be fused together"), _("Fuse Solids")))return;
	CShape::FuseShapes(wxGetApp().m_marked_list->list());
}

void CHeeksFrame::OnCommonButton( wxCommandEvent& event )
{
	if(!wxGetApp().CheckForNOrMore(wxGetApp().m_marked_list->list(), 2, SolidType, _("Pick two or more solids, only the shape that is contained by all of them will remain"), _("Intersection of Solids")))return;
	CShape::CommonShapes(wxGetApp().m_marked_list->list());
}

void CHeeksFrame::OnFilletButton( wxCommandEvent& event )
{
	if(!wxGetApp().CheckForNOrMore(wxGetApp().m_marked_list->list(), 1, EdgeType, _("Pick one or more edges to add a fillet to"), _("Edge Fillet")))return;
	double rad = 2.0;
	HeeksConfig config;
	config.Read(_T("EdgeBlendRadius"), &rad);
	if(wxGetApp().InputDouble(_("Enter Blend Radius"), _("Radius"), rad))
	{
		CShape::FilletOrChamferEdges(wxGetApp().m_marked_list->list(), rad);
		config.Write(_T("EdgeBlendRadius"), rad);
	}
}

void CHeeksFrame::OnChamferButton( wxCommandEvent& event )
{
	if(!wxGetApp().CheckForNOrMore(wxGetApp().m_marked_list->list(), 1, EdgeType, _("Pick one or more edges to add a chamfer to"), _("Edge Chamfer")))return;
	double rad = 2.0;
	HeeksConfig config;
	config.Read(_T("EdgeChamferDist"), &rad);
	if(wxGetApp().InputDouble(_("Enter chamfer distance"), _("Distance"), rad))
	{
		CShape::FilletOrChamferEdges(wxGetApp().m_marked_list->list(), rad, true);
		config.Write(_T("EdgeChamferDist"), rad);
	}
}

void CHeeksFrame::OnRuledSurfaceButton( wxCommandEvent& event )
{
	if(!wxGetApp().CheckForNOrMore(wxGetApp().m_marked_list->list(), 2, SketchType, _("Pick two or more sketches, to create a lofted solid between\n( hold down Ctrl key to select more than one solid )"), _("Lofted Body")))return;
	PickCreateRuledSurface();
}

void CHeeksFrame::OnExtrudeButton( wxCommandEvent& event )
{
	if(!wxGetApp().CheckForNOrMore(wxGetApp().m_marked_list->list(), 1, SketchType, _("Pick one or more sketches, to create extruded body from\n( hold down Ctrl key to select more than one solid )"), _("Extrude")))return;
	PickCreateExtrusion();
}

void CHeeksFrame::OnSphereButton( wxCommandEvent& event )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(true);
	CSphere* new_object = new CSphere(gp_Pnt(0, 0, 0).Transformed(mat), 5, _("Sphere"), HeeksColor(240, 191, 191));
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().m_marked_list->Clear(true);
	wxGetApp().m_marked_list->Add(new_object, true);
	wxGetApp().SetInputMode(wxGetApp().m_select_mode);
	wxGetApp().Repaint();
}

void CHeeksFrame::OnCubeButton( wxCommandEvent& event )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(false);
	CCuboid* new_object = new CCuboid(gp_Ax2(gp_Pnt(0, 0, 0).Transformed(mat), gp_Dir(0, 0, 1).Transformed(mat), gp_Dir(1, 0, 0).Transformed(mat)), 10, 10, 10, _("Cuboid"), HeeksColor(191, 240, 191));
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().m_marked_list->Clear(true);
	wxGetApp().m_marked_list->Add(new_object, true);
	wxGetApp().SetInputMode(wxGetApp().m_select_mode);
	wxGetApp().Repaint();
}

void CHeeksFrame::OnCylButton( wxCommandEvent& event )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(true);
	CCylinder* new_object = new CCylinder(gp_Ax2(gp_Pnt(0, 0, 0).Transformed(mat), gp_Dir(0, 0, 1).Transformed(mat), gp_Dir(1, 0, 0).Transformed(mat)), 5, 10, _("Cylinder"), HeeksColor(191, 191, 240));
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().m_marked_list->Clear(true);
	wxGetApp().m_marked_list->Add(new_object, true);
	wxGetApp().SetInputMode(wxGetApp().m_select_mode);
	wxGetApp().Repaint();
}

void CHeeksFrame::OnConeButton( wxCommandEvent& event )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(true);
	CCone* new_object = new CCone(gp_Ax2(gp_Pnt(0, 0, 0).Transformed(mat), gp_Dir(0, 0, 1).Transformed(mat), gp_Dir(1, 0, 0).Transformed(mat)), 10, 5, 20, _("Cone"), HeeksColor(240, 240, 191));
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().m_marked_list->Clear(true);
	wxGetApp().m_marked_list->Add(new_object, true);
	wxGetApp().SetInputMode(wxGetApp().m_select_mode);
	wxGetApp().Repaint();
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

void CHeeksFrame::OnViewRotateButton( wxCommandEvent& event )
{
	wxGetApp().SetInputMode(wxGetApp().viewrotating);
}

void CHeeksFrame::OnViewZoomButton( wxCommandEvent& event )
{
	wxGetApp().SetInputMode(wxGetApp().viewzooming);
}

void CHeeksFrame::OnFullScreenButton( wxCommandEvent& event )
{
	wxGetApp().m_frame->ShowFullScreen(true);
}

void CHeeksFrame::OnMoveTranslateButton( wxCommandEvent& event )
{
	TransformTools::Translate(false);
}

void CHeeksFrame::OnCopyTranslateButton( wxCommandEvent& event )
{
	TransformTools::Translate(true);
}

void CHeeksFrame::OnMoveRotateButton( wxCommandEvent& event )
{
	TransformTools::Rotate(false);
}

void CHeeksFrame::OnCopyRotateButton( wxCommandEvent& event )
{
	TransformTools::Rotate(true);
}

void CHeeksFrame::OnMoveMirrorButton( wxCommandEvent& event )
{
	TransformTools::Mirror(false);
}

void CHeeksFrame::OnCopyMirrorButton( wxCommandEvent& event )
{
	TransformTools::Mirror(true);
}

void CHeeksFrame::OnMoveScaleButton( wxCommandEvent& event )
{
	TransformTools::Scale(false);
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

int CHeeksFrame::AddToolBarTool(wxToolBar* toolbar, const wxString& title, const wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&))
{
	while(m_external_buttons.find(m_next_id_for_button) != m_external_buttons.end())
	{
		// already used
		m_next_id_for_button++;
	}

	if(m_next_id_for_button >= ID_FIRST_POP_UP_MENU_TOOL)
	{
		// too many button IDs!
		wxMessageBox(_T("too many button IDs!, see CHeeksFrame::AddToolBarTool"));
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

	if(m_next_id_for_button >= ID_FIRST_POP_UP_MENU_TOOL)
	{
		// too many button IDs!
		wxMessageBox(_T("too many button IDs!, see CHeeksFrame::AddMenuCheckItem"));
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

int CHeeksFrame::AddMenuItem(wxMenu* menu, const wxString& title, void(*onButtonFunction)(wxCommandEvent&)){
	while(m_external_buttons.find(m_next_id_for_button) != m_external_buttons.end())
	{
		// already used
		m_next_id_for_button++;
	}

	if(m_next_id_for_button >= ID_FIRST_POP_UP_MENU_TOOL)
	{
		// too many button IDs!
		wxMessageBox(_T("too many button IDs!, see CHeeksFrame::AddMenuItem"));
	}

	int id_to_use = m_next_id_for_button;
	menu->Append(id_to_use, title);
	SExternalButtonFunctions ebf;
	ebf.on_button = onButtonFunction;
	ebf.on_update_button = NULL;
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
		int id_used_for_button = wxGetApp().m_frame->AddToolBarTool(toolbar, tool->GetTitle(), *bitmap, tool->GetToolTip(), OnTool);
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

void CHeeksFrame::OnPrint(wxCommandEvent& WXUNUSED(event))
{
    wxPrintDialogData printDialogData(* wxGetApp().m_printData);

    wxPrinter printer(& printDialogData);
	m_printout = new HeeksPrintout(_T("Heeks printout"));
    if (!printer.Print(this, m_printout, true /*prompt*/))
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

	delete m_printout;
	m_printout = NULL;
}

void CHeeksFrame::OnPrintPreview(wxCommandEvent& WXUNUSED(event))
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

    wxPreviewFrame *frame = new wxPreviewFrame(preview, this, _("Demo Print Preview"), wxPoint(100, 100), wxSize(600, 650));
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show();
}

void CHeeksFrame::OnPageSetup(wxCommandEvent& WXUNUSED(event))
{
    (*wxGetApp().m_pageSetupData) = *(wxGetApp().m_printData);

	wxPageSetupDialog pageSetupDialog(this, wxGetApp().m_pageSetupData);
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

    m_toolBar->AddTool(wxID_NEW, _T("New"), ToolImage(_T("new")), _("New file"));
    m_toolBar->AddTool(wxID_OPEN, _T("Open"), ToolImage(_T("open")), _("Open file"));
    m_toolBar->AddTool(wxID_SAVE, _T("Save"), ToolImage(_T("save")), _("Save file"));
    m_toolBar->AddTool(wxID_CUT, _T("Cut"), ToolImage(_T("cut")), _("Cut selected items to the clipboard"));
    m_toolBar->AddTool(wxID_COPY, _T("Copy"), ToolImage(_T("copy")), _("Copy selected items to the clipboard"));
    m_toolBar->AddTool(wxID_PASTE, _T("Paste"), ToolImage(_T("paste")), _("Paste items from the clipboard"));
    m_toolBar->AddTool(wxID_UNDO, _T("Undo"), ToolImage(_T("undo")), _("Undo the previous command"));
    m_toolBar->AddTool(wxID_REDO, _T("Redo"), ToolImage(_T("redo")), _("Redo the next command"));
    m_toolBar->AddTool(ID_SELECT_MODE, _T("Select"), ToolImage(_T("select")), _("Select Mode"));
    m_geometryBar->AddTool(ID_LINES, _T("Lines"), ToolImage(_T("lines")), _("Draw a sketch"));
    m_geometryBar->AddTool(ID_CIRCLES, _T("Circles"), ToolImage(_T("circles")), _("Start Circle Drawing"));
    m_geometryBar->AddTool(ID_ILINE, _T("ILine"), ToolImage(_T("iline")), _("Start Drawing Infinite Lines"));
    m_geometryBar->AddTool(ID_POINTS, _T("Points"), ToolImage(_T("point")), _("Start Drawing Points"));
    m_geometryBar->AddTool(ID_REGSHAPES, _T("Regular Shapes"), ToolImage(_T("regshapes")), _("Draw regular shapes; rectangles, polygons, obrounds"));
    m_geometryBar->AddTool(ID_TEXT, _T("Text"), ToolImage(_T("text")), _("Add a text object"));
    m_geometryBar->AddTool(ID_COORDINATE_SYSTEM, _T("CoordSys"), ToolImage(_T("coordsys")), _("Create a Coordinate System"));
    m_geometryBar->AddTool(ID_DIMENSIONING, _T("Dimensioning"), ToolImage(_T("dimension")), _("Add a dimension"));
    m_solidBar->AddTool(ID_SPHERE, _T("Sphere"), ToolImage(_T("sphere")), _("Add a sphere"));
    m_solidBar->AddTool(ID_CUBE, _T("Cube"), ToolImage(_T("cube")), _("Add a cube"));
    m_solidBar->AddTool(ID_CYL, _T("Cylinder"), ToolImage(_T("cyl")), _("Add a cylinder"));
    m_solidBar->AddTool(ID_CONE, _T("Cone"), ToolImage(_T("cone")), _("Add a cone"));
    m_solidBar->AddTool(ID_RULED_SURFACE, _T("Ruled Surface"), ToolImage(_T("ruled")), _("Create a lofted face"));
    m_solidBar->AddTool(ID_EXTRUDE, _T("Extrude"), ToolImage(_T("extrude")), _("Extrude a wire or face"));
	m_solidBar->AddTool(ID_SUBTRACT, _T("Cut"), ToolImage(_T("subtract")), _("Cut one solid from another"));
	m_solidBar->AddTool(ID_FUSE, _T("Fuse"), ToolImage(_T("fuse")), _("Fuse one solid to another"));
	m_solidBar->AddTool(ID_COMMON, _T("Common"), ToolImage(_T("common")), _("Find common solid between two solids"));
	m_solidBar->AddTool(ID_FILLET, _T("Fillet"), ToolImage(_T("fillet")), _("Make a fillet on selected edges"));
	m_solidBar->AddTool(ID_CHAMFER, _T("Chamfer"), ToolImage(_T("chamfer")), _("Make a chamfer on selected edges"));
	m_viewingBar->AddTool(ID_MAG_PREVIOUS, _T("View Back"), ToolImage(_T("magprev")), _("Go back to previous view"));
	m_viewingBar->AddTool(ID_MAG, _T("Zoom Window"), ToolImage(_T("mag")), _("Zoom in to a dragged window"));
	m_viewingBar->AddTool(ID_MAG_EXTENTS, _T("Mag Extents"), ToolImage(_T("magextents")), _("Zoom in to fit the extents of the drawing into the graphics window"));
	m_viewingBar->AddTool(ID_MAG_NO_ROT, _T("Mag No Rotation"), ToolImage(_T("magnorot")), _("Zoom in to fit the extents of the drawing into the graphics window, but without rotating the view"));
	m_viewingBar->AddTool(ID_VIEW_ROT, _T("View Rotate"), ToolImage(_T("viewrot")), _("Enter view rotating mode"));
	m_viewingBar->AddTool(ID_VIEW_ZOOM, _T("View Zoom"), ToolImage(_T("zoom")), _("Drag to zoom in and out"));
	m_viewingBar->AddTool(ID_FULL_SCREEN, _T("FullScreen"), ToolImage(_T("fullscreen")), _("Switch to full screen view ( press escape to return )"));
    m_viewingBar->AddTool(ID_REDRAW, _T("Redraw"), ToolImage(_T("redraw")), _("Redraw"));
	m_transformBar->AddTool(ID_MOVE_TRANSLATE, _T("Move Translate"), ToolImage(_T("movet")), _("Translate selected items"));
	m_transformBar->AddTool(ID_COPY_TRANSLATE, _T("Copy Translate"), ToolImage(_T("copyt")), _("Copy and translate selected items"));
	m_transformBar->AddTool(ID_MOVE_ROTATE, _T("Move Rotate"), ToolImage(_T("mover")), _("Rotate selected items"));
	m_transformBar->AddTool(ID_COPY_ROTATE, _T("Copy Rotate"), ToolImage(_T("copyr")), _("Copy and rotate selected items"));
	m_transformBar->AddTool(ID_MOVE_MIRROR, _T("Move Mirror"), ToolImage(_T("movem")), _("Mirror selected items"));
	m_transformBar->AddTool(ID_COPY_MIRROR, _T("Copy Mirror"), ToolImage(_T("copym")), _("Copy and mirror selected items"));
	m_transformBar->AddTool(ID_MOVE_SCALE, _T("Move Scale"), ToolImage(_T("moves")), _("Scale selected items"));
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
