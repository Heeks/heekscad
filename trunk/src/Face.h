// Face.h

#include "../interface/HeeksObj.h"
#include "../interface/Material.h"
#include <TopoDS_Face.hxx>

class CShape;
class CEdge;

class CFace:public HeeksObj{
private:
	CBox m_box;
	Material m_material;
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

	CFace(const TopoDS_Face &face);
	~CFace();

	int GetType()const{return FaceType;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	wxIcon* GetIcon();
	HeeksObj *MakeACopy(void)const{ return new CFace(*this);}
	const wxChar* GetTypeString(void)const{return _T("Face");}
	void GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal = true);
	double Area()const;
	void ModifyByMatrix(const double* m);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	void WriteXML(TiXmlElement *root);

	const TopoDS_Face &Face(){return m_topods_face;}
	const Material &GetMaterial(){return m_material;}
	void SetMaterial(const Material& mat){m_material = mat;}
	gp_Dir GetMiddleNormal(gp_Pnt *pos = NULL)const;
	gp_Dir GetNormalAtUV(double u, double v, gp_Pnt *pos = NULL)const;
	bool GetUVAtPoint(const gp_Pnt &pos, double *u, double *v)const;
	void GetPlaneParams(gp_Pln &p);
	int GetSurfaceType();
	CEdge* GetFirstEdge();
	CEdge* GetNextEdge();
	bool Orientation();
};