// GripperMode.cpp
#include "stdafx.h"
#include "GripperMode.h"
#include "../interface/PropertyChoice.h"

GripperMode::GripperMode()
{
	m_mode = TranslationMode;
	m_make_copies = false;
	m_number_of_copies = 1;
	m_make_as_pattern = false;
	m_use_endof_for_from_except_for_screen_xy = 0;
	m_centre_point = gp_Pnt(0, 0, 0);
	m_centre_point2 = gp_Pnt(0, 0, 0);
	m_rotate_angle_degrees = 90;
	m_scale_x = 1;
	m_scale_y = 1;
	m_scale_z = 1;
	m_shift_x = 0;
	m_shift_y = 0;
	m_shift_z = 0;
}

GripperMode::~GripperMode()
{
}

void GripperMode::GetProfileStrings(){
	wxGetApp().m_config->Read("GripMode", (long*)(&m_mode), 0);
	wxGetApp().m_config->Read("GripMakeCopies", &m_make_copies, false);
	wxGetApp().m_config->Read("GripNumberOfCopies", &m_number_of_copies, 1);
	wxGetApp().m_config->Read("GripMakePattern", &m_make_as_pattern, false);
	double x, y, z;
	wxGetApp().m_config->Read("GrippingCentreX", &x, 0.0);
	wxGetApp().m_config->Read("GrippingCentreY", &y, 0.0);
	wxGetApp().m_config->Read("GrippingCentreZ", &z, 0.0);
	m_centre_point = gp_Pnt(x, y, z);
	wxGetApp().m_config->Read("GrippingCentre2X", &x, 0.0);
	wxGetApp().m_config->Read("GrippingCentre2Y", &y, 0.0);
	wxGetApp().m_config->Read("GrippingCentre2Z", &z, 0.0);
	m_centre_point2 = gp_Pnt(x, y, z);
	wxGetApp().m_config->Read("GripFromUsesEndofExceptScreen", &m_use_endof_for_from_except_for_screen_xy, 1);
	wxGetApp().m_config->Read("GrippingRotateAngle", &m_rotate_angle_degrees, 90.0);
	wxGetApp().m_config->Read("GrippingScaleX", &m_scale_x, 1.0);
	wxGetApp().m_config->Read("GrippingScaleY", &m_scale_y, 1.0);
	wxGetApp().m_config->Read("GrippingScaleZ", &m_scale_z, 1.0);
}

void GripperMode::WriteProfileStrings(){
	wxGetApp().m_config->Write("GripMode", (long)(m_mode));
	wxGetApp().m_config->Write("GripMakeCopies", m_make_copies);
	wxGetApp().m_config->Write("GripNumberOfCopies", m_number_of_copies);
	wxGetApp().m_config->Write("GripMakePattern", m_make_as_pattern);
	wxGetApp().m_config->Write("GrippingCentreX", m_centre_point.X());
	wxGetApp().m_config->Write("GrippingCentreY", m_centre_point.Y());
	wxGetApp().m_config->Write("GrippingCentreZ", m_centre_point.Z());
	wxGetApp().m_config->Write("GrippingCentre2X", m_centre_point2.X());
	wxGetApp().m_config->Write("GrippingCentre2Y", m_centre_point2.Y());
	wxGetApp().m_config->Write("GrippingCentre2Z", m_centre_point2.Z());
	wxGetApp().m_config->Write("GripFromUsesEndofExceptScreen", m_use_endof_for_from_except_for_screen_xy);
	wxGetApp().m_config->Write("GrippingRotateAngle", m_rotate_angle_degrees);
	wxGetApp().m_config->Write("GrippingScaleX", m_scale_x);
	wxGetApp().m_config->Write("GrippingScaleY", m_scale_y);
	wxGetApp().m_config->Write("GrippingScaleZ", m_scale_z);
}

void choice_callback(int choice){
	switch(choice){
		case 0:
			wxGetApp().gripper_mode->m_mode = TranslationMode;
			break;
		case 1:
			wxGetApp().gripper_mode->m_mode = StretchMode;
			break;
		case 2:
			wxGetApp().gripper_mode->m_mode = RotationMode;
			break;
		case 3:
			wxGetApp().gripper_mode->m_mode = ScaleMode;
			break;
		default:
			return;
	}
	wxGetApp().Repaint();
}

void GripperMode::GetProperties(std::list<Property *> *list){
	std::list< std::string > choices;
	choices.push_back(std::string("Translate"));
	choices.push_back(std::string("StretchMode"));
	choices.push_back(std::string("Rotate"));
	choices.push_back(std::string("ScaleMode"));
	int index = 0;
	switch(m_mode){
		case TranslationMode:
			index = 0;
			break;
		case StretchMode:
			index = 1;
			break;
		case RotationMode:
			index = 2;
			break;
		case ScaleMode:
			index = 3;
			break;
		default:
			index = 3;
			break;
	}
	list->push_back( new PropertyChoice("gripper mode",  choices, index, choice_callback));	
}
