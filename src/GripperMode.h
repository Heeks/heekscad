// GripperMode.h

#pragma once

enum EnumTransformationMode{
	TranslationMode,
	RotationMode,
	ScaleMode,
	StretchMode
};

class GripperMode  
{
public:
	EnumTransformationMode m_mode;
	bool m_make_copies;
	int m_number_of_copies;
	bool m_make_as_pattern;
	int m_use_endof_for_from_except_for_screen_xy;
	gp_Pnt m_centre_point;
	gp_Pnt m_centre_point2;
	double m_rotate_angle_degrees;
	double m_scale_x;
	double m_scale_y;
	double m_scale_z;
	double m_shift_x;
	double m_shift_y;
	double m_shift_z;

	GripperMode();
	virtual ~GripperMode();

	void GetProfileStrings();
	void WriteProfileStrings();
	void GetOptions(std::list<Property *> *list);
};
