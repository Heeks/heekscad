// Shape.cpp
#include "stdafx.h"
#include "Solid.h"
#include "Wire.h"
#include "Face.h"
#include "Edge.h"
#include "Loop.h"
#include "HeeksFrame.h"
#include "MarkedList.h"
#include <BRepMesh.hxx>
#include <BRepTools.hxx>
#include "BRepBuilderAPI_Transform.hxx"
#include "BRepAlgoAPI_Cut.hxx"
#include "BRepAlgoAPI_Fuse.hxx"
#include "BRepAlgoAPI_Common.hxx"
#include "STEPControl_Reader.hxx"
#include "STEPControl_Writer.hxx"
#include "STEPControl_Controller.hxx"
#include "IGESControl_Reader.hxx"
#include "IGESControl_Writer.hxx"
#include "IGESControl_Controller.hxx"
#include "BRepOffsetAPI_Sewing.hxx"
#include "TopTools_MapOfShape.hxx"
#include "TopTools_MapIteratorOfMapOfShape.hxx"
#include "STLAPI.hxx"
#include "STLAPI_Reader.hxx"
#include "STLAPI_Writer.hxx"
#include <TopExp_Explorer.hxx>
#include "TopTools_ListOfShape.hxx"
#include "TopTools_ListIteratorOfListOfShape.hxx"
#include "BRepOffsetAPI_MakeOffsetShape.hxx"
#include "TopoDS_Edge.hxx"
#include "Interface_Static.hxx"
#include "../interface/Tool.h"

CShapeData::CShapeData()
{
	m_id = -1;
}

CShapeData::CShapeData(int id, const wchar_t* title)
{
	m_id = id;
	m_title.assign(title);
}

CShapeData::CShapeData(CShape* shape)
{
	m_id = shape->m_id;
	m_title = shape->m_title;
}

void CShapeData::SetShape(CShape* shape)
{
	if(m_id != -1)shape->SetID(m_id);
	if(m_title.length() > 0)shape->m_title = m_title;
}

// static member variable
bool CShape::m_solids_found = false;

CShape::CShape(const TopoDS_Shape &shape, const wxChar* title, bool use_one_gl_list):m_shape(shape), m_title(title), m_gl_list(0), m_use_one_gl_list(use_one_gl_list)
{
	m_faces = new CFaceList;
	m_edges = new CEdgeList;
	Add(m_faces, NULL);
	Add(m_edges, NULL);
	create_faces_and_edges();
}

CShape::CShape(const CShape& s):m_gl_list(0)
{
	m_faces = new CFaceList;
	m_edges = new CEdgeList;
	Add(m_faces, NULL);
	Add(m_edges, NULL);
	operator=(s);
}

CShape::~CShape()
{
	KillGLLists();
	delete_faces_and_edges();
}

const CShape& CShape::operator=(const CShape& s)
{
	// don't copy id
	delete_faces_and_edges();
	m_box = s.m_box;
	m_shape = s.m_shape;
	m_material = s.m_material;
	m_title = s.m_title;
	m_use_one_gl_list = s.m_use_one_gl_list;
	create_faces_and_edges();
	KillGLLists();

	return *this;
}

void CShape::KillGLLists()
{
	if (m_gl_list)
	{
		glDeleteLists(m_gl_list, 1);
		m_gl_list = 0;
		m_box = CBox();
	}
}

void CShape::create_faces_and_edges()
{
	// create the face objects
	TopTools_MapOfShape edgeMap;
	for (TopExp_Explorer expFace(m_shape, TopAbs_FACE); expFace.More(); expFace.Next())
	{
		TopoDS_Face F = TopoDS::Face(expFace.Current());
		CFace* new_object = new CFace(F);
		m_faces->Add(new_object, NULL);
		
		// create the edge objects from each face
		for (TopExp_Explorer expEdge(F, TopAbs_EDGE); expEdge.More(); expEdge.Next())
		{
			edgeMap.Add(expEdge.Current());
		}
	}

	int count = 0;
	TopTools_MapIteratorOfMapOfShape It(edgeMap);
	std::map<const TopoDS_Shape*, CEdge*> edge_finder;
	for (;It.More(); It.Next())
	{
		const TopoDS_Shape &E = It.Key();
		CEdge* new_object = new CEdge(TopoDS::Edge(E));
		m_edges->Add(new_object, NULL);
		edge_finder.insert( std::pair<const TopoDS_Shape*, CEdge*>(&E, new_object) );
		count++;
	}

	// make an edge map for each face
	std::map< CFace*, TopTools_MapOfShape*> face_edge_maps;
	for(HeeksObj* object = m_faces->GetFirstChild(); object; object = m_faces->GetNextChild())
	{
		CFace* face = (CFace*)object;
		TopTools_MapOfShape* newEdgeMap = new TopTools_MapOfShape;
		face_edge_maps.insert( std::make_pair(face, newEdgeMap) );

		for (TopExp_Explorer expEdge(face->Face(), TopAbs_EDGE); expEdge.More(); expEdge.Next())
		{
			const TopoDS_Shape &E = expEdge.Current();
			newEdgeMap->Add(E);
		}
	}

	// for each edge, find which faces it belongs to
	for(HeeksObj* object = m_edges->GetFirstChild(); object; object = m_edges->GetNextChild())
	{
		CEdge* edge = (CEdge*)object;

		const TopoDS_Shape &E = edge->Edge();

		// test each face
		for(std::map< CFace*, TopTools_MapOfShape*>::iterator It = face_edge_maps.begin(); It != face_edge_maps.end(); It++)
		{
			CFace* face = It->first;
			TopTools_MapOfShape *map = It->second;
			if(map->Contains(E)){
				face->m_edges.push_back(edge);
			}
		}
	}

	// create the face loops
	std::set<HeeksObj*> edges_to_delete;
	for(HeeksObj* object = m_faces->GetFirstChild(); object; object = m_faces->GetNextChild())
	{
		CFace* face = (CFace*)object;
		const TopoDS_Shape &F = face->Face();
		for (TopExp_Explorer expWire(F, TopAbs_WIRE); expWire.More(); expWire.Next())
		{
			const TopoDS_Shape &W = expWire.Current();
			std::list<CEdge*> edges;

			for (TopExp_Explorer expEdge(W, TopAbs_EDGE); expEdge.More(); expEdge.Next())
			{
				// look through the face's existing edges to find the CEdge*
				for(CEdge* edge = face->GetFirstEdge(); edge; edge = face->GetNextEdge())
				{
					const TopoDS_Shape &E = edge->Edge();
					if(E.IsSame(expEdge.Current())) {
						edges.push_back(edge);
						edge->m_faces.push_back(face);
						break;
					}
				}
			}

			std::list<CEdge*> edges_for_loop;
			for(std::list<CEdge*>::iterator It = edges.begin(); It != edges.end(); It++){
				CEdge* edge = *It;
				if(edge->m_faces.size() == 2 && edge->m_faces.front() == edge->m_faces.back()){
					if(edges_for_loop.size() > 0){
						CLoop* new_loop = new CLoop(face, edges_for_loop);
						face->m_loops.push_back(new_loop);
						edges_for_loop.clear();
					}
					edges_to_delete.insert(edge);
					face->m_edges.clear();
				}
				else{
					edges_for_loop.push_back(edge);
				}
			}
			if(edges_for_loop.size() > 0){
				CLoop* new_loop = new CLoop(face, edges_for_loop);
				face->m_loops.push_back(new_loop);
			}
		}
	}

	// delete edges
	m_edges->Clear(edges_to_delete);

	// calculate face senses
	for(HeeksObj* object = m_edges->GetFirstChild(); object; object = m_edges->GetNextChild())
	{
		CEdge* edge = (CEdge*)object;
		const TopoDS_Shape &E1 = edge->Edge();
		for(CFace* face = edge->GetFirstFace(); face; face = edge->GetNextFace())
		{
			bool sense = false;
			const TopoDS_Shape &F = face->Face();
			for (TopExp_Explorer expEdge(F, TopAbs_EDGE); expEdge.More(); expEdge.Next())
			{
				const TopoDS_Shape &E2 = expEdge.Current(); // this is the edge on the face
				if(E1.IsSame(E2))// same edge, but maybe different orientation
				{
					if(E1.IsEqual(E2))sense = true; // same orientation too
					break; // same edge ( ignore the face's other edges )
				}
			}
			edge->m_face_senses.push_back(sense); // one sense for each face, for each edge
		}
	}

	// delete the maps
	for(std::map< CFace*, TopTools_MapOfShape*>::iterator It = face_edge_maps.begin(); It != face_edge_maps.end(); It++)
	{
		CFace* face = It->first;
		TopTools_MapOfShape *map = It->second;
		delete map;
	}

#if _DEBUG
	// test InFaceSense
	for(HeeksObj* object = m_edges->GetFirstChild(); object; object = m_edges->GetNextChild())
	{
		CEdge* edge = (CEdge*)object;

		for(CFace* face = edge->GetFirstFace(); face; face = edge->GetNextFace())
		{
			bool in_face_sense = edge->InFaceSense(face);

			bool here = false;
			here = true;
		}
	}
#endif

}

void CShape::delete_faces_and_edges()
{
	m_faces->Clear();
	m_edges->Clear();
}

void CShape::glCommands(bool select, bool marked, bool no_color){
	m_material.glMaterial(1.0);
	if(m_gl_list)
	{
		glCallList(m_gl_list);
	}
	else{
		m_gl_list = glGenLists(1);
		glNewList(m_gl_list, GL_COMPILE_AND_EXECUTE);

		double pixels_per_mm = wxGetApp().GetPixelScale();

		BRepTools::Clean(m_shape);
		BRepMesh::Mesh(m_shape, 1/pixels_per_mm);

		// render all the faces
		m_faces->glCommands(true, marked, no_color);

		// render all the edges
		m_edges->glCommands(true, marked, no_color);

		glEndList();
	}
}

class OffsetShapeTool:public Tool{
	CShape* m_shape;
	static wxBitmap* m_bitmap;
public:
	OffsetShapeTool(CShape* shape):m_shape(shape){}

	// Tool's virtual functions
	void Run(){
		double offset_value = 2.0;
		wxGetApp().m_config->Read(_T("OffsetShapeValue"), &offset_value);
		if(wxGetApp().InputDouble(_T("Enter Offset Value, + for making bigger, - for making smaller"), _T("Offset value"), offset_value))
		{
			TopoDS_Shape new_shape = BRepOffsetAPI_MakeOffsetShape(m_shape->Shape(), offset_value, 0.01, BRepOffset_RectoVerso);
			HeeksObj* new_object = CShape::MakeObject(new_shape, _T("Result of 'Offset Shape'"));
			wxGetApp().AddUndoably(new_object, NULL, NULL);
			wxGetApp().DeleteUndoably(m_shape);
			wxGetApp().m_config->Write(_T("OffsetShapeValue"), offset_value);
		}
	}
	const wxChar* GetTitle(){ return _T("Offset Shape");}
	wxBitmap* Bitmap()
	{
		if(m_bitmap == NULL)
		{
			wxString exe_folder = wxGetApp().GetExeFolder();
			m_bitmap = new wxBitmap(exe_folder + _T("/bitmaps/offsetsolid.png"), wxBITMAP_TYPE_PNG);
		}
		return m_bitmap;
	}
	const wxChar* GetToolTip(){return _T("Offset the shape");}
};

wxBitmap* OffsetShapeTool::m_bitmap = NULL;

void CShape::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	t_list->push_back(new OffsetShapeTool(this));
}

void CShape::ModifyByMatrix(const double* m, bool for_undo){
	if(!for_undo){
		gp_Trsf mat = make_matrix(m);
		BRepBuilderAPI_Transform myBRepTransformation(m_shape,mat);
		TopoDS_Shape new_shape = myBRepTransformation.Shape();
		wxGetApp().AddUndoably(MakeObject(new_shape, m_title.c_str()), m_owner, NULL);
		wxGetApp().DeleteUndoably(this);
	}
}

void CShape::OnEditString(const wxChar* str){
	m_title.assign(str);
	wxGetApp().WasModified(this);
}

// static member function
HeeksObj* CShape::MakeObject(const TopoDS_Shape &shape, const wxChar* title, bool use_one_gl_list, bool stl_body){
	switch(shape.ShapeType()){
		case TopAbs_FACE:
			{
				return new CFace(TopoDS::Face(shape));
			}
		case TopAbs_WIRE:
			{
				return new CWire(TopoDS::Wire(shape), title);
			}
		case TopAbs_EDGE:
			return NULL;
		case TopAbs_VERTEX:
			return NULL;
		case TopAbs_COMPOUND:
		case TopAbs_COMPSOLID:
		case TopAbs_SOLID:
		case TopAbs_SHELL:
		case TopAbs_SHAPE:
			{
				return new CSolid(*((TopoDS_Solid*)(&shape)), title, use_one_gl_list);
			}
	}

	return NULL;
}

static HeeksObj* Cut(HeeksObj* s1, HeeksObj* s2){
	TopoDS_Shape sh1, sh2;
	TopoDS_Shape new_shape = BRepAlgoAPI_Cut(((CShape*)s1)->Shape(), ((CShape*)s2)->Shape());

	HeeksObj* new_object = CShape::MakeObject(new_shape, _T("Result of Cut Operation"));
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().DeleteUndoably(s1);
	wxGetApp().DeleteUndoably(s2);
	return new_object;
}

static HeeksObj* Fuse(HeeksObj* s1, HeeksObj* s2){
	TopoDS_Shape sh1, sh2;
	TopoDS_Shape new_shape = BRepAlgoAPI_Fuse(((CShape*)s1)->Shape(), ((CShape*)s2)->Shape());

	HeeksObj* new_object = CShape::MakeObject(new_shape, _T("Result of Fuse Operation"));
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().DeleteUndoably(s1);
	wxGetApp().DeleteUndoably(s2);
	return new_object;
}

static HeeksObj* Common(HeeksObj* s1, HeeksObj* s2){
	TopoDS_Shape sh1, sh2;
	TopoDS_Shape new_shape = BRepAlgoAPI_Common(((CShape*)s1)->Shape(), ((CShape*)s2)->Shape());

	HeeksObj* new_object = CShape::MakeObject(new_shape, _T("Result of Common Operation"));
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().DeleteUndoably(s1);
	wxGetApp().DeleteUndoably(s2);
	return new_object;
}

CFace* CShape::find(const TopoDS_Face &face)
{
	for(HeeksObj* object = m_faces->GetFirstChild(); object; object = m_faces->GetNextChild())
	{
		CFace* f = (CFace*)object;
		if(f->Face() == face)return f;
	}
	return NULL;
}

void CShape::CutShapes(const std::list<HeeksObj*> &list_in)
{
	// subtract from the first one in the list all the others
	wxGetApp().StartHistory(_T("Shape Cut"));
	HeeksObj* s1 = NULL;
	std::list<HeeksObj*> list = list_in;

	for(std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SolidType || object->GetType() == FaceType)
		{
			if(s1 == NULL)s1 = object;
			else{
				s1 = Cut(s1, object);
			}
		}
	}

	wxGetApp().EndHistory();
	wxGetApp().Repaint();
}

void CShape::FuseShapes(const std::list<HeeksObj*> &list_in)
{
	// fuse with the first one in the list all the others
	wxGetApp().StartHistory(_T("Shape Fuse"));
	HeeksObj* s1 = NULL;
	std::list<HeeksObj*> list = list_in;

	for(std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SolidType || object->GetType() == FaceType)
		{
			if(s1 == NULL)s1 = object;
			else{
				s1 = Fuse(s1, object);
			}
		}
	}

	wxGetApp().EndHistory();
	wxGetApp().Repaint();
}

void CShape::CommonShapes(const std::list<HeeksObj*> &list_in)
{
	// find common solid ( intersect ) with the first one in the list all the others
	wxGetApp().StartHistory(_T("Shape Common"));
	HeeksObj* s1 = NULL;
	std::list<HeeksObj*> list = list_in;

	for(std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SolidType || object->GetType() == FaceType)
		{
			if(s1 == NULL)s1 = object;
			else{
				s1 = Common(s1, object);
			}
		}
	}

	wxGetApp().EndHistory();
	wxGetApp().Repaint();
}

bool CShape::ImportSolidsFile(const wxChar* filepath, bool undoably, std::map<int, CShapeData> *index_map)
{
	// returns true, if suffix handled
	wxString wf(filepath);

	if(wf.EndsWith(_T(".stp")) || wf.EndsWith(_T(".STP")) || wf.EndsWith(_T(".step")) || wf.EndsWith(_T(".STEP")))
	{
		Standard_CString aFileName = (Standard_CString) (Ttc(filepath));
		STEPControl_Reader Reader;
		int status = Reader.ReadFile( aFileName );

		if ( status == IFSelect_RetDone )
		{
			int num = Reader.NbRootsForTransfer();
			for(int i = 1; i<=num; i++)
			{
				Handle_Standard_Transient root = Reader.RootForTransfer(i);
				Reader.TransferEntity(root);
				TopoDS_Shape rShape = Reader.Shape(i);      
				HeeksObj* new_object = MakeObject(rShape, _T("STEP solid"));
				if(undoably)wxGetApp().AddUndoably(new_object, NULL, NULL);
				else wxGetApp().Add(new_object, NULL);
				if(index_map)
				{
					// change the id, to the one in the step file index
					std::map<int, CShapeData>::iterator FindIt = index_map->find(i);
					if(FindIt != index_map->end())
					{
						CShapeData& shape_data = FindIt->second;
						shape_data.SetShape((CShape*)new_object);
					}
				}
			}
			wxGetApp().Repaint();
		}
		else{
			wxMessageBox(_T("STEP import not done!"));
		}
		return true;
	}
	else if(wf.EndsWith(_T(".igs")) || wf.EndsWith(_T(".IGS")) || wf.EndsWith(_T(".iges")) || wf.EndsWith(_T(".IGES")))
	{
		Standard_CString aFileName = (Standard_CString) (Ttc(filepath));
		IGESControl_Reader Reader;
		int status = Reader.ReadFile( aFileName );
	
		if ( status == IFSelect_RetDone )
		{
			Reader.TransferRoots();

			TopoDS_Shape one_shape = Reader.OneShape ();

			BRepOffsetAPI_Sewing face_sewer (0.001);
			TopExp_Explorer explorer (one_shape, TopAbs_FACE);
			while (explorer.More ())
			{
				face_sewer.Add (explorer.Current ());
				explorer.Next ();
			}
			face_sewer.Perform ();


			HeeksObj* new_object = MakeObject(face_sewer.SewedShape (), _T("sewed IGES solid"));
			if(undoably)wxGetApp().AddUndoably(new_object, NULL, NULL);
			else wxGetApp().Add(new_object, NULL);

			wxGetApp().Repaint();
		}
		else{
			wxMessageBox(_T("IGES import not done!"));
		}
		return true;
	}
	else if(wf.EndsWith(_T(".stl")) || wf.EndsWith(_T(".STL")))
	{
		Standard_CString aFileName = (Standard_CString) (Ttc(filepath));
		TopoDS_Shape rShape;
		StlAPI_Reader Reader;
		Reader.Read(rShape, aFileName);
		HeeksObj* new_object = MakeObject(rShape, _T("STL solid"), false, true);
		if(undoably)wxGetApp().AddUndoably(new_object, NULL, NULL);
		else wxGetApp().Add(new_object, NULL);
		wxGetApp().Repaint();
		return true;
	}
	return false;
}

bool CShape::ExportSolidsFile(const wxChar* filepath, std::map<int, CShapeData> *index_map)
{
	// returns true, if suffix handled
	wxString wf(filepath);
	wf.LowerCase();

	if(wf.EndsWith(_T(".stp")) || wf.EndsWith(_T(".step")))
	{
		Standard_CString aFileName = (Standard_CString) (Ttc(filepath));
		STEPControl_Writer writer;
		// add all the solids
		int i = 1;
		for(HeeksObj* object = wxGetApp().GetFirstChild(); object; object = wxGetApp().GetNextChild()){
			if(CShape::IsTypeAShape(object->GetType())){
				if(index_map)index_map->insert( std::pair<int, CShapeData>(i, CShapeData((CShape*)object)) );
				i++;
				writer.Transfer(((CSolid*)object)->Shape(), STEPControl_AsIs);
			}
		}
		writer.Write(aFileName);
		return true;
	}
	else if(wf.EndsWith(_T(".igs")) || wf.EndsWith(_T(".iges")))
	{
		Standard_CString aFileName = (Standard_CString) (Ttc(filepath));

		IGESControl_Controller::Init();
		IGESControl_Writer writer;

		// add all the solids
		for(HeeksObj* object = wxGetApp().GetFirstChild(); object; object = wxGetApp().GetNextChild()){
			if(CShape::IsTypeAShape(object->GetType())){
				writer.AddShape(((CShape*)object)->Shape());
			}
			else if(object->GetType() == WireType){
				writer.AddShape(((CWire*)object)->Shape());
			}
		}
		writer.Write(aFileName);
		return true;
	}
	else if(wf.EndsWith(_T(".stl")))
	{
		Standard_CString aFileName = (Standard_CString) (Ttc(filepath));

		{
			// clear file
			ofstream ofs(aFileName);
		}

		StlAPI_Writer writer;
		// add all the solids
		for(HeeksObj* object = wxGetApp().GetFirstChild(); object; object = wxGetApp().GetNextChild()){
			if(CShape::IsTypeAShape(object->GetType())){
				// append stl to file with default coefficient
				writer.Write(((CShape*)object)->Shape(), aFileName);
			}
		}
		return true;
	}
	return false;
}

void CShape::GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal){
	BRepTools::Clean(m_shape);
	BRepMesh::Mesh(m_shape, cusp);

	return ObjList::GetTriangles(callbackfunc, cusp, just_one_average_normal);
}

double CShape::Area()const{
	double area = 0.0;

	for(HeeksObj* object = m_faces->GetFirstChild(); object; object = m_faces->GetNextChild())
	{
		CFace* f = (CFace*)object;
		area += f->Area();
	}

	return area;
}

// static member function
bool CShape::IsTypeAShape(int t){
	switch(t){
		case SolidType:
		case WireType:
			return true;

		default:
			return false;
	}
}

void CShape::CopyFrom(const HeeksObj* object)
{
	*this = *((CShape*)object);
}

void CShape::WriteXML(TiXmlElement *root)
{
	CShape::m_solids_found = true;
}
