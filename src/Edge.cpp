// Edge.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "Edge.h"
#include "Face.h"
#include "Solid.h"
#include "../interface/Tool.h"
#include "HeeksConfig.h"
#include "Gripper.h"
#include "../interface/PropertyLength.h"

CEdge::CEdge(const TopoDS_Edge &edge):m_topods_edge(edge), m_midpoint_calculated(false), m_temp_attr(0){
	GetCurveParams2(&m_start_u, &m_end_u, &m_isClosed, &m_isPeriodic);
	Evaluate(m_start_u, &m_start_x, &m_start_tangent_x);
	double t[3];
	Evaluate(m_end_u, &m_end_x, t);
	m_orientation = Orientation();
}

CEdge::~CEdge(){
}

void CEdge::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		wxGetApp().glColorEnsuringContrast(HeeksColor(0, 0, 0));
	}

	if(Owner() && Owner()->Owner() && Owner()->Owner()->GetType() == SolidType)
	{
		// triangulate a face on the edge first
		TopTools_IndexedDataMapOfShapeListOfShape lface;
		TopExp::MapShapesAndAncestors(((CShape*)(Owner()->Owner()))->Shape(),TopAbs_EDGE,TopAbs_FACE,lface);
		const TopTools_ListOfShape& lfac = lface.FindFromKey(m_topods_edge);
		Standard_Integer nelem= lfac.Extent();
		if(nelem == 2){
			TopTools_ListIteratorOfListOfShape It;
			It.Initialize(lfac);
			TopoDS_Face Face1 = TopoDS::Face(It.Value());
			TopLoc_Location fL;
			Handle_Poly_Triangulation facing = BRep_Tool::Triangulation(Face1,fL);

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

class BlendTool:public Tool
{
public:
	CEdge* m_edge;

	BlendTool(CEdge* edge):m_edge(edge){}

	const wxChar* GetTitle(){return _("Blend");}
	wxString BitmapPath(){return _T("edgeblend");}
	const wxChar* GetToolTip(){return _T("Blend edge");}
	void Run(){
		double rad = 2.0;
		HeeksConfig config;
		config.Read(_T("EdgeBlendRadius"), &rad);
		if(wxGetApp().InputDouble(_("Enter Blend Radius"), _("Radius"), rad))
		{
			m_edge->Blend(rad);
			config.Write(_T("EdgeBlendRadius"), rad);
		}
	}
};

static BlendTool blend_tool(NULL);

void CEdge::GetTools(std::list<Tool*>* t_list, const wxPoint* p){
	if(Owner() && Owner()->Owner() && Owner()->Owner()->GetType() == SolidType)
		blend_tool.m_edge = this;
		t_list->push_back(&blend_tool);
}

void CEdge::Blend(double radius){
	try{
		if(Owner() && Owner()->Owner() && CShape::IsTypeAShape(Owner()->Owner()->GetType())){
			BRepFilletAPI_MakeFillet fillet(((CShape*)(Owner()->Owner()))->Shape());
			fillet.Add(radius, m_topods_edge);
			TopoDS_Shape new_shape = fillet.Shape();
			wxGetApp().StartHistory();
			wxGetApp().AddUndoably(new CSolid(*((TopoDS_Solid*)(&new_shape)), _("Solid with edge blend"), *(Owner()->Owner()->GetColor())), NULL, NULL);
			wxGetApp().DeleteUndoably(Owner()->Owner());
			wxGetApp().EndHistory();
		}
	}
	catch(wxChar *message)
	{
		wxMessageBox(wxString(_("A fatal error happened during Blend")) + _T(" -\n") + wxString(message));
	}
	catch(...)
	{
		wxMessageBox(_("A fatal error happened during Blend"));
	}
}

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
