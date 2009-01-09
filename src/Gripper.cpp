// Gripper.cpp

#include "stdafx.h"
#include "Gripper.h"
#include "../interface/HeeksColor.h"

static unsigned char circle[18] = {0x1c, 0x00, 0x63, 0x00, 0x41, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x41, 0x00, 0x63, 0x00, 0x1c, 0x00};
static unsigned char translation_circle[30] = {0x00, 0x80, 0x01, 0xc0, 0x00, 0x80, 0x01, 0xc0, 0x06, 0x30, 0x04, 0x10, 0x28, 0x0a, 0x78, 0x0f, 0x28, 0x0a, 0x04, 0x10, 0x06, 0x30, 0x01, 0xc0, 0x00, 0x80, 0x01, 0xc0, 0x00, 0x80};
static unsigned char rotation_circle[26] = {0x01, 0xc0, 0x06, 0x30, 0x04, 0x10, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x04, 0x10, 0x06, 0x31, 0x61, 0xc2, 0x60, 0x02, 0x10, 0x04, 0x0c, 0x18, 0x03, 0xe0};
static unsigned char rotation_object_circle[26] = {0x01, 0xc0, 0x06, 0xb0, 0x04, 0x90, 0x08, 0x88, 0x0f, 0xf8, 0x08, 0x88, 0x04, 0x90, 0x06, 0xb1, 0x61, 0xc2, 0x60, 0x02, 0x10, 0x04, 0x0c, 0x18, 0x03, 0xe0};
static unsigned char scale_circle[30] = {0x03, 0xe0, 0x0c, 0x18, 0x10, 0x04, 0x21, 0xc2, 0x26, 0x32, 0x44, 0x11, 0x48, 0x09, 0x48, 0x09, 0x48, 0x09, 0x44, 0x11, 0x26, 0x32, 0x21, 0xc2, 0x10, 0x04, 0x0c, 0x18, 0x03, 0xe0};
static unsigned char angle_circle[32] = {0x10, 0x04, 0x10, 0x04, 0x08, 0x08, 0x08, 0x08, 0x04, 0x10, 0x04, 0x10, 0x02, 0x40, 0x03, 0xe0, 0x07, 0x70, 0x05, 0x50, 0x08, 0x88, 0x08, 0x88, 0x08, 0x08, 0x04, 0x10, 0x06, 0x30, 0x01, 0xc0};

Gripper::Gripper(const gp_Pnt& pos, const wxChar* str, EnumGripperType gripper_type){
	position = pos;
	prompt = wxString(str);
	m_gripper_type = gripper_type;
}

void Gripper::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		wxGetApp().glColorEnsuringContrast(HeeksColor(0, 0, 0));
	}
	if(select)
	{
		double s = 5.0 / wxGetApp().GetPixelScale();
		double p[8][3] = {
			{-s, -s, -s},
			{s, -s, -s},
			{s, s, -s},
			{-s, s, -s},
			{-s, -s, s},
			{s, -s, s},
			{s, s, s},
			{-s, s, s}
		};

		for(int i = 0; i<8; i++){
			p[i][0] += position.X();
			p[i][1] += position.Y();
			p[i][2] += position.Z();
		}

		glBegin(GL_TRIANGLES);
		glVertex3dv(p[0]);
		glVertex3dv(p[2]);
		glVertex3dv(p[1]);
		glVertex3dv(p[0]);
		glVertex3dv(p[3]);
		glVertex3dv(p[2]);

		glVertex3dv(p[0]);
		glVertex3dv(p[1]);
		glVertex3dv(p[5]);
		glVertex3dv(p[0]);
		glVertex3dv(p[5]);
		glVertex3dv(p[4]);

		glVertex3dv(p[3]);
		glVertex3dv(p[0]);
		glVertex3dv(p[4]);
		glVertex3dv(p[3]);
		glVertex3dv(p[4]);
		glVertex3dv(p[7]);

		glVertex3dv(p[4]);
		glVertex3dv(p[5]);
		glVertex3dv(p[6]);
		glVertex3dv(p[4]);
		glVertex3dv(p[6]);
		glVertex3dv(p[7]);

		glVertex3dv(p[3]);
		glVertex3dv(p[7]);
		glVertex3dv(p[6]);
		glVertex3dv(p[3]);
		glVertex3dv(p[6]);
		glVertex3dv(p[2]);

		glVertex3dv(p[2]);
		glVertex3dv(p[6]);
		glVertex3dv(p[5]);
		glVertex3dv(p[2]);
		glVertex3dv(p[5]);
		glVertex3dv(p[1]);

		glEnd();
	}
	else
	{
		glRasterPos3d(position.X(), position.Y(), position.Z());
		switch(m_gripper_type){
		case GripperTypeTranslate:
			glBitmap(16, 15, 8, 7, 10.0, 0.0, translation_circle);
			break;
		case GripperTypeRotateObject:
			glBitmap(16, 13, 8, 4, 10.0, 0.0, rotation_object_circle);
			break;
		case GripperTypeRotate:
			glBitmap(16, 13, 8, 4, 10.0, 0.0, rotation_circle);
			break;
		case GripperTypeScale:
			glBitmap(16, 15, 8, 7, 10.0, 0.0, scale_circle);
			break;
		case GripperTypeAngle:
			glBitmap(14, 16, 9, 11, 10.0, 0.0, angle_circle);
			break;
		case GripperTypeStretch:
			glBitmap(9, 9, 4, 4, 10.0, 0.0, circle);
			break;
		default:
			glBitmap(9, 9, 4, 4, 10.0, 0.0, circle);
			break;
		}
	}
}

bool Gripper::ModifyByMatrix(const double *m){
	gp_Trsf mat = make_matrix(m);
	position.Transform(mat);
	return false;
}
