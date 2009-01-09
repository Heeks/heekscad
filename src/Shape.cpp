// Shape.cpp
#include "stdafx.h"
#include "Shape.h"
#include "Solid.h"
#include "Wire.h"
#include "Face.h"
#include "Edge.h"
#include "Loop.h"
#include "Cylinder.h"
#include "Cuboid.h"
#include "Sphere.h"
#include "Cone.h"
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
#include <TopExp_Explorer.hxx>
#include <BRepTools_WireExplorer.hxx>
#include "TopTools_ListOfShape.hxx"
#include "TopTools_ListIteratorOfListOfShape.hxx"
#include "BRepOffsetAPI_MakeOffsetShape.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS.hxx"
#include "Interface_Static.hxx"
#include "../interface/Tool.h"
#include "../tinyxml/tinyxml.h"
#include "HeeksConfig.h"
#include "../interface/strconv.h"
#include "HeeksCAD.h"

// static member variable
bool CShape::m_solids_found = false;

CShape::CShape(const TopoDS_Shape &shape, const wxChar* title, const HeeksColor& col):m_gl_list(0), m_shape(shape), m_title(title), m_color(col)
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
	m_title = s.m_title;
	m_color = s.m_color;
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

		TopoDS_Wire outerWire=BRepTools::OuterWire(TopoDS::Face(F));

		for (TopExp_Explorer expWire(F, TopAbs_WIRE); expWire.More(); expWire.Next())
		{
			const TopoDS_Shape &W = expWire.Current();
			bool is_outer = W.IsSame(outerWire) != 0;
			std::list<CEdge*> edges;

			TopAbs_Orientation wo = W.Orientation();
			bool bwo = (wo == TopAbs_FORWARD);
			TopAbs_Orientation fwo = F.Orientation();
			bool bfwo = (fwo == TopAbs_FORWARD);
			bool ooooo = (bwo == bfwo);

			for(BRepTools_WireExplorer expEdge(TopoDS::Wire(W)); expEdge.More(); expEdge.Next())
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
						CLoop* new_loop = new CLoop(face, ooooo, edges_for_loop, is_outer);
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
				CLoop* new_loop = new CLoop(face, ooooo, edges_for_loop, is_outer);
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

void CShape::glCommands(bool select, bool marked, bool no_color)
{
	Material(m_color).glMaterial(1.0);

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
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
		m_faces->glCommands(true, marked, no_color);
		glDisable(GL_LIGHTING);
		glShadeModel(GL_FLAT);

		// render all the edges
		m_edges->glCommands(true, marked, no_color);

		glEndList();
	}
}

void CShape::GetBox(CBox &box)
{
	if(!m_box.m_valid)
	{
		BRepTools::Clean(m_shape);
		BRepMesh::Mesh(m_shape, 1.0);
		m_faces->GetBox(m_box);
	}

	box.Insert(m_box);
}

class OffsetShapeTool:public Tool{
public:
	CShape* m_shape;
	OffsetShapeTool(CShape* shape):m_shape(shape){}

	// Tool's virtual functions
	void Run(){
		double offset_value = 2.0;
		HeeksConfig config;
		config.Read(_T("OffsetShapeValue"), &offset_value);
		if(wxGetApp().InputDouble(_("Enter Offset Value, + for making bigger, - for making smaller"), _("Offset value"), offset_value))
		{
			TopoDS_Shape new_shape = BRepOffsetAPI_MakeOffsetShape(m_shape->Shape(), offset_value, 0.01, BRepOffset_RectoVerso);
			HeeksObj* new_object = CShape::MakeObject(new_shape, _("Result of 'Offset Shape'"), SOLID_TYPE_UNKNOWN, m_shape->m_color);
			wxGetApp().AddUndoably(new_object, NULL, NULL);
			wxGetApp().DeleteUndoably(m_shape);
			config.Write(_T("OffsetShapeValue"), offset_value);
		}
	}
	const wxChar* GetTitle(){ return _("Offset Shape");}
	wxString BitmapPath(){return _T("offsetsolid");}
	const wxChar* GetToolTip(){return _("Offset the shape");}
};

static OffsetShapeTool offset_shape_tool(NULL);

void CShape::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	offset_shape_tool.m_shape = this;
	t_list->push_back(&offset_shape_tool);
}

bool CShape::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	BRepBuilderAPI_Transform myBRepTransformation(m_shape,mat);
	TopoDS_Shape new_shape = myBRepTransformation.Shape();
	wxGetApp().AddUndoably(MakeObject(new_shape, m_title.c_str(), SOLID_TYPE_UNKNOWN, m_color), m_owner, NULL);
	wxGetApp().DeleteUndoably(this);
	return true;
}

void CShape::OnEditString(const wxChar* str){
	m_title.assign(str);
	wxGetApp().WasModified(this);
}

// static member function
HeeksObj* CShape::MakeObject(const TopoDS_Shape &shape, const wxChar* title, SolidTypeEnum solid_type, const HeeksColor& col){
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
				switch(solid_type)
				{
				case SOLID_TYPE_SPHERE:
					return new CSphere(*((TopoDS_Solid*)(&shape)), title, col);
				case SOLID_TYPE_CYLINDER:
					return new CCylinder(*((TopoDS_Solid*)(&shape)), title, col);
				case SOLID_TYPE_CUBOID:
					return new CCuboid(*((TopoDS_Solid*)(&shape)), title, col);
				case SOLID_TYPE_CONE:
					return new CCone(*((TopoDS_Solid*)(&shape)), title, col);
				default:
					return new CSolid(*((TopoDS_Solid*)(&shape)), title, col);
				}
			}
	}

	return NULL;
}

static bool Cut(const std::list<TopoDS_Shape> &shapes, TopoDS_Shape& new_shape){
	if(shapes.size() < 2)return false;

	std::list<TopoDS_Shape>::const_iterator It = shapes.begin();
	TopoDS_Shape current_shape = *It;
	It++;
	while(It != shapes.end())
	{
		const TopoDS_Shape &cutting_shape = *It;
		current_shape = BRepAlgoAPI_Cut(current_shape, cutting_shape);
		It++;
	}

	new_shape = current_shape;
	return true;
}

static HeeksObj* Fuse(HeeksObj* s1, HeeksObj* s2){
	TopoDS_Shape sh1, sh2;
	TopoDS_Shape new_shape = BRepAlgoAPI_Fuse(((CShape*)s1)->Shape(), ((CShape*)s2)->Shape());

	HeeksObj* new_object = CShape::MakeObject(new_shape, _("Result of Fuse Operation"), SOLID_TYPE_UNKNOWN, ((CShape*)s1)->m_color);
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().DeleteUndoably(s1);
	wxGetApp().DeleteUndoably(s2);
	return new_object;
}

static HeeksObj* Common(HeeksObj* s1, HeeksObj* s2){
	TopoDS_Shape sh1, sh2;
	TopoDS_Shape new_shape = BRepAlgoAPI_Common(((CShape*)s1)->Shape(), ((CShape*)s2)->Shape());

	HeeksObj* new_object = CShape::MakeObject(new_shape, _("Result of Common Operation"), SOLID_TYPE_UNKNOWN, ((CShape*)s1)->m_color);
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
	std::list<TopoDS_Shape> shapes;
	std::list<HeeksObj*> delete_list;
	HeeksObj* first_solid = NULL;

	for(std::list<HeeksObj*>::const_iterator It = list_in.begin(); It != list_in.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SolidType)
		{
			shapes.push_back(((CSolid*)object)->Shape());
			if(first_solid == NULL)first_solid = object;
			delete_list.push_back(object);
		}
	}

	TopoDS_Shape new_shape;
	if(Cut(shapes, new_shape))
	{
		wxGetApp().StartHistory();
		HeeksObj* new_object = CShape::MakeObject(new_shape, _("Result of Cut Operation"), SOLID_TYPE_UNKNOWN, ((CShape*)first_solid)->m_color);
		wxGetApp().AddUndoably(new_object, NULL, NULL);
		wxGetApp().DeleteUndoably(delete_list);
		wxGetApp().EndHistory();
		wxGetApp().Repaint();
	}
}

void CShape::FuseShapes(const std::list<HeeksObj*> &list_in)
{
	// fuse with the first one in the list all the others
	wxGetApp().StartHistory();
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
	wxGetApp().StartHistory();
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
				if(index_map)
				{
					// change the id ( and any other data ), to the one in the step file index
					std::map<int, CShapeData>::iterator FindIt = index_map->find(i);
					if(FindIt != index_map->end())
					{
						CShapeData& shape_data = FindIt->second;
						HeeksObj* new_object = MakeObject(rShape, _("STEP solid"), shape_data.m_solid_type, HeeksColor(191, 191, 191));
						if(undoably)wxGetApp().AddUndoably(new_object, NULL, NULL);
						else wxGetApp().Add(new_object, NULL);
						shape_data.SetShape((CShape*)new_object);
					}
				}
				else
				{
					HeeksObj* new_object = MakeObject(rShape, _("STEP solid"), SOLID_TYPE_UNKNOWN, HeeksColor(191, 191, 191));
					if(undoably)wxGetApp().AddUndoably(new_object, NULL, NULL);
					else wxGetApp().Add(new_object, NULL);
				}
			}
			wxGetApp().Repaint();
		}
		else{
			wxMessageBox(_("STEP import not done!"));
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


			HeeksObj* new_object = MakeObject(face_sewer.SewedShape (), _("sewed IGES solid"), SOLID_TYPE_UNKNOWN, HeeksColor(191, 191, 191));
			if(undoably)wxGetApp().AddUndoably(new_object, NULL, NULL);
			else wxGetApp().Add(new_object, NULL);

			wxGetApp().Repaint();
		}
		else{
			wxMessageBox(_("IGES import not done!"));
		}
		return true;
	}
	return false;
}

bool CShape::ExportSolidsFile(const std::list<HeeksObj*>& objects, const wxChar* filepath, std::map<int, CShapeData> *index_map)
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
		for(std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
		{
			HeeksObj* object = *It;
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
		for(std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
		{
			HeeksObj* object = *It;
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
