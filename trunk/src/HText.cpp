// HText.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "HText.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyChoice.h"
#include "PropertyTrsf.h"
#include "Gripper.h"
#include "CxfFont.h"

HText::HText(const gp_Trsf &trsf, const wxString &text, const HeeksColor* col, CxfFont *pFont):m_color(*col),  m_trsf(trsf), m_text(text), m_pFont(pFont)
{
}

HText::HText(const HText &b)
{
	operator=(b);
}

HText::~HText(void)
{
}

const HText& HText::operator=(const HText &b)
{
	HeeksObj::operator=(b);
	m_trsf = b.m_trsf;
	m_text = b.m_text;
	m_color = b.m_color;
	m_pFont = b.m_pFont;
	return *this;
}

void HText::glCommands(bool select, bool marked, bool no_color)
{
	glPushMatrix();
	double m[16];
	extract_transposed(m_trsf, m);
	glMultMatrixd(m);

	if(!no_color)wxGetApp().glColorEnsuringContrast(m_color);

	if (m_pFont == NULL)
	{
		// We're using the internal font
		wxGetApp().render_text(m_text);
	} // End if - then
	else
	{
		// We're using the CxfFonts
		m_pFont->glCommands( m_text, gp_Pnt(0.0, 0.0, 0.0), select, marked, no_color );
	} // End if - else

	glPopMatrix();
}

bool HText::GetTextSize( const wxString & text, float *pWidth, float *pHeight ) const
{
	if (m_pFont == NULL)
	{
		// We're using the internal font.
		if(!wxGetApp().get_text_size(text, pWidth, pHeight))return(false);
	} // End if - then
	else
	{
		// We're using one of the CxfFonts
		if (! m_pFont->get_text_size(text, pWidth, pHeight)) return(false);
	} // End if - else

	return(true);
} // End GetTextSize() method

void HText::GetBox(CBox &box)
{
	gp_Pnt vt(0, 0, 0);
	vt.Transform(m_trsf);
	double p[3];
	extract(vt, p);
	box.Insert(p);

	float width, height;
	if (! GetTextSize( m_text, &width, &height )) return;

	gp_Pnt point[3];
	point[0] = gp_Pnt(width, 0, 0);
	point[1] = gp_Pnt(0, -height, 0);
	point[2] = gp_Pnt(width, -height, 0);

	for(int i = 0; i<3; i++)
	{
		point[i].Transform(m_trsf);
		extract(point[i], p);
		box.Insert(p);
	}
}

HeeksObj *HText::MakeACopy(void)const
{
	return new HText(*this);
}

bool HText::ModifyByMatrix(const double *m)
{
	gp_Trsf mat = make_matrix(m);
	m_trsf = mat * m_trsf;
	return false;
}

void HText::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	float width, height;
	if(! GetTextSize(m_text, &width, &height))return;

	gp_Pnt point[4];
	point[0] = gp_Pnt(0, 0, 0);
	point[1] = gp_Pnt(width, 0, 0);
	point[2] = gp_Pnt(0, -height, 0);
	point[3] = gp_Pnt(width, -height, 0);

	for(int i = 0; i<4; i++)point[i].Transform(m_trsf);

	list->push_back(GripData(GripperTypeTranslate,point[0].X(),point[0].Y(),point[0].Z(),NULL));
	list->push_back(GripData(GripperTypeRotateObject,point[1].X(),point[1].Y(),point[1].Z(),NULL));
	list->push_back(GripData(GripperTypeRotateObject,point[2].X(),point[2].Y(),point[2].Z(),NULL));
	list->push_back(GripData(GripperTypeScale,point[3].X(),point[3].Y(),point[3].Z(),NULL));
}

static void on_set_trsf(const gp_Trsf &trsf, HeeksObj* object){
	((HText*)object)->m_trsf = trsf;
	wxGetApp().Repaint();
}

static void on_set_font(int zero_based_choice, HeeksObj *obj)
{
	if (zero_based_choice == 0)
	{
		((HText*)obj)->m_pFont = NULL;
		return;
	} // End if - then

	std::set<wxString> names = wxGetApp().GetAvailableFonts()->FontNames();
	std::vector<wxString> vector_names;
	vector_names.push_back(_("OpenGL"));	// Keep the zero-based offset.
	std::copy( names.begin(), names.end(), std::inserter( vector_names, vector_names.end() ) );
	if (zero_based_choice < int(vector_names.size()))
	{
		((HText*)obj)->m_pFont = wxGetApp().GetAvailableFonts()->Font( CxfFont::Name_t(vector_names[zero_based_choice].c_str()) );
	}
	wxGetApp().WasModified((HText*)obj);
}

void HText::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyTrsf(_("orientation"), m_trsf, this, on_set_trsf));

	if (wxGetApp().m_pCxfFonts.get() != NULL)
	{
		std::list<wxString> choices;

		choices.push_back( wxString(_("OpenGL (default) font")) );
		int choice = 0;

		int option = 0;
		std::set<CxfFont::Name_t> font_names = wxGetApp().m_pCxfFonts->FontNames();
		for (std::set<CxfFont::Name_t>::const_iterator l_itFontName = font_names.begin();
			l_itFontName != font_names.end(); l_itFontName++)
		{
			option++;
			choices.push_back( *l_itFontName );
			if ((m_pFont != NULL) && (m_pFont->Name() == *l_itFontName)) choice = option;
		} // End for
		list->push_back ( new PropertyChoice ( _("Font"),  choices, choice, this, on_set_font ) );
	}

	HeeksObj::GetProperties(list);
}

bool HText::Stretch(const double *p, const double* shift, void* data)
{
	return false;
}

void HText::OnEditString(const wxChar* str){
	m_text.assign(str);
	wxGetApp().WasModified(this);
}

void HText::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Text" );
	root->LinkEndChild( element );  
	element->SetAttribute("text", Ttc(m_text.c_str()));

	if (m_pFont == NULL)
	{
		element->SetAttribute("font", Ttc(_("OpenGL")));
	} // End if - then
	else
	{
		element->SetAttribute("font", Ttc(m_pFont->Name().c_str()));
	} // End if - else

	double m[16];
	extract(m_trsf, m);

	element->SetAttribute("col", m_color.COLORREF_color());
	element->SetDoubleAttribute("m0", m[0] );
	element->SetDoubleAttribute("m1", m[1] );
	element->SetDoubleAttribute("m2", m[2] );
	element->SetDoubleAttribute("m3", m[3] );
	element->SetDoubleAttribute("m4", m[4] );
	element->SetDoubleAttribute("m5", m[5] );
	element->SetDoubleAttribute("m6", m[6] );
	element->SetDoubleAttribute("m7", m[7] );
	element->SetDoubleAttribute("m8", m[8] );
	element->SetDoubleAttribute("m9", m[9] );
	element->SetDoubleAttribute("ma", m[10]);
	element->SetDoubleAttribute("mb", m[11]);

	WriteBaseXML(element);
}

// static
HeeksObj* HText::ReadFromXMLElement(TiXmlElement* pElem)
{
	double m[16];
	wxString text;
	HeeksColor c;
	wxString font_name;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());	
		if(name == "col"){c = HeeksColor(a->IntValue());}
		else if(name == "text"){text.assign(Ctt(a->Value()));}
		else if(name == "m0"){m[0] = a->DoubleValue();}
		else if(name == "m1"){m[1] = a->DoubleValue();}
		else if(name == "m2"){m[2] = a->DoubleValue();}
		else if(name == "m3"){m[3] = a->DoubleValue();}
		else if(name == "m4"){m[4] = a->DoubleValue();}
		else if(name == "m5"){m[5] = a->DoubleValue();}
		else if(name == "m6"){m[6] = a->DoubleValue();}
		else if(name == "m7"){m[7] = a->DoubleValue();}
		else if(name == "m8"){m[8] = a->DoubleValue();}
		else if(name == "m9"){m[9] = a->DoubleValue();}
		else if(name == "ma"){m[10]= a->DoubleValue();}
		else if(name == "mb"){m[11]= a->DoubleValue();}
		else if(name == "font") { font_name.assign( Ctt(a->Value()) ); }
	}

	CxfFont *pCxfFont = NULL;
	if (wxGetApp().GetAvailableFonts().get() != NULL)
	{
		pCxfFont = wxGetApp().GetAvailableFonts()->Font( font_name );
	}

	HText* new_object = new HText(make_matrix(m), text, &c, pCxfFont );
	new_object->ReadBaseXML(pElem);

	return new_object;
}

HText *pTextForSketchTool = NULL;

class TextToSketch:public Tool{
public:
	void Run(){
		HeeksObj *sketch = pTextForSketchTool->m_pFont->Sketch( pTextForSketchTool->m_text, pTextForSketchTool->m_trsf );
		wxGetApp().AddUndoably( sketch, NULL, NULL );
		wxGetApp().DeleteUndoably( pTextForSketchTool );
		pTextForSketchTool = NULL;
	}
	const wxChar* GetTitle(){return _T("Make a Sketch");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Convert the lines and arcs that make up this text into a sketch object.");}
};
static TextToSketch text_to_sketch;

void HText::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	pTextForSketchTool = this;

	if (m_pFont != NULL)
	{
		// It's only possible for the CXF fonts at the moment.
		t_list->push_back(&text_to_sketch);
	} // End if - then
}

