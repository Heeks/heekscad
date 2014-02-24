// HeeksConfig.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

class HeeksConfig: public wxConfig
{
public:
	bool m_disabled;
	HeeksConfig():wxConfig(wxGetApp().GetAppName()), m_disabled(wxGetApp().m_settings_restored){}
	~HeeksConfig(){}

	bool DoWriteString(const wxString& key, const wxString& value){if(!m_disabled)return wxConfig::DoWriteString(key, value); return false;}
	bool DoWriteLong(const wxString& key, long value){if(!m_disabled)return wxConfig::DoWriteLong(key, value); return false;}
};
