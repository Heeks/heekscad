#include "PropertiesCanvas.h"

class CInputModeCanvas: public CPropertiesCanvas
{
private:
	wxToolBar *m_toolBar;

public:
    CInputModeCanvas(wxWindow* parent);
    virtual ~CInputModeCanvas();

    //virtual void OnDraw(wxDC& dc);
    void OnSize(wxSizeEvent& event);
    void OnPropertyGridChange( wxPropertyGridEvent& event );

public:
	// CPropertiesCanvas's virtual functions
	void RefreshByRemovingAndAddingAll();

    DECLARE_NO_COPY_CLASS(CInputModeCanvas)
    DECLARE_EVENT_TABLE()
};
