// GLList.h

#pragma once

class CGLList  
{
public:
	CGLList();
	virtual ~CGLList();

	bool m_gl_list_exists;
	int m_gl_list;

	void DestroyGLList();
};
