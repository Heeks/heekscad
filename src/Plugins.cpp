// Plugins.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include <sys/stat.h>
#include "stdafx.h"
#include "Plugins.h"
#include "HeeksConfig.h"
#include <fstream>

enum
{
    ID_BUTTON_ADD,
	ID_BUTTON_EDIT,
	ID_BUTTON_REMOVE,
	ID_BUTTON_PLUGIN_BROWSE,
	ID_LISTBOX_CONTROL
};

BEGIN_EVENT_TABLE( CPluginItemDialog, wxDialog )
EVT_BUTTON( ID_BUTTON_PLUGIN_BROWSE, CPluginItemDialog::OnButtonBrowse )
EVT_BUTTON( wxID_OK, CPluginItemDialog::OnButtonOK )
EVT_BUTTON( wxID_CANCEL, CPluginItemDialog::OnButtonCancel )
END_EVENT_TABLE()

CPluginItemDialog::CPluginItemDialog(wxWindow *parent, const wxString& title, PluginData& pd):wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	m_pd = &pd;

    m_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

    // create buttons
    wxButton *button1 = new wxButton(m_panel, wxID_OK);
    wxButton *button2 = new wxButton(m_panel, wxID_CANCEL);

    wxBoxSizer *mainsizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer *gridsizer = new wxFlexGridSizer(3, 5, 5);
	gridsizer->Add(new wxStaticText(m_panel, wxID_ANY, _("Name")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
	m_name_text_ctrl = new wxTextCtrl(m_panel, wxID_ANY, pd.name);
	gridsizer->Add(m_name_text_ctrl, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Expand());
	gridsizer->Add(new wxStaticText(m_panel, wxID_ANY, _T("")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
	gridsizer->Add(new wxStaticText(m_panel, wxID_ANY, _("File Path")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
	m_path_text_ctrl = new wxTextCtrl(m_panel, wxID_ANY, pd.path, wxDefaultPosition, wxSize(400, 0));
	gridsizer->Add(m_path_text_ctrl, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Expand());
	wxButton* browse_button = new wxButton(m_panel, ID_BUTTON_PLUGIN_BROWSE, _T("..."));
    gridsizer->Add(browse_button, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));

	if(pd.hard_coded)
	{
		// don't allow editing of hard coded plugins
		m_name_text_ctrl->Enable(false);
		m_path_text_ctrl->Enable(false);
		browse_button->Enable(false);
	}

	gridsizer->AddGrowableCol(1, 1);

    wxBoxSizer *bottomsizer = new wxBoxSizer( wxHORIZONTAL );

    bottomsizer->Add( button1, 0, wxALL, 10 );
    bottomsizer->Add( button2, 0, wxALL, 10 );

    mainsizer->Add( gridsizer, wxSizerFlags().Align(wxALIGN_CENTER).Border(wxALL, 10).Expand() );
    mainsizer->Add( bottomsizer, wxSizerFlags().Align(wxALIGN_CENTER) );

    // tell frame to make use of sizer (or constraints, if any)
    m_panel->SetAutoLayout( true );
    m_panel->SetSizer( mainsizer );

#ifndef __WXWINCE__
    // don't allow frame to get smaller than what the sizers tell ye
    mainsizer->SetSizeHints( this );
#endif

    Show(true);
}

void CPluginItemDialog::OnButtonOK(wxCommandEvent& event)
{
	// it must have a name
	if(m_name_text_ctrl->GetValue().Len() == 0)
	{
		wxMessageBox(_("Plugin must have a name!"));
		m_name_text_ctrl->SetFocus();
		return;
	}

	// it must have a file path
	if(m_path_text_ctrl->GetValue().Len() == 0)
	{
		wxMessageBox(_("No file specified!"));
		m_path_text_ctrl->SetFocus();
		return;
	}

	m_pd->name = m_name_text_ctrl->GetValue();
	m_pd->path = m_path_text_ctrl->GetValue();

	EndModal(wxID_OK);
}

void CPluginItemDialog::OnButtonCancel(wxCommandEvent& event)
{
	EndModal(wxID_CANCEL);
}

void CPluginItemDialog::OnButtonBrowse(wxCommandEvent& event)
{
#ifdef WIN32
	wxString ext_str(_T("*.dll"));
#else
	wxString ext_str(_T("*.so*"));
#endif
	wxString wildcard_string = wxString(_("shared library files")) + _T(" |") + ext_str;

    wxFileDialog dialog(this, _("Choose shared library file"), wxEmptyString, wxEmptyString, wildcard_string);
    dialog.CentreOnParent();

    if (dialog.ShowModal() == wxID_OK)
    {
		m_path_text_ctrl->SetValue(dialog.GetPath());
    }
}

BEGIN_EVENT_TABLE( CPluginsDialog, wxDialog )
EVT_BUTTON( ID_BUTTON_ADD, CPluginsDialog::OnButtonAdd )
EVT_BUTTON( ID_BUTTON_EDIT, CPluginsDialog::OnButtonEdit )
EVT_UPDATE_UI(ID_BUTTON_EDIT, CPluginsDialog::OnUpdateEdit)
EVT_BUTTON( ID_BUTTON_REMOVE, CPluginsDialog::OnButtonRemove )
EVT_UPDATE_UI(ID_BUTTON_REMOVE, CPluginsDialog::OnUpdateRemove)
EVT_BUTTON( wxID_OK, CPluginsDialog::OnButtonOK )
EVT_BUTTON( wxID_CANCEL, CPluginsDialog::OnButtonCancel )
EVT_LISTBOX_DCLICK(ID_LISTBOX_CONTROL, CPluginsDialog::OnListboxDblClick)
END_EVENT_TABLE()

CPluginsDialog::CPluginsDialog(wxWindow *parent):wxDialog(parent, wxID_ANY, _("HeeksCAD plugin libraries"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
   // make a panel with some controls
    m_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	ReadPluginsList(m_plugins);

    CreateCheckListbox();

    // create buttons
    wxButton *button1 = new wxButton(m_panel, ID_BUTTON_ADD, _("New"));
    wxButton *button2 = new wxButton(m_panel, ID_BUTTON_EDIT, _("Edit"));
    wxButton *button3 = new wxButton(m_panel, ID_BUTTON_REMOVE, _("Delete"));
    wxButton *button4 = new wxButton(m_panel, wxID_OK);
    wxButton *button5 = new wxButton(m_panel, wxID_CANCEL);

    wxBoxSizer *mainsizer = new wxBoxSizer( wxVERTICAL );

    mainsizer->Add( m_pListBox, 1, wxGROW|wxALL, 10 );

    wxBoxSizer *bottomsizer = new wxBoxSizer( wxHORIZONTAL );

    bottomsizer->Add( button1, 0, wxALL, 10 );
    bottomsizer->Add( button2, 0, wxALL, 10 );
    bottomsizer->Add( button3, 0, wxALL, 10 );
    bottomsizer->Add( button4, 0, wxALL, 10 );
    bottomsizer->Add( button5, 0, wxALL, 10 );

    mainsizer->Add( bottomsizer, 0, wxCENTER );

    // tell frame to make use of sizer (or constraints, if any)
    m_panel->SetAutoLayout( true );
    m_panel->SetSizer( mainsizer );

#ifndef __WXWINCE__
    // don't allow frame to get smaller than what the sizers tell ye
    mainsizer->SetSizeHints( this );
#endif

    Show(true);
}

void ReadPluginsList(std::list<PluginData> &plugins)
{
	plugins.clear();

	HeeksConfig plugins_config;
	plugins_config.SetPath(_T("/plugins"));

	wxString key;
	long Index;
	wxString str;

	bool entry_found = false;
	bool hCncConfigured = false;  //if true, heekscnc is a plugin already. don't automatically add it in that case

	entry_found = plugins_config.GetFirstEntry(key, Index);

	while(entry_found)
	{
		plugins_config.Read(key, &str);

		PluginData pd;
		if(str[0] == '#')
		{
			str = str.Mid(1);
			pd.enabled = false;
		}
		else
		{
			pd.enabled = true;
		}

		pd.name = key;
		pd.path = str;
		plugins.push_back(pd);
		if( str.Lower().Matches(_T("*heekscnc*")) )
			hCncConfigured = true; 

		entry_found = plugins_config.GetNextEntry(key, Index);
	}

	//look for heekscnc in the standard install location and automatically add it, if it isn't already configured
	if( !hCncConfigured ) {
		struct stat cncstat;
		bool foundHcncPlugin = false;
#ifdef WIN32
		//this code should work on windows given the correct path
		const char* cncPlugPath = "standard\\windows\\path\\to\\heekscnc.dll";
  #ifndef S_ISREG
  	//if this fails to compile on windows, change it to
  	//#define S_ISREG(mode) true
    #define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
  #endif
#else
		const char* cncPlugPath = "/usr/lib/libheekscnc.so";  //this is the path that cmake installs the lib to
#endif
		if( stat(cncPlugPath, &cncstat) == 0 ) {
			if( S_ISREG(cncstat.st_mode) )
				foundHcncPlugin = true;
		}

		if( foundHcncPlugin ) {
			PluginData pd;
			pd.enabled = true;
			pd.hard_coded = false; //if this was true, the plugin wouldn't be added to the config - meaning the user couldn't disable it
			pd.name = _T("HeeksCNC (Automatically added)");
			pd.path = _T("/usr/lib/libheekscnc.so");
			plugins.push_back(pd);
		}
	}

	// add plugins from the command line
	std::list<wxString> command_line_plugins;
	wxGetApp().GetPluginsFromCommandLineParams(command_line_plugins);
	for(std::list<wxString>::iterator It = command_line_plugins.begin(); It != command_line_plugins.end(); It++)
	{
		PluginData pd;
		pd.enabled = true;
		pd.hard_coded = true;
		pd.name = _T("CommandLinePlugin");
		pd.path = *It;
		plugins.push_back(pd);
	}

	// add code to always start your dll here.
#ifdef MY_OWN_HARDCODED_PLUGIN
	{
		PluginData pd;
		pd.enabled = true;
		pd.hard_coded = true;
		pd.name = _T("MyPlugin");
		pd.path = _T("$(MYEXEPATH)/mypluing/PluginForHeeksCAD.dll");
		plugins.push_back(pd);
	}
#endif


	wxString plugins_file = wxGetApp().GetExeFolder() + _T("/plugins.txt");
    ifstream ifs(Ttc(plugins_file.c_str()));
	if(!(!ifs))
	{
		char s[1024] = "";

		while(!(ifs.eof()))
		{
			ifs.getline(s, 1024);
			wxString str(Ctt(s));

			PluginData pd;
			if(str[0] == '#')
			{
				str = str.Mid(1);
				pd.enabled = false;
			}
			else
			{
				pd.enabled = true;
			}
			pd.hard_coded = true;
			pd.name = str;
			str.Replace(_T("{app}"), wxGetApp().GetExeFolder().c_str());
			pd.path = str;
			plugins.push_back(pd);
		}
	}

}

void CPluginsDialog::CreateCheckListbox(long flags)
{
    wxString *astrChoices = new wxString[m_plugins.size()];
    unsigned int ui = 0;
	for ( std::list<PluginData>::iterator It = m_plugins.begin(); It != m_plugins.end(); It++, ui++ )
	{
		PluginData &pd = *It;
		astrChoices[ui] = pd.name;
	}

    m_pListBox = new wxCheckListBox
        (
         m_panel,               // parent
         ID_LISTBOX_CONTROL,       // control id
         wxPoint(10, 10),       // listbox poistion
         wxSize(400, 100),      // listbox size
         m_plugins.size(),		// number of strings
         astrChoices,           // array of strings
         flags
        );

    delete [] astrChoices;

    ui = 0;
	for ( std::list<PluginData>::iterator It = m_plugins.begin(); It != m_plugins.end(); It++, ui++ )
	{
		PluginData &pd = *It;
		if(pd.enabled)m_pListBox->Check(ui);
	}
}

void CPluginsDialog::EditSelected(unsigned int selection)
{
	unsigned int ui = 0;
	for ( std::list<PluginData>::iterator It = m_plugins.begin(); It != m_plugins.end(); It++, ui++ )
	{
		if(ui == selection)
		{
			PluginData &pd = *It;
			bool enabled = m_pListBox->IsChecked(ui);

			CPluginItemDialog dlg(this, _("Add New Plugin"), pd);
			if(dlg.ShowModal() == wxID_OK)
			{
				// updata the check list
				delete m_pListBox;
				CreateCheckListbox();
				m_pListBox->Check(ui, enabled);
			}
		}
	}
}

void CPluginsDialog::OnButtonAdd(wxCommandEvent& event)
{
	PluginData pd;
	CPluginItemDialog dlg(this, _("Add New Plugin"), pd);
	if(dlg.ShowModal() == wxID_OK)
	{
		// add the new item
		m_plugins.push_back(pd);

		// updata the check list
		delete m_pListBox;
		CreateCheckListbox();
	}
}

void CPluginsDialog::OnButtonEdit(wxCommandEvent& event)
{
	unsigned int selection = m_pListBox->GetSelection();

	EditSelected(selection);
}

void CPluginsDialog::OnUpdateEdit( wxUpdateUIEvent& event )
{
	event.Enable(m_pListBox->GetSelection() >= 0);
}

void CPluginsDialog::OnUpdateRemove( wxUpdateUIEvent& event )
{
	event.Enable(m_pListBox->GetSelection() >= 0);
}

void CPluginsDialog::OnButtonRemove(wxCommandEvent& event)
{
	unsigned int selection = m_pListBox->GetSelection();
	unsigned int ui = 0;
	for ( std::list<PluginData>::iterator It = m_plugins.begin(); It != m_plugins.end(); It++, ui++ )
	{
		if(ui == selection)
		{
			m_plugins.erase(It);
			// updata the check list
			delete m_pListBox;
			CreateCheckListbox();
			break;
		}
	}
}

void CPluginsDialog::OnButtonOK(wxCommandEvent& event)
{
	::wxSetWorkingDirectory(wxGetApp().GetExeFolder());

	HeeksConfig plugins_config;

	plugins_config.DeleteGroup(_T("plugins"));
	plugins_config.SetPath(_T("/plugins"));

	unsigned int ui = 0;
	for ( std::list<PluginData>::iterator It = m_plugins.begin(); It != m_plugins.end(); It++, ui++ )
	{
		PluginData &pd = *It;
		if(pd.hard_coded)continue;
		pd.enabled = m_pListBox->IsChecked(ui);
		wxString value = pd.path;
		if(!(pd.enabled))value.Prepend(_T("#"));
		plugins_config.Write(pd.name, value);	
	}

	EndModal(wxID_OK);
}

void CPluginsDialog::OnButtonCancel(wxCommandEvent& event)
{
	EndModal(wxID_CANCEL);
}

void CPluginsDialog::OnListboxDblClick(wxCommandEvent& WXUNUSED(event))
{
    int selection = m_pListBox->GetSelection();

	EditSelected(selection);
}
