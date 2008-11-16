// TransformTools.h

// menus and tool bar functions

class TransformTools{
	static void RemoveUncopyable();

public:
	static void Translate(bool copy);
	static void Rotate(bool copy);
	static void Mirror(bool copy);
	static void Scale(bool copy);
};