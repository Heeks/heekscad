// GLList.cpp:
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