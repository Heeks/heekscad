// Gripper.cpp

#include "stdafx.h"
#include "Gripper.h"
#include "../interface/HeeksColor.h"

static unsigned char circle[18] = {0x1c, 0x00, 0x63, 0x00, 0x41, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x41, 0x00, 0x63, 0x00, 0x1c, 0x00};

Gripper::Gripper(const gp_Pnt& pos, const char* str){
	position = pos;
	prompt = std::string(str);
}

void Gripper::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		wxGetApp().glColorEnsuringContrast(HeeksColor(0, 0, 0));
	}
	glRasterPos3d(position.X(), position.Y(), position.Z());
	glBitmap(9, 9, 4, 4, 10.0, 0.0, circle);
}

void Gripper::ModifyByMatrix(const double *m){
	gp_Trsf mat = make_matrix(m);
	position.Transform(mat);
}
