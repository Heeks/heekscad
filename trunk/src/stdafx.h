// stdafx.h
#ifdef WIN32
#pragma warning(disable : 4996)
#endif

#include <list>
#include <vector>
#include <map>
#include <set>
#include <fstream>

#include <wx/wx.h>

#ifdef WIN32
#pragma warning(disable:4100)
#pragma warning(  disable : 4244 )        // Issue warning 4244
#endif

#ifdef WIN32
#pragma warning(  default : 4244 )        // Issue warning 4244
#endif

extern "C" {
#include <GL/gl.h>
#ifdef WIN32
#include <GL/glu.h>
#else
#include <GL/glu.h>
#endif
}

