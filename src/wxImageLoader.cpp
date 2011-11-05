// wxImageLoader.cpp

// Licence: probably wxWindows licence
// copied from http://wiki.wxwidgets.org/Using_wxImage_to_load_textures_for_OpenGL

#include "stdafx.h"
#include "wxImageLoader.h"
 
 
GLuint* loadImage(wxString path, int* imageWidth, int* imageHeight, int* textureWidth, int* textureHeight)
{
	
	GLuint* ID=new GLuint[1];
	glGenTextures( 1, &ID[0] );
	
	glBindTexture( GL_TEXTURE_2D, *ID );
	
	// check the file exists
	if(!wxFileExists(path))
	{
		wxString message;
		message << _("Failed to load resource image from ") << path;
		wxMessageBox( message.c_str() );
		exit(1);	
	}
    
	wxImage img( path );
	
	(*imageWidth)=img.GetWidth();
	(*imageHeight)=img.GetHeight();
	
	glPixelStorei(GL_UNPACK_ALIGNMENT,   1   );
	
    /*
     * Many graphics card require that textures be power of two.
     * Below is a simple implementation, probably not optimal but working.
     * If your texture sizes are not restricted to power of 2s, you can
     * of course adapt the bit below as needed.
     */
    
	float power_of_two_that_gives_correct_width=std::log((float)(*imageWidth))/std::log(2.0f);
	float power_of_two_that_gives_correct_height=std::log((float)(*imageHeight))/std::log(2.0f);
	
	// check if image dimensions are a power of two
	if( (int)power_of_two_that_gives_correct_width == power_of_two_that_gives_correct_width &&
		(int)power_of_two_that_gives_correct_height == power_of_two_that_gives_correct_height){
		
		// if yes, everything is fine
		glTexImage2D(GL_TEXTURE_2D,
					 0,
					 img.HasAlpha() ?  4 : 3, 
					 *imageWidth,
					 *imageHeight,
					 0, 
					 img.HasAlpha() ?  GL_RGBA : GL_RGB,
					 GL_UNSIGNED_BYTE,
					 img.GetData());
		
		(*textureWidth)  = (*imageWidth);
		(*textureHeight) = (*imageHeight);
		
	}
	else // texture is not a power of two. We need to resize it
	{
		
		int newWidth=(int)std::pow( 2.0, (int)(std::ceil(power_of_two_that_gives_correct_width)) );
		int newHeight=(int)std::pow( 2.0, (int)(std::ceil(power_of_two_that_gives_correct_height)) );
		
		//printf("Unsupported image size. Recommand values: %i %i\n",newWidth,newHeight);   
		
		GLubyte	*bitmapData=img.GetData();
		GLubyte        *alphaData=img.GetAlpha();
		GLubyte	*imageData;
		
		int old_bytesPerPixel = 3;
		int bytesPerPixel = img.HasAlpha() ?  4 : 3;
		
		int imageSize = newWidth * newHeight * bytesPerPixel;
		imageData=(GLubyte *)malloc(imageSize);
		
		int rev_val=(*imageHeight)-1;
		
		for(int y=0; y<newHeight; y++)
		{
			for(int x=0; x<newWidth; x++)
			{
				
				if( x<(*imageWidth) && y<(*imageHeight) ){
					imageData[(x+y*newWidth)*bytesPerPixel+0]=
					bitmapData[( x+(rev_val-y)*(*imageWidth))*old_bytesPerPixel + 0];
					
					imageData[(x+y*newWidth)*bytesPerPixel+1]=
						bitmapData[( x+(rev_val-y)*(*imageWidth))*old_bytesPerPixel + 1];
					
					imageData[(x+y*newWidth)*bytesPerPixel+2]=
						bitmapData[( x+(rev_val-y)*(*imageWidth))*old_bytesPerPixel + 2];
					
					if(bytesPerPixel==4) imageData[(x+y*newWidth)*bytesPerPixel+3]=
						alphaData[ x+(rev_val-y)*(*imageWidth) ];
					
				}
				else
				{
					
					imageData[(x+y*newWidth)*bytesPerPixel+0] = 0;
					imageData[(x+y*newWidth)*bytesPerPixel+1] = 0;
					imageData[(x+y*newWidth)*bytesPerPixel+2] = 0;
					if(bytesPerPixel==4) imageData[(x+y*newWidth)*bytesPerPixel+3] = 0;
				}
				
			}//next
		}//next
		
		
		glTexImage2D(GL_TEXTURE_2D,
					 0,
					 img.HasAlpha() ?  4 : 3,
					 newWidth,
					 newHeight,
					 0, 
					 img.HasAlpha() ?  GL_RGBA : GL_RGB, 
					 GL_UNSIGNED_BYTE,
					 imageData);
		
		(*textureWidth)=newWidth;
		(*textureHeight)=newHeight;
		
		free(imageData);
	}

	// set texture parameters as you wish
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	return ID;
}

