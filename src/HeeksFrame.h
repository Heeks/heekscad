// HeeksFrame.h
#include <wx/dynlib.h>

class CLeftCanvas;
class CGraphicsCanvas;
class CObjPropsCanvas;
class COptionsCanvas;
class CInputModeCanvas;

struct SExternalButtonFunctions{
	void (*on_button)(wxCommandEvent&);
	void (*on_update_button)(wxUpdateUIEvent&);
};

class CHeeksFrame : public wxFrame
{
private:
	int m_next_id_for_button;
	std::map<int, SExternalButtonFunctions > m_external_buttons;

public:
	CLeftCanvas *m_left;
	CGraphicsCanvas* m_graphics;
	CObjPropsCanvas* m_properties;
	COptionsCanvas* m_options;
	CInputModeCanvas* m_input_canvas;
	wxAuiManager* m_aui_manager;
	wxToolBarBase *m_toolBar;
	wxToolBarBase *m_solidBar;
	wxToolBarBase *m_viewingBar;
	wxStatusBar* m_statusBar;
	wxMenuBar *m_menuBar;
	wxMenu* m_recent_files_menu;
	wxMenu *m_menuView;

	CHeeksFrame( const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize );
	virtual ~CHeeksFrame();

	void OnQuit( wxCommandEvent& event );
	void OnAbout( wxCommandEvent& event );
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
	void OnViewSolidBar( wxCommandEvent& event );
	void OnUpdateViewSolidBar( wxUpdateUIEvent& event );
	void OnViewViewingBar( wxCommandEvent& event );
	void OnUpdateViewViewingBar( wxUpdateUIEvent& event );
	void OnViewStatusBar( wxCommandEvent& event );
	void OnUpdateViewStatusBar( wxUpdateUIEvent& event );
	void OnLinesButton( wxCommandEvent& event );
	void OnImageButton( wxCommandEvent& event );
	void OnViewingButton( wxCommandEvent& event );
	void OnOpenButton( wxCommandEvent& event );
	void OnImportButton( wxCommandEvent& event );
	void OnSaveButton( wxCommandEvent& event );
	void OnUndoButton( wxCommandEvent& event );
	void OnRedoButton( wxCommandEvent& event );
	void OnNewButton( wxCommandEvent& event );
	void OnSubtractButton( wxCommandEvent& event );
	void OnFuseButton( wxCommandEvent& event );
	void OnCommonButton( wxCommandEvent& event );
	void OnSphereButton( wxCommandEvent& event );
	void OnRuledSurfaceButton( wxCommandEvent& event );
	void OnCubeButton( wxCommandEvent& event );
	void OnCylButton( wxCommandEvent& event );
	void OnRedrawButton( wxCommandEvent& event );
	void OnMagButton( wxCommandEvent& event );
	void OnMagExtentsButton( wxCommandEvent& event );
	void OnMagNoRotButton( wxCommandEvent& event );
	void OnMagPreviousButton( wxCommandEvent& event );
	void OnExternalButton( wxCommandEvent& event );
	void OnRecentFile( wxCommandEvent& event );
	void OnUpdateExternalButton( wxUpdateUIEvent& event );
	void OnSize( wxSizeEvent& evt );
	void OnMove( wxMoveEvent& evt );
	int AddToolBarTool(wxToolBar* toolbar, const wxString& title, wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&) = NULL);
	void AddToolBarTool(wxToolBar* toolbar, Tool* tool);
	void ClearToolBar(wxToolBar* m_toolBar);
	int AddMenuCheckItem(wxMenu* menu, const wxString& title, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&) = NULL);

	//wxTopLevelWindow's virtual functions
	bool ShowFullScreen(bool show, long style = wxFULLSCREEN_ALL);

private:

	DECLARE_EVENT_TABLE()
};

enum
{
	Menu_File_Quit = 100,
	Menu_File_About,
	Menu_View_Objects,
	Menu_View_Properties,
	Menu_View_Options,
	Menu_View_Input,
	Menu_View_ToolBar,
	Menu_View_SolidBar,
	Menu_View_ViewingBar,
	Menu_View_StatusBar
};
