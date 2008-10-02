// StlSolid.h

#include "../interface/HeeksObj.h"

class CStlTri{
public:
	float n[3];
	float x[3][3];
};

class CStlSolid:public HeeksObj{
private:
	static wxIcon* m_icon;
	HeeksColor color;
	int m_gl_list;
	CBox m_box;
	wxString m_title;

public:
	std::list<CStlTri> m_list;

	CStlSolid(const HeeksColor* col);
	CStlSolid(const wxChar* filepath, const HeeksColor* col);
	~CStlSolid();

	virtual const CStlSolid& operator=(const CStlSolid& s);

	int GetType()const{return StlSolidType;}
	int GetIDGroupType()const{return SolidType;}
	const wxChar* GetTypeString(void)const{return _T("STL Solid");}
	wxIcon* GetIcon();
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	void KillGLLists(void);
	void ModifyByMatrix(const double* m);
	const wxChar* GetShortString(void)const{return m_title.c_str();}
	bool CanEditString(void)const{return true;}
	void OnEditString(const wxChar* str);
	void GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal = true);
	void CopyFrom(const HeeksObj* object);
	HeeksObj *MakeACopy()const;
	void WriteXML(TiXmlElement *root);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
};