// HImage.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "HImage.h"
#include "wxImageLoader.h"
#include "Gripper.h"

HImage::HImage(const wxChar* file_path)
{
	m_rectangle_intialized = false;
	m_file_path.assign(file_path);
	m_texture_number = 0; // texture not loaded
	m_lots_of_quads = true;
    width = 0;
	height = 0;
	textureWidth = 0;
	textureHeight = 0;
}

HImage::HImage(const HImage &p){
	m_texture_number = 0; // texture not loaded

	operator=(p);
}

const HImage& HImage::operator=(const HImage &p){
	HeeksObj::operator =(p);

	m_rectangle_intialized = p.m_rectangle_intialized;
	m_file_path = p.m_file_path;
	memcpy(m_x, p.m_x, sizeof(double) * 12);
	m_lots_of_quads = p.m_lots_of_quads;
    width = p.width;
	height = p.height;
	textureWidth = p.textureWidth;
	textureHeight = p.textureHeight;

	return *this;
}


HImage::~HImage()
{
	destroy_texture();
}

void HImage::destroy_texture(){
	if(m_texture_number){
		glDeleteTextures (1, &m_texture_number);
		m_texture_number = 0;
	}
}

void HImage::do_vertex_for_lots_of_quads( double x, double y ){
	double p5[3], p6[3], vt[3];
	for(int i = 0; i<3; i++)
	{
		p5[i] = m_x[0][i] + (m_x[1][i] - m_x[0][i]) * (double) x / (double)width;
		p6[i] = m_x[3][i] + (m_x[2][i] - m_x[3][i]) * (double) x / (double)width;
		vt[i] = p5[i] + (p6[i] - p5[i]) * y / (double)height;
	}

	glTexCoord2f((float)x / textureWidth, (float)y / textureHeight);
	glVertex3dv( vt );
}

const wxBitmap &HImage::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/picture.png")));
	return *icon;
}

void HImage::glCommands(bool select, bool marked, bool no_color)
{
	if(m_texture_number == 0){

		unsigned int* t = loadImage(m_file_path.c_str(), &width, &height, &textureWidth, &textureHeight);
		if(t)
		{
			m_texture_number= *t;

			if(!m_rectangle_intialized){
				// initialize rectangle
				m_x[0][0] = 0;
				m_x[0][1] = 0;
				m_x[0][2] = 0;
				m_x[1][0] = width;
				m_x[1][1] = 0;
				m_x[1][2] = 0;
				m_x[2][0] = width;
				m_x[2][1] = height;
				m_x[2][2] = 0;
				m_x[3][0] = 0;
				m_x[3][1] = height;
				m_x[3][2] = 0;
				m_rectangle_intialized = true;
			}
		}
	}

	if(!no_color){
		glColor4ub(255, 255, 255, 255);
		if(m_texture_number){
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, m_texture_number);
		}
	}
	
	if(m_rectangle_intialized){
		glBegin(GL_QUADS);

		if(	m_lots_of_quads ){
			double x_step = (double)width / 20;
			double y_step = (double)height / 20;

			for( double  i = 0; i<width; i += x_step){
				for(double  j = 0; j<height; j += y_step){
					do_vertex_for_lots_of_quads( i, j);
					do_vertex_for_lots_of_quads( i+x_step, j);
					do_vertex_for_lots_of_quads( i+x_step, j+y_step);
					do_vertex_for_lots_of_quads( i, j+y_step);
				}
			}
		}
		else{		
			glTexCoord2f(0, 0);
			glVertex3dv(m_x[0]);
			glTexCoord2f((float)width / textureWidth, 0);
			glVertex3dv(m_x[1]);
			glTexCoord2f((float)width / textureWidth, (float)height / textureHeight);
			glVertex3dv(m_x[2]);
			glTexCoord2f(0, (float)height / textureHeight);
			glVertex3dv(m_x[3]);
		}
		
		glEnd();
	}

	if(!no_color){
		glDisable(GL_TEXTURE_2D);
	}
}

void HImage::GetBox(CBox &box)
{
	for(int i = 0; i<4; i++)box.Insert(m_x[i]);
}

wxString m_global_pic_string;

const wxChar* HImage::GetShortString(void)const{
	m_global_pic_string.assign(_("Picture - "));
	m_global_pic_string.append(m_file_path.c_str());
	return m_global_pic_string.c_str();
}

HeeksObj *HImage::MakeACopy(void)const
{
	return new HImage(*this);
}

void HImage::ModifyByMatrix(const double *m)
{
	gp_Trsf mat = make_matrix(m);

	for(int i = 0; i<4; i++){
		gp_Pnt vt = make_point(m_x[i]);
		extract(vt.Transformed(mat), m_x[i]);
	}
}

void HImage::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	for(int j = 0; j<4; j++){
		list->push_back(GripData(GripperTypeStretch,m_x[j][0],m_x[j][1],m_x[j][2],NULL));
	}
}

void HImage::GetProperties(std::list<Property *> *list)
{
	HeeksObj::GetProperties(list);
}

bool HImage::Stretch(const double *p, const double* shift, void* data){
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	for(int i = 0; i<4; i++){
		gp_Pnt vt = make_point(m_x[i]);
		if(vt.IsEqual(vp, wxGetApp().m_geom_tol)){
			vt = vt.XYZ() + vshift.XYZ();
			extract(vt, m_x[i]);
			break;
		}
	}
	return false;
}	

void HImage::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Image" );
	root->LinkEndChild( element );  
	element->SetAttribute("filepath", m_file_path.utf8_str());

	element->SetDoubleAttribute("x00", m_x[0][0]);
	element->SetDoubleAttribute("x01", m_x[0][1]);
	element->SetDoubleAttribute("x02", m_x[0][2]);
	element->SetDoubleAttribute("x10", m_x[1][0]);
	element->SetDoubleAttribute("x11", m_x[1][1]);
	element->SetDoubleAttribute("x12", m_x[1][2]);
	element->SetDoubleAttribute("x20", m_x[2][0]);
	element->SetDoubleAttribute("x21", m_x[2][1]);
	element->SetDoubleAttribute("x22", m_x[2][2]);
	element->SetDoubleAttribute("x30", m_x[3][0]);
	element->SetDoubleAttribute("x31", m_x[3][1]);
	element->SetDoubleAttribute("x32", m_x[3][2]);
	WriteBaseXML(element);
}

// static member function
HeeksObj* HImage::ReadFromXMLElement(TiXmlElement* pElem)
{
	wxString filepath;

	double x[4][3];

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "filepath"){filepath.assign(Ctt(a->Value()));}
		else if(name == "x00"){x[0][0] = a->DoubleValue();}
		else if(name == "x01"){x[0][1] = a->DoubleValue();}
		else if(name == "x02"){x[0][2] = a->DoubleValue();}
		else if(name == "x10"){x[1][0] = a->DoubleValue();}
		else if(name == "x11"){x[1][1] = a->DoubleValue();}
		else if(name == "x12"){x[1][2] = a->DoubleValue();}
		else if(name == "x20"){x[2][0] = a->DoubleValue();}
		else if(name == "x21"){x[2][1] = a->DoubleValue();}
		else if(name == "x22"){x[2][2] = a->DoubleValue();}
		else if(name == "x30"){x[3][0] = a->DoubleValue();}
		else if(name == "x31"){x[3][1] = a->DoubleValue();}
		else if(name == "x32"){x[3][2] = a->DoubleValue();}
	}

	HImage *new_object = new HImage(filepath.c_str());
	memcpy(new_object->m_x[0], x[0], 12*sizeof(double));
	new_object->m_rectangle_intialized = true;
	new_object->ReadBaseXML(pElem);

	return new_object;
}

