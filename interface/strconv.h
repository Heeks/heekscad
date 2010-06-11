// strconv.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include <vector>
#include <wx/string.h>

#ifdef UNICODE
#define Ttc(s) (const char *) wxString(s).mb_str(wxConvUTF8)

inline wxString Ctt(const char String[] = "")
{
    return wxString(String, wxConvUTF8);
}
#else
#define Ttc(x) x
#define Ctt(x) x
#endif

std::vector<wxString> Tokens( const wxString wxLine, const wxString wxDelimiters );
bool AllNumeric( const wxString wxLine );



