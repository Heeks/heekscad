#include "PropertiesCanvas.h"

class COptionsCanvas: public CPropertiesCanvas
{
public:
    COptionsCanvas(wxWindow* parent);
    virtual ~COptionsCanvas();

    //virtual void OnDraw(wxDC& dc);
    void OnSize(wxSizeEvent& event);
    void OnPropertyGridChange( wxPropertyGridEvent& event );

public:
	void RefreshByRemovingAndAddingAll();

    DECLARE_NO_COPY_CLASS(COptionsCanvas)
    DECLARE_EVENT_TABLE()
};
