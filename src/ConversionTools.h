// ConversionTools.h
#include "../interface/Tool.h"

extern bool ConvertLineArcsToWire2(const std::list<HeeksObj *> &list, TopoDS_Wire& wire);
extern bool ConvertSketchToFace2(HeeksObj* object, TopoDS_Face& face);

class ConvertSketchToFace: public Tool
{
private:
	static wxBitmap* m_bitmap;
public:
	void Run();
	const wxChar* GetTitle(){return _T("Convert Sketch To Face");}
	wxBitmap* Bitmap(){if(m_bitmap == NULL){wxString exe_folder = wxGetApp().GetExeFolder();m_bitmap = new wxBitmap(exe_folder + _T("/bitmaps/la2face.png"), wxBITMAP_TYPE_PNG);}return m_bitmap;}
	const wxChar* GetToolTip(){return _T("Convert sketch to face");}
};

void GetConversionMenuTools(std::list<Tool*>* t_list);
