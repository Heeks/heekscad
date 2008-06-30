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
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	const char* GetTypeString(void)const{return "Face";}
	void GetTriangles(void(*callbackfunc)(double* x, double* n), double cusp);
	void GetCentreNormals(void(*callbackfunc)(double area, double *x, double *n));
	void ModifyByMatrix(const double* m);
	void GetTools(std::list<Tool*>* f_list, const wxPoint* p);

	const TopoDS_Face &Face(){return m_topods_face;}
	const Material &GetMaterial(){return m_material;}
	void SetMaterial(const Material& mat){m_material = mat;}
	gp_Dir GetMiddleNormal(gp_Pnt *pos = NULL)const;
	double Area()const;
};