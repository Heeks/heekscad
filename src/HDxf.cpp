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
#include "HPoint.h"
#include "Sketch.h"
#include "HText.h"
#include "HeeksConfig.h"


// static
bool HeeksDxfRead::m_make_as_sketch = false;
bool HeeksDxfRead::m_ignore_errors = false;
bool HeeksDxfRead::m_read_points = false;
wxString HeeksDxfRead::m_layer_name_suffixes_to_discard = _T("_DOT,_DOTSMALL,_DOTBLANK,_OBLIQUE,_CLOSEDBLANK");

HeeksDxfRead::HeeksDxfRead(const wxChar* filepath, bool undoable) : CDxfRead(Ttc(filepath)), m_undoable(undoable)
{
	HeeksConfig config;

	config.Read(_T("ImportDxfAsSketches"), &m_make_as_sketch);
	config.Read(_T("IgnoreDxfReadErrors"), &m_ignore_errors);
	config.Read(_T("DxfReadPoints"), &m_read_points);
	config.Read(_T("LayerNameSuffixesToDiscard"), m_layer_name_suffixes_to_discard);

	m_current_block = NULL;
	extract(gp_Trsf(), m_ucs_matrix);
}

HeeksColor *HeeksDxfRead::ActiveColorPtr(Aci_t & aci)
{
	static HeeksColor color;
	color = HeeksColor(aci);
	return(&color);
}

HeeksColor hidden_color(128, 128, 128);

void HeeksDxfRead::OnReadUCS(const double* ucs_point)
{
	gp_Trsf tm;
	tm.SetTranslation(make_point(ucs_point), gp_Pnt(0, 0, 0));
	extract(tm, m_ucs_matrix);
}

void HeeksDxfRead::OnReadBlock(const wxString& block_name, const double* base_point)
{
	if (m_blocks.find(block_name) == m_blocks.end())
	{
		m_current_block = new CSketch();
		m_current_block->OnEditString(block_name);
		m_blocks.insert( std::make_pair( block_name, m_current_block ) );
	}
	else
	{
		m_current_block = m_blocks[block_name];
	}
}

void HeeksDxfRead::OnReadInsert(const wxString& block_name, const double* insert_point, double rotation_angle)
{
	if (m_blocks.find(block_name) == m_blocks.end())
	{
		return; // block not foundblock_name, const double* insert_point, double rotation_angle
	}
	else
	{
		BlockName_t b_name = block_name;
#if 0
		// insert an insert object. To be expanded at the end
		HInsert* new_object = new HInsert(block_name, insert_point, rotation_angle);
		AddObject(new_object);
#else
		CSketch* block = m_blocks[b_name];
		CSketch* block_copy = new CSketch(*block);
		gp_Trsf tm;
		tm.SetTranslationPart(make_vector(insert_point));
		gp_Trsf rm;
		rm.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), rotation_angle * 0.01745329251994329);
		double m[16];
		extract(tm * rm, m);
		block_copy->ModifyByMatrix(m);
		AddObject(block_copy);
#endif
		inserted_blocks.insert(b_name);
	}
}

void HeeksDxfRead::OnReadEndBlock()
{
	m_current_block = NULL;
}

void HeeksDxfRead::OnReadLine(const double* s, const double* e, bool hidden)
{
	HLine* new_object = new HLine(make_point(s), make_point(e), hidden ? (&hidden_color) : ActiveColorPtr(m_aci));
	AddObject(new_object);
}

void HeeksDxfRead::OnReadPoint(const double* s)
{
	if(m_read_points)
	{
		HPoint* new_object = new HPoint(make_point(s), ActiveColorPtr(m_aci));
		AddObject(new_object);
	}
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
	HEllipse* new_object = new HEllipse(ellipse, start_angle, end_angle, ActiveColorPtr(m_aci));
	AddObject(new_object);
}

	#define Slice(str, start, end) (str.Mid(start, end))
//Split (Tokenize) string at specified intervals
	//s == string to split
	//retArray == split up string (out)
	//cpszExp == expression to split at
	//crnStart == start postion to split
	//crnCount == max number of split of strings
	//crbCIComp == true if case insensitive
	void Split( const wxString& s, wxArrayString& retArray,  const wxChar* cpszExp, 
				const size_t& crnStart = 0, const size_t& crnCount = (size_t)-1,
				const bool& crbCIComp = false)
	{
		//sanity checks
		wxASSERT_MSG(cpszExp != NULL, wxT("Invalid value for First Param of wxString::Split (cpszExp)"));
		//wxASSERT_MSG(crnCount >= (size_t)-1, wxT("Invalid value for Third Param of wxString::Split (crnCount)"));

		retArray.Clear();

		size_t  nOldPos = crnStart,	  //Current start position in this string
				nPos = crnStart;	  //Current end position in this string

		wxString szComp,			//this string as-is (if bCIComp is false) or converted to lowercase
				 szExp = cpszExp;   //Expression string, normal or lowercase

		if (crbCIComp)
		{
			szComp = s.Lower();
			szExp.MakeLower();
		}
		else
			szComp = s;

		if(crnCount == (size_t)-1)
		{
		for (; (nPos = szComp.find(szExp, nPos)) != wxString::npos;)//Is there another token in the string
			{
			retArray.Add(Slice(s, nOldPos, nPos)); //Insert the token in the array
			nOldPos = nPos += szExp.Length();//Move up the start slice position
			}
	   
		}
		else
		{
		for (int i = crnCount;
				(nPos = szComp.find(szExp, nPos)) != wxString::npos &&
				i != 0;
					--i)//Is there another token in the string && have we met nCount?
		{
			retArray.Add(Slice(s, nOldPos, nPos)); //Insert the token in the array
			nOldPos = nPos += szExp.Length();//Move up the start slice position
		}
		}
		if (nOldPos != s.Length())
			retArray.Add( Slice(s, nOldPos, s.Length()) ); //Add remaining characters in string
	}

void HeeksDxfRead::OnReadText(const double *point, const double height,  const char* text, int hj, int vj)
{
	gp_Trsf trsf;
	trsf.SetTranslation( gp_Vec( gp_Pnt(0,0,0), gp_Pnt(point[0], point[1], point[2]) ) );
	trsf.SetScaleFactor( height * 1.7 );

	wxString txt(Ctt(text));
	txt.Replace(_T("\\P"),_T("\n"),true);
	txt.Replace(_T("%%010"),_T("\n"),true);

	int offset = 0;
	while ((txt.Length() > 0) && (txt[0] == _T('\\')) && ((offset = txt.find(_T(';'))) != -1))
	{
		txt.Remove(0, offset+1);
	}

	wxArrayString retArray;
	Split(txt, retArray, _T("\n"));

	gp_Trsf line_feed_shift;
	line_feed_shift.SetTranslationPart(gp_Vec(0, -height, 0));

	for(unsigned int i = 0; i<retArray.GetCount(); i++)
	{
		HText *new_object = new HText(trsf, retArray[i], ActiveColorPtr(m_aci),
#ifndef WIN32
			NULL, 
#endif
			hj, vj);
		AddObject(new_object);
		trsf = trsf * line_feed_shift;
	}
}

void HeeksDxfRead::OnReadDimension(int dimension_type, double angle, double angle2, double angle3, double radius_leader_length, const double *def_point, const double *mid, const double *p1, const double *p2, const double *p3, const double *p4, const double *p5)
{
	int type = (dimension_type & 0x07);

	gp_Pnt d = make_point(def_point);
	gp_Pnt m = make_point(mid);

	gp_Vec d_to_m = make_vector(d, m);

	gp_Pnt e = gp_Pnt(m.XYZ() + d_to_m.XYZ());

	gp_Dir forward(1,0,0);

	HeeksColor c(0, 0, 0);

	if(type == 0)
	{
		forward.Rotate(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), (angle/* + angle2 + angle3*/) * -0.01745329251);
	}
	else
	{
		forward = gp_Dir(-d_to_m);
		c = HeeksColor(255, 0, 0);
	}

	gp_Dir left(-forward.Y(), forward.X(), 0.0);

	double arrow_size = 5.0;
	gp_Pnt da1 = d.XYZ() + forward.XYZ() * (-arrow_size) + left.XYZ() * (arrow_size * 0.25);
	gp_Pnt da2 = d.XYZ() + forward.XYZ() * (-arrow_size) + left.XYZ() * (arrow_size * -0.25);

	gp_Pnt ea1 = e.XYZ() + forward.XYZ() * (arrow_size) + left.XYZ() * (arrow_size * 0.25);
	gp_Pnt ea2 = e.XYZ() + forward.XYZ() * (arrow_size) + left.XYZ() * (arrow_size * -0.25);

	AddObject(new HLine(d, da1, &c));
	AddObject(new HLine(da1, da2, &c));
	AddObject(new HLine(da2, d, &c));

	AddObject(new HLine(e, ea1, &c));
	AddObject(new HLine(ea1, ea2, &c));
	AddObject(new HLine(ea2, e, &c));
}

/**
	Don't add graphics for layer names included in discard-list:
	There are some graphics packages that add setup graphics that we don't want
	to be seen.
	This function discard names presents in m_layer_name_suffixes_to_discard
	(default: _DOT,_DOTSMALL,_DOTBLANK,_OBLIQUE,_CLOSEDBLANK) 
 */
bool HeeksDxfRead::IsValidLayerName( const wxString layer_name ) const
{
	wxStringTokenizer tokens(m_layer_name_suffixes_to_discard,_T(" :;,"));
	while (tokens.HasMoreTokens())
	{
		wxString token = tokens.GetNextToken();
		if (layer_name.Find(token) != wxNOT_FOUND)
		{
			return(false);  // We do NOT want this one added.
		}
	}

	return(true);   // This layername seems fine.
}

void HeeksDxfRead::AddObject(HeeksObj *object)
{
	if (! IsValidLayerName(LayerName()))
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

		if (m_sketches.find(LayerName()) == m_sketches.end())
		{
			m_sketches.insert( std::make_pair( LayerName(), new CSketch() ) );
		}

		if(m_current_block)m_current_block->Add(object, NULL);
		else
		{
			object->ModifyByMatrix(m_ucs_matrix);
			m_sketches[LayerName()]->Add( object, NULL );
		}
	}
	else
	{
		if(m_current_block)m_current_block->Add(object, NULL);
		else
		{
			object->ModifyByMatrix(m_ucs_matrix);
			if(m_undoable)wxGetApp().AddUndoably(object, NULL, NULL);
			else wxGetApp().Add( object, NULL );
		}
	}
}

void HeeksDxfRead::AddGraphics()
{
	// add one insert of any blocks which haven't been added at all
	for(Blocks_t::const_iterator It = m_blocks.begin(); It != m_blocks.end(); It++)
	{
		if(inserted_blocks.find(It->first) == inserted_blocks.end())
		{
			CSketch* block = It->second;
			if(block->GetNumChildren() > 0)
			{
				CSketch* block_copy = new CSketch(*(It->second));
				AddObject(block_copy);
			}
		}
	}

	if (m_make_as_sketch)
	{
		for (Sketches_t::const_iterator l_itSketch = m_sketches.begin(); l_itSketch != m_sketches.end(); l_itSketch++)
		{
			CSketch *pSketch = (CSketch *)(l_itSketch->second);
			if (pSketch->GetNumChildren() > 0)
			{
				((CSketch *)l_itSketch->second)->OnEditString( l_itSketch->first.c_str() );
				l_itSketch->second->ModifyByMatrix(m_ucs_matrix);
				if(m_undoable)wxGetApp().AddUndoably(l_itSketch->second, NULL, NULL );
				else wxGetApp().Add( l_itSketch->second, NULL );
			} // End if - then
		}
	}
}

