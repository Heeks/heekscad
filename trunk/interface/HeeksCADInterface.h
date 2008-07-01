// HeeksCADInterface.h

// include this in your dynamic library to interface with HeeksCAD

class wxDynamicLibrary;

class HeeksCADInterface{
private:
	wxDynamicLibrary *m_executable;

public:
	HeeksCADInterface(const char* full_path);
	~HeeksCADInterface();

	double GetTolerance();
	void RefreshProperties();
	void Repaint();

	wxFrame* GetMainFrame();
	wxAuiManager* GetAuiManager();
	void Bastart(wxToolBar* toolbar, const wxString& title, wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&));
};
