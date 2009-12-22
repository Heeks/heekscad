// RuledSurface.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "RuledSurface.h"
#include "Wire.h"
#include "Face.h"
#include "ConversionTools.h"
#include "MarkedList.h"
#include "HeeksConfig.h"
#include "../interface/DoubleInput.h"
#include "../interface/PropertyCheck.h"

void PickCreateRuledSurface()
{
	if(wxGetApp().m_marked_list->size() == 0)
	{
		wxGetApp().PickObjects(_("pick some sketches"));
	}

	if(wxGetApp().m_marked_list->size() > 0)
	{
		std::list<TopoDS_Wire> wire_list;

		std::list<HeeksObj*> sketches_to_delete;

		for(std::list<HeeksObj *>::const_iterator It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
		{
			HeeksObj* object = *It;
			if(object->GetType() == SketchType)
			{
				std::list<HeeksObj*> list;
				list.push_back(object);
				TopoDS_Wire wire;
				if(ConvertLineArcsToWire2(list, wire))
				{
					wire_list.push_back(wire);
					if(wxGetApp().m_loft_removes_sketches)sketches_to_delete.push_back(object);
				}
			}
		}

		wxGetApp().Remove(sketches_to_delete);

		TopoDS_Shape shape;
		if(CreateRuledSurface(wire_list, shape))
		{
			HeeksObj* new_object = CShape::MakeObject(shape, _("Ruled Surface"), SOLID_TYPE_UNKNOWN, HeeksColor(51, 45, 51));
			wxGetApp().Add(new_object, NULL);
			wxGetApp().Repaint();
		}

	}
}

HeeksObj* CreateExtrusionOrRevolution(std::list<HeeksObj*> list, double height_or_angle, bool solid_if_possible, bool revolution_not_extrusion)
{
	std::list<TopoDS_Shape> faces_or_wires;

	std::list<HeeksObj*> sketches_or_faces_to_delete;

	for(std::list<HeeksObj *>::const_iterator It = list.begin(); It != list.end(); It++)
	{
		HeeksObj* object = *It;
		switch(object->GetType())
		{
		case SketchType:
		case CircleType:
			{
				if(ConvertSketchToFaceOrWire(object, faces_or_wires, solid_if_possible))
				{
					if(wxGetApp().m_extrude_removes_sketches)sketches_or_faces_to_delete.push_back(object);
				}
			}
			break;

		case FaceType:
			faces_or_wires.push_back(((CFace*)object)->Face());
			if(wxGetApp().m_extrude_removes_sketches)sketches_or_faces_to_delete.push_back(object);
			break;

		default:
			break;
		}
	}

	wxGetApp().Remove(sketches_or_faces_to_delete);

	std::list<TopoDS_Shape> new_shapes;
	gp_Trsf trsf = wxGetApp().GetDrawMatrix(false);
	if(revolution_not_extrusion)
	{
		CreateRevolutions(faces_or_wires, new_shapes, gp_Ax1(gp_Pnt(0, 0, 0).Transformed(trsf), gp_Vec(0, 0, 1).Transformed(trsf)), height_or_angle);
	}
	else
	{
		CreateExtrusions(faces_or_wires, new_shapes, gp_Vec(0, 0, height_or_angle).Transformed(trsf));
	}
	HeeksObj* new_object = 0;
	if(new_shapes.size() > 0)
	{
		for(std::list<TopoDS_Shape>::iterator It = new_shapes.begin(); It != new_shapes.end(); It++){
			TopoDS_Shape& shape = *It;
			new_object = CShape::MakeObject(shape, revolution_not_extrusion ? _("Revolved Solid") : _("Extruded Solid"), SOLID_TYPE_UNKNOWN, wxGetApp().current_color);
			wxGetApp().Add(new_object, NULL);
		}
		wxGetApp().Repaint();
	}
	return new_object;
}

HeeksObj* CreatePipeFromProfile(HeeksObj* spine, HeeksObj* profile)
{
	const TopoDS_Wire wire = ((CWire*)spine)->Wire();
	std::list<TopoDS_Shape> faces;
	std::list<HeeksObj*> pipe_shapes;
	if(ConvertSketchToFaceOrWire(profile, faces, true))
	{
		for(std::list<TopoDS_Shape>::iterator It2 = faces.begin(); It2 != faces.end(); It2++)
		{
			TopoDS_Shape& face = *It2;

			try
			{
				// pipe profile algong spine
				BRepOffsetAPI_MakePipe makePipe(wire, face);
				makePipe.Build();
				TopoDS_Shape shape = makePipe.Shape(); 

				HeeksObj* new_object = CShape::MakeObject(shape, _("Pipe"), SOLID_TYPE_UNKNOWN, wxGetApp().current_color);
				if(new_object)pipe_shapes.push_back(new_object);
			}
			catch (Standard_Failure) {
				Handle_Standard_Failure e = Standard_Failure::Caught();
				wxMessageBox(wxString(_("Error making pipe")) + _T(": ") + Ctt(e->GetMessageString()));
			}
		}
		if(pipe_shapes.size() > 0)
		{
			wxGetApp().CreateUndoPoint();
			for(std::list<HeeksObj*>::iterator It = pipe_shapes.begin(); It != pipe_shapes.end(); It++)
			{
				HeeksObj* object = *It;
				wxGetApp().Add(object, NULL);
			}
			wxGetApp().Remove(profile);
			wxGetApp().Changed();
			return pipe_shapes.front();
		}
	}

	return NULL;
}

static void on_extrude_to_solid(bool onoff, HeeksObj* object)
{
	wxGetApp().m_extrude_to_solid = onoff;
	HeeksConfig config;
	config.Write(_T("ExtrudeToSolid"), wxGetApp().m_extrude_to_solid);
}

class CExtrusionInput:public CDoubleInput
{
public:
	CExtrusionInput(double &value):CDoubleInput(_("Input extrusion height"), _("height"), value){}

	// virtual functions for InputMode
	void GetProperties(std::list<Property *> *list)
	{
		CDoubleInput::GetProperties(list);
		list->push_back(new PropertyCheck(_("Extrude makes a solid"), wxGetApp().m_extrude_to_solid, NULL, on_extrude_to_solid));
	}
};

bool InputExtrusionHeight(double &value)
{
	CInputMode* save_mode = wxGetApp().input_mode_object;
	CExtrusionInput extrusion_input(value);
	wxGetApp().SetInputMode(&extrusion_input);

	wxGetApp().OnRun();

	wxGetApp().SetInputMode(save_mode);

	if(CDoubleInput::m_success)value = extrusion_input.m_value;

	return CDoubleInput::m_success;
}

void PickCreateExtrusion()
{
	if(wxGetApp().m_marked_list->size() == 0)
	{
		wxGetApp().PickObjects(_("pick sketches, faces or circles"), MARKING_FILTER_CIRCLE | MARKING_FILTER_SKETCH | MARKING_FILTER_FACE);
	}

	double height = 10; // to do, this should get written to config file


	if(InputExtrusionHeight(height))
	{
		if(wxGetApp().m_marked_list->size() > 0)
		{
			CreateExtrusionOrRevolution(wxGetApp().m_marked_list->list(),height, wxGetApp().m_extrude_to_solid, false);
		}
	}
}

static void on_revolve_angle(double value, HeeksObj* object)
{
	wxGetApp().m_revolve_angle = value;
	HeeksConfig config;
	config.Write(_T("RevolveAngle"), wxGetApp().m_revolve_angle);
}

class CRevolutionInput:public CDoubleInput
{
public:
	CRevolutionInput(double &value):CDoubleInput(_("Input revolution angle"), _("angle"), value){}

	// virtual functions for InputMode
	void GetProperties(std::list<Property *> *list)
	{
		CDoubleInput::GetProperties(list);
		list->push_back(new PropertyCheck(_("Extrude makes a solid"), wxGetApp().m_extrude_to_solid, NULL, on_extrude_to_solid));
	}
};

void PickCreateRevolution()
{
	if(wxGetApp().m_marked_list->size() == 0)
	{
		wxGetApp().PickObjects(_("pick sketches, faces or circles"), MARKING_FILTER_CIRCLE | MARKING_FILTER_SKETCH | MARKING_FILTER_FACE);
	}

	double angle = 360.0; // to do, this should get written to config file

	if(wxGetApp().InputDouble(_("Input revolution angle"), _("angle"), angle))
	{
		if(wxGetApp().m_marked_list->size() > 0)
		{
			CreateExtrusionOrRevolution(wxGetApp().m_marked_list->list(), angle, true, true);
		}
	}
}

bool CreateRuledSurface(const std::list<TopoDS_Wire> &wire_list, TopoDS_Shape& shape)
{
	if(wire_list.size() > 0)
	{
			BRepOffsetAPI_ThruSections generator( Standard_True, Standard_False );
			for(std::list<TopoDS_Wire>::const_iterator It = wire_list.begin(); It != wire_list.end(); It++)
			{
				const TopoDS_Wire &wire = *It;
				generator.AddWire(wire);
			}

		try{
			generator.Build();
			shape = generator.Shape();
		}
		catch (Standard_Failure) {
			Handle_Standard_Failure e = Standard_Failure::Caught();
			wxMessageBox(wxString(_("Error making ruled solid")) + _T(": ") + Ctt(e->GetMessageString()));
			return false;
		}
		catch(...)
		{
			wxMessageBox(_("Fatal error making ruled solid"));
			return false;
		}
			
		return true;
	}
	return false;
}

void CreateExtrusions(const std::list<TopoDS_Shape> &faces_or_wires, std::list<TopoDS_Shape>& new_shapes, const gp_Vec& extrude_vector)
{
	try{
		for(std::list<TopoDS_Shape>::const_iterator It = faces_or_wires.begin(); It != faces_or_wires.end(); It++)
		{
			const TopoDS_Shape& face_or_wire = *It;
			BRepPrimAPI_MakePrism generator( face_or_wire, extrude_vector );
			generator.Build();
			new_shapes.push_back(generator.Shape());
		}
	}
	catch (Standard_Failure) {
		Handle_Standard_Failure e = Standard_Failure::Caught();
		wxMessageBox(wxString(_("Error making extruded solid")) + _T(": ") + Ctt(e->GetMessageString()));
	}
	catch(...)
	{
		wxMessageBox(_("Fatal error making extruded solid"));
	}
	
}

void CreateRevolutions(const std::list<TopoDS_Shape> &faces_or_wires, std::list<TopoDS_Shape>& new_shapes, const gp_Ax1& axis, double angle)
{
	try{
		for(std::list<TopoDS_Shape>::const_iterator It = faces_or_wires.begin(); It != faces_or_wires.end(); It++)
		{
			const TopoDS_Shape& face_or_wire = *It;
			if(fabs(angle - 360.0) < 0.00001)
			{
				BRepPrimAPI_MakeRevol generator( face_or_wire, axis );
				generator.Build();
				new_shapes.push_back(generator.Shape());
			}
			else
			{
				BRepPrimAPI_MakeRevol generator( face_or_wire, axis, angle * Pi/180 );
				generator.Build();
				new_shapes.push_back(generator.Shape());
			}
		}
	}
	catch (Standard_Failure) {
		Handle_Standard_Failure e = Standard_Failure::Caught();
		wxMessageBox(wxString(_("Error making revolved solid")) + _T(": ") + Ctt(e->GetMessageString()));
	}
	catch(...)
	{
		wxMessageBox(_("Fatal error making revolved solid"));
	}
	
}

