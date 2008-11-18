// ConversionTools.h
#include "../interface/Tool.h"

extern bool ConvertLineArcsToWire2(const std::list<HeeksObj *> &list, TopoDS_Wire& wire);
extern bool ConvertSketchToFace2(HeeksObj* object, TopoDS_Face& face);

class ConvertSketchToFace: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Convert Sketch To Face");}
	wxString BitmapPath(){return _T("la2face");}
	const wxChar* GetToolTip(){return _("Convert sketch to face");}
};

void GetConversionMenuTools(std::list<Tool*>* t_list);
