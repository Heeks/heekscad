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


/**
	Breakup the line of text based on the delimiting characters passed
	in and return a vector of 'words'.
 */
std::vector<wxString> Tokens( const wxString & wxLine, const wxString & wxDelimiters )
{
#ifdef UNICODE
	std::wstring delimiters;
	std::wstring line;
	std::wstring::size_type offset;
#else
	std::string delimiters;
	std::string line;
	std::string::size_type offset;
#endif

	delimiters = wxDelimiters.c_str();
	line = wxLine.c_str();

	std::vector<wxString> tokens;
	while ((offset = line.find_first_of( delimiters )) != line.npos)
	{
		if (offset > 0)
		{
			tokens.push_back( line.substr(0, offset).c_str() );
		} // End if - then

		line.erase(0, offset+1);
	} // End while

	return(tokens);	
} // End Tokens() method


bool AllNumeric( const wxString & wxLine )
{
#ifdef UNICODE
	std::wstring line;
	std::wstring::size_type offset;
#else
	std::string line;
	std::string::size_type offset;
#endif

	line = wxLine.c_str();
	if (line.size() == 0) return(false);
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


