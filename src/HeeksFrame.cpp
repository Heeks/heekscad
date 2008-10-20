// HeeksFrame.cpp

#include "stdafx.h"
#include "HeeksFrame.h"
#include "../interface/Tool.h"
#include "../interface/ToolList.h"
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
#include "RuledSurface.h"
#include "Sphere.h"
#include "Cuboid.h"
#include "Cylinder.h"
#include "Cone.h"
#include "TransformTools.h"
#include "CoordinateSystem.h"
#include "wx/dnd.h"
#include "wx/filename.h"
#include <fstream>

using namespace std;

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
EVT_MENU(wxID_OPEN, CHeeksFrame::OnOpenButton)
EVT_MENU(wxID_SAVE, CHeeksFrame::OnSaveButton)
EVT_MENU(wxID_NEW, CHeeksFrame::OnNewButton)

EVT_MENU(wxID_CUT, CHeeksFrame::OnCutButton)
EVT_UPDATE_UI(wxID_CUT, CHeeksFrame::OnUpdateCut)
EVT_MENU(wxID_COPY, CHeeksFrame::OnCopyButton)
EVT_UPDATE_UI(wxID_COPY, CHeeksFrame::OnUpdateCopy)
EVT_MENU(wxID_PASTE, CHeeksFrame::OnPasteButton)
EVT_UPDATE_UI(wxID_PASTE, CHeeksFrame::OnUpdatePaste)

EVT_UPDATE_UI(ID_OPEN_RECENT, CHeeksFrame::OnUpdateOpenRecent)
EVT_MENU(ID_IMPORT, CHeeksFrame::OnImportButton)
EVT_MENU_RANGE(	ID_RECENT_FIRST, ID_RECENT_FIRST + MAX_RECENT_FILES, CHeeksFrame::OnRecentFile)
EVT_MENU(ID_LINES, CHeeksFrame::OnLinesButton)
EVT_MENU(ID_CIRCLES, CHeeksFrame::OnCirclesButton)
EVT_MENU(ID_ILINE, CHeeksFrame::OnILineButton)
EVT_MENU(ID_COORDINATE_SYSTEM, CHeeksFrame::OnCoordinateSystem)
EVT_MENU(ID_SELECT_MODE, CHeeksFrame::OnSelectModeButton)
EVT_MENU(ID_SPHERE, CHeeksFrame::OnSphereButton)
EVT_MENU(ID_CUBE, CHeeksFrame::OnCubeButton)
EVT_MENU(ID_CYL, CHeeksFrame::OnCylButton)
EVT_MENU(ID_CONE, CHeeksFrame::OnConeButton)
EVT_MENU(ID_SUBTRACT, CHeeksFrame::OnSubtractButton)
EVT_MENU(ID_FUSE, CHeeksFrame::OnFuseButton)
EVT_MENU(ID_COMMON, CHeeksFrame::OnCommonButton)
EVT_MENU(ID_REDRAW, CHeeksFrame::OnRedrawButton)
EVT_MENU(ID_RULED_SURFACE, CHeeksFrame::OnRuledSurfaceButton)
EVT_MENU(ID_EXTRUDE, CHeeksFrame::OnExtrudeButton)
EVT_MENU(ID_MAG, CHeeksFrame::OnMagButton)
EVT_MENU(ID_UNDO, CHeeksFrame::OnUndoButton)
EVT_MENU(ID_REDO, CHeeksFrame::OnRedoButton)
EVT_MENU(ID_MAG_EXTENTS, CHeeksFrame::OnMagExtentsButton)
EVT_MENU(ID_MAG_NO_ROT, CHeeksFrame::OnMagNoRotButton)
EVT_MENU(ID_MAG_PREVIOUS, CHeeksFrame::OnMagPreviousButton)
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

static wxString default_layout_string = _T("layout2|name=Graphics;caption=Graphics;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=800;besth=600;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Objects;caption=Objects;state=2099196;dir=4;layer=1;row=0;pos=0;prop=100000;bestw=300;besth=400;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Options;caption=Options;state=2099196;dir=4;layer=1;row=0;pos=1;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Input;caption=Input;state=2099196;dir=4;layer=1;row=0;pos=2;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=Properties;caption=Properties;state=2099196;dir=4;layer=1;row=0;pos=3;prop=100000;bestw=300;besth=200;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=ToolBar;caption=General Tools;state=2108156;dir=1;layer=10;row=0;pos=0;prop=100000;bestw=351;besth=39;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=GeomBar;caption=Geometry Tools;state=2108156;dir=1;layer=10;row=0;pos=245;prop=100000;bestw=156;besth=39;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=SolidBar;caption=Solid Tools;state=2108156;dir=1;layer=10;row=6;pos=0;prop=100000;bestw=390;besth=39;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=444;floaty=105;floatw=407;floath=65|name=ViewingBar;caption=Viewing Tools;state=2108156;dir=1;layer=10;row=6;pos=401;prop=100000;bestw=195;besth=39;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|name=TransformBar;caption=Transformation Tools;state=2108156;dir=1;layer=10;row=0;pos=373;prop=100000;bestw=273;besth=39;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|dock_size(5,0,0)=504|dock_size(4,1,0)=302|dock_size(1,10,0)=41|dock_size(1,10,6)=41|");

CHeeksFrame::CHeeksFrame( const wxString& title, const wxPoint& pos, const wxSize& size )
	: wxFrame((wxWindow *)NULL, -1, title, pos, size)
{
	wxGetApp().m_frame = this;
	m_next_id_for_button = ID_FIRST_EXTERNAL_BUTTON;

	// File Menu
	wxMenu *menuFile = new wxMenu;
	menuFile->Append( wxID_NEW, wxT( "&New..." ) );
	menuFile->Append( wxID_OPEN, wxT( "&Open..." ) );
	menuFile->Append( wxID_SAVE, wxT( "&Save..." ) );
	menuFile->Append( wxID_SAVEAS, wxT( "Save &As..." ) );

    m_recent_files_menu = new wxMenu;
    m_recent_files_menu->Append(-1, _T("&test"), _T("test submenu item"));
    menuFile->Append(ID_OPEN_RECENT, _T("Open Recent"), m_recent_files_menu);

	menuFile->Append( ID_IMPORT, wxT( "Import..." ) );

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
	m_menuView->AppendCheckItem( Menu_View_GeometryBar, wxT( "&Geometry Tool Bar" ) );
	m_menuView->AppendCheckItem( Menu_View_ViewingBar, wxT( "&Viewing Tool Bar" ) );
	m_menuView->AppendCheckItem( Menu_View_TransformBar, wxT( "T&ransformations Tool Bar" ) );
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

	m_geometryBar = new wxToolBar(this, -1, wxDefaultPosition, wxSize(600, -1), wxTB_NODIVIDER | wxTB_FLAT);
	m_geometryBar->SetToolBitmapSize(wxSize(32, 32));

	m_solidBar = new wxToolBar(this, -1, wxDefaultPosition, wxSize(600, -1), wxTB_NODIVIDER | wxTB_FLAT);
	m_solidBar->SetToolBitmapSize(wxSize(32, 32));

	m_viewingBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
	m_viewingBar->SetToolBitmapSize(wxSize(32, 32));

	m_transformBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
	m_transformBar->SetToolBitmapSize(wxSize(32, 32));

	wxString exe_folder = wxGetApp().GetExeFolder();

	// main tool bar
    m_toolBar->AddTool(wxID_NEW, _T("New"), wxBitmap(exe_folder + _T("/bitmaps/new.png"), wxBITMAP_TYPE_PNG), _T("New file"));
    m_toolBar->AddTool(wxID_OPEN, _T("Open"), wxBitmap(exe_folder + _T("/bitmaps/open.png"), wxBITMAP_TYPE_PNG), _T("Open file"));
    m_toolBar->AddTool(wxID_SAVE, _T("Save"), wxBitmap(exe_folder + _T("/bitmaps/save.png"), wxBITMAP_TYPE_PNG), _T("Save file"));
    m_toolBar->AddTool(wxID_CUT, _T("Cut"), wxBitmap(exe_folder + _T("/bitmaps/cut.png"), wxBITMAP_TYPE_PNG), _T("Cut selected items to the clipboard"));
    m_toolBar->AddTool(wxID_COPY, _T("Copy"), wxBitmap(exe_folder + _T("/bitmaps/copy.png"), wxBITMAP_TYPE_PNG), _T("Copy selected items to the clipboard"));
    m_toolBar->AddTool(wxID_PASTE, _T("Paste"), wxBitmap(exe_folder + _T("/bitmaps/paste.png"), wxBITMAP_TYPE_PNG), _T("Paste items from the clipboard"));
    m_toolBar->AddTool(ID_UNDO, _T("Undo"), wxBitmap(exe_folder + _T("/bitmaps/undo.png"), wxBITMAP_TYPE_PNG), _T("Undo the previous command"));
    m_toolBar->AddTool(ID_REDO, _T("Redo"), wxBitmap(exe_folder + _T("/bitmaps/redo.png"), wxBITMAP_TYPE_PNG), _T("Redo the next command"));
    m_toolBar->AddTool(ID_SELECT_MODE, _T("Select"), wxBitmap(exe_folder + _T("/bitmaps/select.png"), wxBITMAP_TYPE_PNG), _T("Select Mode"));
    m_toolBar->Realize();

	// geometry tool bar
    m_geometryBar->AddTool(ID_LINES, _T("Lines"), wxBitmap(exe_folder + _T("/bitmaps/lines.png"), wxBITMAP_TYPE_PNG), _T("Start Line Drawing"));
    m_geometryBar->AddTool(ID_CIRCLES, _T("Circles"), wxBitmap(exe_folder + _T("/bitmaps/circles.png"), wxBITMAP_TYPE_PNG), _T("Start Circle Drawing"));
    m_geometryBar->AddTool(ID_ILINE, _T("ILine"), wxBitmap(exe_folder + _T("/bitmaps/iline.png"), wxBITMAP_TYPE_PNG), _T("Start Drawing Infinite Lines"));
    m_geometryBar->AddTool(ID_COORDINATE_SYSTEM, _T("CoordSys"), wxBitmap(exe_folder + _T("/bitmaps/coordsys.png"), wxBITMAP_TYPE_PNG), _T("Create a Coordinate System"));
    m_geometryBar->Realize();

	// Solids tool bar
    m_solidBar->AddTool(ID_SPHERE, _T("Sphere"), wxBitmap(exe_folder + _T("/bitmaps/sphere.png"), wxBITMAP_TYPE_PNG), _T("Add a sphere"));
    m_solidBar->AddTool(ID_CUBE, _T("Cube"), wxBitmap(exe_folder + _T("/bitmaps/cube.png"), wxBITMAP_TYPE_PNG), _T("Add a cube"));
    m_solidBar->AddTool(ID_CYL, _T("Cylinder"), wxBitmap(exe_folder + _T("/bitmaps/cyl.png"), wxBITMAP_TYPE_PNG), _T("Add a cylinder"));
    m_solidBar->AddTool(ID_CONE, _T("Cone"), wxBitmap(exe_folder + _T("/bitmaps/cone.png"), wxBITMAP_TYPE_PNG), _T("Add a cone"));
    m_solidBar->AddTool(ID_RULED_SURFACE, _T("Ruled Surface"), wxBitmap(exe_folder + _T("/bitmaps/ruled.png"), wxBITMAP_TYPE_PNG), _T("Create a lofted face"));
    m_solidBar->AddTool(ID_EXTRUDE, _T("Extrude"), wxBitmap(exe_folder + _T("/bitmaps/extrude.png"), wxBITMAP_TYPE_PNG), _T("Extrude a wire or face"));
	m_solidBar->AddTool(ID_SUBTRACT, _T("Cut"), wxBitmap(exe_folder + _T("/bitmaps/subtract.png"), wxBITMAP_TYPE_PNG), _T("Cut one solid from another"));
	m_solidBar->AddTool(ID_FUSE, _T("Fuse"), wxBitmap(exe_folder + _T("/bitmaps/fuse.png"), wxBITMAP_TYPE_PNG), _T("Fuse one solid to another"));
	m_solidBar->AddTool(ID_COMMON, _T("Common"), wxBitmap(exe_folder + _T("/bitmaps/common.png"), wxBITMAP_TYPE_PNG), _T("Find common solid between two solids"));
    m_solidBar->AddTool(ID_REDRAW, _T("Redraw"), wxBitmap(exe_folder + _T("/bitmaps/redraw.png"), wxBITMAP_TYPE_PNG), _T("Redraw"));
	m_solidBar->Realize();

	// viewing tool bar
	m_viewingBar->AddTool(ID_MAG_PREVIOUS, _T("View Back"), wxBitmap(exe_folder + _T("/bitmaps/magprev.png"), wxBITMAP_TYPE_PNG), _T("Go back to previous view"));
	m_viewingBar->AddTool(ID_MAG, _T("Zoom Window"), wxBitmap(exe_folder + _T("/bitmaps/mag.png"), wxBITMAP_TYPE_PNG), _T("Zoom in to a dragged window"));
	m_viewingBar->AddTool(ID_MAG_EXTENTS, _T("Mag Extents"), wxBitmap(exe_folder + _T("/bitmaps/magextents.png"), wxBITMAP_TYPE_PNG), _T("Zoom in to fit the extents of the drawing into the graphics window"));
	m_viewingBar->AddTool(ID_MAG_NO_ROT, _T("Mag No Rotation"), wxBitmap(exe_folder + _T("/bitmaps/magnorot.png"), wxBITMAP_TYPE_PNG), _T("Zoom in to fit the extents of the drawing into the graphics window, but without rotating the view"));
	m_viewingBar->AddTool(ID_FULL_SCREEN, _T("FullScreen"), wxBitmap(exe_folder + _T("/bitmaps/fullscreen.png"), wxBITMAP_TYPE_PNG), _T("Switch to full screen view ( press escape to return )"));
	m_viewingBar->Realize();

	// transformations tool bar
	m_transformBar->AddTool(ID_MOVE_TRANSLATE, _T("Move Translate"), wxBitmap(exe_folder + _T("/bitmaps/movet.png"), wxBITMAP_TYPE_PNG), _T("Translate selected items"));
	m_transformBar->AddTool(ID_COPY_TRANSLATE, _T("Copy Translate"), wxBitmap(exe_folder + _T("/bitmaps/copyt.png"), wxBITMAP_TYPE_PNG), _T("Copy and translate selected items"));
	m_transformBar->AddTool(ID_MOVE_ROTATE, _T("Move Rotate"), wxBitmap(exe_folder + _T("/bitmaps/mover.png"), wxBITMAP_TYPE_PNG), _T("Rotate selected items"));
	m_transformBar->AddTool(ID_COPY_ROTATE, _T("Copy Rotate"), wxBitmap(exe_folder + _T("/bitmaps/copyr.png"), wxBITMAP_TYPE_PNG), _T("Copy and rotate selected items"));
	m_transformBar->AddTool(ID_MOVE_MIRROR, _T("Move Mirror"), wxBitmap(exe_folder + _T("/bitmaps/movem.png"), wxBITMAP_TYPE_PNG), _T("Mirror selected items"));
	m_transformBar->AddTool(ID_COPY_MIRROR, _T("Copy Mirror"), wxBitmap(exe_folder + _T("/bitmaps/copym.png"), wxBITMAP_TYPE_PNG), _T("Copy and mirror selected items"));
	m_transformBar->AddTool(ID_MOVE_SCALE, _T("Move Scale"), wxBitmap(exe_folder + _T("/bitmaps/moves.png"), wxBITMAP_TYPE_PNG), _T("Scale selected items"));
	m_transformBar->Realize();

	m_aui_manager->AddPane(m_graphics, wxAuiPaneInfo().Name(_T("Graphics")).Caption(_T("Graphics")).CentrePane().BestSize(wxSize(800, 600)));
	m_aui_manager->AddPane(m_left, wxAuiPaneInfo().Name(_T("Objects")).Caption(_T("Objects")).Left().Layer(1).BestSize(wxSize(300, 400)));
	m_aui_manager->AddPane(m_options, wxAuiPaneInfo().Name(_T("Options")).Caption(_T("Options")).Left().Layer(1).BestSize(wxSize(300, 200)));
	m_aui_manager->AddPane(m_input_canvas, wxAuiPaneInfo().Name(_T("Input")).Caption(_T("Input")).Left().Layer(1).BestSize(wxSize(300, 200)));
	m_aui_manager->AddPane(m_properties, wxAuiPaneInfo().Name(_T("Properties")).Caption(_T("Properties")).Left().Layer(1).BestSize(wxSize(300, 200)));
	m_aui_manager->AddPane(m_toolBar, wxAuiPaneInfo().Name(_T("ToolBar")).Caption(_T("General Tools")).ToolbarPane().Top());
	m_aui_manager->AddPane(m_geometryBar, wxAuiPaneInfo().Name(_T("GeomBar")).Caption(_T("Geometry Tools")).ToolbarPane().Top());
	m_aui_manager->AddPane(m_solidBar, wxAuiPaneInfo().Name(_T("SolidBar")).Caption(_T("Solid Tools")).ToolbarPane().Top());
	m_aui_manager->AddPane(m_viewingBar, wxAuiPaneInfo().Name(_T("ViewingBar")).Caption(_T("Viewing Tools")).ToolbarPane().Top());
	m_aui_manager->AddPane(m_transformBar, wxAuiPaneInfo().Name(_T("TransformBar")).Caption(_T("Transformation Tools")).ToolbarPane().Top());

	// add to hiding list for full screen mode
	wxGetApp().RegisterHideableWindow(m_left);
	wxGetApp().RegisterHideableWindow(m_options);
	wxGetApp().RegisterHideableWindow(m_input_canvas);
	wxGetApp().RegisterHideableWindow(m_properties);
	wxGetApp().RegisterHideableWindow(m_toolBar);
	wxGetApp().RegisterHideableWindow(m_geometryBar);
	wxGetApp().RegisterHideableWindow(m_solidBar);
	wxGetApp().RegisterHideableWindow(m_viewingBar);
	wxGetApp().RegisterHideableWindow(m_transformBar);

	// set xml reading functions
	wxGetApp().InitializeXMLFunctions();

	// load up any other dlls and call OnStartUp on each of them
	{
		::wxSetWorkingDirectory(wxGetApp().GetExeFolder());

#if wxUSE_UNICODE
		wifstream ifs(_T("AddIns.txt"));
#else
		ifstream ifs(_T("AddIns.txt"));
#endif
		wxChar str[1024];
		if(!(!ifs)){
			while(!ifs.eof()){
				ifs.getline(str, 1024);

				// strip white space
				wxString wstr(str);
				wstr = wstr.Trim();
				wstr = wstr.Trim(false);

				if(wstr.Len() == 0)continue;

				if(wstr[0] == '#')continue;

				wxFileName fn(wstr);
				fn.Normalize();
				wxString path = fn.GetPath();

				::wxSetWorkingDirectory(path);

				wxDynamicLibrary* shared_library = new wxDynamicLibrary(wstr);
				if(shared_library->IsLoaded()){
					wxGetApp().m_loaded_libraries.push_back(shared_library);
					void(*OnStartUp)() = (void (*)())(shared_library->GetSymbol(_T("OnStartUp")));
					(*OnStartUp)();
				}
				else{
					delete shared_library;
				}
			}
		}
	}

	SetDropTarget(new DnDFile(this));

	m_menuView->Append( Menu_View_ResetLayout, wxT( "Reset Layout" ) );

	//Read layout
	wxString str;
	wxGetApp().m_config->Read(_T("AuiPerspective"), &str, default_layout_string);
	m_aui_manager->LoadPerspective(str);

	m_aui_manager->Update();}

CHeeksFrame::~CHeeksFrame()
{
	// call the shared libraries function OnFrameDelete, so they can write profile strings while aui manager still exists
	for(std::list<wxDynamicLibrary*>::iterator It = wxGetApp().m_loaded_libraries.begin(); It != wxGetApp().m_loaded_libraries.end(); It++){
		wxDynamicLibrary* shared_library = *It;
		void(*OnFrameDelete)() = (void(*)())(shared_library->GetSymbol(_T("OnFrameDelete")));
		if(OnFrameDelete)(*OnFrameDelete)();
	}

	//Save the application layout
	wxString str = m_aui_manager->SavePerspective();
#if 1
	{
		ofstream ofs("layout.txt");
		ofs<<str;
	}
#endif

	wxGetApp().m_config->Write(_T("AuiPerspective"), str);

	delete m_aui_manager;
}

bool CHeeksFrame::ShowFullScreen(bool show, long style){
	static bool statusbar_visible = true;

	static std::map< wxWindow*, bool > windows_visible;

	if(show){
		SetMenuBar(NULL);
		statusbar_visible = m_statusBar->IsShown();
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
	if(!wxGetApp().CheckForModifiedDoc())
		return;
	Close(TRUE);
}

void 
CHeeksFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
	wxString str = _T("HeeksCAD, written by Dan Heeks\n\nusing Open CASCADE solid modeller - http://www.opencascade.org\n\nwindows made with wxWidgets - http://wxwidgets.org");
	wxMessageBox( str, wxGetApp().m_version_number, wxOK | wxICON_INFORMATION, this );
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
	m_aui_manager->LoadPerspective(default_layout_string);
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

void 
CHeeksFrame::OnSelectModeButton( wxCommandEvent& WXUNUSED( event ) )
{
	wxGetApp().SetInputMode((CInputMode*)(wxGetApp().m_select_mode));
}

void 
CHeeksFrame::OnLinesButton( wxCommandEvent& WXUNUSED( event ) )
{
	line_strip.drawing_mode = LineDrawingMode;
	wxGetApp().SetInputMode(&line_strip);
}

void 
CHeeksFrame::OnCirclesButton( wxCommandEvent& WXUNUSED( event ) )
{
	line_strip.drawing_mode = CircleDrawingMode;
	wxGetApp().SetInputMode(&line_strip);
}

void 
CHeeksFrame::OnILineButton( wxCommandEvent& WXUNUSED( event ) )
{
	line_strip.drawing_mode = ILineDrawingMode;
	wxGetApp().SetInputMode(&line_strip);
}

void 
CHeeksFrame::OnCoordinateSystem( wxCommandEvent& WXUNUSED( event ) )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(false);
	gp_Pnt o = gp_Pnt(0, 0, 0).Transformed(mat);
	gp_Dir x = gp_Dir(1, 0, 0).Transformed(mat);
	gp_Dir y = gp_Dir(0, 1, 0).Transformed(mat);
	CoordinateSystem* new_object = new CoordinateSystem(_T("Coordinate System"), o, x, y);
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().m_marked_list->Clear();
	wxGetApp().m_marked_list->Add(new_object);
	wxGetApp().Repaint();

	// and pick from three points
	new_object->PickFrom3Points();
}

void CHeeksFrame::OnOpenButton( wxCommandEvent& event )
{
    wxFileDialog dialog(this, _T("Open file"), wxEmptyString, wxEmptyString, wxGetApp().GetKnownFilesWildCardString());
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
    wxFileDialog dialog(this, _T("Import file"), wxEmptyString, wxEmptyString, wxGetApp().GetKnownFilesWildCardString());
    dialog.CentreOnParent();

    if (dialog.ShowModal() == wxID_OK)
    {
		wxGetApp().OpenFile(dialog.GetPath().c_str());
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
	wxGetApp().Reset();
	wxGetApp().OnNewOrOpen(false);
	wxGetApp().SetLikeNewFile();
	wxGetApp().SetFrameTitle();
	wxGetApp().Repaint();
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
	wxGetApp().Paste();
}

void CHeeksFrame::OnUpdatePaste( wxUpdateUIEvent& event )
{
	event.Enable(wxGetApp().IsPasteReady());
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

void CHeeksFrame::OnRuledSurfaceButton( wxCommandEvent& event )
{
	PickCreateRuledSurface();
}

void CHeeksFrame::OnExtrudeButton( wxCommandEvent& event )
{
	PickCreateExtrusion();
}

void CHeeksFrame::OnSphereButton( wxCommandEvent& event )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(true);
	CSphere* new_object = new CSphere(gp_Pnt(0, 0, 0).Transformed(mat), 5, _T("Sphere"));
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().m_marked_list->Clear();
	wxGetApp().m_marked_list->Add(new_object);
	wxGetApp().Repaint();
}

void CHeeksFrame::OnCubeButton( wxCommandEvent& event )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(false);
	CCuboid* new_object = new CCuboid(gp_Ax2(gp_Pnt(0, 0, 0).Transformed(mat), gp_Dir(0, 0, 1).Transformed(mat), gp_Dir(1, 0, 0).Transformed(mat)), 10, 10, 10, _T("Cuboid"));
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().m_marked_list->Clear();
	wxGetApp().m_marked_list->Add(new_object);
	wxGetApp().Repaint();
}

void CHeeksFrame::OnCylButton( wxCommandEvent& event )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(true);
	CCylinder* new_object = new CCylinder(gp_Ax2(gp_Pnt(0, 0, 0).Transformed(mat), gp_Dir(0, 0, 1).Transformed(mat), gp_Dir(1, 0, 0).Transformed(mat)), 5, 10, _T("Cylinder"));
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().m_marked_list->Clear();
	wxGetApp().m_marked_list->Add(new_object);
	wxGetApp().Repaint();
}

void CHeeksFrame::OnConeButton( wxCommandEvent& event )
{
	gp_Trsf mat = wxGetApp().GetDrawMatrix(true);
	CCone* new_object = new CCone(gp_Ax2(gp_Pnt(0, 0, 0).Transformed(mat), gp_Dir(0, 0, 1).Transformed(mat), gp_Dir(1, 0, 0).Transformed(mat)), 10, 5, 20, _T("Cone"));
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().m_marked_list->Clear();
	wxGetApp().m_marked_list->Add(new_object);
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
	wxGetApp().m_config->Write(_T("MainFrameWidth"), width);
	wxGetApp().m_config->Write(_T("MainFrameHeight"), height);

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
	wxGetApp().m_config->Write(_T("MainFramePosX"), posx);
	wxGetApp().m_config->Write(_T("MainFramePosY"), posy);
}

int CHeeksFrame::AddToolBarTool(wxToolBar* toolbar, const wxString& title, wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&))
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
		menu->Append(0, t->GetTitle(), menu2);
	}
	else
	{
		ToolIndex ti;
		ti.m_tool = t;
		ti.m_index = tool_index_list.size();
		tool_index_list.push_back(ti);
		menu->Append(ti.m_index+ID_FIRST_POP_UP_MENU_TOOL, t->GetTitle());
		if(t->Disabled())menu->Enable(ti.m_index+1, false);
		if(t->Checked ())menu->Check(ti.m_index+1, true);
	}
}

