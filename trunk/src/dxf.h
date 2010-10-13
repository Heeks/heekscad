// dxf.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#pragma once

class gp_Pnt;   // Forward declaration.

typedef enum
{
	eUnspecified = 0,	// Unspecified (No units)
	eInches,
	eFeet,
	eMiles,
	eMillimeters,
	eCentimeters,
	eMeters,
	eKilometers,
	eMicroinches,
	eMils,
	eYards,
	eAngstroms,
	eNanometers,
	eMicrons,
	eDecimeters,
	eDekameters,
	eHectometers,
	eGigameters,
	eAstronomicalUnits,
	eLightYears,
	eParsecs
} eDxfUnits_t;


struct SplineData
{
	double norm[3];
	int degree;
	int knots;
	int control_points;
	int fit_points;
	int flag;
	std::list<double> starttanx;
	std::list<double> starttany;
	std::list<double> starttanz;
	std::list<double> endtanx;
	std::list<double> endtany;
	std::list<double> endtanz;
	std::list<double> knot;
	std::list<double> weight;
	std::list<double> controlx;
	std::list<double> controly;
	std::list<double> controlz;
	std::list<double> fitx;
	std::list<double> fity;
	std::list<double> fitz;
};

class CDxfWrite{
private:
	ofstream* m_ofs;
	bool m_fail;

public:
	CDxfWrite(const wxChar* filepath);
	~CDxfWrite();

	bool Failed(){return m_fail;}

	void WriteLine(const double* s, const double* e, const wxString layer_name );
	void WritePoint(const double*, wxString);
	void WriteArc(const double* s, const double* e, const double* c, bool dir, const wxString layer_name );
    void WriteEllipse(const double* c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir, const wxString layer_name );
	void WriteCircle(const double* c, double radius, const wxString layer_name );
};

// derive a class from this and implement it's virtual functions
class CDxfRead{
private:
	ifstream* m_ifs;

	bool m_fail;
	char m_str[1024];
	char m_unused_line[1024];
	eDxfUnits_t m_eUnits;
	wxString m_layer_name;
	wxString m_section_name;
	wxString m_block_name;
	bool m_ignore_errors;


	typedef std::map< std::string,Aci_t > LayerAciMap_t;
	LayerAciMap_t m_layer_aci;  // layer names -> layer color aci map

	bool ReadUnits();
	bool ReadLayer();
	bool ReadLine();
	bool ReadText();
	bool ReadArc();
	bool ReadCircle();
	bool ReadEllipse();
	bool ReadPoint();
	bool ReadSpline();
	bool ReadLwPolyLine();
	bool ReadPolyLine();
	bool ReadVertex(gp_Pnt *pVertex, bool *bulge_found, double *bulge);
	void OnReadArc(double start_angle, double end_angle, double radius, const double* c);
	void OnReadCircle(const double* c, double radius);
    void OnReadEllipse(const double* c, const double* m, double ratio, double start_angle, double end_angle);
	void OnReadSpline(struct SplineData& sd);

	void get_line();
	void put_line(const char *value);
	void DerefACI();

protected:
	Aci_t m_aci; // manifest color name or 256 for layer color

public:
	CDxfRead(const wxChar* filepath); // this opens the file
	~CDxfRead(); // this closes the file

	bool Failed(){return m_fail;}
	void DoRead(const bool ignore_errors = false); // this reads the file and calls the following functions

	double mm( const double & value ) const;

	bool IgnoreErrors() const { return(m_ignore_errors); }

	virtual void OnReadLine(const double* s, const double* e){}
	virtual void OnReadPoint(const double* s){}
	virtual void OnReadText(const double* point, const double height, const wxString text){}
	virtual void OnReadArc(const double* s, const double* e, const double* c, bool dir){}
	virtual void OnReadCircle(const double* s, const double* c, bool dir){}
	virtual void OnReadEllipse(const double* c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir){}
	virtual void OnReadSpline(TColgp_Array1OfPnt &control, TColStd_Array1OfReal &weight, TColStd_Array1OfReal &knot,TColStd_Array1OfInteger &mult, int degree, bool periodic, bool rational){}
	virtual void AddGraphics() const { }

    wxString LayerName() const;

};

class CSketch;

class HeeksDxfRead : public CDxfRead{
private:
    typedef wxString LayerName_t;
	typedef std::map< LayerName_t, CSketch * > Sketches_t;
	Sketches_t m_sketches;
	
	HeeksColor DecodeACI(const int aci);
public:
	HeeksDxfRead(const wxChar* filepath):CDxfRead(filepath){}

	static bool m_make_as_sketch;
	static bool m_ignore_errors;

	// CDxfRead's virtual functions
	void OnReadLine(const double* s, const double* e);
	void OnReadPoint(const double* s);
	void OnReadText(const double* point, const double height, const wxString text);
	void OnReadArc(const double* s, const double* e, const double* c, bool dir);
	void OnReadCircle(const double* s, const double* c, bool dir);
    void OnReadEllipse(const double* c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir);
	void OnReadSpline(TColgp_Array1OfPnt &control, TColStd_Array1OfReal &weight, TColStd_Array1OfReal &knot,TColStd_Array1OfInteger &mult, int degree, bool periodic, bool rational);

	void AddObject(HeeksObj *object);
	void AddGraphics() const;
};
