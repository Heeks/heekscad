// StlSolid.cpp
#include "stdafx.h"
#include "StlSolid.h"

#include "../tinyxml/tinyxml.h"

#include <fstream>

using namespace std;

CStlSolid::CStlSolid(const HeeksColor* col):color(*col), m_gl_list(0){
	m_title.assign(GetTypeString());
}

CStlSolid::CStlSolid(const wxChar* filepath, const HeeksColor* col):color(*col), m_gl_list(0){
	// read the stl file
#if wxUSE_UNICODE
#ifdef __WXMSW__
	wifstream ifs(filepath);
#else
	wifstream ifs(Ttc(filepath));
#endif
#else
	ifstream ifs(filepath);
#endif
	if(!ifs)return;

	wxChar solid_string[6] = _T("aaaaa");
	ifs.read(solid_string, 5);
	if(ifs.eof())return;
#if wxUSE_UNICODE
	if(wcscmp(solid_string, _T("solid")))
#else
	if(strcmp(solid_string, _T("solid")))
#endif
	{
		// try binary file read

		// read the header
		wxChar header[81];
		header[80] = 0;
		memcpy(header, solid_string, 5);
		ifs.read(&header[5], 75);

		unsigned int num_facets = 0;
		ifs.read((wxChar*)(&num_facets), 4);

		for(unsigned int i = 0; i<num_facets; i++)
		{
			CStlTri tri;
			ifs.read((wxChar*)(tri.n), 12);
			ifs.read((wxChar*)(tri.x[0]), 36);
			short attr;
			ifs.read((wxChar*)(&attr), 2);
			m_list.push_back(tri);
		}
	}
	else
	{
		// "solid" already found
		wxChar str[1024] = _T("solid");
		ifs.getline(&str[5], 1024);
		wxChar title[1024];
#if wxUSE_UNICODE
		if(swscanf(str, _T("solid %s"), title) == 1)
#else
		if(sscanf(str, _T("solid %s"), title) == 1)
#endif
			m_title.assign(title);

		CStlTri t;
		wxChar five_chars[6] = _T("aaaaa");

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
#if wxUSE_UNICODE
				if(!wcscmp(five_chars, _T("verte")))
				{
					swscanf(str, _T(" vertex %f %f %f"), &(t.x[vertex][0]), &(t.x[vertex][1]), &(t.x[vertex][2]));
					vertex++;
					if(vertex > 2)vertex = 2;
				}
				else if(!wcscmp(five_chars, _T("facet")))
				{
					swscanf(str, _T(" facet normal %f %f %f"), &(t.n[0]), &(t.n[1]), &(t.n[2]));
					vertex = 0;
				}
				else if(!wcscmp(five_chars, _T("endfa")))
				{
					if(vertex == 2)
					{
						m_list.push_back(t);
					}
				}
#else
				if(!strcmp(five_chars, _T("verte")))
				{
					sscanf(str, _T(" vertex %f %f %f"), &(t.x[vertex][0]), &(t.x[vertex][1]), &(t.x[vertex][2]));
					vertex++;
					if(vertex > 2)vertex = 2;
				}
				else if(!strcmp(five_chars, _T("facet")))
				{
					sscanf(str, _T(" facet normal %f %f %f"), &(t.n[0]), &(t.n[1]), &(t.n[2]));
					vertex = 0;
				}
				else if(!strcmp(five_chars, _T("endfa")))
				{
					if(vertex == 2)
					{
						m_list.push_back(t);
					}
				}
#endif
			}
		}
	}
}

CStlSolid::~CStlSolid(){
	KillGLLists();
}

const CStlSolid& CStlSolid::operator=(const CStlSolid& s)
{
	// don't copy id
	m_box = s.m_box;
	m_title = s.m_title;
	KillGLLists();

	return *this;
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

void CStlSolid::glCommands(bool select, bool marked, bool no_color){
	glEnable(GL_LIGHTING);

	if(m_gl_list)
	{
		glCallList(m_gl_list);
	}
	else{
		m_gl_list = glGenLists(1);
		glNewList(m_gl_list, GL_COMPILE_AND_EXECUTE);

		// to do , render all the triangles
		glBegin(GL_TRIANGLES);
		for(std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++)
		{
			CStlTri &t = *It;
			glNormal3fv(t.n);
			glVertex3fv(t.x[0]);
			glVertex3fv(t.x[1]);
			glVertex3fv(t.x[2]);
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

bool CStlSolid::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	for(std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++)
	{
		CStlTri &t = *It;

		gp_Vec vn(t.n[0], t.n[1], t.n[2]);
		vn.Transform(mat);
		t.n[0] = (float)vn.X();
		t.n[1] = (float)vn.Y();
		t.n[2] = (float)vn.Z();

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

	return false;
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
	wxGetApp().WasModified(this);
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
		n[0] = t.n[0];
		n[1] = t.n[1];
		n[2] = t.n[2];
		if(!just_one_average_normal)
		{
			n[3] = t.n[0];
			n[4] = t.n[1];
			n[5] = t.n[2];
			n[6] = t.n[0];
			n[7] = t.n[1];
			n[8] = t.n[2];
		}
		(*callbackfunc)(x, n);
	}
}

void CStlSolid::WriteXML(TiXmlElement *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "STLSolid" );
	root->LinkEndChild( element );  
	element->SetAttribute("col", color.COLORREF_color());

	for(std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++)
	{
		CStlTri &t = *It;
		TiXmlElement * child_element;
		child_element = new TiXmlElement( "tri" );
		element->LinkEndChild( child_element );  
		child_element->SetDoubleAttribute("nx", t.n[0]);
		child_element->SetDoubleAttribute("ny", t.n[1]);
		child_element->SetDoubleAttribute("nz", t.n[2]);
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
		if(name == "col"){c = HeeksColor(a->IntValue());}
	}

	CStlSolid* new_object = new CStlSolid(&c);

	// loop through all the "tri" objects
	for(TiXmlElement* pTriElem = TiXmlHandle(pElem).FirstChildElement().Element(); pTriElem;	pTriElem = pTriElem->NextSiblingElement())
	{
		// get the attributes
		for(TiXmlAttribute* a = pTriElem->FirstAttribute(); a; a = a->Next())
		{
			std::string name(a->Name());
			if(name == "nx"){t.n[0] = (float)a->DoubleValue();}
			else if(name == "ny"){t.n[1] = (float)a->DoubleValue();}
			else if(name == "nz"){t.n[2] = (float)a->DoubleValue();}
			else if(name == "p1x"){t.x[0][0] = (float)a->DoubleValue();}
			else if(name == "p1y"){t.x[0][1] = (float)a->DoubleValue();}
			else if(name == "p1z"){t.x[0][2] = (float)a->DoubleValue();}
			else if(name == "p2x"){t.x[1][0] = (float)a->DoubleValue();}
			else if(name == "p2y"){t.x[1][1] = (float)a->DoubleValue();}
			else if(name == "p2z"){t.x[1][2] = (float)a->DoubleValue();}
			else if(name == "p3x"){t.x[2][0] = (float)a->DoubleValue();}
			else if(name == "p3y"){t.x[2][1] = (float)a->DoubleValue();}
			else if(name == "p3z"){t.x[2][2] = (float)a->DoubleValue();}
		}
		new_object->m_list.push_back(t);
	}

	new_object->ReadBaseXML(pElem);

	return new_object;
}

