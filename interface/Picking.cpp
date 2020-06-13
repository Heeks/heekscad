#include "stdafx.h"
#include "Picking.h"

#define PRETTY_PICKING_COLOUR

#ifdef PRETTY_PICKING_COLOUR
// for pretty testing, use a list of colors

unsigned char test_colours[16][3] = {
	{0xff, 0x00, 0x00},
	{0xff, 0x80, 0x01},
	{0xff, 0xff, 0x02},
	{0x00, 0xff, 0x43},
	{0x00, 0xff, 0xf4},
	{0x00, 0x80, 0xf5},
	{0x00, 0x00, 0xf6},
	{0x80, 0x00, 0xf7},
	{0xff, 0x00, 0xf8},
	{0x00, 0x00, 0x09},
	{0xff, 0xff, 0xfa},
	{0x80, 0x80, 0x7b},
	{0x80, 0x40, 0x3c},
	{0x80, 0x80, 0x0d},
	{0xff, 0x80, 0xfe},
	{0x00, 0x80, 0x3f},
};

#endif



void SetPickingColor(unsigned int name)
{
	unsigned char r = name & 0xff;
	unsigned char g = (name & 0xff00) >> 8;
	unsigned char b = (name & 0xff0000) >> 16;
#ifdef PRETTY_PICKING_COLOUR
	unsigned int col = name % 16;
	r += test_colours[col][0];
	g += test_colours[col][1];
	b = test_colours[col][2];
#endif
	glColor3ub(r, g, b); 
}

unsigned int GetPickingName(unsigned char r, unsigned char g, unsigned char b)
{
#ifdef PRETTY_PICKING_COLOUR
	if(r == 0 && g == 0 && b == 0)
		return 0;
	unsigned char mod = b & 0x0f;
	r -= test_colours[mod][0];
	g -= test_colours[mod][1];
	b = 0;
#endif
	return r | (g << 8) | (b << 16);
}