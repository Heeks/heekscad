// svg.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

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
	void ReadSVGElement(TiXmlElement* pElem);
	void ReadTransform(TiXmlElement* pElem);
	void ReadPath(TiXmlElement* pElem);
	void ReadRect(TiXmlElement* pElem);
	void ReadCircle(TiXmlElement* pElem);
	void ReadEllipse(TiXmlElement* pElem);
	void ReadLine(TiXmlElement* pElem);
	void ReadPolyline(TiXmlElement* pElem, bool close);
	gp_Pnt ReadStart(const char *text,gp_Pnt ppnt,bool isupper);
	void ReadClose(gp_Pnt ppnt, gp_Pnt spnt);
	gp_Pnt ReadLine(const char *text,gp_Pnt ppnt,bool isupper);
	gp_Pnt ReadHorizontal(const char *text,gp_Pnt ppnt,bool isupper);
	gp_Pnt ReadVertical(const char *text,gp_Pnt ppnt,bool isupper);
	struct TwoPoints ReadCubic(const char *text,gp_Pnt ppnt,bool isupper);
	struct TwoPoints ReadCubic(const char *text,gp_Pnt ppnt,gp_Pnt pcpnt, bool isupper);
	struct TwoPoints ReadQuadratic(const char *text,gp_Pnt ppnt,bool isupper);
	struct TwoPoints ReadQuadratic(const char *text,gp_Pnt ppnt,gp_Pnt pcpnt,bool isupper);
	gp_Pnt ReadEllipse(const char *text,gp_Pnt ppnt,bool isupper);
public:
	CSvgRead(); // this opens the file
	~CSvgRead(); // this closes the file

	gp_Trsf m_transform;

	void Read(const wxChar* filepath);

	virtual void OnReadStart(){}
	virtual void OnReadLine(gp_Pnt p1, gp_Pnt p2){}
	virtual void OnReadCubic(gp_Pnt s, gp_Pnt c1, gp_Pnt c2, gp_Pnt e){}
	virtual void OnReadQuadratic(gp_Pnt s, gp_Pnt c, gp_Pnt e){}
	virtual void OnReadEllipse(gp_Pnt c, double maj_r, double min_r, double rot, double start, double end){}
	virtual void OnReadCircle(gp_Pnt c, double r){}

	bool Failed(){return m_fail;}
};

class CSketch;

class HeeksSvgRead : public CSvgRead{
	bool m_usehspline;
	bool m_undoably;
	CSketch* m_sketch;
public:
	HeeksSvgRead(const wxChar* filepath, bool usehspline);

	void AddSketchIfNeeded();
	void ModifyByMatrix(HeeksObj* object);
	void OnReadStart();
	void OnReadLine(gp_Pnt p1, gp_Pnt p2);
	void OnReadCubic(gp_Pnt s, gp_Pnt c1, gp_Pnt c2, gp_Pnt e);
	void OnReadQuadratic(gp_Pnt s, gp_Pnt c, gp_Pnt e);
	void OnReadEllipse(gp_Pnt c, double maj_r, double min_r, double rot, double start, double end);
	void OnReadCircle(gp_Pnt c, double r);
};
