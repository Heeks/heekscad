// GLList.cpp:
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "GLList.h"


CGLList::CGLList()
{
	m_gl_list_exists = false;
}

CGLList::~CGLList()
{
	DestroyGLList();
}

void CGLList::DestroyGLList(){
	if(m_gl_list_exists){
		glDeleteLists(m_gl_list, 1);
		m_gl_list_exists = false;
	}
}

