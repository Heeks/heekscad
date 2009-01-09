// dxf.h
#include <fstream>
using namespace std;

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

	bool ReadLine();
	bool ReadArc();
	void OnReadArc(double start_angle, double end_angle, double radius, const double* c);

public:
	CDxfRead(const wxChar* filepath); // this opens the file
	~CDxfRead(); // this closes the file

	bool Failed(){return m_fail;}
	void DoRead(); // this reads the file and calls the following functions

	virtual void OnReadLine(const double* s, const double* e){}
	virtual void OnReadArc(const double* s, const double* e, const double* c, bool dir){}
};

