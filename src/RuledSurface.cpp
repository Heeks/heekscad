// RuledSurface.cpp

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
		wxGetApp().PickObjects(_T("pick some wires or line-arcs"));
	}

	if(wxGetApp().m_marked_list->size() > 0)
	{
		std::list<TopoDS_Wire> wire_list;

		for(std::list<HeeksObj *>::const_iterator It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
		{
			HeeksObj* object = *It;
			if(object->GetType() == WireType)
			{
				wire_list.push_back(((CWire*)object)->Wire());
			}
			else if(object->GetType() == LineArcCollectionType)
			{
				std::list<HeeksObj*> list;
				list.push_back(object);
				TopoDS_Wire wire;
				if(ConvertLineArcsToWire2(list, wire))
				{
					wire_list.push_back(wire);
				}
			}
		}

		TopoDS_Shape shape;
		if(CreateRuledSurface(wire_list, shape))
		{
			HeeksObj* new_object = CShape::MakeObject(shape, _T("Ruled Surface"), SOLID_TYPE_UNKNOWN, HeeksColor(51, 45, 51));
			wxGetApp().AddUndoably(new_object, NULL, NULL);
			wxGetApp().Repaint();
		}
	}
}

void PickCreateExtrusion()
{
	if(wxGetApp().m_marked_list->size() == 0)
	{
		wxGetApp().PickObjects(_T("pick a wire, face or line/arc collection"));
	}

	double height = 10;
	wxGetApp().InputDouble(_T("Input extrusion height"), _T("height"), height);

	if(wxGetApp().m_marked_list->size() > 0)
	{
		std::list<TopoDS_Face> faces;
		std::list<HeeksObj*> lines_arcs;

		for(std::list<HeeksObj *>::const_iterator It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
		{
			HeeksObj* object = *It;
			switch(object->GetType())
			{
			case WireType:
				{
					std::list<TopoDS_Wire> wires;
					wires.push_back(((CWire*)object)->Wire());
					ConvertWireToFace2(wires, faces);
				}
				break;

			case FaceType:
				faces.push_back(((CFace*)object)->Face());
				break;

			case LineArcCollectionType:
			case LineType:
			case ArcType:
				lines_arcs.push_back(object);
				break;
			}
		}

		{
			TopoDS_Face face;
			if(ConvertLineArcsToFace2(lines_arcs, face))
			{
				faces.push_back(face);
			}
		}

		TopoDS_Shape shape;
		if(CreateExtrusion(faces, shape, gp_Vec(0, 0, height).Transformed(wxGetApp().GetDrawMatrix(false))))
		{
			HeeksObj* new_object = CShape::MakeObject(shape, _T("Extruded Solid"), SOLID_TYPE_UNKNOWN, HeeksColor(89, 95, 89));
			wxGetApp().AddUndoably(new_object, NULL, NULL);
			wxGetApp().Repaint();
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

		generator.Build();
		shape = generator.Shape();

		return true;
	}
	return false;
}

bool CreateExtrusion(const std::list<TopoDS_Face> &faces, TopoDS_Shape& shape, gp_Vec& extrude_vector)
{
	if(faces.size() > 0)
	{
		BRepPrimAPI_MakePrism generator( faces.front(), extrude_vector );
		generator.Build();
		shape = generator.Shape();

		return true;
	}
	return false;
}