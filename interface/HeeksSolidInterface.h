// HeeksSolidInterface.h

// include this in your dynamic library to interface with HeeksSolid

class wxDynamicLibrary;

class HeeksSolidInterface{
private:
	wxDynamicLibrary *m_executable;

public:
	HeeksSolidInterface(const char* full_path);
	~HeeksSolidInterface();

	double GetTolerance();
	void RefreshProperties();
	void Repaint();

	wxFrame* GetMainFrame();
	wxAuiManager* GetAuiManager();
	void Bastart(wxToolBar* toolbar, const wxString& title, wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&));
};
