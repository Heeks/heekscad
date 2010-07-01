// GripData.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

class GripData{
public:
	double m_x,m_y,m_z;
	void* m_data;
	EnumGripperType m_type;
	int m_alternative_icon; // 0 for standard icon, 1, 2, 3, etc. for additional icons, where available
	bool m_move_relative; // true if dragged from the digitized position, false if dragged from the gripper

	GripData(EnumGripperType type,double x, double y, double z, void* data = NULL, bool move_relative = false, int alternative_icon = 0){m_type = type; m_x = x; m_y = y; m_z = z; m_data = data; m_alternative_icon = alternative_icon; m_move_relative = move_relative;}
};

