// Material.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class Material{
public:
	GLfloat matf[10];
	
	// initial values
	Material(void){
		matf[0] = 0.3f;
		matf[1] = 0.3f;
		matf[2] = 0.3f;
		matf[3] = 0.8f;
		matf[4] = 0.8f;
		matf[5] = 0.8f;
		matf[6] = 0.3f;
		matf[7] = 0.3f;
		matf[8] = 0.3f;
		matf[9] = 15.0f;
	}
	
	Material(float *m){memcpy(matf, m, 10*sizeof(float));}

	Material(const HeeksColor& col){
		matf[0] = 0.1f + 0.2666666667f * (float)col.red/255;
		matf[1] = 0.1f + 0.2666666667f * (float)col.green/255;
		matf[2] = 0.1f + 0.2666666667f * (float)col.blue/255;
		matf[3] = 0.2f + 0.8f * (float)col.red / 255;
		matf[4] = 0.2f + 0.8f * (float)col.green / 255;
		matf[5] = 0.2f + 0.8f * (float)col.blue / 255;
		matf[6] = 0.3f;
		matf[7] = 0.3f;
		matf[8] = 0.3f;
		matf[9] = 15.0f;
	}

	bool glMaterial(float opacity)
	{
		bool blend_enabled = false;
		if (opacity<1) {
			glEnable(GL_BLEND);
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			glDepthMask(0);
			blend_enabled = true;
		}
		float f[4] = {0.0f, 0.0f, 0.0f, opacity};
		memcpy(f, &matf[0], 3*sizeof(float));
		glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, f );
		memcpy(f, &matf[3], 3*sizeof(float));
		glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, f );
		memcpy(f, &matf[6], 3*sizeof(float));
		glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, f );
		f[0] = 0.0f; f[1] = 0.0f; f[2] = 0.0f;
		glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, f );
		glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, matf[9] );
		return blend_enabled;
	}
};
