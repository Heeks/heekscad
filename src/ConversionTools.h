// ConversionTools.h
/*
 * Copyright (c) 2009, Dan Heeks
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */
#include "../interface/Tool.h"

extern bool ConvertLineArcsToWire2(const std::list<HeeksObj *> &list, TopoDS_Wire& wire);
extern bool ConvertSketchToFace2(HeeksObj* object, TopoDS_Face& face);

class ConvertSketchToFace: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Convert sketch to face");}
	wxString BitmapPath(){return _T("la2face");}
};

class MakeLineArcsToSketch: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Make To Sketch");}
	wxString BitmapPath(){return _T("makesketch");}
	const wxChar* GetToolTip(){return _("Make selected lines and arcs into a new sketch");}
};

class CombineSketches: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Combine sketches");}
	wxString BitmapPath(){return _T("sketchjoin");}
	const wxChar* GetToolTip(){return _("Combine selected sketches");}
};

void GetConversionMenuTools(std::list<Tool*>* t_list);
