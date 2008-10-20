// ConversionTools.cpp
#include "stdafx.h"
#include "ConversionTools.h"
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeShape.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include "MarkedList.h"
#include "HLine.h"
#include "HArc.h"
#include "Wire.h"
#include "Face.h"
#include "Edge.h"
#include "Shape.h"

void GetConversionMenuTools(std::list<Tool*>* t_list){
	bool lines_or_arcs_in_marked_list = false;
	bool wire_in_marked_list = false;
	bool face_in_marked_list = false;
	bool edge_in_marked_list = false;

	// check to see what types have been marked
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case LineType:
			case ArcType:
			case LineArcCollectionType:
				lines_or_arcs_in_marked_list = true;
				break;
			case WireType:
				wire_in_marked_list = true;
				break;
			case FaceType:
				face_in_marked_list = true;
				break;
			case EdgeType:
				edge_in_marked_list = true;
				break;
		}
	}

	if(lines_or_arcs_in_marked_list){
		t_list->push_back(new ConvertLineArcsToWire);
		t_list->push_back(new ConvertLineArcsToFace);
	}
	if(wire_in_marked_list){
		t_list->push_back(new ConvertWireToFace);
	}
	if(edge_in_marked_list){
		t_list->push_back(new ConvertEdgesToWire);
	}
}

bool ConvertLineArcsToWire2(const std::list<HeeksObj *> &list, TopoDS_Wire &wire)
{
	std::list<TopoDS_Edge> edges;
	std::list<HeeksObj*> list2;
	std::list<HeeksObj*>::const_iterator It;
	for(It = list.begin(); It != list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == LineArcCollectionType){
			for(HeeksObj* child = object->GetFirstChild(); child; child = object->GetNextChild())
			{
				list2.push_back(child);
			}
		}
		else{
			list2.push_back(object);
		}
	}

	for(std::list<HeeksObj*>::iterator It = list2.begin(); It != list2.end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case LineType:
				{
					HLine* line = (HLine*)object;
					edges.push_back(BRepBuilderAPI_MakeEdge(line->A, line->B));
				}
				break;
			case ArcType:
				{
					HArc* arc = (HArc*)object;
					edges.push_back(BRepBuilderAPI_MakeEdge(arc->m_circle, arc->A, arc->B));
				}
				break;
		}
	}

	if(edges.size() > 0){
		BRepBuilderAPI_MakeWire wire_maker;
		std::list<TopoDS_Edge>::iterator It;
		for(It = edges.begin(); It != edges.end(); It++)
		{
			TopoDS_Edge &edge = *It;
			wire_maker.Add(edge);
		}

		wire = wire_maker.Wire();
		return true;
	}

	return false;
}

bool ConvertLineArcsToFace2(const std::list<HeeksObj *> &list, TopoDS_Face& face)
{
	std::list<HeeksObj*> list2;
	std::list<HeeksObj*>::const_iterator It;
	for(It = list.begin(); It != list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == LineArcCollectionType){
			for(HeeksObj* child = object->GetFirstChild(); child; child = object->GetNextChild())
			{
				list2.push_back(child);
			}
		}
		else{
			list2.push_back(object);
		}
	}

	std::list<TopoDS_Edge> edges;
	for(std::list<HeeksObj*>::const_iterator It = list2.begin(); It != list2.end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case LineType:
				{
					HLine* line = (HLine*)object;
					edges.push_back(BRepBuilderAPI_MakeEdge(line->A, line->B));
				}
				break;
			case ArcType:
				{
					HArc* arc = (HArc*)object;
					edges.push_back(BRepBuilderAPI_MakeEdge(arc->m_circle, arc->A, arc->B));
				}
				break;
		}
	}

	if(edges.size() > 0){
		BRepBuilderAPI_MakeWire wire_maker;
		std::list<TopoDS_Edge>::iterator It;
		for(It = edges.begin(); It != edges.end(); It++)
		{
			TopoDS_Edge &edge = *It;
			wire_maker.Add(edge);
		}

		face = BRepBuilderAPI_MakeFace(wire_maker.Wire());
		return true;
	}

	return false;
}

bool ConvertWireToFace2(const std::list<TopoDS_Wire> &list, std::list<TopoDS_Face>& faces)
{
	for(std::list<TopoDS_Wire>::const_iterator It = list.begin(); It != list.end(); It++)
	{
		const TopoDS_Wire &wire = *It;
		faces.push_back(BRepBuilderAPI_MakeFace(wire));
	}

	return true;
}

wxBitmap* ConvertLineArcsToWire::m_bitmap = NULL;
wxBitmap* ConvertWireToFace::m_bitmap = NULL;
wxBitmap* ConvertEdgesToWire::m_bitmap = NULL;
wxBitmap* ConvertLineArcsToFace::m_bitmap = NULL;

void ConvertLineArcsToWire::Run(){
	TopoDS_Wire wire;
	if(ConvertLineArcsToWire2(wxGetApp().m_marked_list->list(), wire))
	{
		wxGetApp().AddUndoably(new CWire(wire, _T("Wire")), NULL, NULL);
		wxGetApp().Repaint();
	}
}

void ConvertWireToFace::Run(){
	std::list<TopoDS_Wire> wires;
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case WireType:
				{
					wires.push_back(((CWire*)object)->Wire());
				}
				break;
		}
	}

	std::list<TopoDS_Face> faces;

	ConvertWireToFace2(wires, faces);

	if(faces.size() > 0){
		for(std::list<TopoDS_Face>::iterator It = faces.begin(); It != faces.end(); It++)
		{
			TopoDS_Face &face = *It;
			wxGetApp().AddUndoably(new CFace(face), NULL, NULL);
			wxGetApp().Repaint();
		}
	}
}


void ConvertEdgesToWire::Run(){
	std::list<HeeksObj*> edges;
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case EdgeType:
				{
					edges.push_back(object);
				}
				break;
		}
	}

	if(edges.size() > 0){
		BRepBuilderAPI_MakeWire wire_maker;
		std::list<HeeksObj*>::iterator It;
		for(It = edges.begin(); It != edges.end(); It++)
		{
			HeeksObj* object = *It;
			wire_maker.Add(TopoDS::Edge(((CEdge*)object)->Edge()));
		}

		wxGetApp().AddUndoably(new CWire(wire_maker.Wire(), _T("Wire")), NULL, NULL);
		wxGetApp().Repaint();
	}
}
void ConvertLineArcsToFace::Run(){
	std::list<TopoDS_Edge> edges;
	std::list<HeeksObj*> list2;
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == LineArcCollectionType){
			for(HeeksObj* child = object->GetFirstChild(); child; child = object->GetNextChild())
			{
				list2.push_back(child);
			}
		}
		else{
			list2.push_back(object);
		}
	}

	TopoDS_Face face;

	if(ConvertLineArcsToFace2(list2, face))
	{
		wxGetApp().AddUndoably(new CFace(face), NULL, NULL);
		wxGetApp().Repaint();
	}

}
