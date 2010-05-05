// SketchTools.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "MarkedList.h"
#include "Sketch.h"
#include "Group.h"
#include "Pad.h"
#include "Part.h"
#include "Pocket.h"
#include "ConversionTools.h"
#include "../interface/HeeksCADInterface.h"
#include "../interface/PropertyList.h"
#include "../interface/PropertyCheck.h"
#include "../interface/PropertyLength.h"
#include "HeeksConfig.h"

extern CHeeksCADInterface heekscad_interface;

/**
	These settings relate to the fixes available in the ShapeFix_Wire class.
 */
class SketchToolOptions
{
public:
    SketchToolOptions()
    {
        HeeksConfig config;
        config.Read(_T("SketchToolOptions_reorder"), &m_reorder, true);
        config.Read(_T("SketchToolOptions_max_gap"), &m_max_gap, 0.001);
        config.Read(_T("SketchToolOptions_fix_small"), &m_fix_small, true);
        config.Read(_T("SketchToolOptions_fix_small_precision"), &m_fix_small_precision, 0.0254);
        config.Read(_T("SketchToolOptions_fix_self_intersection"), &m_fix_self_intersection, true);
        config.Read(_T("SketchToolOptions_fix_lacking"), &m_fix_lacking, true);
        config.Read(_T("SketchToolOptions_fix_degenerated"), &m_fix_degenerated, true);

        config.Read(_T("SketchToolOptions_modify_topology"), &m_modify_topology, true);
        config.Read(_T("SketchToolOptions_modify_geometry"), &m_modify_geometry, true);
        config.Read(_T("SketchToolOptions_closed_wire"), &m_closed_wire, true);
        config.Read(_T("SketchToolOptions_preference_pcurve"), &m_preference_pcurve, true);
        config.Read(_T("SketchToolOptions_fix_gaps_by_ranges"), &m_fix_gaps_by_ranges, true);

  // Standard_Integer& ModifyRemoveLoopMode() ;


    }

    void SaveSettings()
    {
        HeeksConfig config;
        config.Write(_T("SketchToolOptions_reorder"), m_reorder);
        config.Write(_T("SketchToolOptions_max_gap"), m_max_gap);
        config.Write(_T("SketchToolOptions_fix_small"), m_fix_small);
        config.Write(_T("SketchToolOptions_fix_small_precision"), m_fix_small_precision);
        config.Write(_T("SketchToolOptions_fix_self_intersection"), m_fix_self_intersection);
        config.Write(_T("SketchToolOptions_fix_lacking"), m_fix_lacking);
        config.Write(_T("SketchToolOptions_fix_degenerated"), m_fix_degenerated);
        config.Write(_T("SketchToolOptions_modify_topology"), m_modify_topology);
        config.Write(_T("SketchToolOptions_modify_geometry"), m_modify_geometry);
        config.Write(_T("SketchToolOptions_closed_wire"), m_closed_wire);
        config.Write(_T("SketchToolOptions_preference_pcurve"), m_preference_pcurve);
        config.Write(_T("SketchToolOptions_fix_gaps_by_ranges"), m_fix_gaps_by_ranges);
    }

public:
    bool m_reorder;
    double m_max_gap;
    bool m_fix_small;
    double m_fix_small_precision;
    bool m_fix_self_intersection;
    bool m_fix_lacking;
    bool m_fix_degenerated;
    bool m_modify_topology;
    bool m_modify_geometry;
    bool m_closed_wire;
    bool m_preference_pcurve;
    bool m_fix_gaps_by_ranges;
};

static SketchToolOptions sketch_tool_options;



class PadSketch:public Tool{
public:
	void Run(){
		double height = 10;
		wxGetApp().InputLength(_("Input extrusion height"), _("height"), height);
		CSketch* sketch = (CSketch*)(*wxGetApp().m_marked_list->list().begin());

		CPad::PadSketch(sketch,height);
	}
	const wxChar* GetTitle(){return _T("Pad Sketch");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Pad Sketch");}
};

class PocketSketch:public Tool{
public:
	void Run(){
		double height = 10;
		wxGetApp().InputLength(_("Input pad depth"), _("height"), height);
		CSketch* sketch = (CSketch*)(*wxGetApp().m_marked_list->list().begin());

		CPocket::PocketSketch(sketch,height);
	}
	const wxChar* GetTitle(){return _T("Pocket Sketch");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Pocket Sketch");}
};


class MakeToPart:public Tool{
public:
	void Run(){
		CSketch* sketch = (CSketch*)(*wxGetApp().m_marked_list->list().begin());
		CPart* part = new CPart();
		wxGetApp().Add(part,NULL);
		sketch->Owner()->Remove(sketch);
		sketch->RemoveOwners();
		part->Add(sketch,NULL);
	}
	const wxChar* GetTitle(){return _T("Make To Part");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Make To Part");}
};

class AddToPart:public Tool{
public:
	void Run()
	{
		std::list<HeeksObj*>::const_iterator It;
		CSketch* sketch=NULL;
		CPad* pad=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
		{
			HeeksObj* obj = (HeeksObj*)*It;
			if(obj->GetType() == SketchType)
				sketch = (CSketch*)obj;
			else
				pad = (CPad*)obj;
		}
		sketch->Owner()->Remove(sketch);
		sketch->RemoveOwners();
		pad->Add(sketch,NULL);
	}
	const wxChar* GetTitle(){return _T("Add To Part");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Add To Part");}
};


class FixWire:public Tool{
public:
	void Run()
	{
		std::list<HeeksObj*>::const_iterator It;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
		{
			HeeksObj* obj = (HeeksObj*)*It;
			if(obj->GetType() == SketchType)
			{
				CSketch* sketch = (CSketch*)obj;

                std::list<TopoDS_Shape> wires;
                if (! ConvertSketchToFaceOrWire( sketch, wires, false))
                {
                    wxMessageBox(_("Failed to convert sketch to wire"));
                } // End if - then
                else
                {
                    try {
                        // For all wires in this sketch...
                        for(std::list<TopoDS_Shape>::iterator It2 = wires.begin(); It2 != wires.end(); It2++)
                        {
                            TopoDS_Shape& wire_to_fix = *It2;
                            ShapeFix_Wire fix;
                            fix.Load( TopoDS::Wire(wire_to_fix) );
                            // fix.ClearModes();

                            fix.ModifyTopologyMode() = sketch_tool_options.m_modify_topology;
                            fix.ModifyGeometryMode() = sketch_tool_options.m_modify_geometry;
                            fix.ClosedWireMode() = sketch_tool_options.m_closed_wire;
                            fix.PreferencePCurveMode() = sketch_tool_options.m_preference_pcurve;
                            fix.FixGapsByRangesMode() = sketch_tool_options.m_fix_gaps_by_ranges;
                            fix.FixSelfIntersectionMode() = sketch_tool_options.m_fix_self_intersection;
                            fix.FixReorderMode() = sketch_tool_options.m_reorder;
                            fix.FixDegeneratedMode() = sketch_tool_options.m_fix_degenerated;

                            if (sketch_tool_options.m_fix_small)
                            {
                                fix.FixSmallMode() = true;
                                fix.FixSmall( false, sketch_tool_options.m_fix_small_precision );
                            }

                            if (sketch_tool_options.m_fix_lacking)
                            {
                                fix.FixLackingMode() = sketch_tool_options.m_fix_lacking;
                                fix.FixLacking(false);
                            }

                            if (! fix.Perform())
                            {
                                wxMessageBox(_("Wire fix failed"));
                            }
                            else
                            {
                                TopoDS_Shape wire = fix.Wire();
                                HeeksObj *new_sketch = heekscad_interface.NewSketch();
                                double deviation = 0.001;
                                if (!  ConvertWireToSketch(TopoDS::Wire(wire), new_sketch, deviation))
                                {
                                    wxMessageBox(_("Failed to convert wire to sketch"));
                                }
                                else
                                {
                                    if (sketch != NULL)
                                    {
                                        heekscad_interface.Remove( sketch );
                                        sketch = NULL;
                                    }

                                    heekscad_interface.Add(new_sketch, NULL);
                                }
                            }
                        }
                    } // End try
                    catch (Standard_Failure & error) {
                        (void) error;	// Avoid the compiler warning.
                        Handle_Standard_Failure e = Standard_Failure::Caught();
                        wxMessageBox(_("Failed to fix wire"));
                    } // End catch
                }
			}
		}
	}
	const wxChar* GetTitle(){return _T("Fix Wire");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Add To Part");}
};



static PadSketch pad_sketch;
static MakeToPart make_to_part;
static AddToPart add_to_part;
static PocketSketch pocket_sketch;
static FixWire fix_wire;


void GetSketchMenuTools(std::list<Tool*>* t_list){
	int count=0;
	bool gotsketch=false;
	bool gotpart=false;
	// check to see what types have been marked
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SketchType)
			gotsketch=true;
		if(object->GetType() == PartType)
			gotpart=true;
		count++;
	}

	if(count == 2 && gotsketch && gotpart)
		t_list->push_back(&add_to_part);

	if(count!=1 || !gotsketch)
		return;

	t_list->push_back(&pad_sketch);
	t_list->push_back(&pocket_sketch);
	t_list->push_back(&make_to_part);
	// t_list->push_back(&fix_wire);    /* This is not ready yet */
}

void on_set_sketchtool_option(bool value, HeeksObj* object){
	*((bool *) object) = value;
	sketch_tool_options.SaveSettings();
}

void on_set_sketchtool_option(double value, HeeksObj* object){
	*((double *) object) = value;
	sketch_tool_options.SaveSettings();
}

void SketchTools_GetOptions(std::list<Property *> *list)
{
    return; // This is not ready yet.

    PropertyList* sketchtools = new PropertyList(_("fix wire options"));
	sketchtools->m_list.push_back(new PropertyCheck(_("reorder"), sketch_tool_options.m_reorder, (HeeksObj *) (&sketch_tool_options.m_reorder), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("fix small"), sketch_tool_options.m_fix_small, (HeeksObj *) (&sketch_tool_options.m_fix_small), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyLength(_("fix small precision"), sketch_tool_options.m_fix_small_precision, (HeeksObj *) (&sketch_tool_options.m_fix_small_precision), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("fix self intersection"), sketch_tool_options.m_fix_self_intersection, (HeeksObj *) (&sketch_tool_options.m_fix_self_intersection), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("fix lacking"), sketch_tool_options.m_fix_lacking, (HeeksObj *) (&sketch_tool_options.m_fix_lacking), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("fix degenerated"), sketch_tool_options.m_fix_degenerated, (HeeksObj *) (&sketch_tool_options.m_fix_degenerated), on_set_sketchtool_option));

	sketchtools->m_list.push_back(new PropertyCheck(_("modify topology"), sketch_tool_options.m_modify_topology, (HeeksObj *) (&sketch_tool_options.m_modify_topology), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("modify geometry"), sketch_tool_options.m_modify_geometry, (HeeksObj *) (&sketch_tool_options.m_modify_geometry), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("closed wire"), sketch_tool_options.m_closed_wire, (HeeksObj *) (&sketch_tool_options.m_closed_wire), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("preference pcurve"), sketch_tool_options.m_preference_pcurve, (HeeksObj *) (&sketch_tool_options.m_preference_pcurve), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("fix gaps by ranges"), sketch_tool_options.m_fix_gaps_by_ranges, (HeeksObj *) (&sketch_tool_options.m_fix_gaps_by_ranges), on_set_sketchtool_option));

	list->push_back(sketchtools);
}
