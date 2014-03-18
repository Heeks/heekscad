// TransformTools.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

// menus and tool bar functions

class TransformTools{
	static void RemoveUncopyable();

public:
	static void Translate(bool copy);
	static void Rotate(bool copy);
	static void Mirror(bool copy);
	static void Scale(bool copy);
};

