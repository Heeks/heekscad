// HDxf.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "HDxf.h"
#include "HLine.h"
#include "HArc.h"
#include "HCircle.h"
#include "HEllipse.h"
#include "HSpline.h"
#include "Sketch.h"
#include "HText.h"
#include "HeeksConfig.h"


// static
bool HeeksDxfRead::m_make_as_sketch = false;
bool HeeksDxfRead::m_ignore_errors = false;
wxString HeeksDxfRead::m_layer_name_suffixes_to_discard = _T("_DOT,_DOTSMALL,_DOTBLANK,_OBLIQUE,_CLOSEDBLANK");

HeeksDxfRead::HeeksDxfRead(const wxChar* filepath) : CDxfRead(Ttc(filepath))
{
    HeeksConfig config;

	config.Read(_T("ImportDxfAsSketches"), &m_make_as_sketch);
	config.Read(_T("IgnoreDxfReadErrors"), &m_ignore_errors);
	config.Read(_T("LayerNameSuffixesToDiscard"), m_layer_name_suffixes_to_discard);
}

HeeksColor *HeeksDxfRead::ActiveColorPtr(Aci_t & aci)
{
    static HeeksColor color;
    color = HeeksColor(aci);
    return(&color);
}

HeeksColor hidden_color(128, 128, 128);

void HeeksDxfRead::OnReadLine(const double* s, const double* e, bool hidden)
{
	HLine* new_object = new HLine(make_point(s), make_point(e), hidden ? (&hidden_color) : ActiveColorPtr(m_aci));
    AddObject(new_object);
}

void HeeksDxfRead::OnReadPoint(const double* s)
{
    HPoint* new_object = new HPoint(make_point(s), ActiveColorPtr(m_aci));
    AddObject(new_object);
}

void HeeksDxfRead::OnReadArc(const double* s, const double* e, const double* c, bool dir, bool hidden)
{
	gp_Pnt p0 = make_point(s);
	gp_Pnt p1 = make_point(e);
	gp_Dir up(0, 0, 1);
	if(!dir)up = -up;
	gp_Pnt pc = make_point(c);
	gp_Circ circle(gp_Ax2(pc, up), p1.Distance(pc));
	HArc* new_object = new HArc(p0, p1, circle, hidden ? (&hidden_color) : ActiveColorPtr(m_aci));
	AddObject(new_object);
}

void HeeksDxfRead::OnReadCircle(const double* s, const double* c, bool dir, bool hidden)
{
	gp_Pnt p0 = make_point(s);
	//gp_Pnt p1 = make_point(e);
	gp_Dir up(0, 0, 1);
	if(!dir)up = -up;
	gp_Pnt pc = make_point(c);
	gp_Circ circle(gp_Ax2(pc, up), p0.Distance(pc));
	HCircle* new_object = new HCircle(circle, hidden ? (&hidden_color) : ActiveColorPtr(m_aci));
	AddObject(new_object);
}

void HeeksDxfRead::OnReadSpline(TColgp_Array1OfPnt &control, TColStd_Array1OfReal &weight, TColStd_Array1OfReal &knot,TColStd_Array1OfInteger &mult, int degree, bool periodic, bool rational)
{
	try{
		Geom_BSplineCurve spline(control,weight,knot,mult,degree,periodic,rational);
		HSpline* new_object = new HSpline(spline, ActiveColorPtr(m_aci));
		AddObject(new_object);
	}
	catch(Standard_Failure)
	{
		if (! IgnoreErrors()) throw;	// Re-throw the exception.
	}
}

void HeeksDxfRead::OnReadSpline(struct SplineData& sd)
{
	bool closed = (sd.flag & 1) != 0;
	bool periodic = (sd.flag & 2) != 0;
	bool rational = (sd.flag & 4) != 0;
	// bool planar = (sd.flag & 8) != 0;
	// bool linear = (sd.flag & 16) != 0;

	SplineData sd_copy = sd;

	if(closed)
	{
		// add some more control points
		sd_copy.control_points += 3;

		//for(int i = 0; i<3; i++
		//sd_copy.controlx
	}

	TColgp_Array1OfPnt control (1,/*closed ? sd.controlx.size() + 1:*/sd.controlx.size());
	TColStd_Array1OfReal weight (1,sd.controlx.size());

	std::list<double> knoto;
	std::list<int> multo;

	std::list<double>::iterator ity = sd.controly.begin();
	std::list<double>::iterator itz = sd.controlz.begin();
	std::list<double>::iterator itw = sd.weight.begin();

	unsigned i=1; //int i=1;
	for(std::list<double>::iterator itx = sd.controlx.begin(); itx!=sd.controlx.end(); ++itx)
	{
		gp_Pnt pnt(*itx,*ity,*itz);
		control.SetValue(i,pnt);
		if(sd.weight.empty())
			weight.SetValue(i,1);
		else
		{
			weight.SetValue(i,*itw);
			++itw;
		}
		++i;
		++ity;
		++itz;
	}

	i=1;
	double last_knot = -1;
	for(std::list<double>::iterator it = sd.knot.begin(); it!=sd.knot.end(); ++it)
	{
		if(*it != last_knot)
		{
			knoto.push_back(*it);
			multo.push_back(1);
			i++;
		}
		else
		{
			int temp = multo.back();
			multo.pop_back();
			multo.push_back(temp+1);
		}
		last_knot = *it;
	}

	TColStd_Array1OfReal knot (1, knoto.size());
	TColStd_Array1OfInteger mult (1, knoto.size());

	std::list<int>::iterator itm = multo.begin();
	i = 1;
	for(std::list<double>::iterator it = knoto.begin(); it!=knoto.end(); ++it)
	{
		knot.SetValue(i,*it);
		int m = *itm;
		if(closed && (i == 1 || i == knoto.size()))m = 1;
		mult.SetValue(i, m);
		++itm;
		++i;
	}

    OnReadSpline(control, weight, knot, mult, sd.degree, periodic, rational);
}

void HeeksDxfRead::OnReadEllipse(const double* c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir)
{
	gp_Dir up(0, 0, 1);
	if(!dir)up = -up;
	gp_Pnt pc = make_point(c);
	gp_Elips ellipse(gp_Ax2(pc, up), major_radius, minor_radius);
	ellipse.Rotate(gp_Ax1(pc,up),rotation);
	HEllipse* new_object = new HEllipse(ellipse, ActiveColorPtr(m_aci));
	AddObject(new_object);
}

void HeeksDxfRead::OnReadText(const double *point, const double height, const wxString text)
{
    gp_Trsf trsf;
    trsf.SetTranslation( gp_Vec( gp_Pnt(0,0,0), gp_Pnt(point[0], point[1], point[2]) ) );
    trsf.SetScaleFactor( height );

    wxString txt(text);
    txt.Replace(_T("\\P"),_T("\n"),true);

    int offset = 0;
    while ((txt.Length() > 0) && (txt[0] == _T('\\')) && ((offset = txt.find(_T(';'))) != -1))
    {
        txt.Remove(0, offset+1);
    }

    HText *new_object = new HText(trsf, txt, ActiveColorPtr(m_aci), NULL);
    AddObject(new_object);
}

/**
    Don't add graphics for layer names included in this list.  There are
    some graphics packages that add setup graphics that we don't want
    to be seen.
 */
bool HeeksDxfRead::IsValidLayerName( const wxString layer_name ) const
{
    wxStringTokenizer tokens(m_layer_name_suffixes_to_discard,_T(" :;,"));
    while (tokens.HasMoreTokens())
    {
        wxString token = tokens.GetNextToken();
        if (layer_name.find(token) != -1)
        {
            return(false);  // We do NOT want this one added.
        }
    }

    return(true);   // This layername seems fine.
}

void HeeksDxfRead::AddObject(HeeksObj *object)
{

    if (! IsValidLayerName(Ctt(LayerName().c_str())))
    {
        // This is one of the forbidden layer names.  Discard the
        // graphics object and move on.

        delete object;
        return;
    }

	if(wxGetApp().m_in_OpenFile && wxGetApp().m_file_open_matrix)
	{
		object->ModifyByMatrix(wxGetApp().m_file_open_matrix);
	}

	if(m_make_as_sketch)
	{
		// Check to see if we've already added a sketch for the current layer name.  If not
		// then add one now.

		if (m_sketches.find( wxString(Ctt(LayerName().c_str())) ) == m_sketches.end())
		{
			m_sketches.insert( std::make_pair( wxString(Ctt(LayerName().c_str())), new CSketch() ) );
		}

		m_sketches[wxString(Ctt(LayerName().c_str()))]->Add( object, NULL );
	}
	else
	{
        wxGetApp().Add( object, NULL );
	}
}

void HeeksDxfRead::AddGraphics() const
{
    if (m_make_as_sketch)
    {
        for (Sketches_t::const_iterator l_itSketch = m_sketches.begin(); l_itSketch != m_sketches.end(); l_itSketch++)
        {
			CSketch *pSketch = (CSketch *)(l_itSketch->second);
			if (pSketch->GetNumChildren() > 0)
			{
				((CSketch *)l_itSketch->second)->OnEditString( l_itSketch->first.c_str() );
				wxGetApp().Add( l_itSketch->second, NULL );
			} // End if - then
        }
    }
}

