// RuledSurface.cpp

#include "stdafx.h"
#include "RuledSurface.h"
#include "Wire.h"
#include "ConversionTools.h"
#include "MarkedList.h"

void PickCreateRuledSurface()
{
	if(wxGetApp().m_marked_list->size() == 0)
	{
		wxGetApp().PickObjects("pick some wires or line-arcs");
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
			HeeksObj* new_object = CShape::MakeObject(shape, "Ruled Surface");
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