// strconv.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include <string>
#include <wx/string.h>
#include <vector>

#ifdef UNICODE

static std::string str_for_Ttc;

const char* Ttc(const wchar_t* str)
{
	// convert a wchar_t* string into a char* string
	str_for_Ttc.clear();
	while (*str)
		str_for_Ttc.push_back((char) *str++);
	return str_for_Ttc.c_str();
}

static std::wstring wstr_for_Ttc;

const wchar_t* Ctt(const char* str)
{
	// convert a char* string into a wchar_t* string
	wstr_for_Ttc.clear();
	while (*str)
		wstr_for_Ttc.push_back((wchar_t) *str++);
	return wstr_for_Ttc.c_str();
}
#endif

wxString ss_to_wxstring( const std::string & text )
{
	wxString result;
	for (std::string::const_iterator l_itChar = text.begin(); l_itChar != text.end(); l_itChar++)
	{
		result.Append( *l_itChar, 1 );
	} // End for
	return(result);
}

wxString ws_to_wxstring( const std::wstring & text )
{
	wxString result;
	for (std::wstring::const_iterator l_itChar = text.begin(); l_itChar != text.end(); l_itChar++)
	{
		result.Append( *l_itChar, 1 );
	} // End for
	return(result);
}

static wxString::size_type find_first_of( const wxString & line, const wxString & delimiters )
{
	wxString::size_type offset = 0;
	bool offset_value_set = false;
	for (wxString::size_type delimiter = 0; delimiter < delimiters.Length(); delimiter++)
	{
		wxString::size_type here = line.Find( delimiters[delimiter] );
		if (here >= 0)
		{
			if (offset_value_set == false) { offset = here; offset_value_set = true; }
			if (here < offset) offset = here;
		}
	} // End for

	return(offset);
}

/**
	Breakup the line of text based on the delimiting characters passed
	in and return a vector of 'words'.
 */
std::vector<wxString> Tokens( const wxString & wxLine, const wxString & wxDelimiters )
{
	std::vector<wxString> tokens;
	wxString line(wxLine);	// non-const copy

	wxString::size_type offset;
	while ((offset = find_first_of( line, wxDelimiters )) != line.npos)
	{
		if (offset > 0)
		{
			tokens.push_back( line.substr(0, offset) );
		} // End if - then

		line.Remove(0, offset+1);
	} // End while

	if (line.size() > 0)
	{
		tokens.push_back( line );
	} // End if - then

	return(tokens);

} // End Tokens() method


bool AllNumeric( const wxString & wxLine )
{
	if (wxLine.Length() == 0) return(false);

	wxString line( wxLine );	// non-const copy
	wxString::size_type offset;

	for (offset=0; offset<line.size(); offset++)
	{
		if ((((line[offset] >= _T('0')) &&
		     (line[offset] <= _T('9'))) ||
		    (line[offset] == '+') ||
		    (line[offset] == '-') ||
		    (line[offset] == '.')) == false) return(false);
	} // End for
	return(true);
} // End Tokens() method


