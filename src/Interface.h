// declares all the exported functions for HeeksCAD

extern "C"{
#define HEEKS_EXPORT __declspec( dllexport ) __cdecl

class wxFrame;
class wxAuiManager;

double	HEEKS_EXPORT	HeeksGetTolerance(void);
void	HEEKS_EXPORT	HeeksRefreshProperties(void);
void	HEEKS_EXPORT	HeeksRepaint(void);
int		HEEKS_EXPORT	HeeksGetMainFrame(void);
int		HEEKS_EXPORT	HeeksGetAuiManager();
void	HEEKS_EXPORT	HeeksAddToolBarTool(wxToolBar* toolbar, const wxString& title, wxBitmap& bitmap, const wxString& caption, void(*onButtonFunction)(wxCommandEvent&));
}