// strconv.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#ifdef UNICODE
#include <string>

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


