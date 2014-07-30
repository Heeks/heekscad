// ConversionTools.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/Tool.h"

extern bool ConvertLineArcsToWire2(const std::list<HeeksObj *> &list, TopoDS_Wire& wire);
extern void SortEdges( std::vector<TopoDS_Edge> & edges );
extern bool ConvertEdgesToFaceOrWire(const std::vector<TopoDS_Edge> &edges, std::list<TopoDS_Shape> &face_or_wire, bool face_not_wire);
extern bool ConvertSketchToFaceOrWire(HeeksObj* object, std::list<TopoDS_Shape> &face_or_wire, bool face_not_wire);
extern bool ConvertFaceToSketch2(const TopoDS_Face& face, HeeksObj* sketch, double deviation);
extern bool ConvertWireToSketch(const TopoDS_Wire& wire, HeeksObj* sketch, double deviation);
extern bool ConvertEdgeToSketch2(const TopoDS_Edge& edge, HeeksObj* sketch, double deviation, bool reverse = false);
extern HeeksObj* SplitArcsIntoLittleLines(HeeksObj* sketch);
extern bool ConvertSketchToEdges(HeeksObj *object, std::list< std::vector<TopoDS_Edge> > &edges);
extern TopoDS_Wire EdgesToWire(const std::vector<TopoDS_Edge> &edges);
extern bool SketchToWires(HeeksObj* sketch, std::list<TopoDS_Wire> &wire_list);

class ConvertAreasToSketches: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Convert areas to sketches");}
	wxString BitmapPath(){return _T("sketch");}
};

class SketchesArcsToLines: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Split arcs to little lines");}
	wxString BitmapPath(){return _T("splitarcs");}
};

class ConvertToArea: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Convert to Area");}
	wxString BitmapPath(){return _T("area");}
};

class AreaUnion: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Unite Areas");}
	wxString BitmapPath(){return _T("areaunite");}
};


class AreaCut: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Cut Areas");}
	wxString BitmapPath(){return _T("areacut");}
};


class AreaIntersect: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Intersect Areas");}
	wxString BitmapPath(){return _T("areaintersect");}
};

class AreaXor: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Xor Areas");}
	wxString BitmapPath(){return _T("areaxor");}
};

class ConvertSketchesToFace: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Convert sketch to face");}
	wxString BitmapPath(){return _T("la2face");}
};

class MakeToSketch: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Make To Sketch");}
	wxString BitmapPath(){return _T("makesketch");}
	const wxChar* GetToolTip(){return _("Make selected objects into a new sketch");}
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

class TransformToCoordSys: public Tool
{
public:
	CoordinateSystem* coordsys1;
	CoordinateSystem* coordsys2;
	TransformToCoordSys():coordsys1(NULL),coordsys2(NULL){}
	void Run();
	const wxChar* GetTitle(){return _("Transform to Coordinate System");}
	wxString BitmapPath(){return _T("trsf2");}
	const wxChar* GetToolTip(){return _("Transform selected items using two coordinate systems");}
};

void GetConversionMenuTools(std::list<Tool*>* t_list);

