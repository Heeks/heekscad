#include "PropertiesCanvas.h"

class CInputModeCanvas: public CPropertiesCanvas
{
private:
	wxToolBar *m_toolBar;
	std::list<Tool*> m_previous_tools; // to decide whether to remove and reshow toolbar

public:
    CInputModeCanvas(wxWindow* parent);
    virtual ~CInputModeCanvas();

    //virtual void OnDraw(wxDC& dc);
    void OnSize(wxSizeEvent& event);
    void OnPropertyGridChange( wxPropertyGridEvent& event );

public:
	// CPropertiesCanvas's virtual functions
	void RefreshByRemovingAndAddingAll();
	void AddToolBar();

    DECLARE_NO_COPY_CLASS(CInputModeCanvas)
    DECLARE_EVENT_TABLE()
};
