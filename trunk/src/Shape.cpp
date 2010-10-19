// Shape.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "Shape.h"
#include "Solid.h"
#include "Wire.h"
#include "Group.h"
#include "Face.h"
#include "Edge.h"
#include "Vertex.h"
#include "Loop.h"
#include "Cylinder.h"
#include "Cuboid.h"
#include "Sphere.h"
#include "Cone.h"
#include "HeeksFrame.h"
#include "MarkedList.h"
#include "../interface/Tool.h"
#include "HeeksConfig.h"
#include "../interface/MarkedObject.h"
#include "../interface/PropertyDouble.h"
#include <locale.h>

// static member variable
bool CShape::m_solids_found = false;

CShape::CShape(const TopoDS_Shape &shape, const wxChar* title, const HeeksColor& col, float opacity):m_face_gl_list(0), m_edge_gl_list(0), m_shape(shape), m_opacity(opacity), m_title(title), m_color(col), m_picked_face(NULL)
{
	Init();
}

CShape::CShape(const CShape& s):m_face_gl_list(0), m_edge_gl_list(0), m_picked_face(NULL)
{
	// the faces, edges, vertices children are not copied, because we don't need them for copies in the undo engine
	m_faces = NULL;
	m_edges = NULL;
	m_vertices = NULL;
	operator=(s);
}

CShape::~CShape()
{
	KillGLLists();
	delete_faces_and_edges();
}

const CShape& CShape::operator=(const CShape& s)
{
    HeeksObj::operator = (s);

	// don't copy id
	delete_faces_and_edges();
	m_box = s.m_box;
	m_shape = s.m_shape;
	m_title = s.m_title;
	m_color = s.m_color;
	m_creation_time = s.m_creation_time;
	m_opacity = s.m_opacity;

	KillGLLists();

	return *this;
}

bool CShape::IsDifferent(HeeksObj* other)
{
	CShape* shape = (CShape*)other;
	if(shape->m_color.COLORREF_color() != m_color.COLORREF_color() || shape->m_title.CompareTo(m_title))
		return true;

	if(m_creation_time != shape->m_creation_time)
		return true;

	return false;
	// don't check all the faces and edges ( m_creation_time should be enough ) return ObjList::IsDifferent(other);
}

void CShape::Init()
{
	m_creation_time = wxGetLocalTimeMillis();
	m_faces = new CFaceList;
	m_edges = new CEdgeList;
	m_vertices = new CVertexList;
	Add(m_faces, NULL);
	Add(m_edges, NULL);
	Add(m_vertices, NULL);
	create_faces_and_edges();
}

void CShape::KillGLLists()
{
	if (m_face_gl_list)
	{
		glDeleteLists(m_face_gl_list, 1);
		m_face_gl_list = 0;
	}

	if (m_edge_gl_list)
	{
		glDeleteLists(m_edge_gl_list, 1);
		m_edge_gl_list = 0;
	}

	m_box = CBox();

	if(m_faces)
	{
		for(HeeksObj* object = m_faces->GetFirstChild(); object; object = m_faces->GetNextChild())
		{
			CFace* f = (CFace*)object;
			f->KillMarkingGLList();
		}
	}
}

void CShape::create_faces_and_edges()
{
	if(m_faces == NULL)
	{
		m_faces = new CFaceList;
		m_edges = new CEdgeList;
		m_vertices = new CVertexList;
		Add(m_faces, NULL);
		Add(m_edges, NULL);
		Add(m_vertices, NULL);
	}
	CreateFacesAndEdges(m_shape, m_faces, m_edges, m_vertices);
}

void CShape::delete_faces_and_edges()
{
	if(m_faces)m_faces->Clear();
	if(m_edges)m_edges->Clear();
}

void CShape::CallMesh()
{
	double pixels_per_mm = wxGetApp().GetPixelScale();
	BRepTools::Clean(m_shape);
	BRepMesh::Mesh(m_shape, 1/pixels_per_mm);
}

void CShape::glCommands(bool select, bool marked, bool no_color)
{
	bool blend_enabled = Material(m_color).glMaterial(m_opacity);

	bool mesh_called = false;
	bool draw_faces = (wxGetApp().m_solid_view_mode == SolidViewFacesAndEdges || wxGetApp().m_solid_view_mode == SolidViewFacesOnly);
	bool draw_edges = (wxGetApp().m_solid_view_mode == SolidViewFacesAndEdges || wxGetApp().m_solid_view_mode == SolidViewEdgesOnly);

	if(draw_faces)
	{
		for(HeeksObj* object = m_faces->GetFirstChild(); object; object = m_faces->GetNextChild())
		{
			CFace* f = (CFace*)object;
			f->MakeSureMarkingGLListExists();
		}

		if(!m_face_gl_list)
		{
			if(!mesh_called)
			{
				CallMesh();
				mesh_called = true;
			}

			// make the display list
			m_face_gl_list = glGenLists(1);
			glNewList(m_face_gl_list, GL_COMPILE);

			// render all the faces
			m_faces->glCommands(true, marked, no_color);

			glEndList();
		}

		// update faces marking display list
		GLint currentListIndex;
		glGetIntegerv(GL_LIST_INDEX, &currentListIndex);
		if(currentListIndex == 0){
			for(HeeksObj* object = m_faces->GetFirstChild(); object; object = m_faces->GetNextChild())
			{
				CFace* f = (CFace*)object;
				f->UpdateMarkingGLList(wxGetApp().m_marked_list->ObjectMarked(f));
			}
		}
	}

	if(draw_edges && !m_edge_gl_list)
	{
		if(!mesh_called)
		{
			CallMesh();
			mesh_called = true;
		}

		// make the display list
		m_edge_gl_list = glGenLists(1);
		glNewList(m_edge_gl_list, GL_COMPILE);

		// render all the edges
		m_edges->glCommands(true, marked, no_color);

		// render all the vertices
		m_vertices->glCommands(true, false, false);

		glEndList();
	}

	if(draw_faces && m_face_gl_list)
	{
		// draw the face display list
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
		glCallList(m_face_gl_list);
		glDisable(GL_LIGHTING);
		glShadeModel(GL_FLAT);
	}

	if(blend_enabled)
	{
		// turn off transparency
		glDisable(GL_BLEND);
		glDepthMask(1);
	}

	if(draw_edges && m_edge_gl_list)
	{
		// draw the edge display list
		glCallList(m_edge_gl_list);
	}
}

void CShape::GetBox(CBox &box)
{
	if(!m_box.m_valid)
	{
		if(m_faces == NULL)create_faces_and_edges();
		BRepTools::Clean(m_shape);
		BRepMesh::Mesh(m_shape, 1.0);
		if(m_faces)m_faces->GetBox(m_box);
	}

	box.Insert(m_box);
}

static CShape* shape_for_tools = NULL;

class OffsetShapeTool:public Tool{
public:
	// Tool's virtual functions
	void Run(){
		double offset_value = 2.0;
		HeeksConfig config;
		config.Read(_T("OffsetShapeValue"), &offset_value);
		if(wxGetApp().InputLength(_("Enter Offset Value, + for making bigger, - for making smaller"), _("Offset value"), offset_value))
		{
			try
			{
				TopoDS_Shape new_shape = BRepOffsetAPI_MakeOffsetShape(shape_for_tools->Shape(), offset_value, wxGetApp().m_geom_tol);

#ifdef TESTNEWSHAPE
				//This will end up throwing 90% of the exceptions caused by a bad offset
				BRepTools::Clean(new_shape);
				BRepMesh::Mesh(new_shape, 1.0);
#endif

				HeeksObj* new_object = CShape::MakeObject(new_shape, _("Result of 'Offset Shape'"), SOLID_TYPE_UNKNOWN, shape_for_tools->m_color, shape_for_tools->GetOpacity());
				shape_for_tools->Owner()->Add(new_object, NULL);
				shape_for_tools->Owner()->Remove(shape_for_tools);
				config.Write(_T("OffsetShapeValue"), offset_value);
			}
			catch (Standard_Failure) {
				Handle_Standard_Failure e = Standard_Failure::Caught();
				wxMessageBox(wxString(_("Error making offset")) + _T(": ") + Ctt(e->GetMessageString()));
			}
		}
	}
	const wxChar* GetTitle(){ return _("Offset Shape");}
	wxString BitmapPath(){return _T("offsetsolid");}
};

static OffsetShapeTool offset_shape_tool;

void CShape::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	shape_for_tools = this;
	t_list->push_back(&offset_shape_tool);
}

void CShape::MakeTransformedShape(const gp_Trsf &mat)
{
	BRepBuilderAPI_Transform myBRepTransformation(m_shape,mat);
	m_shape = myBRepTransformation.Shape();
}

wxString CShape::StretchedName()
{
	return _("Stretched Shape");
}

void CShape::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);

	if(IsMatrixDifferentialScale(mat))
	{
        gp_GTrsf gm(mat);
		BRepBuilderAPI_GTransform t(m_shape, gm);
        m_shape = t.Shape();
	}
	else
	{
		MakeTransformedShape(mat);
	}
	delete_faces_and_edges();
	KillGLLists();
	create_faces_and_edges();
}

void CShape::OnEditString(const wxChar* str){
	m_title.assign(str);
}

// static member function
HeeksObj* CShape::MakeObject(const TopoDS_Shape &shape, const wxChar* title, SolidTypeEnum solid_type, const HeeksColor& col, float opacity){
	if(shape.IsNull())return NULL;

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
			{
				return new CEdge(TopoDS::Edge(shape));
			}
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
					return new CSphere(*((TopoDS_Solid*)(&shape)), title, col, opacity);
				case SOLID_TYPE_CYLINDER:
					return new CCylinder(*((TopoDS_Solid*)(&shape)), title, col, opacity);
				case SOLID_TYPE_CUBOID:
					return new CCuboid(*((TopoDS_Solid*)(&shape)), title, col, opacity);
				case SOLID_TYPE_CONE:
					return new CCone(*((TopoDS_Solid*)(&shape)), title, col, opacity);
				default:
					// check there are some faces
					if(TopExp_Explorer(shape, TopAbs_FACE).More())
						return new CSolid(*((TopoDS_Solid*)(&shape)), title, col, opacity);
					return NULL;
				}
			}
	}

	return NULL;
}

static bool Cut(const std::list<TopoDS_Shape> &shapes, TopoDS_Shape& new_shape){
	if(shapes.size() < 2)return false;

	try
	{
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
	catch (Standard_Failure) {
		Handle_Standard_Failure e = Standard_Failure::Caught();
		wxMessageBox(wxString(_("Error with cut operation")) + _T(": ") + Ctt(e->GetMessageString()));
		return false;
	}
}

static HeeksObj* Fuse(HeeksObj* s1, HeeksObj* s2){
	try
	{
		TopoDS_Shape sh1, sh2;
		TopoDS_Shape new_shape;
		if(wxGetApp().useOldFuse)new_shape = BRepAlgo_Fuse(((CShape*)s1)->Shape(), ((CShape*)s2)->Shape());
		else new_shape = BRepAlgoAPI_Fuse(((CShape*)s1)->Shape(), ((CShape*)s2)->Shape());

		HeeksObj* new_object = CShape::MakeObject(new_shape, _("Result of Fuse Operation"), SOLID_TYPE_UNKNOWN, ((CShape*)s1)->m_color, ((CShape*)s1)->GetOpacity());
		wxGetApp().Add(new_object, NULL);
		wxGetApp().Remove(s1);
		wxGetApp().Remove(s2);
		return new_object;
	}
	catch (Standard_Failure) {
		Handle_Standard_Failure e = Standard_Failure::Caught();
		wxMessageBox(wxString(_("Error with fuse operation")) + _T(": ") + Ctt(e->GetMessageString()));
		return NULL;
	}
}

static HeeksObj* Common(HeeksObj* s1, HeeksObj* s2){
	if(s1 == NULL)
	{
		wxGetApp().Remove(s2);
		return NULL;
	}

	try
	{
		TopoDS_Shape sh1, sh2;
		TopoDS_Shape new_shape = BRepAlgoAPI_Common(((CShape*)s1)->Shape(), ((CShape*)s2)->Shape());

		HeeksObj* new_object = CShape::MakeObject(new_shape, _("Result of Common Operation"), SOLID_TYPE_UNKNOWN, ((CShape*)s1)->m_color, ((CShape*)s1)->GetOpacity());
		wxGetApp().Add(new_object, NULL);
		wxGetApp().Remove(s1);
		wxGetApp().Remove(s2);
		return new_object;
	}
	catch (Standard_Failure) {
		Handle_Standard_Failure e = Standard_Failure::Caught();
		wxMessageBox(wxString(_("Error with common operation")) + _T(": ") + Ctt(e->GetMessageString()));
		return NULL;
	}
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

bool CShape::GetExtents(double* extents, const double* orig, const double* xdir, const double* ydir, const double* zdir)
{
	gp_Pnt p_orig(0, 0, 0);
	if(orig)p_orig = gp_Pnt(orig[0], orig[1], orig[2]);
	gp_Vec v_x(1, 0, 0);
	if(xdir)v_x = gp_Vec(xdir[0], xdir[1], xdir[2]);
	gp_Vec v_y(0, 1, 0);
	if(ydir)v_y = gp_Vec(ydir[0], ydir[1], ydir[2]);
	gp_Vec v_z(0, 0, 1);
	if(zdir)v_z = gp_Vec(zdir[0], zdir[1], zdir[2]);

	BRepPrimAPI_MakeBox cuboid_plus_x(gp_Ax2(gp_Pnt(p_orig.XYZ() + 2000000 * v_x.XYZ() + (-1000000) * v_z.XYZ() + (-1000000) * v_y.XYZ()), v_x, v_y), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_minus_x(gp_Ax2(gp_Pnt(p_orig.XYZ() + (-2000000) * v_x.XYZ() + (-1000000) * v_z.XYZ() + (-1000000) * v_y.XYZ()), -v_x, v_z), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_plus_y(gp_Ax2(gp_Pnt(p_orig.XYZ() + 2000000 * v_y.XYZ() + (-1000000) * v_z.XYZ() + (-1000000) * v_x.XYZ()), v_y, v_z), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_minus_y(gp_Ax2(gp_Pnt(p_orig.XYZ() + (-2000000) * v_y.XYZ() + (-1000000) * v_z.XYZ() + (-1000000) * v_x.XYZ()), -v_y, v_x), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_plus_z(gp_Ax2(gp_Pnt(p_orig.XYZ() + 2000000 * v_z.XYZ() + (-1000000) * v_x.XYZ() + (-1000000) * v_y.XYZ()), v_z, v_x), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_minus_z(gp_Ax2(gp_Pnt(p_orig.XYZ() + (-2000000) * v_z.XYZ() + (-1000000) * v_x.XYZ() + (-1000000) * v_y.XYZ()), -v_z, v_y), 1000000, 1000000, 1000000);

	gp_Vec v_orig(p_orig.XYZ());

	TopoDS_Solid shape[6] =
	{
		cuboid_minus_x,
		cuboid_minus_y,
		cuboid_minus_z,
		cuboid_plus_x,
		cuboid_plus_y,
		cuboid_plus_z
	};

	gp_Vec vector[6] =
	{
		v_x,
		v_y,
		v_z,
		v_x,
		v_y,
		v_z
	};

	for(int i = 0; i<6; i++){
		BRepExtrema_DistShapeShape extrema(m_shape, shape[i]);
		extrema.Perform();
		gp_Pnt p = extrema.PointOnShape1(1);
		gp_Vec v(p.XYZ());
		double dp = v * vector[i];
		double dp_o = v_orig * vector[i];
		extents[i] = dp - dp_o;
	}

	return true;
}

void CShape::CopyIDsFrom(const CShape* shape_from)
{
	SetID(shape_from->m_id);
	HeeksObj* face_from = shape_from->m_faces->GetFirstChild();
	for(HeeksObj* face_to = m_faces->GetFirstChild(); face_from && face_to; face_from = shape_from->m_faces->GetNextChild(), face_to = m_faces->GetNextChild())
	{
		face_to->SetID(face_from->m_id);
	}
	HeeksObj* edge_from = shape_from->m_edges->GetFirstChild();
	for(HeeksObj* edge_to = m_edges->GetFirstChild(); edge_from && edge_to; edge_from = shape_from->m_edges->GetNextChild(), edge_to = m_edges->GetNextChild())
	{
		edge_to->SetID(edge_from->m_id);
	}
}

HeeksObj* CShape::CutShapes(std::list<HeeksObj*> &list_in, bool dodelete)
{
	if(list_in.front()->GetType() == GroupType)
	{
		CGroup* group = (CGroup*)list_in.front();
		CGroup* newgroup = new CGroup();
		group->Owner()->Add(newgroup,NULL);

		std::list<HeeksObj*> children;
		HeeksObj* child = group->GetFirstChild();
		while(child)
		{
			children.push_back(child);
			child = group->GetNextChild();
		}

		std::list<HeeksObj*>::iterator iter = children.begin();
		while(iter != children.end())
		{
			std::list<HeeksObj*> newlist;
			std::list<HeeksObj*>::const_iterator it = list_in.begin();
			while(it!=list_in.end())
			{
				newlist.push_back(*it);
				++it;
			}
			newlist.pop_front();
			newlist.push_front(*iter);
			HeeksObj* newshape = CutShapes(newlist,false);
			newshape->Owner()->Remove(newshape);
			newgroup->Add(newshape,NULL);
			++iter;
		}

		group->Owner()->Remove(group);
		wxGetApp().Remove(list_in);
		return newgroup;
	}

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
		else if(object->GetType() == FaceType)
		{
			shapes.push_back(((CFace*)object)->Face());
			if(first_solid == NULL)first_solid = object;
			delete_list.push_back(object);
		}
	}

	TopoDS_Shape new_shape;
	if(Cut(shapes, new_shape))
	{
		HeeksObj* new_object = CShape::MakeObject(new_shape, _("Result of Cut Operation"), SOLID_TYPE_UNKNOWN, ((CShape*)first_solid)->m_color, ((CShape*)first_solid)->m_opacity);
		wxGetApp().Add(new_object, NULL);
		if(dodelete)
		{
			wxGetApp().Remove(delete_list);
			wxGetApp().Repaint();
		}
		return new_object;
	}
	return first_solid;
}

HeeksObj* CShape::FuseShapes(std::list<HeeksObj*> &list_in)
{
	// fuse with the first one in the list all the others
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

	wxGetApp().Repaint();

	return s1;
}

HeeksObj* CShape::CommonShapes(std::list<HeeksObj*> &list_in)
{
	// find common solid ( intersect ) with the first one in the list all the others
	HeeksObj* s1 = NULL;
	bool s1_set = false;
	std::list<HeeksObj*> list = list_in;

	for(std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SolidType || object->GetType() == FaceType)
		{
			if(!s1_set)
			{
				s1 = object;
				s1_set = true;
			}
			else{
				s1 = Common(s1, object);
			}
		}
	}

	wxGetApp().Repaint();

	return s1;
}

void CShape::FilletOrChamferEdges(std::list<HeeksObj*> &list, double radius, bool chamfer_not_fillet)
{
	// make a map with a list of edges for each solid
	std::map< HeeksObj*, std::list< HeeksObj* > > solid_edge_map;

	for(std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++){
		HeeksObj* edge = *It;
		if(edge->GetType() == EdgeType)
		{
			HeeksObj* solid = edge->Owner()->Owner();
			if(solid && solid->GetType() == SolidType)
			{
				std::map< HeeksObj*, std::list< HeeksObj* > >::iterator FindIt = solid_edge_map.find(solid);
				if(FindIt == solid_edge_map.end())
				{
					std::list< HeeksObj* > empty_list;
					solid_edge_map.insert( make_pair(solid, empty_list) );
					FindIt = solid_edge_map.find(solid);
				}

				std::list< HeeksObj* > &list = FindIt->second;
				list.push_back(edge);
			}
		}
	}

	// do each solid
	for(std::map< HeeksObj*, std::list< HeeksObj* > >::iterator It = solid_edge_map.begin(); It != solid_edge_map.end(); It++)
	{
		HeeksObj* solid = It->first;
		std::list< HeeksObj* > &list = It->second;

		try{
			if(chamfer_not_fillet)
			{
				BRepFilletAPI_MakeChamfer chamfer(((CShape*)solid)->Shape());
				for(std::list< HeeksObj* >::iterator It2 = list.begin(); It2 != list.end(); It2++)
				{
					CEdge* edge = (CEdge*)(*It2);
					for(CFace* face = (CFace*)(edge->GetFirstFace()); face; face = (CFace*)(edge->GetNextFace()))
					{
						chamfer.Add(radius, TopoDS::Edge(edge->Edge()), TopoDS::Face(face->Face()));
					}
				}
				TopoDS_Shape new_shape = chamfer.Shape();
				wxGetApp().Add(new CSolid(*((TopoDS_Solid*)(&new_shape)), _("Solid with edge blend"), *(solid->GetColor()), ((CShape*)solid)->m_opacity), NULL);
				wxGetApp().Remove(solid);
			}
			else
			{
				BRepFilletAPI_MakeFillet fillet(((CShape*)solid)->Shape());
				for(std::list< HeeksObj* >::iterator It2 = list.begin(); It2 != list.end(); It2++)
				{
					fillet.Add(radius, TopoDS::Edge(((CEdge*)(*It2))->Edge()));
				}
				TopoDS_Shape new_shape = fillet.Shape();
				wxGetApp().Add(new CSolid(*((TopoDS_Solid*)(&new_shape)), _("Solid with edge blend"), *(solid->GetColor()), ((CShape*)solid)->m_opacity), NULL);
				wxGetApp().Remove(solid);
			}
		}
		catch (Standard_Failure) {
			Handle_Standard_Failure e = Standard_Failure::Caught();
			wxMessageBox(wxString(_("Error making fillet")) + _T(": ") + Ctt(e->GetMessageString()));
		}
		catch(...)
		{
			wxMessageBox(_("A fatal error happened during Blend"));
		}
	}

	wxGetApp().Repaint();
}

bool CShape::ImportSolidsFile(const wxChar* filepath, std::map<int, CShapeData> *index_map, HeeksObj* paste_into)
{
	// only allow paste of solids at top level or to groups
	if(paste_into && paste_into->GetType() != GroupType)return false;

	// returns true, if suffix handled
	wxString wf(filepath);

	HeeksObj* add_to = &wxGetApp();
	if(paste_into)add_to = paste_into;

	if(wf.EndsWith(_T(".stp")) || wf.EndsWith(_T(".STP")) || wf.EndsWith(_T(".step")) || wf.EndsWith(_T(".STEP")))
	{
		char oldlocale[1000];
		strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

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
						HeeksObj* new_object = MakeObject(rShape, _("STEP solid"), shape_data.m_solid_type, HeeksColor(191, 191, 191), 1.0f);
						if(new_object)
						{
							add_to->Add(new_object, NULL);
							shape_data.SetShape((CShape*)new_object);
						}
					}
				}
				else
				{
					HeeksObj* new_object = MakeObject(rShape, _("STEP solid"), SOLID_TYPE_UNKNOWN, HeeksColor(191, 191, 191), 1.0f);
					add_to->Add(new_object, NULL);
				}
			}
		}
		else{
			wxMessageBox(_("STEP import not done!"));
		}

		setlocale(LC_NUMERIC, oldlocale);

		return true;
	}
	else if(wf.EndsWith(_T(".igs")) || wf.EndsWith(_T(".IGS")) || wf.EndsWith(_T(".iges")) || wf.EndsWith(_T(".IGES")))
	{
		char oldlocale[1000];
		strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

		Standard_CString aFileName = (Standard_CString) (Ttc(filepath));
		IGESControl_Reader Reader;
		int status = Reader.ReadFile( aFileName );

		if ( status == IFSelect_RetDone )
		{
			Reader.TransferRoots();
			TopoDS_Shape shape = Reader.OneShape();
			HeeksObj* new_object = MakeObject(shape, _("IGES shape"), SOLID_TYPE_UNKNOWN, HeeksColor(191, 191, 191), 1.0f);
			add_to->Add(new_object, NULL);
#if 0
			Reader.TransferRoots();
			int num_shapes = Reader.NbShapes();
			if(num_shapes > 0)
			{
				BRepOffsetAPI_Sewing face_sewing (0.001);
				int shapes_added_for_sewing = 0;
				for(int j = 1; j<= num_shapes; j++)
				{
					TopoDS_Shape rShape = Reader.Shape(j);
					if(rShape.ShapeType() == TopAbs_EDGE)
					{
						HeeksObj* new_object = new CEdge(TopoDS::Edge(rShape));
						add_to->Add(new_object, NULL);
					}
					else
					{
						face_sewing.Add (rShape);
						shapes_added_for_sewing++;
					}
				}

				if(shapes_added_for_sewing > 0)
				{
					face_sewing.Perform ();

					if(!face_sewing.SewedShape().IsNull())
					{
						HeeksObj* new_object = MakeObject(face_sewing.SewedShape(), _("sewed IGES solid"), SOLID_TYPE_UNKNOWN, HeeksColor(191, 191, 191));
						add_to->Add(new_object, NULL);
					}
				}
			}
#endif

		}
		else{
			wxMessageBox(_("IGES import not done!"));
		}

		setlocale(LC_NUMERIC, oldlocale);

		return true;
	}
	else if(wf.EndsWith(_T(".brep")) || wf.EndsWith(_T(".BREP")))
	{
		char oldlocale[1000];
		strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

		TopoDS_Shape shape;
		BRep_Builder builder;
		Standard_Boolean result = BRepTools::Read(  shape,(char *) Ttc(filepath), builder );

		if(result)
		{
			HeeksObj* new_object = MakeObject(shape, _("BREP solid"), SOLID_TYPE_UNKNOWN, HeeksColor(191, 191, 191), 1.0f);
			add_to->Add(new_object, NULL);
		}
		else{
			wxMessageBox(_("STEP import not done!"));
		}

		setlocale(LC_NUMERIC, oldlocale);

		return true;
	}
	return false;
}

static void WriteShapeOrGroup(STEPControl_Writer &writer, HeeksObj* object, std::map<int, CShapeData> *index_map, int &i)
{
	if(CShape::IsTypeAShape(object->GetType())){

		if(index_map)index_map->insert( std::pair<int, CShapeData>(i, CShapeData((CShape*)object)) );
		i++;
		writer.Transfer(((CSolid*)object)->Shape(), STEPControl_AsIs);
	}

	if(object->GetType() == GroupType)
	{
		for(HeeksObj* o = object->GetFirstChild(); o; o = object->GetNextChild())
		{
			WriteShapeOrGroup(writer, o, index_map, i);
		}
	}
}

bool CShape::ExportSolidsFile(const std::list<HeeksObj*>& objects, const wxChar* filepath, std::map<int, CShapeData> *index_map)
{
	// returns true, if suffix handled
	wxString wf(filepath);
	wf.LowerCase();

	if(wf.EndsWith(_T(".stp")) || wf.EndsWith(_T(".step")))
	{
		char oldlocale[1000];
		strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

		Standard_CString aFileName = (Standard_CString) (Ttc(filepath));
		STEPControl_Writer writer;
		// add all the solids
		int i = 1;
		for(std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
		{
			HeeksObj* object = *It;
			WriteShapeOrGroup(writer, object, index_map, i);
		}
		writer.Write(aFileName);

		setlocale(LC_NUMERIC, oldlocale);

		return true;
	}
	else if(wf.EndsWith(_T(".igs")) || wf.EndsWith(_T(".iges")))
	{
		char oldlocale[1000];
		strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

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

		setlocale(LC_NUMERIC, oldlocale);

		return true;
	}
	else if(wf.EndsWith(_T(".brep")))
	{
#ifdef __WXMSW__
		ofstream ofs(filepath);
#else
		ofstream ofs(Ttc(filepath));
#endif
		for(std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
		{
			HeeksObj* object = *It;
			if(CShape::IsTypeAShape(object->GetType())){
				BRepTools::Write(((CShape*)object)->Shape(), ofs);
			}
		}
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

// static
bool CShape::IsMatrixDifferentialScale(const gp_Trsf& trsf)
{
	double scalex = gp_Vec(1, 0, 0).Transformed(trsf).Magnitude();
	double scaley = gp_Vec(0, 1, 0).Transformed(trsf).Magnitude();
	double scalez = gp_Vec(0, 0, 1).Transformed(trsf).Magnitude();

	if(fabs(scalex - scaley) > 0.000000000001)return true;
	if(fabs(scalex - scalez) > 0.000000000001)return true;
	return false;
}

void CShape::CopyFrom(const HeeksObj* object)
{
	*this = *((CShape*)object);
}

void CShape::WriteXML(TiXmlNode *root)
{
	CShape::m_solids_found = true;
}

void CShape::SetClickMarkPoint(MarkedObject* marked_object, const double* ray_start, const double* ray_direction)
{
	// set picked face
	m_picked_face = NULL;
	if(marked_object->m_map.size() > 0)
	{
		MarkedObject* sub_marked_object = marked_object->m_map.begin()->second;
		if(sub_marked_object)
		{
			if(sub_marked_object->m_map.size() > 0)
			{
				HeeksObj* object = sub_marked_object->m_map.begin()->first;
				if(object && object->GetType() == FaceType)
				{
					m_picked_face = (CFace*)object;
				}
			}
		}
	}
}

float CShape::GetOpacity()
{
	return m_opacity;
}

void CShape::SetOpacity(float opacity)
{
	m_opacity = opacity;
	if(m_opacity < 0.0)m_opacity = 0.0f;
	if(m_opacity > 1.0)m_opacity = 1.0f;
}

static void on_set_opacity(double value, HeeksObj* object){
	((CShape*)object)->SetOpacity((float)value);
}

void CShape::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyDouble(_("opacity"), m_opacity, this, on_set_opacity));

	ObjList::GetProperties(list);
}
