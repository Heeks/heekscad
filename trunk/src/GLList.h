// GLList.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#ifdef WIN32
#pragma once
#endif

class CGLList  
{
public:
	CGLList();
	virtual ~CGLList();

	bool m_gl_list_exists;
	int m_gl_list;

	void DestroyGLList();
};
