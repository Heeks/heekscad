// HeeksRibbon.h

// Copyright (c) 2014, Dan Heeks
// This program is released under the New BSD license. See the file COPYING for details.

#ifdef USING_RIBBON

#include "wx/ribbon/bar.h"
#include "wx/ribbon/buttonbar.h"
#include "wx/ribbon/gallery.h"
#include "wx/ribbon/toolbar.h"

class RibbonButtonData
{
public:
	wxString m_title;
	wxBitmap m_bitmap;
	wxString m_caption;
	void(*m_on_button)(wxCommandEvent&);
	void(*m_on_update_button)(wxUpdateUIEvent&);
	void(*m_on_dropdown)(wxRibbonButtonBarEvent&);

	RibbonButtonData(const wxString& title, const wxBitmap& bitmap, const wxString& caption, void(*on_button)(wxCommandEvent&), void(*on_update_button)(wxUpdateUIEvent&) = NULL, void(*on_dropdown)(wxRibbonButtonBarEvent&) = NULL) :m_title(title), m_bitmap(bitmap), m_caption(caption), m_on_button(on_button), m_on_update_button(on_update_button), m_on_dropdown(on_dropdown){}
};

class HeeksRibbon : public wxRibbonBar{
private:
	int m_next_id_for_button;
	std::map<int, RibbonButtonData > m_ribbon_button_map;
	RibbonButtonData* GetRibbonButtonData(int id);

public:
	HeeksRibbon(wxWindow* parent);

	wxRibbonButtonBarButtonBase* AddToolBarTool(wxRibbonButtonBar* toolbar, const RibbonButtonData& data);
	wxRibbonToolBarToolBase* AddToolBarTool(wxRibbonToolBar* toolbar, const RibbonButtonData& data);

	int MakeNextIDForTool(const RibbonButtonData& data);
	void OnExternalBarButton(wxRibbonButtonBarEvent& event);
	void OnBarButtonDropDown(wxRibbonButtonBarEvent& event);
	void OnExternalToolButton(wxRibbonToolBarEvent& event);
	void OnUpdateExternalButton(wxUpdateUIEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif