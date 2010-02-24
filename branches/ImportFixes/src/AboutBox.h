// AboutBox.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

class CAboutBox: public wxDialog{
public:
	CAboutBox(wxWindow *parent);

    void OnButtonOK(wxCommandEvent& event);

private:
	DECLARE_EVENT_TABLE()
};
