// GripData.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

class GripData{
public:
	double m_x,m_y,m_z;
	void* m_data;
	EnumGripperType m_type;

	GripData(EnumGripperType type,double x, double y, double z, void* data){m_type = type; m_x = x; m_y = y; m_z = z; m_data = data;}
};

