// svg.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>

struct TwoPoints
{
	gp_Pnt ppnt;
	gp_Pnt pcpnt;
};

// derive a class from this and implement it's virtual functions
class CSvgRead{
private:
	std::list<gp_Trsf> m_transform_stack;
	bool m_fail;

	std::string RemoveCommas(std::string input);
	void ReadSVGElement(TiXmlElement* pElem, bool undoably);
	void ReadTransform(TiXmlElement* pElem);
	void ReadPath(TiXmlElement* pElem, bool undoably);
	void ReadRect(TiXmlElement* pElem, bool undoably);
	void ReadCircle(TiXmlElement* pElem, bool undoably);
	void ReadEllipse(TiXmlElement* pElem, bool undoably);
	void ReadLine(TiXmlElement* pElem, bool undoably);
	void ReadPolyline(TiXmlElement* pElem, bool close, bool undoably);
	gp_Pnt ReadStart(const char *text,gp_Pnt ppnt,bool isupper,bool undoably);
	void ReadClose(gp_Pnt ppnt, gp_Pnt spnt,bool undoably);
	gp_Pnt ReadLine(const char *text,gp_Pnt ppnt,bool isupper,bool undoably);
	gp_Pnt ReadHorizontal(const char *text,gp_Pnt ppnt,bool isupper,bool undoably);
	gp_Pnt ReadVertical(const char *text,gp_Pnt ppnt,bool isupper,bool undoably);
	struct TwoPoints ReadCubic(const char *text,gp_Pnt ppnt,bool isupper,bool undoably);
	struct TwoPoints ReadCubic(const char *text,gp_Pnt ppnt,gp_Pnt pcpnt, bool isupper,bool undoably);
	struct TwoPoints ReadQuadratic(const char *text,gp_Pnt ppnt,bool isupper,bool undoably);
	struct TwoPoints ReadQuadratic(const char *text,gp_Pnt ppnt,gp_Pnt pcpnt,bool isupper,bool undoably);
	gp_Pnt ReadEllipse(const char *text,gp_Pnt ppnt,bool isupper,bool undoably);
public:
	CSvgRead(); // this opens the file
	~CSvgRead(); // this closes the file

	gp_Trsf m_transform;

	void Read(const wxChar* filepath, bool undoably);

	virtual void OnReadStart(bool undoably){}
	virtual void OnReadLine(gp_Pnt p1, gp_Pnt p2, bool undoably){}
	virtual void OnReadCubic(gp_Pnt s, gp_Pnt c1, gp_Pnt c2, gp_Pnt e, bool undoably){}
	virtual void OnReadQuadratic(gp_Pnt s, gp_Pnt c, gp_Pnt e, bool undoably){}
	virtual void OnReadEllipse(gp_Pnt c, double maj_r, double min_r, double rot, double start_a, double end_a, bool undoably){}
	virtual void OnReadCircle(gp_Pnt c, double r, bool undoably){}

	bool Failed(){return m_fail;}
};

class CSketch;

class HeeksSvgRead : public CSvgRead{
	bool m_usehspline;
	bool m_undoably;
	CSketch* m_sketch;
public:
	HeeksSvgRead(const wxChar* filepath, bool undoably, bool usehspline);

	void AddSketchIfNeeded(bool undoably);
	void ModifyByMatrix(HeeksObj* object);
	void OnReadStart(bool undoably);
	void OnReadLine(gp_Pnt p1, gp_Pnt p2, bool undoably);
	void OnReadCubic(gp_Pnt s, gp_Pnt c1, gp_Pnt c2, gp_Pnt e, bool undoably);
	void OnReadQuadratic(gp_Pnt s, gp_Pnt c, gp_Pnt e, bool undoably);
	void OnReadEllipse(gp_Pnt c, double maj_r, double min_r, double rot, double start_a, double end_a, bool undoably);
	void OnReadCircle(gp_Pnt c, double r, bool undoably);
};
