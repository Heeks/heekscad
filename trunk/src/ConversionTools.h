// ConversionTools.h
#include "../interface/Tool.h"

extern bool ConvertLineArcsToWire2(const std::list<HeeksObj *> &list, TopoDS_Wire& wire);
extern bool ConvertLineArcsToFace2(const std::list<HeeksObj *> &list, TopoDS_Face& face);
extern bool ConvertWireToFace2(const std::list<TopoDS_Wire> &list, std::list<TopoDS_Face>& faces);

class ConvertLineArcsToWire: public Tool
{
private:
	static wxBitmap* m_bitmap;
public:
	void Run();
	const wxChar* GetTitle(){return _T("LA To Wire");}
	wxBitmap* Bitmap(){if(m_bitmap == NULL){wxString exe_folder = wxGetApp().GetExeFolder();m_bitmap = new wxBitmap(exe_folder + _T("/bitmaps/la2wire.png"), wxBITMAP_TYPE_PNG);}return m_bitmap;}
	const wxChar* GetToolTip(){return _T("Convert lines and/or arcs to wire");}
};

class ConvertWireToFace: public Tool
{
private:
	static wxBitmap* m_bitmap;
public:
	void Run();
	const wxChar* GetTitle(){return _T("Wire To Face");}
	wxBitmap* Bitmap(){if(m_bitmap == NULL){wxString exe_folder = wxGetApp().GetExeFolder();m_bitmap = new wxBitmap(exe_folder + _T("/bitmaps/wire2face.png"), wxBITMAP_TYPE_PNG);}return m_bitmap;}
	const wxChar* GetToolTip(){return _T("Convert wire to face");}
};

class ConvertEdgesToWire: public Tool
{
private:
	static wxBitmap* m_bitmap;
public:
	void Run();
	const wxChar* GetTitle(){return _T("Convert Edges To Wire");}
	wxBitmap* Bitmap(){if(m_bitmap == NULL){wxString exe_folder = wxGetApp().GetExeFolder();m_bitmap = new wxBitmap(exe_folder + _T("/bitmaps/edges2wire.png"), wxBITMAP_TYPE_PNG);}return m_bitmap;}
	const wxChar* GetToolTip(){return _T("Convert edges to wire");}
};

class ConvertLineArcsToFace: public Tool
{
private:
	static wxBitmap* m_bitmap;
public:
	void Run();
	const wxChar* GetTitle(){return _T("Convert Lines and/or Arcs To Face");}
	wxBitmap* Bitmap(){if(m_bitmap == NULL){wxString exe_folder = wxGetApp().GetExeFolder();m_bitmap = new wxBitmap(exe_folder + _T("/bitmaps/la2face.png"), wxBITMAP_TYPE_PNG);}return m_bitmap;}
	const wxChar* GetToolTip(){return _T("Convert lines and/or arcs to face");}
};

void GetConversionMenuTools(std::list<Tool*>* t_list);
