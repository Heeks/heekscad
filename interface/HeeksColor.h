// HeeksColor.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class HeeksColor{
public:
	unsigned char red;
	unsigned char green;
	unsigned char blue;

	HeeksColor(void){}
	HeeksColor(unsigned char a, unsigned char b, unsigned char c);
	HeeksColor(long color);
	long COLORREF_color(void)const;
	bool operator==(const HeeksColor &b)const{if(red == b.red && green == b.green && blue == b.blue)return true;else return false;}
	void glMaterial(double opacity, GLenum face = GL_FRONT_AND_BACK)const;
	HeeksColor best_black_or_white(void)const;
	void glColor(void)const;
	void glClearColor(GLclampf alpha)const;
};
