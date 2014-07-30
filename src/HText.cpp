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
#include "OrientationModifier.h"

HText::HText(const gp_Trsf &trsf, const wxString &text, const HeeksColor* col, VectorFont *pFont):m_color(*col),  m_trsf(trsf), m_text(text), m_pFont(pFont)
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
    if (this != &b)
    {
        ObjList::operator=(b);
        m_trsf = b.m_trsf;
        m_text = b.m_text;
        m_color = b.m_color;
        m_pFont = b.m_pFont;
    }

	return *this;
}

const wxBitmap &HText::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/text.png")));
	return *icon;
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
		// Look to see if we have an OrientationModifier object as a child.  If so, pass it in
		COrientationModifier *pOrientationModifier = NULL;
		for (HeeksObj *child = GetFirstChild(); child != NULL; child = GetNextChild())
		{
			if ((child) && (child->GetType() == OrientationModifierType))
			{
				pOrientationModifier = (COrientationModifier *)child;
				break;
			}
		}

		float height, width;
		GetTextSize( m_text, &width, &height );
		m_pFont->glCommands( m_text, gp_Pnt(0.0, 0.0, 0.0), select, marked, no_color, pOrientationModifier, m_trsf, width );
	} // End if - else

	glPopMatrix();

	ObjList::glCommands(select, marked, no_color);
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

void HText::ModifyByMatrix(const double *m)
{
	gp_Trsf mat = make_matrix(m);
	m_trsf = mat * m_trsf;
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

	if (m_pFont != NULL)
	{
		// We're using the vector fonts.  These have the opposite meanings for the Y axis values.
            for (::size_t i=0; i<sizeof(point)/sizeof(point[0]); i++)
            {
                point[i].SetY( point[i].Y() * -1.0 );
            }
	} // End if - then

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
	vector_names.push_back(_T("OpenGL"));	// Keep the zero-based offset.
	std::copy( names.begin(), names.end(), std::inserter( vector_names, vector_names.end() ) );
	if (zero_based_choice < int(vector_names.size()))
	{
		((HText*)obj)->m_pFont = wxGetApp().GetAvailableFonts()->Font( VectorFont::Name_t(vector_names[zero_based_choice].c_str()) );
	}
	wxGetApp().Changed();
}

void HText::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyTrsf(_("orientation"), m_trsf, this, on_set_trsf));

    if (wxGetApp().m_pVectorFonts.get() == NULL)
    {
        wxGetApp().GetAvailableFonts();
    }

    if (wxGetApp().m_pVectorFonts.get() != NULL)
	{
		std::list<wxString> choices;

		choices.push_back( wxString(_("OpenGL (default) font")) );
		int choice = 0;

		int option = 0;
		std::set<VectorFont::Name_t> font_names = wxGetApp().m_pVectorFonts->FontNames();
		for (std::set<VectorFont::Name_t>::const_iterator l_itFontName = font_names.begin();
			l_itFontName != font_names.end(); l_itFontName++)
		{
			option++;
			choices.push_back( *l_itFontName );
			if ((m_pFont != NULL) && (m_pFont->Name() == *l_itFontName)) choice = option;
		} // End for
		list->push_back ( new PropertyChoice ( _("Font"),  choices, choice, this, on_set_font ) );
	}

	ObjList::GetProperties(list);
}

bool HText::Stretch(const double *p, const double* shift, void* data)
{
	return false;
}

void HText::OnEditString(const wxChar* str){
	m_text.assign(str);
	wxGetApp().Changed();
}

void HText::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Text" );
	root->LinkEndChild( element );
	element->SetAttribute("text", Ttc(m_text) );

	if (m_pFont == NULL)
	{
		element->SetAttribute("font", "OpenGL");
	} // End if - then
	else
	{
		element->SetAttribute("font", Ttc(m_pFont->Name()));
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
		if(name == "col"){c = HeeksColor((long)(a->IntValue()));}
		else if(name == "text")	{ text.assign( Ctt(a->Value()) ); }
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

	VectorFont *pVectorFont = NULL;
	if (wxGetApp().GetAvailableFonts().get() != NULL)
	{
		pVectorFont = wxGetApp().GetAvailableFonts()->Font( font_name );
	}

	HText* new_object = new HText(make_matrix(m), text, &c, pVectorFont );
	new_object->ReadBaseXML(pElem);

	return new_object;
}

bool HText::CanAdd(HeeksObj* object)
{
	if (object == NULL) return(false);
	if (GetNumChildren() > 0)
	{
		wxMessageBox(_("Only a single orientation modifier is supported"));
		return(false);
	}

	if (object->GetType() == OrientationModifierType) return(true);
	return(false);
}


HText *pTextForSketchTool = NULL;

class TextToSketch:public Tool{
public:
	void Run(){
	    // See if this text object has an OrientationModifier child.
	    COrientationModifier *pOrientationModifier = NULL;
	    for (HeeksObj *child = pTextForSketchTool->GetFirstChild(); child != NULL; child = pTextForSketchTool->GetNextChild())
	    {
	        if (child->GetType() == OrientationModifierType)
	        {
	            pOrientationModifier = (COrientationModifier *) child;
	            break;
	        }
	    } // End for

		float height, width;
		pTextForSketchTool->GetTextSize( pTextForSketchTool->m_text, &width, &height );
		HeeksObj *sketch = pTextForSketchTool->m_pFont->Sketch( pTextForSketchTool->m_text, pTextForSketchTool->m_trsf, width, pOrientationModifier );
		wxGetApp().Add( sketch, NULL);
		wxGetApp().Remove( pTextForSketchTool );
		pTextForSketchTool = NULL;
	}
	const wxChar* GetTitle(){return _("Make a Sketch");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Convert the lines and arcs that make up this text into a sketch object.");}
};
static TextToSketch text_to_sketch;


HText *pOrientationForTextTool = NULL;

class AddOrientationModifierTool:public Tool{
public:
	void Run(){
		HeeksObj *orientation = new COrientationModifier();
		pOrientationForTextTool->Add( orientation, NULL);
		pOrientationForTextTool = NULL;
	}
	const wxChar* GetTitle(){return _("Add Orientation Modifier");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Add graphics to orient text");}
};
static AddOrientationModifierTool orientation_tool;



void HText::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	pTextForSketchTool = this;
	pOrientationForTextTool = this;

	if (GetNumChildren() == 0)
	{
	   // t_list->push_back(&orientation_tool); // Not yet.  It produces a mess.  Need to get the Get() method working properly.
	}

	if (m_pFont != NULL)
	{
		// It's only possible for the CXF fonts at the moment.
		t_list->push_back(&text_to_sketch);
	} // End if - then
}

