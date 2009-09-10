// HeeksFrame.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

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
	ID_RECENT_FIRST = 1,
	Menu_View_ResetLayout = ID_RECENT_FIRST + MAX_RECENT_FILES,
	Menu_View_SetToolBarsToLeft,
	ID_TREE_CTRL,
	ID_FIRST_EXTERNAL_BUTTON,
	ID_FIRST_POP_UP_MENU_TOOL = ID_FIRST_EXTERNAL_BUTTON + 1000,
	ID_NEXT_ID = ID_FIRST_POP_UP_MENU_TOOL + 1000
};

class CFlyOutToolBar{
public:
	wxToolBar* toolbar;
};

class CFlyOutItem{
public:
	wxString m_title_and_bitmap;
	wxString m_tooltip;
	void(*m_onButtonFunction)(wxCommandEvent&);

	CFlyOutItem(const wxString& title_and_bitmap, const wxString& tooltip, void(*onButtonFunction)(wxCommandEvent&));
};

class CFlyOutButton : public wxBitmapButton
{
public:

};

class CHeeksFrame : public wxFrame
{
private:
	int m_next_id_for_button;
	std::map<int, SExternalButtonFunctions > m_external_buttons;

	int MakeNextIDForTool(void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&));

public:
	wxLogWindow* m_logger;
	CTreeCanvas *m_tree_canvas;
	CGraphicsCanvas* m_graphics;
	CObjPropsCanvas* m_properties;
	COptionsCanvas* m_options;
	CInputModeCanvas* m_input_canvas;
	wxAuiManager* m_aui_manager;
	wxToolBar *m_toolBar;
	wxToolBar *m_geometryBar;
	wxToolBar *m_solidBar;
	wxToolBar *m_viewingBar;
	wxToolBar *m_transformBar;
	wxStatusBar* m_statusBar;
	wxMenuBar *m_menuBar;
	wxMenu* m_recent_files_menu;
	wxMenu *m_menuWindow;
	HeeksPrintout* m_printout;
	wxString m_extra_about_box_str;

	CHeeksFrame( const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize );
	virtual ~CHeeksFrame();

    void OnClose( wxCloseEvent& event );
	void OnResetLayout( wxCommandEvent& event );
	void OnSetToolBarsToLeft( wxCommandEvent& event );
	void OnExternalButton( wxCommandEvent& event );
	void OnRecentFile( wxCommandEvent& event );
	void OnUpdateExternalButton( wxUpdateUIEvent& event );
	void OnSize( wxSizeEvent& evt );
	void OnMove( wxMoveEvent& evt );
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	int AddToolBarTool(wxToolBar* toolbar, const wxString& title, const wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&) = NULL);
	void AddToolBarTool(wxToolBar* toolbar, Tool* tool);
	void AddToolBarFlyout(wxToolBar* toolbar, const wxString& title, const std::list<CFlyOutItem> &flyout_list);

	void ClearToolBar(wxToolBar* m_toolBar);
	int AddMenuItem(wxMenu* menu, const wxString& text, const wxBitmap& bitmap, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&) = NULL, wxMenu* submenu = NULL, bool check_item = false);
    void Draw(wxDC& dc);
	void OnChangeBitmapSize();
	void MakeMenus();
	void AddToolBars();
	void LoadPerspective(const wxString& str);
	void SetDefaultLayout(const wxString& str); // call this from dll's OnStartUp
	void SetToolBarsToLeft();
	void SetToolBarsSize();

	//wxTopLevelWindow's virtual functions
	bool ShowFullScreen(bool show, long style = wxFULLSCREEN_ALL);
	static void AddToolToListAndMenu(Tool *t, std::vector<ToolIndex> &tool_index_list, wxMenu *menu);
private:

	DECLARE_EVENT_TABLE()
};
