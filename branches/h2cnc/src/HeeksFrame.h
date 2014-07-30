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
	ID_RECENT_FIRST = wxID_HIGHEST + 1,
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
	wxString m_title;
	wxBitmap m_bitmap;
	wxString m_tooltip;
	void(*m_onButtonFunction)(wxCommandEvent&);

	CFlyOutItem(const wxString& title, const wxBitmap& bitmap, const wxString& tooltip, void(*onButtonFunction)(wxCommandEvent&));

	virtual bool IsAList(){return false;}
};

class CFlyOutList: public CFlyOutItem
{
public:
	std::list<CFlyOutItem> m_list;

	CFlyOutList(const wxString& title);

	// CFlyOutItem's virtual functions
	bool IsAList(){return true;}

	// Get the item that should appear on the main toolbar
	const CFlyOutItem* GetMainItem()const;
	void SetMainItem(const CFlyOutItem* item);
};

class CHeeksFrame : public wxFrame
{
private:
	int m_next_id_for_button;
	std::map<int, SExternalButtonFunctions > m_external_buttons;



public:
	wxLogWindow* m_logger;
	static int m_loglevel; 			// for wxLog::SetLogLevel((wxLogLevel)m_loglevel)
	static bool m_logrepeatcounts; 	// for wxLog::SetRepetitionCounting(m_logrepeatcounts)
	static bool m_logtimestamps; 	// for wxLog::SetLogTimestamps(m_logtimestamps)

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
	wxMenuBar *m_menuBar;
	wxMenu* m_recent_files_menu;
	wxMenu *m_menuWindow;
	HeeksPrintout* m_printout;
	wxString m_extra_about_box_str;

	bool m_main_toolbar_removed;
	bool m_geometry_toolbar_removed;
	bool m_solid_toolbar_removed;
	bool m_viewing_toolbar_removed;
	bool m_transform_toolbar_removed;
	int m_objects_menu_id;
	int m_log_menu_id;
	int m_options_menu_id;
	int m_input_menu_id;
	int m_properties_menu_id;
	int m_main_toolbar_menu_id;
	int m_solids_toolbar_menu_id;
	int m_geometry_toolbar_menu_id;
	int m_viewing_toolbar_menu_id;
	int m_transform_toolbar_menu_id;

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
	wxToolBarToolBase* AddToolBarTool(wxToolBar* toolbar, const wxString& title, const wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&) = NULL);
	void AddToolBarTool(wxToolBar* toolbar, Tool* tool);
	void AddToolBarFlyout(wxToolBar* toolbar, const CFlyOutList& flyout);
	int MakeNextIDForTool(void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&));
	void SetToolFunctions(int Id, void(*onButtonFunction)(wxCommandEvent&), void(*onUpdateButtonFunction)(wxUpdateUIEvent&));
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
	void SetLogLevel(const int level);
	void SetLogRepeatCounting(const bool repeatcounting);
	void SetLogLogTimestamps(const bool uselogtimestamps);
	void RefreshInputCanvas();
	void RefreshProperties();
	void RefreshOptions();

	//wxTopLevelWindow's virtual functions
	bool ShowFullScreen(bool show, long style = wxFULLSCREEN_ALL);
	static void AddToolToListAndMenu(Tool *t, std::vector<ToolIndex> &tool_index_list, wxMenu *menu);
private:

	DECLARE_EVENT_TABLE()
};
