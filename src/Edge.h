// Edge.h

#include "../interface/HeeksObj.h"
#include <TopoDS_Edge.hxx>

class CSolid;

class CEdge:public HeeksObj{
private:
	CBox m_box;
	TopoDS_Edge m_topods_edge;
	static wxIcon* m_icon;

public:
	CEdge(const TopoDS_Edge &edge);
	~CEdge();

	int GetType()const{return EdgeType;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	HeeksObj *MakeACopy(void)const{ return new CEdge(*this);}
	wxIcon* GetIcon();
	const char* GetTypeString(void)const{return "Edge";}
	void GetTools(std::list<Tool*>* f_list, const wxPoint* p);

	const TopoDS_Shape &Edge(){return m_topods_edge;}
	void Blend(double radius);
};