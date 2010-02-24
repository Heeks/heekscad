// strconv.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include <vector>
#include <wx/string.h>

#ifdef UNICODE
extern const char* Ttc(const wchar_t* str);
extern const wchar_t* Ctt(const char* str);
#else
#define Ttc(x) x
#define Ctt(x) x
#endif

wxString ss_to_wxstring( const std::string & text );
wxString ws_to_wxstring( const std::wstring & text );

std::vector<wxString> Tokens( const wxString & wxLine, const wxString & wxDelimiters );
bool AllNumeric( const wxString & wxLine );

