// Solid.h

#include "Shape.h"
#include <TopoDS_Solid.hxx>

enum SolidTypeEnum{
	SOLID_TYPE_UNKNOWN, // probably not a primitive solid anymore
	SOLID_TYPE_SPHERE,
	SOLID_TYPE_CYLINDER,
	SOLID_TYPE_CUBOID,
	SOLID_TYPE_CONE
};

class CSolid:public CShape{
private:
	static wxIcon* m_icon;

public:
	SolidTypeEnum m_type; // so the solid can be stretched in specific ways, if it's still a primitive solid

	CSolid(const TopoDS_Solid &solid, const wxChar* title, bool use_one_gl_list = false);
	~CSolid();

	virtual const CSolid& operator=(const CSolid& s){ return *this;}

	int GetType()const{return SolidType;}
	long GetMarkingMask()const{return MARKING_FILTER_SOLID;}
	const wxChar* GetTypeString(void)const{return _T("Solid");}
	wxIcon* GetIcon();
	HeeksObj *MakeACopy(void)const;

	virtual SolidTypeEnum GetSolidType(){return SOLID_TYPE_UNKNOWN;}
};

class CSphere: public CSolid{
private:
	static wxIcon* m_icon;

public:
	gp_Pnt m_pos;
	double m_radius;

	CSphere(const gp_Pnt& pos, double radius);

	// HeeksObj's virtual functions
	const wxChar* GetTypeString(void)const{return _T("Sphere");}
	wxIcon* GetIcon();
	HeeksObj *MakeACopy(void)const;

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_SPHERE;}
};

class CCuboid: public CSolid{
private:
	static wxIcon* m_icon;

public:
	gp_Ax2 m_pos; // coordinate system defining position and orientation
	double m_x; // width
	double m_y; // height
	double m_z; // depth

	CCuboid(const gp_Ax2& pos, double x, double y, double z);

	// HeeksObj's virtual functions
	const wxChar* GetTypeString(void)const{return _T("Cuboid");}
	wxIcon* GetIcon();
	HeeksObj *MakeACopy(void)const;

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_CUBOID;}
};

class CCylinder: public CSolid{
private:
	static wxIcon* m_icon;

public:
	gp_Ax2 m_pos;
	double m_radius;
	double m_height;

	CCylinder(const gp_Ax2& pos, double radius, double height);

	// HeeksObj's virtual functions
	const wxChar* GetTypeString(void)const{return _T("Cylinder");}
	wxIcon* GetIcon();
	HeeksObj *MakeACopy(void)const;

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_CYLINDER;}
};

class CCone: public CSolid{
private:
	static wxIcon* m_icon;

public:
	gp_Ax2 m_pos;
	double m_r1;
	double m_r2;
	double m_height;

	CCone(const gp_Ax2& pos, double r1, double r2, double height);

	// HeeksObj's virtual functions
	const wxChar* GetTypeString(void)const{return _T("Cone");}
	wxIcon* GetIcon();
	HeeksObj *MakeACopy(void)const;

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_CONE;}
};
