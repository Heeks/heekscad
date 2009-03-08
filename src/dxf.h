// dxf.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

class CDxfWrite{
private:
	ofstream* m_ofs;
	bool m_fail;

public:
	CDxfWrite(const wxChar* filepath);
	~CDxfWrite();

	bool Failed(){return m_fail;}

	void WriteLine(const double* s, const double* e);
	void WriteArc(const double* s, const double* e, const double* c, bool dir);
};

// derive a class from this and implement it's virtual functions
class CDxfRead{
private:
	ifstream* m_ifs;

	bool m_fail;
	char m_str[1024];

	bool ReadLine(bool undoably);
	bool ReadArc(bool undoably);
	bool ReadCircle(bool undoably);
	bool ReadLwPolyLine(bool undoably);
	void OnReadArc(double start_angle, double end_angle, double radius, const double* c, bool undoably);
	void OnReadCircle(const double* c, double radius, bool undoably);
	void get_line();

public:
	CDxfRead(const wxChar* filepath); // this opens the file
	~CDxfRead(); // this closes the file

	bool Failed(){return m_fail;}
	void DoRead(bool undoably); // this reads the file and calls the following functions

	virtual void OnReadLine(const double* s, const double* e, bool undoably){}
	virtual void OnReadArc(const double* s, const double* e, const double* c, bool dir, bool undoably){}
	virtual void OnReadCircle(const double* s, const double* c, bool dir, bool undoably){}
};

class CSketch;

class HeeksDxfRead : public CDxfRead{
public:
	CSketch* m_sketch;
	HeeksDxfRead(const wxChar* filepath):CDxfRead(filepath), m_sketch(NULL){}

	static bool m_make_as_sketch;

	// CDxfRead's virtual functions
	void OnReadLine(const double* s, const double* e, bool undoably);
	void OnReadArc(const double* s, const double* e, const double* c, bool dir, bool undoably);
	void OnReadCircle(const double* s, const double* c, bool dir, bool undoably);

	void AddSketchIfNeeded(bool undoably);
};
