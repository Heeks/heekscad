// GripperMode.cpp
#include "stdafx.h"
#include "GripperMode.h"
#include "../interface/PropertyChoice.h"
#include "../interface/PropertyDouble.h"
#include "HeeksFrame.h"
#include "InputModeCanvas.h"

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
	wxGetApp().m_config->Read(_T("GripMode"), (long*)(&m_mode), 0);
	wxGetApp().m_config->Read(_T("GripMakeCopies"), &m_make_copies, false);
	wxGetApp().m_config->Read(_T("GripNumberOfCopies"), &m_number_of_copies, 1);
	wxGetApp().m_config->Read(_T("GripMakePattern"), &m_make_as_pattern, false);
	double x, y, z;
	wxGetApp().m_config->Read(_T("GrippingCentreX"), &x, 0.0);
	wxGetApp().m_config->Read(_T("GrippingCentreY"), &y, 0.0);
	wxGetApp().m_config->Read(_T("GrippingCentreZ"), &z, 0.0);
	m_centre_point = gp_Pnt(x, y, z);
	wxGetApp().m_config->Read(_T("GrippingCentre2X"), &x, 0.0);
	wxGetApp().m_config->Read(_T("GrippingCentre2Y"), &y, 0.0);
	wxGetApp().m_config->Read(_T("GrippingCentre2Z"), &z, 0.0);
	m_centre_point2 = gp_Pnt(x, y, z);
	wxGetApp().m_config->Read(_T("GripFromUsesEndofExceptScreen"), &m_use_endof_for_from_except_for_screen_xy, 1);
	wxGetApp().m_config->Read(_T("GrippingRotateAngle"), &m_rotate_angle_degrees, 90.0);
	wxGetApp().m_config->Read(_T("GrippingScaleX"), &m_scale_x, 1.0);
	wxGetApp().m_config->Read(_T("GrippingScaleY"), &m_scale_y, 1.0);
	wxGetApp().m_config->Read(_T("GrippingScaleZ"), &m_scale_z, 1.0);
}

void GripperMode::WriteProfileStrings(){
	wxGetApp().m_config->Write(_T("GripMode"), (long)(m_mode));
	wxGetApp().m_config->Write(_T("GripMakeCopies"), m_make_copies);
	wxGetApp().m_config->Write(_T("GripNumberOfCopies"), m_number_of_copies);
	wxGetApp().m_config->Write(_T("GripMakePattern"), m_make_as_pattern);
	wxGetApp().m_config->Write(_T("GrippingCentreX"), m_centre_point.X());
	wxGetApp().m_config->Write(_T("GrippingCentreY"), m_centre_point.Y());
	wxGetApp().m_config->Write(_T("GrippingCentreZ"), m_centre_point.Z());
	wxGetApp().m_config->Write(_T("GrippingCentre2X"), m_centre_point2.X());
	wxGetApp().m_config->Write(_T("GrippingCentre2Y"), m_centre_point2.Y());
	wxGetApp().m_config->Write(_T("GrippingCentre2Z"), m_centre_point2.Z());
	wxGetApp().m_config->Write(_T("GripFromUsesEndofExceptScreen"), m_use_endof_for_from_except_for_screen_xy);
	wxGetApp().m_config->Write(_T("GrippingRotateAngle"), m_rotate_angle_degrees);
	wxGetApp().m_config->Write(_T("GrippingScaleX"), m_scale_x);
	wxGetApp().m_config->Write(_T("GrippingScaleY"), m_scale_y);
	wxGetApp().m_config->Write(_T("GrippingScaleZ"), m_scale_z);
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
	wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
	wxGetApp().Repaint();
}

static GripperMode* gripper_mode_for_on_set = NULL;

static void on_set_centre_x(double value)
{
	gripper_mode_for_on_set->m_centre_point.SetX(value);
}

static void on_set_centre_y(double value)
{
	gripper_mode_for_on_set->m_centre_point.SetY(value);
}

static void on_set_centre_z(double value)
{
	gripper_mode_for_on_set->m_centre_point.SetZ(value);
}

void GripperMode::GetOptions(std::list<Property *> *list){
	std::list< wxString > choices;
	choices.push_back(wxString(_T("Translate")));
	choices.push_back(wxString(_T("StretchMode")));
	choices.push_back(wxString(_T("Rotate")));
	choices.push_back(wxString(_T("ScaleMode")));
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
	list->push_back( new PropertyChoice(_T("gripper mode"),  choices, index, choice_callback));	
	switch(m_mode){
		case RotationMode:
			list->push_back(new PropertyDouble(_T("X rotate about"), m_centre_point.X(), on_set_centre_x));
			list->push_back(new PropertyDouble(_T("Y rotate about"), m_centre_point.Y(), on_set_centre_y));
			list->push_back(new PropertyDouble(_T("Z rotate about"), m_centre_point.Z(), on_set_centre_z));
			break;
		case ScaleMode:
			list->push_back(new PropertyDouble(_T("X scale about"), m_centre_point.X(), on_set_centre_x));
			list->push_back(new PropertyDouble(_T("Y scale about"), m_centre_point.Y(), on_set_centre_y));
			list->push_back(new PropertyDouble(_T("Z scale about"), m_centre_point.Z(), on_set_centre_z));
			break;
	}
}
