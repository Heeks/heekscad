// RuledSurface.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "RuledSurface.h"
#include "Wire.h"
#include "Face.h"
#include "ConversionTools.h"
#include "MarkedList.h"
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>

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

		wxGetApp().StartHistory();
		wxGetApp().DeleteUndoably(sketches_to_delete);

		TopoDS_Shape shape;
		if(CreateRuledSurface(wire_list, shape))
		{
			HeeksObj* new_object = CShape::MakeObject(shape, _("Ruled Surface"), SOLID_TYPE_UNKNOWN, HeeksColor(51, 45, 51));
			wxGetApp().AddUndoably(new_object, NULL, NULL);
			wxGetApp().Repaint();
		}

		wxGetApp().EndHistory();
	}
}

HeeksObj* CreateExtrusion(std::list<HeeksObj*> list, double height)
{
	std::list<TopoDS_Face> faces;

	std::list<HeeksObj*> sketches_to_delete;

	for(std::list<HeeksObj *>::const_iterator It = list.begin(); It != list.end(); It++)
	{
		HeeksObj* object = *It;
		switch(object->GetType())
		{
		case SketchType:
			TopoDS_Face face;
			if(ConvertSketchToFace2(object, face))
			{
				faces.push_back(face);
				if(wxGetApp().m_extrude_removes_sketches)sketches_to_delete.push_back(object);
			}
		}
	}

	wxGetApp().StartHistory();

	wxGetApp().DeleteUndoably(sketches_to_delete);

	std::list<TopoDS_Shape> new_shapes;
	CreateExtrusions(faces, new_shapes, gp_Vec(0, 0, height).Transformed(wxGetApp().GetDrawMatrix(false)));
	HeeksObj* new_object = 0;
	if(new_shapes.size() > 0)
	{
		for(std::list<TopoDS_Shape>::iterator It = new_shapes.begin(); It != new_shapes.end(); It++){
			TopoDS_Shape& shape = *It;
			new_object = CShape::MakeObject(shape, _("Extruded Solid"), SOLID_TYPE_UNKNOWN, wxGetApp().current_color);
			wxGetApp().AddUndoably(new_object, NULL, NULL);
		}
		wxGetApp().Repaint();
	}

	wxGetApp().EndHistory();
	return new_object;
}

HeeksObj* CreatePipeFromProfile(HeeksObj* spine, HeeksObj* profile)
{
	std::list<TopoDS_Face> faces;

	TopoDS_Face face;
	ConvertSketchToFace2(profile, face);
	
	wxGetApp().StartHistory();

	wxGetApp().DeleteUndoably(profile);

	const TopoDS_Wire wire = ((CWire*)spine)->Wire();

	// pipe profile algong spine
	BRepOffsetAPI_MakePipe makePipe(wire, face);
	makePipe.Build();
	TopoDS_Shape shape = makePipe.Shape(); 

	HeeksObj* new_object = CShape::MakeObject(shape, _("Pipe"), SOLID_TYPE_UNKNOWN, wxGetApp().current_color);
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().Repaint();

	wxGetApp().EndHistory();
	return new_object;
}

void PickCreateExtrusion()
{
	if(wxGetApp().m_marked_list->size() == 0)
	{
		wxGetApp().PickObjects(_("pick sketches"));
	}

	double height = 10;
	wxGetApp().InputDouble(_("Input extrusion height"), _("height"), height);

	if(wxGetApp().m_marked_list->size() > 0)
	{
		CreateExtrusion(wxGetApp().m_marked_list->list(),height);
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
		catch(...)
		{
			wxMessageBox(_("Fatal error making ruled solid"));
			return false;
		}
			
		return true;
	}
	return false;
}

void CreateExtrusions(const std::list<TopoDS_Face> &faces, std::list<TopoDS_Shape>& new_shapes, const gp_Vec& extrude_vector)
{
	try{
		for(std::list<TopoDS_Face>::const_iterator It = faces.begin(); It != faces.end(); It++)
		{
			const TopoDS_Face& face = *It;
			BRepPrimAPI_MakePrism generator( face, extrude_vector );
			generator.Build();
			new_shapes.push_back(generator.Shape());
		}
	}
	catch(...)
	{
		wxMessageBox(_("Fatal error making extruded solid"));
	}
	
}

