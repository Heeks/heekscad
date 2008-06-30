// declares all the exported functions for HeeksCAD

extern "C"{
#define EXPORT __declspec( dllexport ) __cdecl

class wxFrame;
class wxAuiManager;

double	EXPORT	HeeksGetTolerance(void);
void	EXPORT	HeeksRefreshProperties(void);
void	EXPORT	HeeksRepaint(void);
int		EXPORT	HeeksGetMainFrame(void);
int		EXPORT	HeeksGetAuiManager();
void	EXPORT	HeeksAddToolBarTool(wxToolBar* toolbar, const wxString& title, wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&));
}