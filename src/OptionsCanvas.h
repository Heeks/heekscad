#include "PropertiesCanvas.h"

class COptionsCanvas: public CPropertiesCanvas
{
private:
	wxToolBar *m_toolBar;

public:
    COptionsCanvas(wxWindow* parent);
    virtual ~COptionsCanvas();

    //virtual void OnDraw(wxDC& dc);
    void OnSize(wxSizeEvent& event);
    void OnPropertyGridChange( wxPropertyGridEvent& event );

public:
	// CPropertiesCanvas's virtual functions
	void RefreshByRemovingAndAddingAll();

    DECLARE_NO_COPY_CLASS(COptionsCanvas)
    DECLARE_EVENT_TABLE()
};
