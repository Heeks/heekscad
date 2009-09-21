// TransientObject.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class TransientObject: public HeeksObj{
public:
	HeeksObj* m_object;

	TransientObject(HeeksObj* object);

	// HeeksObj's virtual functions
	HeeksObj *MakeACopy()const;
};
