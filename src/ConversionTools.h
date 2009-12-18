// ConversionTools.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/Tool.h"

extern bool ConvertLineArcsToWire2(const std::list<HeeksObj *> &list, TopoDS_Wire& wire);
extern bool ConvertSketchToFaceOrWire(HeeksObj* object, TopoDS_Shape& face_or_wire, bool face_not_wire);
extern bool ConvertFaceToSketch2(const TopoDS_Face& face, HeeksObj* sketch, double deviation);
extern bool ConvertEdgeToSketch2(const TopoDS_Edge& edge, HeeksObj* sketch, double deviation);
extern HeeksObj* SplitArcsIntoLittleLines(HeeksObj* sketch);

class ConvertSketchesToFace: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Convert sketch to face");}
	wxString BitmapPath(){return _T("la2face");}
};

class SketchesArcsToLines: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Split arcs to little lines");}
	wxString BitmapPath(){return _T("splitarcs");}
};

class MakeLineArcsToSketch: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Make To Sketch");}
	wxString BitmapPath(){return _T("makesketch");}
	const wxChar* GetToolTip(){return _("Make selected lines and arcs into a new sketch");}
};

class MakeEdgesToSketch: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Make To Sketch");}
	wxString BitmapPath(){return _T("makesketch");}
	const wxChar* GetToolTip(){return _("Make selected edges into a new sketch");}
};

class CombineSketches: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Combine sketches");}
	wxString BitmapPath(){return _T("sketchjoin");}
	const wxChar* GetToolTip(){return _("Combine selected sketches");}
};

class UniteSketches: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Unite sketches");}
	wxString BitmapPath(){return _T("sketchunite");}
	const wxChar* GetToolTip(){return _("Unite selected sketches");}
};

class GroupSelected: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Group");}
	wxString BitmapPath(){return _T("group");}
	const wxChar* GetToolTip(){return _("Group selected items");}
};

class UngroupSelected: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Ungroup");}
	wxString BitmapPath(){return _T("ungroup");}
	const wxChar* GetToolTip(){return _("Ungroup selected items");}
};

void GetConversionMenuTools(std::list<Tool*>* t_list);

