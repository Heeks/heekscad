// HDxf.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "dxf.h"

class CSketch;

class HeeksDxfRead : public CDxfRead{
private:
    typedef wxString LayerName_t;
	typedef std::map< LayerName_t, CSketch * > Sketches_t;
	Sketches_t m_sketches;

	HeeksColor DecodeACI(const int aci);
	void OnReadSpline(TColgp_Array1OfPnt &control, TColStd_Array1OfReal &weight, TColStd_Array1OfReal &knot,TColStd_Array1OfInteger &mult, int degree, bool periodic, bool rational);
	bool IsValidLayerName( const wxString layer_name ) const;

protected:
	HeeksColor *ActiveColorPtr(Aci_t & aci);

public:
	HeeksDxfRead(const wxChar* filepath);

	static bool m_make_as_sketch;
	static bool m_ignore_errors;
	static wxString m_layer_name_suffixes_to_discard;

	// CDxfRead's virtual functions
	void OnReadLine(const double* s, const double* e, bool hidden);
	void OnReadPoint(const double* s);
	void OnReadText(const double* point, const double height, const wxString text);
	void OnReadArc(const double* s, const double* e, const double* c, bool dir, bool hidden);
	void OnReadCircle(const double* s, const double* c, bool dir, bool hidden);
    void OnReadEllipse(const double* c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir);
	void OnReadSpline(struct SplineData& sd);

	void AddObject(HeeksObj *object);
	void AddGraphics() const;
};
