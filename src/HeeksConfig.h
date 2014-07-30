// HeeksConfig.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

class HeeksConfig: public wxConfig
{
public:
	HeeksConfig():wxConfig(wxGetApp().GetAppName()){}
	~HeeksConfig(){}
};
