// Wire.h

#include "Shape.h"
#include <TopoDS_Wire.hxx>

class CWire:public CShape{
private:
	static wxIcon* m_icon;

public:
	CWire(const TopoDS_Wire &shape, const char* title);
	~CWire();

	int GetType()const{return WireType;}
	const char* GetTypeString(void)const{return "Wire";}
	HeeksObj *MakeACopy(void)const{ return new CWire(*this);}
	wxIcon* GetIcon();
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);

	const TopoDS_Wire &Wire()const;
};