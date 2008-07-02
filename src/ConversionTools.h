// ConversionTools.h
#include "../interface/Tool.h"

class ConvertLineArcsToWire: public Tool
{
public:
	void Run();
	const char* GetTitle(){return "Convert Lines and/or Arcs To Wire";}
};

class ConvertWireToFace: public Tool
{
public:
	void Run();
	const char* GetTitle(){return "Convert Wire To Face";}
};

class ConvertLineArcsToFace: public Tool
{
public:
	void Run();
	const char* GetTitle(){return "Convert Lines and/or Arcs To Face";}
};

void GetConversionMenuTools(std::list<Tool*>* t_list, const wxPoint* p, MarkedObject* marked_object);
