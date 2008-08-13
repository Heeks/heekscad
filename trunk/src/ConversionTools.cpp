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
#include "Solid.h"

void GetConversionMenuTools(std::list<Tool*>* t_list, const wxPoint* p, MarkedObject* marked_object){
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

void ConvertLineArcsToWire::Run(){
	TopoDS_Wire wire;
	if(ConvertLineArcsToWire2(wxGetApp().m_marked_list->list(), wire))
	{
		wxGetApp().AddUndoably(new CWire(wire, "Wire"), NULL, NULL);
		wxGetApp().Repaint();
	}
}

void ConvertWireToFace::Run(){
	std::list<HeeksObj*> faces;
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case WireType:
				{
					faces.push_back(new CFace(BRepBuilderAPI_MakeFace(((CWire*)object)->Wire())));
				}
				break;
		}
	}

	if(faces.size() > 0){
		wxGetApp().AddUndoably(faces, NULL);
		wxGetApp().Repaint();
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

		wxGetApp().AddUndoably(new CWire(wire_maker.Wire(), "Wire"), NULL, NULL);
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

		wxGetApp().AddUndoably(new CFace(BRepBuilderAPI_MakeFace(wire_maker.Wire())), NULL, NULL);
		wxGetApp().Repaint();
	}
}
