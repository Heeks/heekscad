// Solid.h

#include "Shape.h"
#include <TopoDS_Solid.hxx>

class CSolid:public CShape{
private:
	static wxIcon* m_icon;

public:
	CSolid(const TopoDS_Solid &solid, const char* title, bool use_one_gl_list = false);
	~CSolid();

	virtual const CSolid& operator=(const CSolid& s){ return *this;}

	int GetType()const{return SolidType;}
	const char* GetTypeString(void)const{return "Solid";}
	wxIcon* GetIcon();
	HeeksObj *MakeACopy(void)const;
};