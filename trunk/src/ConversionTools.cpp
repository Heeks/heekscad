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
#include "Solid.h"

void GetConversionMenuTools(std::list<Tool*>* t_list, const wxPoint* p, MarkedObject* marked_object){
	bool lines_or_arcs_in_marked_list = false;
	bool wire_in_marked_list = false;
	bool face_in_marked_list = false;

	// check to see what types have been marked
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case LineType:
			case ArcType:
				lines_or_arcs_in_marked_list = true;
				break;
			case WireType:
				wire_in_marked_list = true;
				break;
			case FaceType:
				face_in_marked_list = true;
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
}

void ConvertLineArcsToWire::Run(){
	std::list<TopoDS_Edge> edges;
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
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
					if(arc->m_dir){
						edges.push_back(BRepBuilderAPI_MakeEdge(arc->m_circle, arc->A, arc->B));
					}
					else{
						gp_Circ circle = arc->m_circle;
						circle.SetAxis( circle.Axis().Reversed());
						edges.push_back(BRepBuilderAPI_MakeEdge(circle, arc->A, arc->B));
					}
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

		wxGetApp().AddUndoably(new CWire(wire_maker.Wire(), "Wire"), NULL, NULL);
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

void ConvertLineArcsToFace::Run(){
	std::list<TopoDS_Edge> edges;
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
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
					if(arc->m_dir){
						edges.push_back(BRepBuilderAPI_MakeEdge(arc->m_circle, arc->A, arc->B));
					}
					else{
						gp_Circ circle = arc->m_circle;
						circle.SetAxis( circle.Axis().Reversed());
						edges.push_back(BRepBuilderAPI_MakeEdge(circle, arc->A, arc->B));
					}
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
