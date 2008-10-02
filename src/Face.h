// Face.h

#include "../interface/HeeksObj.h"
#include "../interface/Material.h"
#include <TopoDS_Face.hxx>

class CShape;

class CFace:public HeeksObj{
private:
	CBox m_box;
	Material m_material;
	TopoDS_Face m_topods_face;
	static wxIcon* m_icon;

public:
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
};