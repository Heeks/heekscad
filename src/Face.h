// Face.h

#include "../interface/HeeksObj.h"
#include "../interface/Material.h"
#include <TopoDS_Face.hxx>

class CShape;
class CEdge;
class CLoop;

class CFace:public HeeksObj{
private:
	CBox m_box;
	TopoDS_Face m_topods_face;
	static wxIcon* m_icon;
#if _DEBUG
	double m_pos_x;
	double m_pos_y;
	double m_pos_z;
	double m_normal_x;
	double m_normal_y;
	double m_normal_z;
	bool m_orientation;
#endif

public:
	int m_temp_attr; // not saved with the model
	std::list<CEdge*>::iterator m_edgeIt;
	std::list<CEdge*> m_edges;
	std::list<CLoop*>::iterator m_loopIt;
	std::list<CLoop*> m_loops;

	CFace(const TopoDS_Face &face);
	~CFace();

	int GetType()const{return FaceType;}
	long GetMarkingMask()const{return MARKING_FILTER_FACE;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	wxString GetIcon(){return _T("face");}
	HeeksObj *MakeACopy(void)const{ return new CFace(*this);}
	const wxChar* GetTypeString(void)const{return _("Face");}
	void GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal = true);
	double Area()const;
	bool ModifyByMatrix(const double* m);
	void WriteXML(TiXmlElement *root);
	void GetProperties(std::list<Property *> *list);
	bool UsesID(){return true;}

	const TopoDS_Face &Face(){return m_topods_face;}
	gp_Dir GetMiddleNormal(gp_Pnt *pos = NULL)const;
	gp_Dir GetNormalAtUV(double u, double v, gp_Pnt *pos = NULL)const;
	bool GetUVAtPoint(const gp_Pnt &pos, double *u, double *v)const;
	bool GetClosestPoint(const gp_Pnt &pos, gp_Pnt &closest_pnt)const;
	void GetPlaneParams(gp_Pln &p);
	void GetCylinderParams(gp_Cylinder &c);
	void GetSphereParams(gp_Sphere &s);
	void GetConeParams(gp_Cone &c);
	int GetSurfaceType();
	wxString GetSurfaceTypeStr();
	CEdge* GetFirstEdge();
	CEdge* GetNextEdge();
	CLoop* GetFirstLoop();
	CLoop* GetNextLoop();
	bool Orientation();
	void GetUVBox(double *uv_box);
};

