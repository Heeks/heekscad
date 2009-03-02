// HeeksFrame.h
/*
 * Copyright (c) 2009, Dan Heeks
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */
#include <wx/dynlib.h>

class CTreeCanvas;
class CGraphicsCanvas;
class CObjPropsCanvas;
class COptionsCanvas;
class CInputModeCanvas;
class HeeksPrintout;

struct SExternalButtonFunctions{
	void (*on_button)(wxCommandEvent&);
	void (*on_update_button)(wxUpdateUIEvent&);
};

struct ToolIndex{
	Tool *m_tool;
	int m_index;
};

enum{
	ID_LINES = 1,
	ID_CIRCLES,
	ID_ILINE,
	ID_POINTS,
	ID_REGSHAPES,
	ID_TEXT,
	ID_DIMENSIONING,
	ID_SELECT_MODE,
	ID_SUBTRACT,
	ID_SPHERE,
	ID_CUBE,
	ID_CYL,
	ID_CONE,
	ID_REDRAW,
	ID_FUSE,
	ID_COMMON,
	ID_FILLET,
	ID_CHAMFER,
	ID_VEWING,
	ID_MAG,
	WXPRINT_PRINT,
	WXPRINT_PAGE_SETUP,
	WXPRINT_PREVIEW,
	ID_MAG_EXTENTS,
	ID_MAG_NO_ROT,
	ID_MAG_PREVIOUS,
	ID_VIEW_ROT,
	ID_VIEW_ZOOM,
	ID_FULL_SCREEN,
	ID_DELETE,
	ID_RECENT_FIRST,
	ID_OPEN_RECENT = ID_RECENT_FIRST + MAX_RECENT_FILES,
	ID_MOVE_TRANSLATE,
	ID_COPY_TRANSLATE,
	ID_MOVE_ROTATE,
	ID_COPY_ROTATE,
	ID_MOVE_MIRROR,
	ID_COPY_MIRROR,
	ID_MOVE_SCALE,
	ID_COPY_SCALE,
	ID_COORDINATE_SYSTEM,
	Menu_File_Quit,
	Menu_File_About,
	Menu_File_Plugins,
	Menu_View_Objects,
	Menu_View_Properties,
	Menu_View_Options,
	Menu_View_Input,
	Menu_View_ToolBar,
	Menu_View_GeometryBar,
	Menu_View_SolidBar,
	Menu_View_ViewingBar,
	Menu_View_TransformBar,
	Menu_View_StatusBar,
	Menu_View_ResetLayout,
	Menu_View_SetToolBarsToLeft,
	ID_TREE_CTRL,
	ID_IMPORT,
	ID_RULED_SURFACE,
	ID_EXTRUDE,
	ID_FIRST_EXTERNAL_BUTTON,
	ID_FIRST_POP_UP_MENU_TOOL = ID_FIRST_EXTERNAL_BUTTON + 1000,
	ID_NEXT_ID = ID_FIRST_POP_UP_MENU_TOOL + 1000
};

class CHeeksFrame : public wxFrame
{
private:
	int m_next_id_for_button;
	std::map<int, SExternalButtonFunctions > m_external_buttons;

public:
	CTreeCanvas *m_tree_canvas;
	CGraphicsCanvas* m_graphics;
	CObjPropsCanvas* m_properties;
	COptionsCanvas* m_options;
	CInputModeCanvas* m_input_canvas;
	wxAuiManager* m_aui_manager;
	wxToolBarBase *m_toolBar;
	wxToolBarBase *m_geometryBar;
	wxToolBarBase *m_solidBar;
	wxToolBarBase *m_viewingBar;
	wxToolBarBase *m_transformBar;
	wxStatusBar* m_statusBar;
	wxMenuBar *m_menuBar;
	wxMenu* m_recent_files_menu;
	wxMenu *m_menuWindow;
	HeeksPrintout* m_printout;

	CHeeksFrame( const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize );
	virtual ~CHeeksFrame();

    void OnClose( wxCloseEvent& event );
	void OnQuit( wxCommandEvent& event );
	void OnAbout( wxCommandEvent& event );
	void OnPlugins( wxCommandEvent& event );
	void OnViewObjects( wxCommandEvent& event );
	void OnUpdateViewObjects( wxUpdateUIEvent& event );
	void OnViewOptions( wxCommandEvent& event );
	void OnUpdateViewOptions( wxUpdateUIEvent& event );
	void OnViewInput( wxCommandEvent& event );
	void OnUpdateViewInput( wxUpdateUIEvent& event );
	void OnViewProperties( wxCommandEvent& event );
	void OnUpdateOpenRecent( wxUpdateUIEvent& event );
	void OnUpdateViewProperties( wxUpdateUIEvent& event );
	void OnViewToolBar( wxCommandEvent& event );
	void OnUpdateViewToolBar( wxUpdateUIEvent& event );
	void OnViewGeometryBar( wxCommandEvent& event );
	void OnUpdateViewGeometryBar( wxUpdateUIEvent& event );
	void OnViewSolidBar( wxCommandEvent& event );
	void OnUpdateViewSolidBar( wxUpdateUIEvent& event );
	void OnViewViewingBar( wxCommandEvent& event );
	void OnUpdateViewViewingBar( wxUpdateUIEvent& event );
	void OnViewTransformBar( wxCommandEvent& event );
	void OnUpdateViewTransformBar( wxUpdateUIEvent& event );
	void OnViewStatusBar( wxCommandEvent& event );
	void OnUpdateViewStatusBar( wxUpdateUIEvent& event );
	void OnResetLayout( wxCommandEvent& event );
	void OnSetToolBarsToLeft( wxCommandEvent& event );
	void OnLinesButton( wxCommandEvent& event );
	void OnCirclesButton( wxCommandEvent& event );
	void OnILineButton( wxCommandEvent& event );
	void OnPointsButton( wxCommandEvent& event );
	void OnRegularShapesButton( wxCommandEvent& event );
	void OnTextButton( wxCommandEvent& event );
	void OnDimensioningButton( wxCommandEvent& event );
	void OnCoordinateSystem( wxCommandEvent& event );
	void OnSelectModeButton( wxCommandEvent& event );
	void OnOpenButton( wxCommandEvent& event );
	void OnImportButton( wxCommandEvent& event );
	void OnSaveButton( wxCommandEvent& event );
	void OnUndoButton( wxCommandEvent& event );
	void OnRedoButton( wxCommandEvent& event );
	void OnNewButton( wxCommandEvent& event );
	void OnCutButton( wxCommandEvent& event );
	void OnUpdateCut( wxUpdateUIEvent& event );
	void OnCopyButton( wxCommandEvent& event );
	void OnUpdateCopy( wxUpdateUIEvent& event );
	void OnPasteButton( wxCommandEvent& event );
	void OnUpdateDelete( wxUpdateUIEvent& event );
	void OnDeleteButton( wxCommandEvent& event );
	void OnUpdatePaste( wxUpdateUIEvent& event );
	void OnSubtractButton( wxCommandEvent& event );
	void OnFuseButton( wxCommandEvent& event );
	void OnCommonButton( wxCommandEvent& event );
	void OnFilletButton( wxCommandEvent& event );
	void OnChamferButton( wxCommandEvent& event );
	void OnSphereButton( wxCommandEvent& event );
	void OnRuledSurfaceButton( wxCommandEvent& event );
	void OnExtrudeButton( wxCommandEvent& event );
	void OnCubeButton( wxCommandEvent& event );
	void OnCylButton( wxCommandEvent& event );
	void OnConeButton( wxCommandEvent& event );
	void OnRedrawButton( wxCommandEvent& event );
	void OnMagButton( wxCommandEvent& event );
	void OnMagExtentsButton( wxCommandEvent& event );
	void OnMagNoRotButton( wxCommandEvent& event );
	void OnMagPreviousButton( wxCommandEvent& event );
	void OnViewRotateButton( wxCommandEvent& event );
	void OnViewZoomButton( wxCommandEvent& event );
	void OnFullScreenButton( wxCommandEvent& event );
	void OnMoveTranslateButton( wxCommandEvent& event );
	void OnCopyTranslateButton( wxCommandEvent& event );
	void OnMoveRotateButton( wxCommandEvent& event );
	void OnCopyRotateButton( wxCommandEvent& event );
	void OnMoveMirrorButton( wxCommandEvent& event );
	void OnCopyMirrorButton( wxCommandEvent& event );
	void OnMoveScaleButton( wxCommandEvent& event );
	void OnExternalButton( wxCommandEvent& event );
	void OnRecentFile( wxCommandEvent& event );
	void OnUpdateExternalButton( wxUpdateUIEvent& event );
	void OnSize( wxSizeEvent& evt );
	void OnMove( wxMoveEvent& evt );
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	int AddToolBarTool(wxToolBar* toolbar, const wxString& title, const wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&) = NULL);
	void AddToolBarTool(wxToolBar* toolbar, Tool* tool);
	void ClearToolBar(wxToolBar* m_toolBar);
	int AddMenuCheckItem(wxMenu* menu, const wxString& title, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&) = NULL);
	int AddMenuItem(wxMenu* menu, const wxString& title, void(*onButtonFunction)(wxCommandEvent&));
    void Draw(wxDC& dc);
    void OnPrint(wxCommandEvent& event);
    void OnPrintPreview(wxCommandEvent& event);
    void OnPageSetup(wxCommandEvent& event);
	void OnChangeBitmapSize();
	void AddToolBars();
	void LoadPerspective(const wxString& str);
	void SetToolBarsToLeft();
	void SetToolBarsSize();

	//wxTopLevelWindow's virtual functions
	bool ShowFullScreen(bool show, long style = wxFULLSCREEN_ALL);
	static void AddToolToListAndMenu(Tool *t, std::vector<ToolIndex> &tool_index_list, wxMenu *menu);
private:

	DECLARE_EVENT_TABLE()
};
