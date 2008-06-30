// StlSolid.h

#include "../interface/HeeksObj.h"

class CStlSolid:public HeeksObj{
private:
	static wxIcon* m_icon;

public:
	CStlSolid();
	~CStlSolid();

	int GetType()const{return StlSolidType;}
	const char* GetTypeString(void)const{return "StlSolid";}
	wxIcon* GetIcon();
};