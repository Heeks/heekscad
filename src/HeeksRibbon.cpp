#include "stdafx.h"
#include "HeeksFrame.h"
#include "HeeksRibbon.h"

#ifdef USING_RIBBON

BEGIN_EVENT_TABLE(HeeksRibbon, wxRibbonBar)

#define EVT_RIBBONBUTTONBAR_CLICK_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_RIBBONBUTTONBAR_CLICKED, id1, id2, wxRibbonButtonBarEventHandler(func))
#define EVT_RIBBONTOOLBAR_CLICK_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_RIBBONTOOLBAR_CLICKED, id1, id2, wxRibbonToolBarEventHandler(func))
#define EVT_RIBBONBUTTONBAR_DROPDOWN_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_RIBBONBUTTONBAR_DROPDOWN_CLICKED, id1, id2, wxRibbonButtonBarEventHandler(func))
#define EVT_RIBBONTOOLBAR_DROPDOWN_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_RIBBONTOOLBAR_DROPDOWN_CLICKED, id1, id2, wxRibbonToolBarEventHandler(func))
EVT_RIBBONBUTTONBAR_CLICK_RANGE(ID_FIRST_EXTERNAL_BUTTON, ID_FIRST_POP_UP_MENU_TOOL + 1000, HeeksRibbon::OnExternalBarButton)
//EVT_RIBBONTOOLBAR_CLICK_RANGE(ID_FIRST_EXTERNAL_BUTTON, ID_FIRST_POP_UP_MENU_TOOL + 1000, HeeksRibbon::OnExternalToolButton)
//EVT_RIBBONBUTTONBAR_DROPDOWN_RANGE(ID_FIRST_EXTERNAL_BUTTON, ID_FIRST_POP_UP_MENU_TOOL + 1000, HeeksRibbon::OnExternalBarButton)

END_EVENT_TABLE()

void test(std::initializer_list <int> t)
{
	int tt = 0;
	for (auto i : t) {
		wxMessageBox(wxString::Format(_T("%d"), i));

		tt += i;
	}
}

HeeksRibbon::HeeksRibbon(wxWindow* parent) :wxRibbonBar(parent, -1, wxDefaultPosition, wxDefaultSize,
wxRIBBON_BAR_FLOW_HORIZONTAL
	| wxRIBBON_BAR_SHOW_PAGE_LABELS
//	| wxRIBBON_BAR_SHOW_PANEL_EXT_BUTTONS
//	| wxRIBBON_BAR_SHOW_TOGGLE_BUTTON
//	| wxRIBBON_BAR_SHOW_HELP_BUTTON
	)
{
	//wxAcceleratorEntry[] = 
	//{
	//	wxAcceleratorEntry(int flags = 0, int keyCode = 0, int cmd = 0, wxMenuItem *item = NULL);

	//wxAcceleratorTable()

	m_next_id_for_button = ID_FIRST_EXTERNAL_BUTTON;
	wxRibbonPage* home = new wxRibbonPage(this, wxID_ANY, wxT("Page"), ToolImage(_T("new")));
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("File"), ToolImage(_T("new")), wxDefaultPosition, wxDefaultSize);
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("New"), ToolImage(_T("new")), _("New file"), OnNewButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Open"), ToolImage(_T("open")), _("Open file"), OnOpenButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Import"), ToolImage(_T("import")), _("Import file"), OnImportButton));
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("Save"), ToolImage(_T("save")), _("Save file"), OnSaveButton, OnUpdateSave));
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("Save As"), ToolImage(_T("saveas")), _("Save file with given name"), OnSaveAsButton, OnUpdateSave));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("Print"), ToolImage(_T("new")), wxDefaultPosition, wxDefaultSize);
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("New"), ToolImage(_T("new")), _("New file"), OnNewButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Open"), ToolImage(_T("open")), _("Open file"), OnOpenButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Import"), ToolImage(_T("import")), _("Import file"), OnImportButton));
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("Save"), ToolImage(_T("save")), _("Save file"), OnSaveButton, OnUpdateSave));
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("Save As"), ToolImage(_T("saveas")), _("Save file with given name"), OnSaveAsButton, OnUpdateSave));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("Edit"), ToolImage(_T("cut")), wxDefaultPosition, wxDefaultSize);
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("Cut"), ToolImage(_T("cut")), _("Cut selected items to the clipboard"), OnCutButton, OnUpdateCut));
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("Copy"), ToolImage(_T("copy")), _("Copy selected items to the clipboard"), OnCopyButton, OnUpdateCopy));
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("Paste"), ToolImage(_T("paste")), _("Paste items from the clipboard"), OnPasteButton, OnUpdatePaste));
		AddToolBarTool(toolbar, RibbonButtonData(_("Undo"), ToolImage(_T("undo")), _("Undo the previous command"), OnUndoButton, OnUpdateUndo));
		AddToolBarTool(toolbar, RibbonButtonData(_("Redo"), ToolImage(_T("redo")), _("Redo the next command"), OnRedoButton, OnUpdateRedo));
		AddToolBarTool(toolbar, RibbonButtonData(_("Select"), ToolImage(_T("select")), _("Select Mode"), OnSelectModeButton));
	}
	this->AddPageHighlight(this->GetPageCount() - 1);

	this->Realize();

}

std::map<int, RibbonButtonData > m_ribbon_button_map;

int HeeksRibbon::MakeNextIDForTool(const RibbonButtonData& data)
{
	while (m_ribbon_button_map.find(m_next_id_for_button) != m_ribbon_button_map.end())
	{
		// already used
		m_next_id_for_button++;
	}

	if (m_next_id_for_button >= ID_FIRST_POP_UP_MENU_TOOL)
	{
		// too many button IDs!
		wxMessageBox(_T("too many button IDs!, see CHeeksFrame::GetNextIDForTool"));
	}

	int id_to_use = m_next_id_for_button;

	m_ribbon_button_map.insert(std::pair<int, RibbonButtonData>(id_to_use, data));
	m_next_id_for_button++;
	return id_to_use;
}

RibbonButtonData* HeeksRibbon::GetRibbonButtonData(int id)
{
	std::map<int, RibbonButtonData>::iterator FindIt = m_ribbon_button_map.find(id);
	if (FindIt == m_ribbon_button_map.end())return NULL;
	return &(FindIt->second);
}

wxRibbonButtonBarButtonBase* HeeksRibbon::AddToolBarTool(wxRibbonButtonBar* toolbar, const RibbonButtonData& data)
{
	int id_to_use = MakeNextIDForTool(data);
	if (data.m_children.size() > 0)
	{
		return toolbar->AddHybridButton(id_to_use, data.m_title, data.m_bitmap, data.m_caption);
	}
	else
	{
		return toolbar->AddButton(id_to_use, data.m_title, data.m_bitmap, data.m_caption);
	}
}

wxRibbonToolBarToolBase* HeeksRibbon::AddToolBarTool(wxRibbonToolBar* toolbar, const RibbonButtonData& data)
{
	int id_to_use = MakeNextIDForTool(data);
	return toolbar->AddDropdownTool(id_to_use, data.m_bitmap, data.m_caption);
}

void HeeksRibbon::OnExternalBarButton(wxRibbonButtonBarEvent& event)
{
	wxRibbonButtonBar* bar = event.GetBar();
	wxRibbonButtonBarButtonBase* button = event.GetButton();

	RibbonButtonData* data = GetRibbonButtonData(event.GetId());
	if (data)
	{
		(*(data->m_on_button))(event);
		event.Skip();
	}
}

void HeeksRibbon::OnBarButtonDropDown(wxRibbonButtonBarEvent& event)
{
	RibbonButtonData* data = GetRibbonButtonData(event.GetId());
	//if (data)
	//{
		//data->m_children
	//}
	if (data)
	{
		(*(data->m_on_button))(event);
	}
}

void HeeksRibbon::OnExternalToolButton(wxRibbonToolBarEvent& event)
{
	RibbonButtonData* data = GetRibbonButtonData(event.GetId());
	if (data)
	{
		(*(data->m_on_button))(event);
	}
}

void HeeksRibbon::OnUpdateExternalButton(wxUpdateUIEvent& event)
{
	RibbonButtonData* data = GetRibbonButtonData(event.GetId());
	if (data)
	{
		(*(data->m_on_update_button))(event);
	}
}

#endif