// Shape.cpp
#include "stdafx.h"
#include "Shape.h"
#include "Solid.h"
#include "Wire.h"
#include "Face.h"
#include "Edge.h"
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
#include "STLAPI.hxx"
#include "STLAPI_Reader.hxx"
#include "STLAPI_Writer.hxx"
#include "TopTools_ListOfShape.hxx"
#include "TopTools_ListIteratorOfListOfShape.hxx"
#include "BRepOffsetAPI_MakeOffsetShape.hxx"
#include "../interface/Tool.h"

CShape::CShape(const TopoDS_Shape &shape, const char* title, bool use_one_gl_list):m_shape(shape), m_title(title), m_gl_list(0), m_use_one_gl_list(use_one_gl_list)
{
	create_face_objects();
	create_edge_objects();
}

CShape::CShape(const CShape& s):m_gl_list(0)
{
	operator=(s);
}

CShape::~CShape()
{
	KillGLLists();
	delete_face_objects();
	delete_edge_objects();
}

const CShape& CShape::operator=(const CShape& s)
{
	delete_face_objects();
	delete_edge_objects();
	m_box = s.m_box;
	m_shape = s.m_shape;
	m_material = s.m_material;
	m_title = s.m_title;
	m_use_one_gl_list = s.m_use_one_gl_list;
	create_face_objects();
	create_edge_objects();
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

void CShape::create_face_objects()
{
	// creates the face objects
	TopExp_Explorer ex;
	for ( ex.Init( m_shape, TopAbs_FACE ) ; ex.More(); ex.Next() )
	{
		TopoDS_Face F = TopoDS::Face(ex.Current());
		CFace* new_object = new CFace(F);
		new_object->m_owner = this;
		m_faces.push_back(new_object);
	}
}

void CShape::create_edge_objects()
{
	TopExp_Explorer ex;
	for ( ex.Init( m_shape, TopAbs_EDGE ) ; ex.More(); ex.Next() )
	{
		TopoDS_Edge E = TopoDS::Edge(ex.Current());
		CEdge* new_object = new CEdge(E);
		new_object->m_owner = this;
		m_edges.push_back(new_object);
	}
}

void CShape::delete_face_objects()
{
	std::list<CFace*>::iterator It;
	for(It = m_faces.begin(); It != m_faces.end(); It++){
		CFace* face = *It;
		delete face;
	}

	m_faces.clear();
}

void CShape::delete_edge_objects()
{
	std::list<CEdge*>::iterator It;
	for(It = m_edges.begin(); It != m_edges.end(); It++){
		CEdge* edge = *It;
		delete edge;
	}

	m_edges.clear();
}

void CShape::glCommands(bool select, bool marked, bool no_color){
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
		{
			std::list<CFace*>::iterator It;
			for(It = m_faces.begin(); It != m_faces.end(); It++){
				CFace* face = *It;
				glPushName((unsigned int)face);
				face->glCommands(select, marked || wxGetApp().m_marked_list->ObjectMarked(face), no_color);
				glPopName();
			}
		}

		// render all the edges
		if(GetType() != StlSolidType){
			std::list<CEdge*>::iterator It;
			for(It = m_edges.begin(); It != m_edges.end(); It++){
				CEdge* edge = *It;
				glPushName((unsigned int)edge);
				edge->glCommands(select, marked || wxGetApp().m_marked_list->ObjectMarked(edge), no_color);
				glPopName();
			}
		}

		glEndList();
	}
}

void CShape::GetBox(CBox &box){
	if(!m_box.m_valid)
	{
		std::list<CFace*>::iterator It;
		for(It = m_faces.begin(); It != m_faces.end(); It++){
			CFace* face = *It;
			face->GetBox(m_box);
		}
	}

	box.Insert(m_box);
}

void CShape::GetGripperPositions(std::list<double> *list, bool just_for_endof)
{
	CBox box;
	GetBox(box);
	if(box.m_valid)
	{
		list->push_back(0);
		list->push_back(box.m_x[0]);
		list->push_back(box.m_x[1]);
		list->push_back(box.m_x[2]);

		list->push_back(0);
		list->push_back(box.m_x[3]);
		list->push_back(box.m_x[1]);
		list->push_back(box.m_x[2]);

		list->push_back(0);
		list->push_back(box.m_x[3]);
		list->push_back(box.m_x[4]);
		list->push_back(box.m_x[2]);

		list->push_back(0);
		list->push_back(box.m_x[0]);
		list->push_back(box.m_x[4]);
		list->push_back(box.m_x[2]);

		list->push_back(0);
		list->push_back(box.m_x[0]);
		list->push_back(box.m_x[1]);
		list->push_back(box.m_x[5]);

		list->push_back(0);
		list->push_back(box.m_x[3]);
		list->push_back(box.m_x[1]);
		list->push_back(box.m_x[5]);

		list->push_back(0);
		list->push_back(box.m_x[3]);
		list->push_back(box.m_x[4]);
		list->push_back(box.m_x[5]);

		list->push_back(0);
		list->push_back(box.m_x[0]);
		list->push_back(box.m_x[4]);
		list->push_back(box.m_x[5]);
	}
}

class OffsetShapeTool:public Tool{
	CShape* m_shape;
public:
	OffsetShapeTool(CShape* shape):m_shape(shape){}

	// Tool's virtual functions
	void Run(){
		TopoDS_Shape new_shape = BRepOffsetAPI_MakeOffsetShape(m_shape->Shape(), 2.0, 0.01, BRepOffset_RectoVerso);
		HeeksObj* new_object = CShape::MakeObject(new_shape, "Result of MakeOffsetShape");
		wxGetApp().AddUndoably(new_object, NULL, NULL);
		wxGetApp().DeleteUndoably(m_shape);
	}
	const char* GetTitle(){ return "Offset Shape";}
};

void CShape::GetTools(std::list<Tool*>* f_list, const wxPoint* p)
{
	f_list->push_back(new OffsetShapeTool(this));
}

void CShape::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	BRepBuilderAPI_Transform myBRepTransformation(m_shape,mat);
	TopoDS_Shape new_shape = myBRepTransformation.Shape();
	wxGetApp().Add(MakeObject(new_shape, m_title.c_str()), NULL);
	wxGetApp().DeleteUndoably(this);
}

void CShape::OnEditString(const char* str){
	m_title.assign(str);
	wxGetApp().WasModified(this);
}

// static member function
HeeksObj* CShape::MakeObject(const TopoDS_Shape &shape, const char* title, bool use_one_gl_list, bool stl_body){
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

	HeeksObj* new_object = CShape::MakeObject(new_shape, "Result of Cut Operation");
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().DeleteUndoably(s1);
	wxGetApp().DeleteUndoably(s2);
	return new_object;
}

static HeeksObj* Fuse(HeeksObj* s1, HeeksObj* s2){
	TopoDS_Shape sh1, sh2;
	TopoDS_Shape new_shape = BRepAlgoAPI_Fuse(((CShape*)s1)->Shape(), ((CShape*)s2)->Shape());

	HeeksObj* new_object = CShape::MakeObject(new_shape, "Result of Fuse Operation");
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().DeleteUndoably(s1);
	wxGetApp().DeleteUndoably(s2);
	return new_object;
}

static HeeksObj* Common(HeeksObj* s1, HeeksObj* s2){
	TopoDS_Shape sh1, sh2;
	TopoDS_Shape new_shape = BRepAlgoAPI_Common(((CShape*)s1)->Shape(), ((CShape*)s2)->Shape());

	HeeksObj* new_object = CShape::MakeObject(new_shape, "Result of Common Operation");
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().DeleteUndoably(s1);
	wxGetApp().DeleteUndoably(s2);
	return new_object;
}

CFace* CShape::find(const TopoDS_Face &face)
{
	std::list<CFace*>::iterator It;
	for(It = m_faces.begin(); It != m_faces.end(); It++){
		CFace* f = *It;
		if(f->Face() == face)return f;
	}
	return NULL;
}

void CShape::AddASphere()
{
	TopoDS_Solid solid = BRepPrimAPI_MakeSphere(5);
	wxGetApp().AddUndoably(new CSolid(solid, "Sphere"), NULL, NULL);
	wxGetApp().Repaint();
}

void CShape::AddACube()
{
	TopoDS_Solid solid = BRepPrimAPI_MakeBox(10, 10, 10);
	wxGetApp().AddUndoably(new CSolid(solid, "Cube"), NULL, NULL);
	wxGetApp().Repaint();
}

void CShape::AddACylinder()
{
	TopoDS_Solid solid = BRepPrimAPI_MakeCylinder(5, 10);
	wxGetApp().AddUndoably(new CSolid(solid, "Cylinder"), NULL, NULL);
	wxGetApp().Repaint();
}

void CShape::CutShapes(const std::list<HeeksObj*> &list_in)
{
	// subtract from the first one in the list all the others
	wxGetApp().StartHistory("Shape Cut");
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
	wxGetApp().StartHistory("Shape Fuse");
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
	wxGetApp().StartHistory("Shape Common");
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

bool CShape::ImportSolidsFile(const char* filepath)
{
	// returns true, if suffix handled
	wxString wf(filepath);

	if(wf.EndsWith(".stp") || wf.EndsWith(".STP") || wf.EndsWith(".step") || wf.EndsWith(".STEP"))
	{
		Standard_CString aFileName = (Standard_CString) (wf.c_str());
		STEPControl_Reader Reader;
		int status = Reader.ReadFile( aFileName );

		if ( status == IFSelect_RetDone )
		{
			Reader.TransferRoots();
			int nshapes = Reader.NbShapes();
			for(int i = 1; i<=nshapes; i++){
				TopoDS_Shape rShape;
				rShape = Reader.Shape(i);      
				wxGetApp().AddUndoably(MakeObject(rShape, "STEP solid"), NULL, NULL);
			}
			wxGetApp().Repaint();
		}
		else{
			wxMessageBox("STEP import not done!");
		}
		return true;
	}
	else if(wf.EndsWith(".igs") || wf.EndsWith(".IGS") || wf.EndsWith(".iges") || wf.EndsWith(".IGES"))
	{
		Standard_CString aFileName = (Standard_CString) (wf.c_str());
		IGESControl_Reader Reader;
		int status = Reader.ReadFile( aFileName );

		if ( status == IFSelect_RetDone )
		{
			Reader.TransferRoots();
			int nshapes = Reader.NbShapes();
			for(int i = 1; i<=nshapes; i++){
				TopoDS_Shape rShape;
				rShape = Reader.Shape(i);      
				wxGetApp().AddUndoably(MakeObject(rShape, "IGES solid"), NULL, NULL);
			}

			// but these are all sheets, so we need to sew them
			// to do...
			wxGetApp().Repaint();
		}
		else{
			wxMessageBox("IGES import not done!");
		}
		return true;
	}
	else if(wf.EndsWith(".stl") || wf.EndsWith(".STL"))
	{
		Standard_CString aFileName = (Standard_CString) (wf.c_str());
		TopoDS_Shape rShape;
		StlAPI_Reader Reader;
		Reader.Read(rShape, aFileName);
		wxGetApp().AddUndoably(MakeObject(rShape, "STL solid", false, true), NULL, NULL);
		wxGetApp().Repaint();
		return true;
	}
	return false;
}

bool CShape::ExportSolidsFile(const char* filepath)
{
	// returns true, if suffix handled
	wxString wf(filepath);

	if(wf.EndsWith(".stp") || wf.EndsWith(".STP") || wf.EndsWith(".step") || wf.EndsWith(".STEP"))
	{
		Standard_CString aFileName = (Standard_CString) (wf.c_str());
		STEPControl_Writer writer;
		// add all the solids
		for(HeeksObj* object = wxGetApp().GetFirstChild(); object; object = wxGetApp().GetNextChild()){
			if(CShape::IsTypeAShape(object->GetType())){
				writer.Transfer(((CSolid*)object)->Shape(), STEPControl_ManifoldSolidBrep);
			}
		}
		writer.Write(aFileName);
		return true;
	}
	else if(wf.EndsWith(".igs") || wf.EndsWith(".IGS") || wf.EndsWith(".iges") || wf.EndsWith(".IGES"))
	{
		Standard_CString aFileName = (Standard_CString) (wf.c_str());

		IGESControl_Controller::Init();
		IGESControl_Writer writer;

		// add all the solids
		for(HeeksObj* object = wxGetApp().GetFirstChild(); object; object = wxGetApp().GetNextChild()){
			if(CShape::IsTypeAShape(object->GetType())){
				writer.AddShape(((CSolid*)object)->Shape());
			}
			else if(object->GetType() == WireType){
				writer.AddShape(((CWire*)object)->Shape());
			}
		}
		writer.Write(aFileName);
		return true;
	}
	else if(wf.EndsWith(".stl") || wf.EndsWith(".STL"))
	{
		Standard_CString aFileName = (Standard_CString) (wf.c_str());

		{
			// clear file
			ofstream ofs(aFileName);
		}

		StlAPI_Writer writer;
		// add all the solids
		for(HeeksObj* object = wxGetApp().GetFirstChild(); object; object = wxGetApp().GetNextChild()){
			if(CShape::IsTypeAShape(object->GetType())){
				// append stl to file with default coefficient
				writer.Write(((CSolid*)object)->Shape(), aFileName);
			}
		}
		return true;
	}
	return false;
}

void CShape::GetTriangles(void(*callbackfunc)(double* x, double* n), double cusp){
	BRepTools::Clean(m_shape);
	BRepMesh::Mesh(m_shape, cusp);

	std::list<CFace*>::iterator It;
	for(It = m_faces.begin(); It != m_faces.end(); It++){
		CFace* face = *It;
		face->GetTriangles(callbackfunc, cusp);
	}
}

void CShape::GetCentreNormals(void(*callbackfunc)(double area, double *x, double *n)){
	std::list<CFace*>::iterator It;
	for(It = m_faces.begin(); It != m_faces.end(); It++){
		CFace* face = *It;
		face->GetCentreNormals(callbackfunc);
	}
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
