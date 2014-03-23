#include "stdafx.h"
#include "HeeksRibbon.h"

HeeksRibbon::HeeksRibbon(wxWindow* parent) :wxRibbonBar(parent, -1, wxDefaultPosition, wxDefaultSize, wxRIBBON_BAR_FLOW_HORIZONTAL
	| wxRIBBON_BAR_SHOW_PAGE_LABELS
	| wxRIBBON_BAR_SHOW_PANEL_EXT_BUTTONS
	| wxRIBBON_BAR_SHOW_TOGGLE_BUTTON
	| wxRIBBON_BAR_SHOW_HELP_BUTTON
	)
{
		{
			wxRibbonPage* home = new wxRibbonPage(this, wxID_ANY, wxT("File"), ToolImage(_T("new")));
			wxRibbonPanel *toolbar_panel = new wxRibbonPanel(home, wxID_ANY, wxT("Toolbar"),
				wxNullBitmap, wxDefaultPosition, wxDefaultSize,
				wxRIBBON_PANEL_NO_AUTO_MINIMISE |
				wxRIBBON_PANEL_EXT_BUTTON);
			wxRibbonToolBar *toolbar = new wxRibbonToolBar(toolbar_panel);
			toolbar->AddTool(wxID_NEW, ToolImage(_T("new")), _("New\tCtrl+N"));
			toolbar->AddTool(wxID_OPEN, ToolImage(_T("open")), _("Open\tCtrl+O"));
			//toolbar->SetRows(2, 3);
		}
	this->AddPageHighlight(this->GetPageCount() - 1);

	this->Realize();

}
