// Edge.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "Edge.h"
#include "Face.h"
#include "Vertex.h"
#include "Solid.h"
#include "Shape.h"
#include "../interface/Tool.h"
#include "HeeksConfig.h"
#include "Gripper.h"
#include "../interface/PropertyLength.h"

CEdge::CEdge(const TopoDS_Edge &edge):m_topods_edge(edge), m_vertex0(NULL), m_vertex1(NULL), m_midpoint_calculated(false), m_temp_attr(0){
	GetCurveParams2(&m_start_u, &m_end_u, &m_isClosed, &m_isPeriodic);
	Evaluate(m_start_u, &m_start_x, &m_start_tangent_x);
	double t[3];
	Evaluate(m_end_u, &m_end_x, t);
	m_orientation = Orientation();
}

CEdge::~CEdge(){
}

const wxBitmap &CEdge::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/edge.png")));
	return *icon;
}

void CEdge::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		wxGetApp().glColorEnsuringContrast(HeeksColor(0, 0, 0));
	}

	if(Owner() && Owner()->Owner() && Owner()->Owner()->GetType() == SolidType)
	{
		// triangulate a face on the edge first
		if(this->m_faces.size() > 0)
		{
			TopLoc_Location fL;
			Handle_Poly_Triangulation facing = BRep_Tool::Triangulation(m_faces.front()->Face(),fL);

			if(!facing.IsNull())
			{
				// Get polygon
				Handle_Poly_PolygonOnTriangulation polygon = BRep_Tool::PolygonOnTriangulation(m_topods_edge, facing, fL);
				gp_Trsf tr = fL;
				double m[16];
				extract_transposed(tr, m);
				glPushMatrix();
				glMultMatrixd(m);

				if (!polygon.IsNull())
				{
					glBegin(GL_LINE_STRIP);
					const TColStd_Array1OfInteger& Nodes = polygon->Nodes();
					const TColgp_Array1OfPnt& FNodes = facing->Nodes();
					int nnn = polygon->NbNodes();
					for (int nn = 1; nn <= nnn; nn++)
					{
						gp_Pnt v = FNodes(Nodes(nn));
						glVertex3d(v.X(), v.Y(), v.Z());
					}
					glEnd();
				}

				glPopMatrix();
			}
		}
	}
	else
	{
		bool glwidth_done = false;
		GLfloat save_depth_range[2];
		if(Owner() == NULL || Owner()->Owner() == NULL || Owner()->Owner()->GetType() != WireType)
		{
			BRepTools::Clean(m_topods_edge);
			double pixels_per_mm = wxGetApp().GetPixelScale();
			BRepMesh::Mesh(m_topods_edge, 1/pixels_per_mm);
			if(marked){
				glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
				glDepthRange(0, 0);
				glLineWidth(2);
				glwidth_done = true;
			}
		}

		TopLoc_Location L;
		Handle(Poly_Polygon3D) Polyg = BRep_Tool::Polygon3D(m_topods_edge, L);
		if (!Polyg.IsNull()) {
			const TColgp_Array1OfPnt& Points = Polyg->Nodes();
			Standard_Integer po;
			glBegin(GL_LINE_STRIP);
			for (po = Points.Lower(); po <= Points.Upper(); po++) {
				gp_Pnt p = (Points.Value(po)).Transformed(L);
				glVertex3d(p.X(), p.Y(), p.Z());
			}
			glEnd();
		}

		if(glwidth_done)
		{
			glLineWidth(1);
			glDepthRange(save_depth_range[0], save_depth_range[1]);
		}
	}
}

void CEdge::GetBox(CBox &box){
	// just use the vertices for speed
	for (TopExp_Explorer expVertex(m_topods_edge, TopAbs_VERTEX); expVertex.More(); expVertex.Next())
	{
		const TopoDS_Shape &V = expVertex.Current();
		gp_Pnt pos = BRep_Tool::Pnt(TopoDS::Vertex(V));
		double p[3];
		extract(pos, p);
		box.Insert(p);
	}
}

void CEdge::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	if(just_for_endof)
	{
		list->push_back(GripData(GripperTypeTranslate,m_start_x,m_start_y,m_start_z,NULL));
		list->push_back(GripData(GripperTypeTranslate,m_end_x,m_end_y,m_end_z,NULL));
	}
	else
	{
		// add a gripper in the middle, just to show the edge is selected
		if(!m_midpoint_calculated)
		{
			BRepAdaptor_Curve curve(m_topods_edge);
			double us = curve.FirstParameter();
			double ue = curve.LastParameter();
			double umiddle = (us+ue)/2;
			Evaluate(umiddle, m_midpoint, NULL);
		}

		list->push_back(GripData(GripperTypeTranslate,m_midpoint[0],m_midpoint[1],m_midpoint[2],NULL));
	}
}

static CEdge* edge_for_tools = NULL;

class FilletTool:public Tool
{
public:
	const wxChar* GetTitle(){return _("Blend edge");}
	wxString BitmapPath(){return _T("edgeblend");}
	void Run(){
		double rad = 2.0;
		HeeksConfig config;
		config.Read(_T("EdgeBlendRadius"), &rad);
		if(wxGetApp().InputLength(_("Enter Blend Radius"), _("Radius"), rad))
		{
			edge_for_tools->Blend(rad,false);
			config.Write(_T("EdgeBlendRadius"), rad);
		}
	}
};

static FilletTool fillet_tool;


class ChamferTool:public Tool
{
public:
	const wxChar* GetTitle(){return _("Chamfer");}
	wxString BitmapPath(){return _T("edgeblend");}
	void Run(){
		double rad = 2.0;
		HeeksConfig config;
		config.Read(_T("EdgeChamferDist"), &rad);
		if(wxGetApp().InputLength(_("Enter Chamfer Distance"), _("Radius"), rad))
		{	//CShape::FilletOrChamferEdges(edge_for_tools->list(), rad, true);
			edge_for_tools->Blend(rad,true);
			config.Write(_T("EdgeChamferDist"), rad);
		}
	}
};

static ChamferTool chamfer_tool;



class EdgeToSketchTool:public Tool
{
public:
	const wxChar* GetTitle(){return _("Make a sketch from edge");}
	wxString BitmapPath(){return _T("edge2sketch");}
	void Run(){
		CSketch* new_object = new CSketch();
		ConvertEdgeToSketch2(edge_for_tools->Edge(), new_object, FaceToSketchTool::deviation);
		wxGetApp().Add(new_object, NULL);
	}
};

static EdgeToSketchTool make_sketch_tool;

void CEdge::GetTools(std::list<Tool*>* t_list, const wxPoint* p){
	edge_for_tools = this;
	if(GetParentBody())t_list->push_back(&fillet_tool);
	t_list->push_back(&chamfer_tool);
	t_list->push_back(&make_sketch_tool);
}



void CEdge::Blend(double radius,  bool chamfer_not_fillet){
	
		if(chamfer_not_fillet)  //chamfer 
		{
			try{
				
				wxGetApp().CreateUndoPoint();
				CShape::FilletOrChamferEdges(wxGetApp().m_marked_list->list(), radius, true);
										
				wxGetApp().m_marked_list->Clear(true);
				wxGetApp().Changed();
					
			}
			catch (Standard_Failure) {
				Handle_Standard_Failure e = Standard_Failure::Caught();
				wxMessageBox(wxString(_("Error making fillet")) + _T(": ") + Ctt(e->GetMessageString()));
			}
			catch(...)
			{
				wxMessageBox(_("A fatal error happened during Blend"));
			}


		} //end if- chamfer 

		else  //fillet 
		{
			try{
				wxGetApp().CreateUndoPoint();
				CShape* body = GetParentBody();
				if(body){
					BRepFilletAPI_MakeFillet fillet(body->Shape());
					fillet.Add(radius, m_topods_edge);
					TopoDS_Shape new_shape = fillet.Shape();
					wxGetApp().Add(new CSolid(*((TopoDS_Solid*)(&new_shape)), _("Solid with edge blend"), *(body->GetColor()), body->GetOpacity()), NULL);
					wxGetApp().Remove(body);
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


		} //end else- fillet 


	
}// end Blend function

void CEdge::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyLength(_("length"), Length(), NULL));

	HeeksObj::GetProperties(list);
}

void CEdge::WriteXML(TiXmlNode *root)
{
	CShape::m_solids_found = true;
}

CFace* CEdge::GetFirstFace()
{
	if (m_faces.size()==0) return NULL;
	m_faceIt = m_faces.begin();
	return *m_faceIt;
}

CFace* CEdge::GetNextFace()
{
	if (m_faces.size()==0 || m_faceIt==m_faces.end()) return NULL;
	m_faceIt++;
	if (m_faceIt==m_faces.end()) return NULL;
	return *m_faceIt;
}

int CEdge::GetCurveType()
{
	// enum GeomAbs_CurveType
	// 0 - GeomAbs_Line
	// 1 - GeomAbs_Circle
	// 2 - GeomAbs_Ellipse
	// 3 - GeomAbs_Hyperbola
	// 4 - GeomAbs_Parabola
	// 5 - GeomAbs_BezierCurve
	// 6 - GeomAbs_BSplineCurve
	// 7 - GeomAbs_OtherCurve

	BRepAdaptor_Curve curve(m_topods_edge);
	GeomAbs_CurveType curve_type = curve.GetType();
	return curve_type;
}

void CEdge::GetCurveParams(double* start, double* end, double* uStart, double* uEnd, int* Reversed)
{
	BRepAdaptor_Curve curve(m_topods_edge);
	double us = curve.FirstParameter();
	double ue = curve.LastParameter();
	if(uStart)*uStart = us;
	if(uEnd)*uEnd = ue;
	if(start)extract(curve.Value(us), start);
	if(end)extract(curve.Value(ue), end);
	if(Reversed)*Reversed = Orientation() ? 0:1;
}

void CEdge::GetCurveParams2(double *uStart, double *uEnd, int *isClosed, int *isPeriodic)
{
	BRepAdaptor_Curve curve(m_topods_edge);
	*uStart = curve.FirstParameter();
	*uEnd = curve.LastParameter();
	if(isClosed)*isClosed = curve.IsClosed();
	if(isPeriodic)*isPeriodic = curve.IsPeriodic();
}

bool CEdge::InFaceSense(CFace* face)
{
	std::list<CFace*>::iterator FIt = m_faces.begin();
	std::list<bool>::iterator FSIt = m_face_senses.begin();
	for(;FIt != m_faces.end(); FIt++, FSIt++)
	{
		CFace* f = *FIt;
		bool sense = *FSIt;

		if(f == face)
		{
			bool eo = Orientation();
			return (sense == eo);
		}
	}

	return false; // shouldn't get here
}

void CEdge::Evaluate(double u, double *p, double *tangent)
{
	BRepAdaptor_Curve curve(m_topods_edge);
	gp_Pnt P;
	gp_Vec V;
	curve.D1(u, P, V);
	extract(P, p);
	if(tangent)extract(V, tangent);
}

bool CEdge::GetClosestPoint(const gp_Pnt &pos, gp_Pnt &closest_pnt, double &u)const{
    Standard_Real start_u, end_u;
	Handle(Geom_Curve) curve = BRep_Tool::Curve(m_topods_edge, start_u, end_u);
	GeomAPI_ProjectPointOnCurve projection(pos, curve);

	if(projection.NbPoints() > 0)
	{
		closest_pnt = projection.NearestPoint();
		u = projection.LowerDistanceParameter();
		return true;
	}
	return false;
}

bool CEdge::GetLineParams(double *d6)
{
	BRepAdaptor_Curve curve(m_topods_edge);
	if(curve.GetType() != GeomAbs_Line)return false;
	gp_Lin line = curve.Line();
	const gp_Pnt& pos = line.Location();
	const gp_Dir& dir = line.Direction();
	d6[0] = pos.X();
	d6[1] = pos.Y();
	d6[2] = pos.Z();
	d6[3] = dir.X();
	d6[4] = dir.Y();
	d6[5] = dir.Z();
	return true;
}

bool CEdge::GetCircleParams(double *d7)
{
	//center.x, center.y, center.z, axis.x, axis.y, axis.z, radius
	BRepAdaptor_Curve curve(m_topods_edge);
	if(curve.GetType() != GeomAbs_Circle)return false;
	gp_Circ c = curve.Circle();
	const gp_Pnt& pos = c.Location();
	const gp_Dir& dir = c.Axis().Direction();
	d7[0] = pos.X();
	d7[1] = pos.Y();
	d7[2] = pos.Z();
	d7[3] = dir.X();
	d7[4] = dir.Y();
	d7[5] = dir.Z();
	d7[6] = c.Radius();
	return true;
}

bool CEdge::GetEllipseParams(double *d11)
{
	// centerptX, centerptY, centerptZ, majorRad, majorAxisX, majorAxisY, majorAxisZ, minorRad, minorAxisX, minorAxisY, minorAxisZ
	BRepAdaptor_Curve curve(m_topods_edge);
	if(curve.GetType() != GeomAbs_Ellipse)return false;
	gp_Elips e = curve.Ellipse();
	const gp_Pnt& pos = e.Axis().Location();
	const gp_Dir& major = e.XAxis().Direction();
	const gp_Dir& minor = e.YAxis().Direction();
	d11[0] = pos.X();
	d11[1] = pos.Y();
	d11[2] = pos.Z();
	d11[3] = e.MajorRadius();
	d11[4] = major.X();
	d11[5] = major.Y();
	d11[6] = major.Z();
	d11[7] = e.MinorRadius();
	d11[8] = minor.X();
	d11[9] = minor.Y();
	d11[10] = minor.Z();
	return true;
}

bool CEdge::Orientation()
{
	TopAbs_Orientation o = m_topods_edge.Orientation();
	return (o == TopAbs_FORWARD);
}

double CEdge::Length()
{
	BRepAdaptor_Curve c(m_topods_edge);
	double len = GCPnts_AbscissaPoint::Length( c );
	return len;
}

double CEdge::Length2(double uStart, double uEnd)
{
	BRepAdaptor_Curve c(m_topods_edge);
	double len = GCPnts_AbscissaPoint::Length( c, uStart, uEnd );
	return len;
}

void CEdge::FindVertices()
{
	CShape* body = GetParentBody();
	if(body)
	{
		int i = 0;
		for (TopExp_Explorer expVertex(m_topods_edge, TopAbs_VERTEX); expVertex.More() && i<2; expVertex.Next(), i++)
		{
			const TopoDS_Shape &V = expVertex.Current();
			for(HeeksObj* object = body->m_vertices->GetFirstChild(); object; object = body->m_vertices->GetNextChild())
			{
				CVertex* v = (CVertex*)object;
				if(v->Vertex().IsSame(V))
				{
					if(i == 0)m_vertex0 = v;
					else m_vertex1 = v;
					break;
				}
			}
		}
	}
}

CVertex* CEdge::GetVertex0()
{
	if(m_vertex0 == NULL)FindVertices();
	return m_vertex0;
}

CVertex* CEdge::GetVertex1()
{
	if(m_vertex1 == NULL)FindVertices();
	return m_vertex1;
}

CShape* CEdge::GetParentBody()
{
	if(Owner() == NULL)return NULL;
	if(Owner()->Owner() == NULL)return NULL;
	if(Owner()->Owner()->GetType() != SolidType)return NULL;
	return (CShape*)(Owner()->Owner());
}

bool CEdge::GetMidPoint(double* pos)
{
	BRepAdaptor_Curve curve(m_topods_edge);
	double us = curve.FirstParameter();
	double ue = curve.LastParameter();
	double um = (us + ue)/2;
	extract(curve.Value(um), pos);
	return true;
}

bool CEdge::GetStartPoint(double* pos)
{
	BRepAdaptor_Curve curve(m_topods_edge);
	extract(curve.Value(curve.FirstParameter()), pos);
	return true;
}

bool CEdge::GetEndPoint(double* pos)
{
	BRepAdaptor_Curve curve(m_topods_edge);
	extract(curve.Value(curve.LastParameter()), pos);
	return true;
}
