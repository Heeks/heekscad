// Plugin.h

#pragma once

class Plugin
{
public:
	wxString name;
	wxString path;
	wxDynamicLibrary* dynamic_library;

	Plugin(const wxString& Name, const wxString& Path, wxDynamicLibrary* lib):name(Name), path(Path), dynamic_library(lib){}
};
