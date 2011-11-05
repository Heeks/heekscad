// wxImageLoader.h

// Licence: probably wxWindows licence
// copied from http://wiki.wxwidgets.org/Using_wxImage_to_load_textures_for_OpenGL

#ifndef _image_loader
#define _image_loader
 
#ifdef __WXMAC__
#include "OpenGL/gl.h"
#else
#include <GL/gl.h>
#endif
#include "wx/wx.h"
	
GLuint* loadImage(wxString path, int* imageWidth, int* imageHeight, int* textureWidth, int* textureHeight);
 
#endif 

