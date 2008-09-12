// dxf.h

class CDXF{
private:
	ofstream* m_ofs;
	bool m_fail;

public:
	CDXF(const char* filepath);
	~CDXF();

	bool Failed(){return m_fail;}

	void WriteLine(const double* s, const double* e);
	void WriteArc(const double* s, const double* e, const double* c, bool dir);
};