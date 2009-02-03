// HImage.h

#pragma once

#include "../interface/HeeksObj.h"

class HImage: public HeeksObj
{
private:
	// in the case of the movie "C:\image00001.jpg" ( for example ) will be changed to "C:\image00045.jpg", 
	// where 45 is the frame number stored in theApp.m_animation_current_frame
	unsigned int m_texture_number;
	int m_frame_when_loaded;// for movies only. only valid if m_texture_number != 0
    int width, height, textureWidth, textureHeight;
	bool m_rectangle_intialized;
	static wxIcon* m_icon;

	void destroy_texture();
	const wxChar* GetTextureFileName(const wxString &file_path, int is_a_movie);
	void do_vertex_for_lots_of_quads( double x, double y );

public:
	double m_x[4][3]; // bottom left, bottom right, top right, top left
	wxString m_file_path;
	bool m_lots_of_quads;

	HImage(const wxChar* file_path);
	HImage(const HImage &p);
	virtual ~HImage();

	// operators
	const HImage& operator=(const HImage &p);

	// HeeksObj's virtual functions
	int GetType()const{return ImageType;}
	long GetMarkingMask()const{return MARKING_FILTER_IMAGE;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	wxString GetIcon(){return _T("picture");}
	const wxChar* GetShortString(void)const;
	const wxChar* GetTypeString(void)const{return _("Image");}
	HeeksObj *MakeACopy(void)const;
	bool ModifyByMatrix(const double *mat);
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool Stretch(const double *p, const double* shift);
	void CopyFrom(const HeeksObj* object){operator=(*((HImage*)object));}
	void WriteXML(TiXmlNode *root);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
};
