// strconv.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#ifdef UNICODE
extern const char* Ttc(const wchar_t* str);
extern const wchar_t* Ctt(const char* str);
#else
#define Ttc(x) x
#define Ctt(x) x
#endif
