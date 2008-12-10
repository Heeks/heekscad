// Wire.h

#include "Shape.h"
#include <TopoDS_Wire.hxx>

class CWire:public CShape{
private:
	static wxIcon* m_icon;

public:
	CWire(const TopoDS_Wire &shape, const wxChar* title);
	~CWire();

	int GetType()const{return WireType;}
	long GetMarkingMask()const{return MARKING_FILTER_WIRE;}
	const wxChar* GetTypeString(void)const{return _("Wire");}
	HeeksObj *MakeACopy(void)const{ return new CWire(*this);}
	wxString GetIcon(){return _T("wire");}
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);

	const TopoDS_Wire &Wire()const;
};

