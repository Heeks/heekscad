// Plugins.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

class PluginData{
public:
	wxString name;
	wxString path;
	bool enabled;
	bool hard_coded;

	PluginData():enabled(true), hard_coded(false){}
};

class CPluginItemDialog: public wxDialog{
public:
    wxPanel *m_panel;
	PluginData* m_pd;
	wxTextCtrl* m_name_text_ctrl;
	wxTextCtrl* m_path_text_ctrl;

	CPluginItemDialog(wxWindow *parent, const wxString& title, PluginData& pd);

    void OnButtonOK(wxCommandEvent& event);
    void OnButtonCancel(wxCommandEvent& event);
    void OnButtonBrowse(wxCommandEvent& event);

private:
	DECLARE_EVENT_TABLE()
};


class CPluginsDialog: public wxDialog{
	void CreateCheckListbox(long flags = 0);
	void EditSelected(unsigned int selection);

public:
    wxPanel *m_panel;
    wxCheckListBox *m_pListBox;
	std::list<PluginData> m_plugins;

	CPluginsDialog(wxWindow *parent);

    void OnButtonAdd(wxCommandEvent& event);
    void OnButtonEdit(wxCommandEvent& event);
	void OnUpdateEdit( wxUpdateUIEvent& event );
    void OnButtonRemove(wxCommandEvent& event);
	void OnUpdateRemove( wxUpdateUIEvent& event );
    void OnButtonOK(wxCommandEvent& event);
    void OnButtonCancel(wxCommandEvent& event);
    void OnListboxDblClick(wxCommandEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

extern void ReadPluginsList(std::list<PluginData> &plugins);
