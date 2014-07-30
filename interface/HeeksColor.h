// HeeksColor.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

typedef int Aci_t; // AutoCAD color index

class HeeksColor{
public:
	unsigned char red;
	unsigned char green;
	unsigned char blue;

	HeeksColor(void){red = 0; green = 0; blue = 0;}
	HeeksColor(unsigned char a, unsigned char b, unsigned char c);
	HeeksColor(long color);
	HeeksColor(Aci_t aci);
	long COLORREF_color(void)const;
	bool operator==(const HeeksColor &b)const{if(red == b.red && green == b.green && blue == b.blue)return true;else return false;}
	HeeksColor best_black_or_white(void)const;
	void glColor(void)const;
	void glClearColor(GLclampf alpha)const;

	bool operator!= ( const HeeksColor & rhs ) const { return(! (*this == rhs)); }
};
