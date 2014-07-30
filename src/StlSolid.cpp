// StlSolid.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "StlSolid.h"

using namespace std;

CStlTri::CStlTri(const float* t)
{
	memcpy(x[0], t, 9*sizeof(float));
}

CStlTri::CStlTri(const double* t)
{
	x[0][0] = (float)t[0];
	x[0][1] = (float)t[1];
	x[0][2] = (float)t[2];
	x[1][0] = (float)t[3];
	x[1][1] = (float)t[4];
	x[1][2] = (float)t[5];
	x[2][0] = (float)t[6];
	x[2][1] = (float)t[7];
	x[2][2] = (float)t[8];
}

CStlSolid::CStlSolid(const HeeksColor* col):m_color(*col), m_gl_list(0){
	m_title.assign(GetTypeString());
}

CStlSolid::CStlSolid():m_color(wxGetApp().current_color), m_gl_list(0){
	m_title.assign(GetTypeString());
}

#ifdef UNICODE
// constructor for the Boost Python interface
CStlSolid::CStlSolid(const std::wstring& filepath):m_color(wxGetApp().current_color), m_gl_list(0){
	m_title.assign(GetTypeString());
	read_from_file(filepath.c_str());

	if(wxGetApp().m_in_OpenFile && wxGetApp().m_file_open_matrix)
	{
		ModifyByMatrix(wxGetApp().m_file_open_matrix);
	}
}
#endif

CStlSolid::CStlSolid(const wxChar* filepath, const HeeksColor* col):m_color(*col), m_gl_list(0){
	m_title.assign(GetTypeString());
	read_from_file(filepath);

	if(wxGetApp().m_in_OpenFile && wxGetApp().m_file_open_matrix)
	{
		ModifyByMatrix(wxGetApp().m_file_open_matrix);
	}
}

void CStlSolid::read_from_file(const wxChar* filepath)
{
	// read the stl file
	ifstream ifs(Ttc(filepath), ios::binary);
	if(!ifs)return;

	char solid_string[6] = "aaaaa";
	ifs.read(solid_string, 5);
	if(ifs.eof())return;
	if(strcmp(solid_string, "solid"))
	{
		// try binary file read

		// read the header
		char header[81];
		header[80] = 0;
		memcpy(header, solid_string, 5);
		ifs.read(&header[5], 75);

		unsigned int num_facets = 0;
		ifs.read((char*)(&num_facets), 4);

		for(unsigned int i = 0; i<num_facets; i++)
		{
			CStlTri tri;
			float n[3];
			ifs.read((char*)(n), 12);
			ifs.read((char*)(tri.x[0]), 36);
			short attr;
			ifs.read((char*)(&attr), 2);
			m_list.push_back(tri);
		}
	}
	else
	{
		// "solid" already found
		char str[1024] = "solid";
		ifs.getline(&str[5], 1024);
		char title[1024];
		if(sscanf(str, "solid %s", title) == 1)
			m_title.assign(Ctt(title));

		CStlTri t;
		char five_chars[6] = "aaaaa";

		int vertex = 0;

		while(!ifs.eof())
		{
			ifs.getline(str, 1024);

			int i = 0, j = 0;
			for(; i<5; i++, j++)
			{
				if(str[j] == 0)break;
				while(str[j] == ' ' || str[j] == '\t')j++;
				five_chars[i] = str[j];
			}
			if(i == 5)
			{
				if(!strcmp(five_chars, "verte"))
				{
#ifdef WIN32
					sscanf(str, " vertex %f %f %f", &(t.x[vertex][0]), &(t.x[vertex][1]), &(t.x[vertex][2]));
#else
					std::istringstream ss(str);
					ss.imbue(std::locale("C"));
					while(ss.peek() == ' ') ss.seekg(1, ios_base::cur);
					ss.seekg(std::string("vertex").size(), ios_base::cur);
					ss >> t.x[vertex][0] >> t.x[vertex][1] >> t.x[vertex][2];
#endif
					vertex++;
					if(vertex > 2)vertex = 2;
				}
				else if(!strcmp(five_chars, "facet"))
				{
					vertex = 0;
				}
				else if(!strcmp(five_chars, "endfa"))
				{
					if(vertex == 2)
					{
						m_list.push_back(t);
					}
				}
			}
		}
	}
}

CStlSolid::~CStlSolid(){
	KillGLLists();
}

const CStlSolid& CStlSolid::operator=(const CStlSolid& s)
{
    HeeksObj::operator = (s);

	// don't copy id
	m_box = s.m_box;
	m_title = s.m_title;
	KillGLLists();

	m_color = s.m_color;

	m_list.clear();
	std::copy( s.m_list.begin(), s.m_list.end(), std::inserter( m_list, m_list.begin() ));

	return *this;
}

bool CStlSolid::IsDifferent(HeeksObj* other)
{
	CStlSolid* shape = (CStlSolid*)other;
	if(shape->m_color.COLORREF_color() != m_color.COLORREF_color() || shape->m_title.CompareTo(m_title) || shape->m_box != m_box)
		return true;

	return HeeksObj::IsDifferent(other);
}


void CStlSolid::KillGLLists()
{
	if (m_gl_list)
	{
		glDeleteLists(m_gl_list, 1);
		m_gl_list = 0;
		m_box = CBox();
	}
}

const wxBitmap &CStlSolid::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/stlsolid.png")));
	return *icon;
}

void CStlSolid::glCommands(bool select, bool marked, bool no_color){
	glEnable(GL_LIGHTING);
	Material(m_color).glMaterial(1.0);

	if(m_gl_list)
	{
		glCallList(m_gl_list);
	}
	else{
		m_gl_list = glGenLists(1);
		glNewList(m_gl_list, GL_COMPILE_AND_EXECUTE);

		// render all the triangles
		glBegin(GL_TRIANGLES);
		for(std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++)
		{
			CStlTri &t = *It;
			gp_Pnt p0(t.x[0][0], t.x[0][1], t.x[0][2]);
			gp_Pnt p1(t.x[1][0], t.x[1][1], t.x[1][2]);
			gp_Pnt p2(t.x[2][0], t.x[2][1], t.x[2][2]);
			gp_Vec v1(p0, p1);
			gp_Vec v2(p0, p2);
			try
			{
				gp_Vec norm = (v1 ^ v2).Normalized();
				float n[3];
				n[0] = (float)(norm.X());
				n[1] = (float)(norm.Y());
				n[2] = (float)(norm.Z());
				glNormal3fv(n);
				glVertex3fv(t.x[0]);
				glVertex3fv(t.x[1]);
				glVertex3fv(t.x[2]);
			}
			catch(...)
			{
			}
		}
		glEnd();

		glEndList();
	}

	glDisable(GL_LIGHTING);
}

void CStlSolid::GetBox(CBox &box){
	if(!m_box.m_valid)
	{
		// calculate the box for all the triangles
		for(std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++)
		{
			CStlTri &t = *It;
			m_box.Insert(t.x[0][0], t.x[0][1], t.x[0][2]);
			m_box.Insert(t.x[1][0], t.x[1][1], t.x[1][2]);
			m_box.Insert(t.x[2][0], t.x[2][1], t.x[2][2]);
		}
	}

	box.Insert(m_box);
}

void CStlSolid::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	for(std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++)
	{
		CStlTri &t = *It;
		for(int i = 0; i<3; i++){
			gp_Pnt vx;
			vx = gp_Pnt(t.x[i][0], t.x[i][1], t.x[i][2]);
			vx.Transform(mat);
			t.x[i][0] = (float)vx.X();
			t.x[i][1] = (float)vx.Y();
			t.x[i][2] = (float)vx.Z();
		}
	}

	KillGLLists();
}

CStlSolid::CStlSolid( const CStlSolid & rhs ) : m_gl_list(0)
{
    *this = rhs;    // Call the assignment operator.
}

HeeksObj *CStlSolid::MakeACopy(void)const{
	CStlSolid *new_object = new CStlSolid(*this);
	return new_object;
}

void CStlSolid::CopyFrom(const HeeksObj* object)
{
	operator=(*((CStlSolid*)object));
}

void CStlSolid::OnEditString(const wxChar* str){
	m_title.assign(str);
	wxGetApp().Changed();
}

void CStlSolid::GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal){
	double x[9];
	double n[9];
	for(std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++)
	{
		CStlTri &t = *It;
		x[0] = t.x[0][0];
		x[1] = t.x[0][1];
		x[2] = t.x[0][2];
		x[3] = t.x[1][0];
		x[4] = t.x[1][1];
		x[5] = t.x[1][2];
		x[6] = t.x[2][0];
		x[7] = t.x[2][1];
		x[8] = t.x[2][2];
		gp_Pnt p0(t.x[0][0], t.x[0][1], t.x[0][2]);
		gp_Pnt p1(t.x[1][0], t.x[1][1], t.x[1][2]);
		gp_Pnt p2(t.x[2][0], t.x[2][1], t.x[2][2]);
		gp_Vec v1(p0, p1);
		gp_Vec v2(p0, p2);
		try
		{
			gp_Vec norm = (v1 ^ v2).Normalized();
			n[0] = norm.X();
			n[1] = norm.Y();
			n[2] = norm.Z();
			if(!just_one_average_normal)
			{
				n[3] = n[0];
				n[4] = n[1];
				n[5] = n[2];
				n[6] = n[0];
				n[7] = n[1];
				n[8] = n[2];
			}
			(*callbackfunc)(x, n);
		}
		catch(...)
		{
		}
	}
}

void CStlSolid::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "STLSolid" );
	root->LinkEndChild( element );
	element->SetAttribute("col", m_color.COLORREF_color());

	for(std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++)
	{
		CStlTri &t = *It;
		TiXmlElement * child_element;
		child_element = new TiXmlElement( "tri" );
		element->LinkEndChild( child_element );
		child_element->SetDoubleAttribute("p1x", t.x[0][0]);
		child_element->SetDoubleAttribute("p1y", t.x[0][1]);
		child_element->SetDoubleAttribute("p1z", t.x[0][2]);
		child_element->SetDoubleAttribute("p2x", t.x[1][0]);
		child_element->SetDoubleAttribute("p2y", t.x[1][1]);
		child_element->SetDoubleAttribute("p2z", t.x[1][2]);
		child_element->SetDoubleAttribute("p3x", t.x[2][0]);
		child_element->SetDoubleAttribute("p3y", t.x[2][1]);
		child_element->SetDoubleAttribute("p3z", t.x[2][2]);
	}

	WriteBaseXML(element);
}


// static member function
HeeksObj* CStlSolid::ReadFromXMLElement(TiXmlElement* pElem)
{
	HeeksColor c;
	CStlTri t;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){c = HeeksColor((long)(a->IntValue()));}
	}

	CStlSolid* new_object = new CStlSolid(&c);

	// loop through all the "tri" objects
	double x[3][3];

	for(TiXmlElement* pTriElem = TiXmlHandle(pElem).FirstChildElement().Element(); pTriElem;	pTriElem = pTriElem->NextSiblingElement())
	{
		// get the attributes
		pTriElem->Attribute("p1x", &x[0][0]);
		pTriElem->Attribute("p1y", &x[0][1]);
		pTriElem->Attribute("p1z", &x[0][2]);
		pTriElem->Attribute("p2x", &x[1][0]);
		pTriElem->Attribute("p2y", &x[1][1]);
		pTriElem->Attribute("p2z", &x[1][2]);
		pTriElem->Attribute("p3x", &x[2][0]);
		pTriElem->Attribute("p3y", &x[2][1]);
		pTriElem->Attribute("p3z", &x[2][2]);
		new_object->m_list.push_back(CStlTri(&x[0][0]));
	}

	new_object->ReadBaseXML(pElem);

	return new_object;
}

void CStlSolid::AddTriangle(float* t)
{
	CStlTri tri(t);
	m_list.push_back(tri);
}

