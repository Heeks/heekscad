// HeeksCAD.cpp

#include "stdafx.h"
#include "HeeksCAD.h"
#include <wx/filedlg.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/cmdline.h>
#include <wx/clipbrd.h>
#include "../interface/Tool.h"
#include "../interface/Material.h"
#include "../interface/ToolList.h"
#include "../interface/HeeksObj.h"
#include "../interface/MarkedObject.h"
#include "../interface/PropertyColor.h"
#include "../interface/PropertyChoice.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyInt.h"
#include "../interface/PropertyCheck.h"
#include "../interface/PropertyString.h"
#include "HeeksFrame.h"
#include "GraphicsCanvas.h"
#include "OptionsCanvas.h"
#include "InputModeCanvas.h"
#include "LeftCanvas.h"
#include "ObjPropsCanvas.h"
#include "SelectMode.h"
#include "MagDragWindow.h"
#include "ViewRotating.h"
#include "DigitizeMode.h"
#include "Shape.h"
#include "ViewPoint.h"
#include "MarkedList.h"
#include "History.h"
#include "../interface/Observer.h"
#include "TransformTool.h"
#include "Grid.h"
#include "Ruler.h"
#include "StretchTool.h"
#include "HLine.h"
#include "HArc.h"
#include "HILine.h"
#include "HCircle.h"
#include "HImage.h"
#include "HPoint.h"
#include "RemoveOrAddTool.h"
#include "../tinyxml/tinyxml.h"
#include "BezierCurve.h"
#include "StlSolid.h"
#include "dxf.h"
#include "CoordinateSystem.h"
#include "../interface/PropertyChoice.h"
#include "../interface/PropertyCheck.h"
#include "../interface/PropertyString.h"
#include "../interface/PropertyList.h"

#include <sstream>
using namespace std;

IMPLEMENT_APP(HeeksCADapp)

HeeksCADapp::HeeksCADapp(): ObjList()
{
	m_version_number = _T("0 3 2");
	m_geom_tol = 0.001;
	background_color = HeeksColor(0, 0, 0);
	current_color = HeeksColor(0, 0, 0);
	construction_color = HeeksColor(0, 0, 255);
	input_mode_object = NULL;
	cur_mouse_pos.x = 0;
	cur_mouse_pos.y = 0;
	drag_gripper = NULL;
	cursor_gripper = NULL;
	magnification = new MagDragWindow();
	viewrotating = new ViewRotating;
	m_select_mode = new CSelectMode();
	m_digitizing = new DigitizeMode();
	digitize_end = true;
	digitize_inters = false;
	digitize_centre = false;
	digitize_midpoint = false;
	digitize_nearest = false;
	digitize_tangent = false;
	digitize_coords = true;
	digitize_screen = false;
	digitizing_radius = 5.0;
	draw_to_grid = true;
	digitizing_grid = 1.0;
	grid_mode = 3;
	m_rotate_mode = 0;
	m_antialiasing = false;
	m_light_push_matrix = true;
	m_config = NULL;
	m_marked_list = new MarkedList;
	history = new MainHistory;
	m_doing_rollback = false;
	m_hide_marked_list_stack = 0;
	mouse_wheel_forward_away = true;
	ctrl_does_rotate = true;
	m_show_ruler = true;
	m_show_datum_coords_system = true;
	m_filepath.assign(_T("Untitled.heeks"));
	m_in_OpenFile = false;
	m_transform_gl_list = 0;
	m_current_coordinate_system = NULL;
	m_mark_newly_added_objects = false;
}

HeeksCADapp::~HeeksCADapp()
{
	delete m_marked_list;
	m_marked_list = NULL;
	observers.clear();
	EndHistory();
	delete history;
	delete magnification;
	delete m_select_mode;
	delete m_digitizing;
	delete m_config;
}

bool HeeksCADapp::OnInit()
{
	m_config = new wxConfig(_T("HeeksCAD"));
	wxInitAllImageHandlers();

	// initialise glut
#ifdef WIN32
#else
	int argc = 0;
	char* argv;
	glutInit(&argc, &argv);
#endif
	ClearHistory();
	int width = 600;
	int height = 400;
	int posx = 200;
	int posy = 200;
	m_config->Read(_T("MainFrameWidth"), &width);
	m_config->Read(_T("MainFrameHeight"), &height);
	m_config->Read(_T("MainFramePosX"), &posx);
	m_config->Read(_T("MainFramePosY"), &posy);
	if(posx < 0)posx = 0;
	if(posy < 0)posy = 0;
	m_config->Read(_T("DrawEnd"), &digitize_end, true);
	m_config->Read(_T("DrawInters"), &digitize_inters, false);
	m_config->Read(_T("DrawCentre"), &digitize_centre, false);
	m_config->Read(_T("DrawMidpoint"), &digitize_midpoint, false);
	m_config->Read(_T("DrawNearest"), &digitize_nearest, false);
	m_config->Read(_T("DrawTangent"), &digitize_tangent, false);
	m_config->Read(_T("DrawCoords"), &digitize_coords, true);
	m_config->Read(_T("DrawScreen"), &digitize_screen, false);
	m_config->Read(_T("DrawToGrid"), &draw_to_grid, false);
	m_config->Read(_T("DrawGrid"), &digitizing_grid);
	m_config->Read(_T("DrawRadius"), &digitizing_radius);
	{
		wxString str;
		m_config->Read(_T("BackgroundColor"), &str, _T("242 204 162"));
		int r = 0, g = 0, b = 0;
		_stscanf(str, _T("%d %d %d"), &r, &g, &b);
		background_color = HeeksColor(r, g, b);
	}
	{
		wxString str;
		m_config->Read(_T("CurrentColor"), &str, _T("0 0 0"));
		int r = 0, g = 0, b = 0;
		_stscanf(str, _T("%d %d %d"), &r, &g, &b);
		current_color = HeeksColor(r, g, b);
	}
	{
		wxString str;
		m_config->Read(_T("ConstructionColor"), &str, _T("0 0 255"));
		int r = 0, g = 0, b = 255;
		_stscanf(str, _T("%d %d %d"), &r, &g, &b);
		construction_color = HeeksColor(r, g, b);
	}
	m_config->Read(_T("RotateMode"), &m_rotate_mode);
	m_config->Read(_T("Antialiasing"), &m_antialiasing);
	m_config->Read(_T("GridMode"), &grid_mode);
	m_config->Read(_T("m_light_push_matrix"), &m_light_push_matrix);
	m_config->Read(_T("WheelForwardAway"), &mouse_wheel_forward_away);
	m_config->Read(_T("CtrlDoesRotate"), &ctrl_does_rotate);
	m_config->Read(_T("DrawDatum"), &m_show_datum_coords_system, true);
	m_config->Read(_T("DatumSize"), &CoordinateSystem::size, 30);
	m_config->Read(_T("DatumSizeIsPixels"), &CoordinateSystem::size_is_pixels, true);

	GetRecentFilesProfileString();

	wxImage::AddHandler(new wxPNGHandler);
	m_frame = new CHeeksFrame( wxT( "HeeksCAD free Solid Modelling software based on Open CASCADE" ), wxPoint(posx, posy), wxSize(width, height));

	m_frame->SetIcon(wxICON(HeeksCAD));
	SetInputMode(m_select_mode);
	m_frame->Show(TRUE);
	SetTopWindow(m_frame);

	OnNewOrOpen(false);
	SetLikeNewFile();
	SetFrameTitle();

	{
		// Open the file passed in the command line argument
		wxCmdLineEntryDesc cmdLineDesc[2];
		cmdLineDesc[0].kind = wxCMD_LINE_PARAM;
		cmdLineDesc[0].shortName = NULL;
		cmdLineDesc[0].longName = NULL;
		cmdLineDesc[0].description = wxT("input files");
		cmdLineDesc[0].type = wxCMD_LINE_VAL_STRING;
		cmdLineDesc[0].flags = wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE;

		cmdLineDesc[1].kind = wxCMD_LINE_NONE;

		//gets the passed files from cmd line
		wxCmdLineParser parser (cmdLineDesc, argc, argv);

		// get filenames from the commandline
		if (parser.Parse() == 0)
		{
			for (size_t paramNr=0; paramNr < parser.GetParamCount(); ++paramNr)
			{
				OpenFile((parser.GetParam (paramNr)));
				break;
			}
		}
	}

	return TRUE;
} 

int HeeksCADapp::OnExit(){
	int result = wxApp::OnExit();
	m_config->Write(_T("DrawEnd"), digitize_end);
	m_config->Write(_T("DrawInters"), digitize_inters);
	m_config->Write(_T("DrawCentre"), digitize_centre);
	m_config->Write(_T("DrawMidpoint"), digitize_midpoint);
	m_config->Write(_T("DrawNearest"), digitize_nearest);
	m_config->Write(_T("DrawTangent"), digitize_tangent);
	m_config->Write(_T("DrawCoords"), digitize_coords);
	m_config->Write(_T("DrawScreen"), digitize_screen);
	m_config->Write(_T("DrawToGrid"), draw_to_grid);
	m_config->Write(_T("DrawGrid"), digitizing_grid);
	m_config->Write(_T("DrawRadius"), digitizing_radius);
	{
		wxChar str[1024];
		wsprintf(str, _T("%d %d %d"), background_color.red, background_color.green, background_color.blue);
		m_config->Write(_T("BackgroundColor"), str);
	}
	{
		wxChar str[1024];
		wsprintf(str, _T("%d %d %d"), current_color.red, current_color.green, current_color.blue);
		m_config->Write(_T("CurrentColor"), str);
	}
	{
		wxChar str[1024];
		wsprintf(str, _T("%d %d %d"), construction_color.red, construction_color.green, construction_color.blue);
		m_config->Write(_T("ConstructionColor"), str);
	}
	m_config->Write(_T("RotateMode"), m_rotate_mode);	
	m_config->Write(_T("Antialiasing"), m_antialiasing);	
	m_config->Write(_T("GridMode"), grid_mode);
	m_config->Write(_T("m_light_push_matrix"), m_light_push_matrix);
	m_config->Write(_T("WheelForwardAway"), mouse_wheel_forward_away);
	m_config->Write(_T("CtrlDoesRotate"), ctrl_does_rotate);
	m_config->Write(_T("DrawDatum"), m_show_datum_coords_system);
	m_config->Write(_T("DatumSize"), CoordinateSystem::size);
	m_config->Write(_T("DatumSizeIsPixels"), CoordinateSystem::size_is_pixels);

	WriteRecentFilesProfileString();

	return result;
}

#if wxUSE_UNICODE
static std::string str_for_Ttc;

const char* Ttc(const wchar_t* str)
{
	str_for_Ttc.assign("");
	while (*str)
		str_for_Ttc.push_back((char) *str++);
	return str_for_Ttc.c_str();
}

static std::wstring wstr_for_Ttc;

const wchar_t* Ctt(const char* str)
{
	wstr_for_Ttc.assign(_T(""));
	while (*str)
		wstr_for_Ttc.push_back((wchar_t) *str++);
	return wstr_for_Ttc.c_str();
}
#endif

void HeeksCADapp::SetInputMode(CInputMode *new_mode){
	if(!new_mode)return;
	m_frame->m_graphics->EndDrawFront();
	if(new_mode->OnModeChange()){
		input_mode_object = new_mode;
	}
	else{
		input_mode_object = m_select_mode;
	}
	if(m_frame && m_frame->m_input_canvas)m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
	if(m_frame && m_frame->m_options)m_frame->m_options->RefreshByRemovingAndAddingAll();
}

void HeeksCADapp::FindMarkedObject(const wxPoint &point, MarkedObject* marked_object){
	m_frame->m_graphics->FindMarkedObject(point, marked_object);
}

void HeeksCADapp::CreateLights(void)
{
	GLfloat amb[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat dif[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat spec[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat pos[4] = {0.1f, 0.1f, 1.1f, 0.0f};
    GLfloat lmodel_amb[] = { 0.2f, 0.2f, 0.2f, 1.0 };
    GLfloat local_viewer[] = { 0.0 };
	if(m_light_push_matrix){
		glPushMatrix();
		glLoadIdentity();
	}
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_amb);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_viewer);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	if(m_light_push_matrix){
		glPopMatrix();
	}
    glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);
}

void HeeksCADapp::DestroyLights(void)
{
    glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
    glDisable(GL_AUTO_NORMAL);
    glDisable(GL_NORMALIZE);
}

void HeeksCADapp::Reset(){
	m_marked_list->Clear();
	std::set<Observer*>::iterator It;
	for(It = observers.begin(); It != observers.end(); It++){
		Observer *ov = *It;
		ov->Clear();
	}
	Clear();
	EndHistory();
	delete history;
	history = new MainHistory;
	m_current_coordinate_system = NULL;
	m_doing_rollback = false;
	m_frame->m_graphics->m_view_point.SetView(gp_Vec(0, 1, 0), gp_Vec(0, 0, 1));
	m_filepath.assign(_T("Untitled.heeks"));
	ResetIDs();
}

static std::map< std::string, HeeksObj*(*)(TiXmlElement* pElem) > *xml_read_fn_map = NULL;

static HeeksObj* ReadSTEPFileFromXMLElement(TiXmlElement* pElem)
{
	std::map<int, CShapeData> index_map;

	// get the children ( an index map)
	for(TiXmlElement* subElem = TiXmlHandle(pElem).FirstChildElement().Element(); subElem; subElem = subElem->NextSiblingElement())
	{
		std::string subname(subElem->Value());
		if(subname == std::string("index_map"))
		{
			// loop through all the child elements, looking for index_pair items
			for(TiXmlElement* subsubElem = TiXmlHandle(subElem).FirstChildElement().Element(); subsubElem; subsubElem = subsubElem->NextSiblingElement())
			{
				std::string subsubname(subsubElem->Value());
				if(subsubname == std::string("index_pair"))
				{
					int index = -1;
					CShapeData shape_data;

					// get the attributes
					for(TiXmlAttribute* a = subsubElem->FirstAttribute(); a; a = a->Next())
					{
						std::string attr_name(a->Name());
						if(attr_name == std::string("index")){index = a->IntValue();}
						else if(attr_name == std::string("id")){shape_data.m_id = a->IntValue();}
						else if(attr_name == std::string("title")){shape_data.m_title.assign(Ctt(a->Value()));}
						else if(attr_name == std::string("solid_type")){shape_data.m_solid_type = (SolidTypeEnum)(a->IntValue());}
						else shape_data.m_xml_element.SetAttribute(a->Name(), a->Value());
					}
					if(index != -1)index_map.insert(std::pair<int, CShapeData>(index, shape_data));
				}
			}
		}
		else if(subname == std::string("file_text"))
		{
			const char* file_text = subElem->GetText();
			if(file_text)
			{
				wxStandardPaths sp;
				sp.GetTempDir();
				wxString temp_file = sp.GetTempDir() + _T("/temp_HeeksCAD_STEP_file.step");
				{
#if wxUSE_UNICODE
					wofstream ofs(temp_file);
#else
					ofstream ofs(temp_file);
#endif
					ofs<<file_text;
				}
				CShape::ImportSolidsFile(temp_file, false, &index_map);
			}
		}
	}

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "text")
		{
			wxStandardPaths sp;
			sp.GetTempDir();
			wxString temp_file = sp.GetTempDir() + _T("/temp_HeeksCAD_STEP_file.step");
			{
				ofstream ofs(temp_file);
				ofs<<a->Value();
			}
			CShape::ImportSolidsFile(temp_file, false, &index_map);
		}
	}

	return NULL;
}

void HeeksCADapp::InitializeXMLFunctions()
{
	// set up function map
	if(xml_read_fn_map == NULL)
	{
		xml_read_fn_map = new std::map< std::string, HeeksObj*(*)(TiXmlElement* pElem) >;
		xml_read_fn_map->insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Line", HLine::ReadFromXMLElement ) );
		xml_read_fn_map->insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Arc", HArc::ReadFromXMLElement ) );
		xml_read_fn_map->insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "InfiniteLine", HILine::ReadFromXMLElement ) );
		xml_read_fn_map->insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Circle", HCircle::ReadFromXMLElement ) );
		xml_read_fn_map->insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Point", HPoint::ReadFromXMLElement ) );
		xml_read_fn_map->insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Image", HImage::ReadFromXMLElement ) );
		xml_read_fn_map->insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "STEP_file", ReadSTEPFileFromXMLElement ) );
		xml_read_fn_map->insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "STLSolid", CStlSolid::ReadFromXMLElement ) );
		xml_read_fn_map->insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "CoordinateSystem", CoordinateSystem::ReadFromXMLElement ) );
	}
}

void HeeksCADapp::RegisterReadXMLfunction(const char* type_name, HeeksObj*(*read_xml_function)(TiXmlElement* pElem))
{
	if(xml_read_fn_map->find(type_name) != xml_read_fn_map->end()){
		wxMessageBox(_T("Error - trying to register an XML read function for an exisiting type"));
		return;
	}
	xml_read_fn_map->insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( type_name, read_xml_function ) );
}

HeeksObj* HeeksCADapp::ReadXMLElement(TiXmlElement* pElem)
{
	std::string name(pElem->Value());

	std::map< std::string, HeeksObj*(*)(TiXmlElement* pElem) >::iterator FindIt = xml_read_fn_map->find( name );
	if(FindIt != xml_read_fn_map->end())
	{
		HeeksObj* object = (*(FindIt->second))(pElem);
		return object;
	}

	return NULL;
}

void HeeksCADapp::OpenXMLFile(const wxChar *filepath)
{
	TiXmlDocument doc(Ttc(filepath));
	if (!doc.LoadFile())
	{
		if(doc.Error())
		{
			wxMessageBox(Ctt(doc.ErrorDesc()));
		}
		return;
	}

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	// block: name
	{
		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) return;
		std::string name(pElem->Value());

		if(name != "HeeksCAD_Document")
		{
			wxMessageBox(_T("This is not a HeeksCAD document!"));
			return;
		}

		// save this for later
		hRoot=TiXmlHandle(pElem);
	}

	// loop through all the objects
	for(pElem = hRoot.FirstChildElement().Element(); pElem;	pElem = pElem->NextSiblingElement())
	{
		HeeksObj* object = ReadXMLElement(pElem);
		if(object)Add(object, NULL);
	}
}

static void add_line_from_bezier_curve(const gp_Pnt& vt0, const gp_Pnt& vt1)
{
	HLine* new_object = new HLine(vt0, vt1, &(wxGetApp().current_color));
	wxGetApp().Add(new_object, NULL);
}

void HeeksCADapp::ReadSVGElement(TiXmlElement* pElem)
{
	std::string name(pElem->Value());

	if(name == "g")
	{
		// loop through all the child elements, looking for path
		for(pElem = TiXmlHandle(pElem).FirstChildElement().Element(); pElem; pElem = pElem->NextSiblingElement())
		{
			ReadSVGElement(pElem);
		}
		return;
	}

	if(name == "path")
	{
		// get the attributes
		for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
		{
			std::string name(a->Name());
			if(name == "d")
			{
				// add lines and arcs and bezier curves
				const char* d = a->Value();

				double sx, sy;
				double px, py;

				int pos = 0;
				while(1){
					if(d[pos] == 'M'){
						// make a line arc collection
						pos++;
						sscanf(&d[pos], "%lf,%lf", &sx, &sy);
						sy = -sy;
						px = sx; py = sy;
					}
					else if(d[pos] == 'L'){
						// add a line
						pos++;
						double x, y;
						sscanf(&d[pos], "%lf,%lf", &x, &y);
						y = -y;
						HLine* new_object = new HLine(gp_Pnt(px, py, 0), gp_Pnt(x, y, 0), &current_color);
						px = x; py = y;
						Add(new_object, NULL);
					}
					else if(d[pos] == 'C'){
						// add a bezier curve ( just split into lines for now )
						pos++;
						double x1, y1, x2, y2, x3, y3;
						sscanf(&d[pos], "%lf,%lf %lf,%lf %lf,%lf", &x1, &y1, &x2, &y2, &x3, &y3);
						y1 = -y1; y2 = -y2; y3 = -y3;
						split_bezier_curve(3, gp_Pnt(px, py,  0), gp_Pnt(x3, y3, 0), gp_Pnt(x1, y1, 0), gp_Pnt(x2, y2, 0), add_line_from_bezier_curve);
						px = x3; py = y3;
					}
					else if(toupper(d[pos]) == 'Z'){
						// join to end
						pos++;
						HLine* new_object = new HLine(gp_Pnt(px, py, 0), gp_Pnt(sx, sy, 0), &current_color);
						px = sx; py = sy;
						Add(new_object, NULL);
					}
					else if(d[pos] == 0){
						break;
					}
					else{
						pos++;
					}
				}
			}
		}
	}
}

void HeeksCADapp::OpenSVGFile(const wxChar *filepath)
{
	TiXmlDocument doc(Ttc(filepath));
	if (!doc.LoadFile())
	{
		if(doc.Error())
		{
			wxMessageBox(Ctt(doc.ErrorDesc()));
		}
		return;
	}

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	// block: name
	{
		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) return;
		std::string name(pElem->Value());

		if(name != "svg")
		{
			wxMessageBox(_T("This is not an SVG document!"));
			return;
		}

		// save this for later
		hRoot=TiXmlHandle(pElem);
	}

	// loop through all the objects
	for(pElem = hRoot.FirstChildElement().Element(); pElem;	pElem = pElem->NextSiblingElement())
	{
		ReadSVGElement(pElem);
	}
}

void HeeksCADapp::OpenSTLFile(const wxChar *filepath)
{
	CStlSolid* new_object = new CStlSolid(filepath, &wxGetApp().current_color);
	wxGetApp().Add(new_object, NULL);
}

class HeeksDxfRead : public CDxfRead{
public:
	HeeksDxfRead(const wxChar* filepath):CDxfRead(filepath){}

	// CDxfRead's virtual functions
	void OnReadLine(const double* s, const double* e);
	void OnReadArc(const double* s, const double* e, const double* c, bool dir);
};

void HeeksDxfRead::OnReadLine(const double* s, const double* e)
{
	HLine* new_object = new HLine(make_point(s), make_point(e), &(wxGetApp().current_color));
	wxGetApp().Add(new_object, NULL);
}

void HeeksDxfRead::OnReadArc(const double* s, const double* e, const double* c, bool dir)
{
	gp_Pnt p0 = make_point(s);
	gp_Pnt p1 = make_point(e);
	gp_Dir up(0, 0, 1);
	if(!dir)up = -up;
	gp_Pnt pc = make_point(c);
	gp_Circ circle(gp_Ax2(pc, up), p1.Distance(pc));
	HArc* new_object = new HArc(p0, p1, circle, &wxGetApp().current_color);
	wxGetApp().Add(new_object, NULL);
}

void HeeksCADapp::OpenDXFFile(const wxChar *filepath)
{
	{
		HeeksDxfRead dxf_file(filepath);
		dxf_file.DoRead();
	}
}

bool HeeksCADapp::OpenImageFile(const wxChar *filepath)
{
	wxString wf(filepath);
	wf.LowerCase();

	wxList handlers = wxImage::GetHandlers();
	for(wxList::iterator It = handlers.begin(); It != handlers.end(); It++)
	{
		wxImageHandler* handler = (wxImageHandler*)(*It);
		wxString ext = _T(".") + handler->GetExtension();
		if(wf.EndsWith(ext))
		{
			wxGetApp().AddUndoably(new HImage(filepath), NULL, NULL);
			wxGetApp().Repaint();
			return true;
		}
	}

	return false;
}

bool HeeksCADapp::OpenFile(const wxChar *filepath, bool import_not_open)
{
	wxGetApp().m_in_OpenFile = true;

	// returns true if file open was successful
	wxString wf(filepath);
	wf.LowerCase();

	bool open_failed = false;

	if(wf.EndsWith(_T(".heeks")))
	{
		OpenXMLFile(filepath);
	}
	else if(wf.EndsWith(_T(".svg")))
	{
		OpenSVGFile(filepath);
	}
	else if(wf.EndsWith(_T(".svg")))
	{
		OpenSVGFile(filepath);
	}
	else if(wf.EndsWith(_T(".stl")))
	{
		OpenSTLFile(filepath);
	}
	else if(wf.EndsWith(_T(".dxf")))
	{
		OpenDXFFile(filepath);
	}

	// check for images
	else if(OpenImageFile(filepath))
	{
	}

	// check for solid files
	else if(CShape::ImportSolidsFile(filepath, false))
	{
	}
	else
	{
		// error
		wxChar mess[1024];
		wsprintf(mess, _T("Invalid file type chosen ( expecting file with %s suffix )"), wxGetApp().GetKnownFilesCommaSeparatedList());
		wxMessageBox(mess);
		open_failed = true;
	}

	if(!open_failed)
	{
		WereAdded(m_objects);
		if(!import_not_open)
		{
			m_filepath.assign(filepath);
			InsertRecentFileItem(filepath);
			SetFrameTitle();
		}
	}

	wxGetApp().m_in_OpenFile = false;

	return true;
}

static void WriteDXFEntity(HeeksObj* object, CDxfWrite& dxf_file)
{
	switch(object->GetType())
	{
	case LineType:
		{
			HLine* l = (HLine*)object;
			double s[3], e[3];
			extract(l->A, s);
			extract(l->B, e);
			dxf_file.WriteLine(s, e);
		}
		break;
	case ArcType:
		{
			HArc* a = (HArc*)object;
			double s[3], e[3], c[3];
			extract(a->A, s);
			extract(a->B, e);
			extract(a->m_circle.Location(), c);
			bool dir = a->m_circle.Axis().Direction().Z() > 0;
			dxf_file.WriteArc(s, e, c, dir);
		}
		break;
	default:
		{
			for(HeeksObj* child = object->GetFirstChild(); child; child = object->GetNextChild())
			{
				// recursive
				WriteDXFEntity(child, dxf_file);
			}
		}
	}
}

void HeeksCADapp::SaveDXFFile(const wxChar *filepath)
{
	CDxfWrite dxf_file(filepath);
	if(dxf_file.Failed())
	{
		wxChar str[1024];
		wsprintf(str, _T("couldn't open file - %s"), filepath);
		wxMessageBox(str);
		return;
	}

	// write all the objects
	for(std::list<HeeksObj*>::iterator It = m_objects.begin(); It != m_objects.end(); It++)
	{
		HeeksObj* object = *It;
		WriteDXFEntity(object, dxf_file);
	}

	// when dxf_file goes out of scope it writes the file, see ~CDxfWrite
}

static ofstream* ofs_for_write_stl_triangle = NULL;

static void write_stl_triangle(const double* x, const double* n)
{
	wxChar str[1024];
	wsprintf(str, _T(" facet normal %g %g %g"), n[0], n[1], n[2]);
	(*ofs_for_write_stl_triangle)<<str<<endl;
	(*ofs_for_write_stl_triangle)<<"   outer loop"<<endl;
	wsprintf(str, _T("     vertex %g %g %g"), x[0], x[1], x[2]);
	(*ofs_for_write_stl_triangle)<<str<<endl;
	wsprintf(str, _T("     vertex %g %g %g"), x[3], x[4], x[5]);
	(*ofs_for_write_stl_triangle)<<str<<endl;
	wsprintf(str, _T("     vertex %g %g %g"), x[6], x[7], x[8]);
	(*ofs_for_write_stl_triangle)<<str<<endl;
	(*ofs_for_write_stl_triangle)<<"   endloop"<<endl;
	(*ofs_for_write_stl_triangle)<<" endfacet"<<endl;
}

void HeeksCADapp::SaveSTLFile(const wxChar *filepath)
{
	ofstream ofs(filepath);
	if(!ofs)
	{
		wxChar str[1024];
		wsprintf(str, _T("couldn't open file - %s"), filepath);
		wxMessageBox(str);
		return;
	}

	ofs<<"solid"<<endl;

	// write all the objects
	ofs_for_write_stl_triangle = &ofs;
	for(std::list<HeeksObj*>::iterator It = m_objects.begin(); It != m_objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->GetTriangles(write_stl_triangle, 0.1);
	}

	ofs<<"endsolid"<<endl;
}

void HeeksCADapp::SaveXMLFile(const std::list<HeeksObj*>& objects, const wxChar *filepath)
{
	// write an xml file
	TiXmlDocument doc;  
	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  

	TiXmlElement * root = new TiXmlElement( "HeeksCAD_Document" );  
	doc.LinkEndChild( root );  

	// loop through all the objects writing them
	CShape::m_solids_found = false;
	for(std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->WriteXML(root);
	}

	// write a step file for all the solids
	if(CShape::m_solids_found){
		wxStandardPaths sp;
		sp.GetTempDir();
		wxString temp_file = sp.GetTempDir() + _T("/temp_HeeksCAD_STEP_file.step");
		std::map<int, CShapeData> index_map;
		CShape::ExportSolidsFile(objects, temp_file, &index_map);

		TiXmlElement *step_file_element = new TiXmlElement( "STEP_file" );
		root->LinkEndChild( step_file_element );  

		// write the index map as a child of step_file
		{
			TiXmlElement *index_map_element = new TiXmlElement( "index_map" );
			step_file_element->LinkEndChild( index_map_element );
			for(std::map<int, CShapeData>::iterator It = index_map.begin(); It != index_map.end(); It++)
			{
				TiXmlElement *index_pair_element = new TiXmlElement( "index_pair" );
				index_map_element->LinkEndChild( index_pair_element );
				int index = It->first;
				CShapeData& shape_data = It->second;
				index_pair_element->SetAttribute("index", index);
				index_pair_element->SetAttribute("id", shape_data.m_id);
				index_pair_element->SetAttribute("title", Ttc(shape_data.m_title.c_str()));
				if(shape_data.m_solid_type != SOLID_TYPE_UNKNOWN)index_pair_element->SetAttribute("solid_type", shape_data.m_solid_type);
				// get the CShapeData attributes
				for(TiXmlAttribute* a = shape_data.m_xml_element.FirstAttribute(); a; a = a->Next())
				{
					index_pair_element->SetAttribute(a->Name(), a->Value());
				}

			}
		}

		// write the step file as a string attribute of step_file
		ifstream ifs(temp_file);
		if(!(!ifs)){
			std::string fstr;
			char str[1024];
			while(!(ifs.eof())){
				ifs.getline(str, 1022);
				strcat(str, "\n");
				fstr.append(str);
				if(!ifs)break;
			}

			TiXmlElement *file_text_element = new TiXmlElement( "file_text" );
			step_file_element->LinkEndChild( file_text_element );
			TiXmlText *text = new TiXmlText(fstr.c_str());
			text->SetCDATA(true);
			file_text_element->LinkEndChild( text );
		}
	}

	doc.SaveFile( Ttc(filepath) );  
}

bool HeeksCADapp::SaveFile(const wxChar *filepath, bool use_dialog, bool update_recent_file_list, bool set_app_caption)
{
	if(use_dialog){
		wxFileDialog fd(m_frame, _T("Save graphical data file"), wxEmptyString, filepath, GetKnownFilesWildCardString(false), wxSAVE|wxOVERWRITE_PROMPT);
		fd.SetFilterIndex(1);
		if (fd.ShowModal() == wxID_CANCEL)return false;
		return SaveFile( fd.GetPath().c_str(), false, update_recent_file_list );
	}

	wxString wf(filepath);
	wf.LowerCase();

	if(wf.EndsWith(_T(".heeks")))
	{
		SaveXMLFile(filepath);
	}
	else if(wf.EndsWith(_T(".dxf")))
	{
		SaveDXFFile(filepath);
	}
	else if(wf.EndsWith(_T(".stl")))
	{
		SaveSTLFile(filepath);
	}
	else if(CShape::ExportSolidsFile(m_objects, filepath))
	{
	}
	else
	{
		wxChar mess[1024];
		wsprintf(mess, _T("Invalid file type chosen ( expecting file with %s suffix )"), wxGetApp().GetKnownFilesCommaSeparatedList(false));
		wxMessageBox(mess);
		return false;
	}

	m_filepath.assign(filepath);
	if(update_recent_file_list)InsertRecentFileItem(filepath);
	if(set_app_caption)SetFrameTitle();
	SetLikeNewFile();

	return true;
}


void HeeksCADapp::Repaint(bool soon)
{
	if(soon && m_frame->IsShown()){
//#ifdef __WXMSW__
//		::SendMessage((HWND)(m_frame->m_graphics->GetHandle()), WM_PAINT, 0, 0);
//#else
		m_frame->m_graphics->Refresh(0);
		m_frame->m_graphics->Update();
//#endif
	}
	else{
		m_frame->m_graphics->Refresh(0);
	}
}

void HeeksCADapp::RecalculateGLLists()
{
	for(HeeksObj* object = GetFirstChild(); object; object = GetNextChild()){
		object->KillGLLists();
	}
}

void HeeksCADapp::glCommandsAll(bool select, const CViewPoint &view_point)
{
	CreateLights();
	glDisable(GL_LIGHTING);
	Material().glMaterial(1.0);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1);
	glDepthMask(1);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glShadeModel(GL_FLAT);
	m_frame->m_graphics->m_view_point.SetPolygonOffset();
	glCommands(select, false, false);

	for(std::list< void(*)() >::iterator It = m_on_glCommands_list.begin(); It != m_on_glCommands_list.end(); It++)
	{
		void(*callbackfunc)() = *It;
		(*callbackfunc)();
	}

	input_mode_object->OnRender();

	if(m_transform_gl_list)
	{
        glPushMatrix();
		double m[16];
		extract_transposed(m_drag_matrix, m);
		glMultMatrixd(m);
		glCallList(m_transform_gl_list);
		glPopMatrix();
	}

	// draw the grid
	RenderGrid(&view_point);

	// draw the ruler
	if(m_show_ruler)RenderRuler();

	// draw the datum
	if(m_show_datum_coords_system)
	{
		bool bright = (m_current_coordinate_system == NULL);

		// make the datum appear at the front of everything, by setting the depth range
		GLfloat save_depth_range[2];
		glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
		glDepthRange(0, 0);

		CoordinateSystem::RenderDatum(bright);

		// restore the depth range
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}

	DestroyLights();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL );
	if(m_hide_marked_list_stack == 0)m_marked_list->GrippersGLCommands(select, false);
}

void HeeksCADapp::glCommands(bool select, bool marked, bool no_color)
{
	if(m_hide_marked_list_stack>0){
		HeeksObj::glCommands(select, marked, no_color);

		std::list<HeeksObj*>::iterator It;
		for(It=m_objects.begin(); It!=m_objects.end() ;It++)
		{
			if(m_marked_list->ObjectMarked(*It) == 0){
				if(select)glPushName((unsigned int)(*It));
				(*It)->glCommands(select, marked, no_color);
				if(select)glPopName();
			}
		}
	}
	else{
		__super::glCommands(select, marked, no_color);
	}
}

void HeeksCADapp::GetBox(CBox &box){
	CBox temp_box;
	__super::GetBox(temp_box);
	if(temp_box.m_valid && temp_box.Radius() > 0.000001)
		box.Insert(temp_box);
}

void HeeksCADapp::clear_marked_list(void){
	std::list<HeeksObj *>::iterator It;
	m_marked_list->Clear();
	Repaint();
}

double HeeksCADapp::GetPixelScale(void){
	return m_frame->m_graphics->m_view_point.pixel_scale;
}

bool HeeksCADapp::IsModified(void){
	return history->IsModified();
}

void HeeksCADapp::SetAsModified(){
	history->SetAsModified();
}

void HeeksCADapp::SetLikeNewFile(void){
	history->SetLikeNewFile();
}

void HeeksCADapp::ClearHistory(void){
	history->ClearFromFront();
	history->SetLikeNewFile();
}

static void AddToolListWithSeparator(std::list<Tool*> &l, std::list<Tool*> &temp_l)
{
	if(temp_l.size()>0)
	{
		if(l.size()>0)l.push_back(NULL);
		std::list<Tool*>::iterator FIt;
		for(FIt = temp_l.begin(); FIt != temp_l.end(); FIt++)
			l.push_back(*FIt);
	}
}

static std::vector<ToolIndex> tool_index_list;

class CFullScreenTool : public Tool  
{
public:
	// Tool's virtual functions
	const wxChar* CFullScreenTool::GetTitle(){
		if (wxGetApp().m_frame->IsFullScreen()) return _T("Exit Full Screen Mode");
		else return _T("Show Full Screen");
	}
	void CFullScreenTool::Run(){
		wxGetApp().m_frame->ShowFullScreen(!wxGetApp().m_frame->IsFullScreen());
	}
};

void HeeksCADapp::DoDropDownMenu(wxWindow *wnd, const wxPoint &point, MarkedObject* marked_object, bool dont_use_point_for_functions, bool from_graphics_canvas, bool control_pressed)
{
	tool_index_list.clear();
	wxPoint new_point = point;
	if(dont_use_point_for_functions){
		new_point.x = -1;
		new_point.y = -1;
	}
	wxMenu menu;
	std::list<Tool*> f_list;
	std::list<Tool*> temp_f_list;

	GetTools(marked_object, f_list, new_point, from_graphics_canvas, control_pressed);

	m_marked_list->GetTools(&f_list, &new_point);

	temp_f_list.clear();
	if(input_mode_object)input_mode_object->GetTools(&temp_f_list, &new_point);
	AddToolListWithSeparator(f_list, temp_f_list);
	temp_f_list.clear();

	if(point.x>=0 && point.y>=0)temp_f_list.push_back(new CFullScreenTool);



	AddToolListWithSeparator(f_list, temp_f_list);
	std::list<Tool*>::iterator FIt;
	for (FIt = f_list.begin(); FIt != f_list.end(); FIt++)
		m_frame->AddToolToListAndMenu(*FIt, tool_index_list, &menu);
	wnd->PopupMenu(&menu, point);
}

void HeeksCADapp::on_menu_event(wxCommandEvent& event)
{
	int id = event.GetId();
	if(id){
		Tool *t = tool_index_list[id - ID_FIRST_POP_UP_MENU_TOOL].m_tool;
		if(t->Undoable())DoToolUndoably(t);
		else{
			t->Run();
		}
		Repaint();
	}
}


void HeeksCADapp::DoToolUndoably(Tool *t)
{
	history->DoToolUndoably(t);
}

bool HeeksCADapp::RollBack(void)
{
	m_doing_rollback = true;
	bool result = history->InternalRollBack();
	m_doing_rollback = false;
	return result;
}

bool HeeksCADapp::RollForward(void)
{
	m_doing_rollback = true;
	bool result = history->InternalRollForward();
	m_doing_rollback = false;
	return result;
}

void HeeksCADapp::StartHistory(const wxChar* str)
{
	history->StartHistory(str);
}

void HeeksCADapp::EndHistory(void)
{
	history->EndHistory();
}

void HeeksCADapp::ClearRollingForward(void)
{
	history->ClearFromCurPos();
}

void HeeksCADapp::RegisterObserver(Observer* observer)
{
	if (observer==NULL) return;
	observers.insert(observer);
	observer->OnChanged(&m_objects, NULL, NULL);
}

void HeeksCADapp::RemoveObserver(Observer* observer){
	observers.erase(observer);
}

void HeeksCADapp::ObserversOnChange(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified){
	std::set<Observer*>::iterator It;
	for(It = observers.begin(); It != observers.end(); It++){
		Observer *ov = *It;
		ov->OnChanged(added, removed, modified);
	}
}

void HeeksCADapp::ObserversMarkedListChanged(bool all_marked, bool none_marked, const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed){
	std::set<Observer*>::iterator It;
	for(It = observers.begin(); It != observers.end(); It++){
		Observer *ov = *It;
		ov->WhenMarkedListChanges(all_marked, none_marked,  added, removed);
	}
}

bool HeeksCADapp::Add(HeeksObj *object, HeeksObj* prev_object)
{
	if (!ObjList::Add(object, prev_object)) return false;

	if(object->GetType() == CoordinateSystemType && !m_in_OpenFile)
	{
		m_current_coordinate_system = (CoordinateSystem*)object;
	}

	if(m_mark_newly_added_objects)
	{
		m_marked_list->Add(object);
	}

	return true;
}

void HeeksCADapp::Remove(HeeksObj* object)
{
	ObjList::Remove(object);
	if(object == m_current_coordinate_system)m_current_coordinate_system = NULL;
}

void HeeksCADapp::DeleteUndoably(HeeksObj *object){
	if(object == NULL)return;
	RemoveObjectTool *tool = new RemoveObjectTool(object);
	wxChar str[1024];
	wsprintf(str, _T("Deleting %s"), object->GetShortStringOrTypeString());
	StartHistory(str);
	DoToolUndoably(tool);
	EndHistory();
}

void HeeksCADapp::DeleteUndoably(const std::list<HeeksObj*>& list)
{
	if(list.size() == 0)return;
	RemoveObjectsTool *tool = new RemoveObjectsTool(list, list.front()->m_owner);
	DoToolUndoably(tool);
}

void HeeksCADapp::TransformUndoably(HeeksObj *object, double *m)
{
	if(!object)return;
	gp_Trsf mat = make_matrix(m);
	gp_Trsf im = mat;
	im.Invert();
	TransformTool *tool = new TransformTool(object, mat, im);
	DoToolUndoably(tool);
}

void HeeksCADapp::TransformUndoably(const std::list<HeeksObj*> &list, double *m)
{
	if(list.size() == 0)return;
	gp_Trsf mat = make_matrix(m);
	gp_Trsf im = mat;
	im.Invert();
	TransformObjectsTool *tool = new TransformObjectsTool(list, mat, im);
	DoToolUndoably(tool);
}

void HeeksCADapp::WasModified(HeeksObj *object)
{
	std::list<HeeksObj*> list;
	list.push_back(object);
	WereModified(list);
}

void HeeksCADapp::WasAdded(HeeksObj *object)
{
	std::list<HeeksObj*> list;
	list.push_back(object);
	WereAdded(list);
}

void HeeksCADapp::WasRemoved(HeeksObj *object)
{
	std::list<HeeksObj*> list;
	list.push_back(object);
	WereRemoved(list);
}

void HeeksCADapp::WereModified(const std::list<HeeksObj*>& list)
{
	if (list.size() == 0) return;
	HeeksObj* object = *(list.begin());
	if (object == NULL) return;
	ObserversOnChange(NULL, NULL, &list);
	SetAsModified();
}

void HeeksCADapp::WereAdded(const std::list<HeeksObj*>& list)
{
	if (list.size() == 0) return;
	HeeksObj* object = *(list.begin());
	if (object == NULL) return;
	ObserversOnChange(&list, NULL, NULL);
	SetAsModified();
}

void HeeksCADapp::WereRemoved(const std::list<HeeksObj*>& list)
{
	if (list.size() == 0) return;
	HeeksObj* object = *(list.begin());
	if (object == NULL) return;

	wxGetApp().m_marked_list->Remove(list);

	ObserversOnChange(NULL, &list, NULL);
	SetAsModified();
}

gp_Trsf HeeksCADapp::GetDrawMatrix(bool get_the_appropriate_orthogonal)
{
	if(get_the_appropriate_orthogonal){
		// choose from the three orthoganal possibilities, the one where it's z-axis closest to the camera direction
		gp_Vec vx, vy;
		m_frame->m_graphics->m_view_point.GetTwoAxes(vx, vy, false, 0);
		{
			gp_Pnt o(0, 0, 0);
			if(wxGetApp().m_current_coordinate_system)o.Transform(wxGetApp().m_current_coordinate_system->GetMatrix());
			return make_matrix(o, vx, vy);
		}
	}

	gp_Trsf mat;
	if(wxGetApp().m_current_coordinate_system)mat = wxGetApp().m_current_coordinate_system->GetMatrix();
	return mat;
}

void on_set_background_color(HeeksColor value, HeeksObj* object)
{
	wxGetApp().background_color = value;
	wxGetApp().Repaint();
}

void on_set_current_color(HeeksColor value, HeeksObj* object)
{
	wxGetApp().current_color = value;
}

void on_set_construction_color(HeeksColor value, HeeksObj* object)
{
	wxGetApp().construction_color = value;
}

void on_set_grid_mode(int value, HeeksObj* object)
{
	wxGetApp().grid_mode = value;
	wxGetApp().Repaint();
}

void on_grid(bool onoff, HeeksObj* object)
{
	wxGetApp().draw_to_grid = onoff;
	wxGetApp().Repaint();
}

void on_grid_edit(double grid_value, HeeksObj* object)
{
	wxGetApp().digitizing_grid = grid_value;
	wxGetApp().Repaint();
}

void on_set_geom_tol(double value, HeeksObj* object)
{
	wxGetApp().m_geom_tol = value;
	wxGetApp().Repaint();
}

void on_set_selection_filter(int value, HeeksObj* object)
{
	wxGetApp().m_marked_list->m_filter = value;
}

void on_set_show_datum(bool onoff, HeeksObj* object)
{
	wxGetApp().m_show_datum_coords_system = onoff;
	wxGetApp().Repaint();
}

void on_set_rotate_mode(int value, HeeksObj* object)
{
	wxGetApp().m_rotate_mode = value;
	if(!wxGetApp().m_rotate_mode)
	{
		wxGetApp().m_frame->m_graphics->m_view_point.SetView(gp_Vec(0, 1, 0), gp_Vec(0, 0, 1));
		wxGetApp().m_frame->m_graphics->StoreViewPoint();
		wxGetApp().Repaint();
	}
}

void on_set_antialiasing(bool value, HeeksObj* object)
{
	wxGetApp().m_antialiasing = value;
	wxGetApp().Repaint();
}

void on_set_light_push_matrix(bool value, HeeksObj* object)
{
	wxGetApp().m_light_push_matrix = value;
	wxGetApp().Repaint();
}

void on_set_reverse_mouse_wheel(bool value, HeeksObj* object)
{
	wxGetApp().mouse_wheel_forward_away = !value;
}

void on_set_ctrl_does_rotate(bool value, HeeksObj* object)
{
	wxGetApp().ctrl_does_rotate = value;
}

void on_intersection(bool onoff, HeeksObj* object){
	wxGetApp().digitize_inters = onoff;
}

void on_centre(bool onoff, HeeksObj* object){
	wxGetApp().digitize_centre = onoff;
}

void on_end_of(bool onoff, HeeksObj* object){
	wxGetApp().digitize_end = onoff;
}

void on_mid_point(bool onoff, HeeksObj* object){
	wxGetApp().digitize_midpoint = onoff;
}

void on_nearest(bool onoff, HeeksObj* object){
	wxGetApp().digitize_nearest = onoff;
}

void on_tangent(bool onoff, HeeksObj* object){
	wxGetApp().digitize_tangent = onoff;
}

void on_radius(double value, HeeksObj* object){
	wxGetApp().digitizing_radius = value;
}

void on_coords(bool onoff, HeeksObj* object){
	wxGetApp().digitize_coords = onoff;
}

void on_relative(bool onoff, HeeksObj* object){
	wxGetApp().digitize_screen = onoff;
}

static std::list<Property *> *list_for_GetOptions = NULL;

static void AddPropertyCallBack(Property* p)
{
	list_for_GetOptions->push_back(p);
}

void on_set_datum_size(double value, HeeksObj* object){
	CoordinateSystem::size = value;
}

void on_set_size_is_pixels(bool value, HeeksObj* object){
	CoordinateSystem::size_is_pixels = value;
}

void HeeksCADapp::GetOptions(std::list<Property *> *list)
{
	PropertyList* view_options = new PropertyList(_T("view options"));

	std::list< wxString > choices;
	choices.push_back ( wxString ( _T("stay upright") ) );
	choices.push_back ( wxString ( _T("free") ) );
	view_options->m_list.push_back ( new PropertyChoice ( _T("rotate mode"),  choices, wxGetApp().m_rotate_mode, NULL, on_set_rotate_mode ) );
	view_options->m_list.push_back( new PropertyCheck(_T("antialiasing"), wxGetApp().m_antialiasing, NULL, on_set_antialiasing));
#if _DEBUG
	view_options->m_list.push_back( new PropertyCheck(_T("fixed light"), wxGetApp().m_light_push_matrix, NULL, on_set_light_push_matrix));
#endif
	view_options->m_list.push_back( new PropertyCheck(_T("reverse mouse wheel"), !(wxGetApp().mouse_wheel_forward_away), NULL, on_set_reverse_mouse_wheel));
	view_options->m_list.push_back( new PropertyCheck(_T("Ctrl key does rotate"), wxGetApp().ctrl_does_rotate, NULL, on_set_ctrl_does_rotate));
	view_options->m_list.push_back(new PropertyCheck(_T("show datum"), m_show_datum_coords_system, NULL, on_set_show_datum));
	view_options->m_list.push_back(new PropertyDouble(_T("datum size"), CoordinateSystem::size, NULL, on_set_datum_size));
	view_options->m_list.push_back(new PropertyCheck(_T("datum size is pixels not mm"), CoordinateSystem::size_is_pixels, NULL, on_set_size_is_pixels));
	view_options->m_list.push_back ( new PropertyColor ( _T("background color"),  background_color, NULL, on_set_background_color ) );
	{
		std::list< wxString > choices;
		choices.push_back ( wxString ( _T("no grid") ) );
		choices.push_back ( wxString ( _T("faint color") ) );
		choices.push_back ( wxString ( _T("alpha blending") ) );
		choices.push_back ( wxString ( _T("colored alpha blending") ) );
		view_options->m_list.push_back ( new PropertyChoice ( _T("grid mode"),  choices, grid_mode, NULL, on_set_grid_mode ) );
	}
	list->push_back(view_options);

	PropertyList* digitizing = new PropertyList(_T("digitizing"));
	digitizing->m_list.push_back(new PropertyCheck(_T("end"), wxGetApp().digitize_end, NULL, on_end_of));
	digitizing->m_list.push_back(new PropertyCheck(_T("intersection"), wxGetApp().digitize_inters, NULL, on_intersection));
	digitizing->m_list.push_back(new PropertyCheck(_T("centre"), wxGetApp().digitize_centre, NULL, on_centre));
	digitizing->m_list.push_back(new PropertyCheck(_T("midpoint"), wxGetApp().digitize_midpoint, NULL, on_mid_point));
	digitizing->m_list.push_back(new PropertyCheck(_T("nearest"), wxGetApp().digitize_nearest, NULL, on_nearest));
	digitizing->m_list.push_back(new PropertyCheck(_T("tangent"), wxGetApp().digitize_tangent, NULL, on_tangent));
	digitizing->m_list.push_back(new PropertyDouble(_T("radius for undefined circles"), wxGetApp().digitizing_radius, NULL, on_radius));
	digitizing->m_list.push_back(new PropertyCheck(_T("coordinates"), wxGetApp().digitize_coords, NULL, on_coords));
	digitizing->m_list.push_back(new PropertyCheck(_T("screen"), wxGetApp().digitize_screen, NULL, on_relative));
	list->push_back(digitizing);

	list->push_back ( new PropertyColor ( _T("current color"),  current_color, NULL, on_set_current_color ) );
	list->push_back ( new PropertyColor ( _T("construction color"),  construction_color, NULL, on_set_construction_color ) );
	list->push_back(new PropertyDouble(_T("grid size"), digitizing_grid, NULL, on_grid_edit));
	list->push_back(new PropertyCheck(_T("grid"), draw_to_grid, NULL, on_grid));
	list->push_back(new PropertyDouble(_T("geometry tolerance"), m_geom_tol, NULL, on_set_geom_tol));
	for(std::list<wxDynamicLibrary*>::iterator It = m_loaded_libraries.begin(); It != m_loaded_libraries.end(); It++){
		wxDynamicLibrary* shared_library = *It;
		list_for_GetOptions = list;
		void(*GetOptions)(void(*)(Property*)) = (void(*)(void(*)(Property*)))(shared_library->GetSymbol(_T("GetOptions")));
		(*GetOptions)(AddPropertyCallBack);
	}

	list->push_back(new PropertyInt(_T("selection filter"), m_marked_list->m_filter, NULL, on_set_selection_filter));
}

void HeeksCADapp::DeleteMarkedItems()
{
	std::list<HeeksObj *> list = m_marked_list->list();

	// clear first, so properties cancel happens first
	m_marked_list->Clear();

	if(list.size() == 1){
		DeleteUndoably(*(list.begin()));
	}
	else if(list.size()>1){
		StartHistory(_T("Delete Marked Items"));
		DeleteUndoably(list);
		EndHistory();
	}
	Repaint(0);
}

void HeeksCADapp::AddUndoably(HeeksObj *object, HeeksObj* owner, HeeksObj* prev_object)
{
	if(object == NULL)return;
	if(owner == NULL)owner = this;
	AddObjectTool *tool = new AddObjectTool(object, owner, prev_object);
	wxChar str[1024];
	wsprintf(str, _T("*Adding %s"), object->GetShortStringOrTypeString());
	StartHistory(str);
	DoToolUndoably(tool);
	EndHistory();
}

void HeeksCADapp::AddUndoably(const std::list<HeeksObj*> &list, HeeksObj* owner)
{
	if(list.size() == 0)return;
	if(owner == NULL)owner = this;
	AddObjectsTool *tool = new AddObjectsTool(list, owner);
	DoToolUndoably(tool);
}

void HeeksCADapp::glColorEnsuringContrast(const HeeksColor &c)
{
	if(c == background_color)background_color.best_black_or_white().glColor();
	else c.glColor();
}

static wxString known_file_ext;

const wxChar* HeeksCADapp::GetKnownFilesWildCardString(bool open)const
{
	if(open){
		wxList handlers = wxImage::GetHandlers();
		wxString imageExtStr;
		wxString imageExtStr2;
		for(wxList::iterator It = handlers.begin(); It != handlers.end(); It++)
		{
			wxImageHandler* handler = (wxImageHandler*)(*It);
			wxString ext = handler->GetExtension();
			if(It != handlers.begin())imageExtStr.Append(_T(";"));
			imageExtStr.Append(_T("*."));
			imageExtStr.Append(ext);
			if(It != handlers.begin())imageExtStr2.Append(_T(" "));
			imageExtStr2.Append(_T("*."));
			imageExtStr2.Append(ext);
		}
		known_file_ext = _T("Known Files |*.heeks;*.igs;*.iges;*.stp;*.step;*.stl;*.svg;*.dxf;") + imageExtStr + _T("|Heeks files (*.heeks)|*.heeks|IGES files (*.igs *.iges)|*.igs;*.iges|STEP files (*.stp *.step)|*.stp;*.step|STL files (*.stl)|*.stl|Scalar Vector Graphics files (*.svg)|*.svg|DXF files (*.dxf)|*.dxf|Picture files (") + imageExtStr2 + _T(")|") + imageExtStr;
		return known_file_ext.c_str();
	}
	else{
		// file save
		return _T("Known Files |*.heeks;*.igs;*.iges;*.stp;*.step;*.stl;*.dxf|Heeks files (*.heeks)|*.heeks|IGES files (*.igs *.iges)|*.igs;*.iges|STEP files (*.stp *.step)|*.stp;*.step|STL files (*.stl)|*.stl|DXF files (*.dxf)|*.dxf");
	}
}

const wxChar* HeeksCADapp::GetKnownFilesCommaSeparatedList(bool open)const
{
	if(open){
		wxList handlers = wxImage::GetHandlers();
		wxString known_ext_str = _T("heeks, igs, iges, stp, step, stl, svg, dxf");
		for(wxList::iterator It = handlers.begin(); It != handlers.end(); It++)
		{
			wxImageHandler* handler = (wxImageHandler*)(*It);
			wxString ext = handler->GetExtension();
			known_ext_str.Append(_T(", "));
			known_ext_str.Append(ext);
		}

		return known_ext_str;
	}
	else{
		// file save
		return _T("heeks, igs, iges, stp, step, stl, dxf");
	}
}

class MarkObjectTool:public Tool{
public:
	MarkedObject *m_marked_object;
	wxPoint m_point;
	bool m_xor_marked_list;

	MarkObjectTool():m_marked_object(NULL), m_xor_marked_list(false){}

	// Tool's virtual functions
	const wxChar* GetTitle(){
		if(m_xor_marked_list){
			if(wxGetApp().m_marked_list->ObjectMarked(m_marked_object->GetObject())){
				return _T("Unmark");
			}
			else{
				return _T("Mark");
			}
		}
		return _T("Properties");
	}
	void Run(){
		if(m_marked_object == NULL)return;
		if(m_marked_object->GetObject() == NULL)return;
		if(m_xor_marked_list){
			if(wxGetApp().m_marked_list->ObjectMarked(m_marked_object->GetObject())){
				wxGetApp().m_marked_list->Remove(m_marked_object->GetObject());
			}
			else{
				wxGetApp().m_marked_list->Add(m_marked_object->GetObject());
			}
		}
		else{
			wxGetApp().m_marked_list->Clear();
			wxGetApp().m_marked_list->Add(m_marked_object->GetObject());
			if(!(wxGetApp().m_frame->m_properties->IsShown())){
				wxGetApp().m_frame->m_aui_manager->GetPane(wxGetApp().m_frame->m_properties).Show();
				wxGetApp().m_frame->m_aui_manager->Update();
			}
		}
		if(m_point.x >= 0){
			gp_Lin ray = wxGetApp().m_frame->m_graphics->m_view_point.SightLine(m_point);
			double ray_start[3], ray_direction[3];
			extract(ray.Location(), ray_start);
			extract(ray.Direction(), ray_direction);
			m_marked_object->GetObject()->SetClickMarkPoint(m_marked_object, ray_start, ray_direction);
		}
		wxGetApp().Repaint();
	}
};

static MarkObjectTool mark_object_tool;
static RemoveObjectTool remove_object_tool(NULL);

void HeeksCADapp::GetTools(MarkedObject* marked_object, std::list<Tool*>& t_list, const wxPoint& point, bool from_graphics_canvas, bool control_pressed)
{
	std::map<HeeksObj*, MarkedObject*>::iterator It;
	for (It = marked_object->m_map.begin(); It != marked_object->m_map.end(); It++)
	{
		MarkedObject* object = It->second;
		GetTools(object, t_list, point, from_graphics_canvas, control_pressed);
	}
	if (marked_object->GetObject())
	{
		std::list<Tool*> tools;

		marked_object->GetObject()->GetTools(&tools, &point);
		if(tools.size() > 0)tools.push_back(NULL);

		remove_object_tool.m_object = marked_object->GetObject();
		remove_object_tool.m_owner = remove_object_tool.m_object->m_owner;
		tools.push_back(&remove_object_tool);
		tools.push_back(NULL);

		mark_object_tool.m_marked_object = marked_object;
		mark_object_tool.m_point = point;
		mark_object_tool.m_xor_marked_list = control_pressed;
		tools.push_back(&mark_object_tool);

		if (tools.size()>0)
		{
			if (from_graphics_canvas || 1)
			{
				ToolList *function_list = new ToolList(marked_object->GetObject()->GetShortStringOrTypeString());
				function_list->Add(tools);
				t_list.push_back(function_list);
			}
			else
			{
				std::list<Tool*>::iterator It;
				for (It=tools.begin();It!=tools.end();It++)
					t_list.push_back(*It);
			}
		}
	}
}

wxString HeeksCADapp::GetExeFolder()const
{
	wxStandardPaths sp;
	wxString exepath = sp.GetExecutablePath();
	int last_fs = exepath.Find('/', true);
	int last_bs = exepath.Find('\\', true);
	wxString exedir;
	if(last_fs > last_bs){
		exedir = exepath.Truncate(last_fs);
	}
	else{
		exedir = exepath.Truncate(last_bs);
	}

	return exedir;
}

// do your own glBegin and glEnd
void HeeksCADapp::get_2d_arc_segments(double xs, double ys, double xe, double ye, double xc, double yc, bool dir, bool want_start, double pixels_per_mm, void(*callbackfunc)(const double* xy)){
	double ax = xs - xc;
	double ay = ys - yc;
	double bx = xe - xc;
	double by = ye - yc;

	double start_angle = atan2(ay, ax);
	double end_angle = atan2(by, bx);

	if(dir){
		if(start_angle > end_angle)end_angle += 6.28318530717958;
	}
	else{
		if(end_angle > start_angle)start_angle += 6.28318530717958;
	}

	double dxc = xs - xc;
	double dyc = ys - yc;
	double radius = sqrt(dxc*dxc + dyc*dyc);
	double d_angle = end_angle - start_angle;
	int segments = (int)(pixels_per_mm * radius * fabs(d_angle) / 6.28318530717958 + 1);
    
    double theta = d_angle / (double)segments;
    double tangetial_factor = tan(theta);
    double radial_factor = 1 - cos(theta);
    
    double x = radius * cos(start_angle);
    double y = radius * sin(start_angle);

    for(int i = 0; i < segments + 1; i++)
    {
		if(i != 0 || want_start){
			double xy[2] = {xc + x, yc + y};
			(*callbackfunc)(xy);
		}
        
        double tx = -y;
        double ty = x;
        
        x += tx * tangetial_factor;
        y += ty * tangetial_factor;
        
        double rx = - x;
        double ry = - y;
        
        x += rx * radial_factor;
        y += ry * radial_factor;
    }
}

void HeeksCADapp::PassMouseWheelToGraphics(wxMouseEvent& event)
{
	m_frame->m_graphics->OnMouse(event);
}

int HeeksCADapp::PickObjects(const wxChar* str, long marking_filter, bool just_one)
{
	CInputMode* save_mode = input_mode_object;
	m_select_mode->m_prompt_when_doing_a_main_loop.assign(str);
	m_select_mode->m_doing_a_main_loop = true;
	m_select_mode->m_just_one = just_one;
	SetInputMode(m_select_mode);

	// set marking filter
	long save_filter = wxGetApp().m_marked_list->m_filter;
	wxGetApp().m_marked_list->m_filter = marking_filter;

	// stay in an input loop until finished picking
	OnRun();

	// restore marking filter
	wxGetApp().m_marked_list->m_filter = save_filter;

	m_select_mode->m_doing_a_main_loop = false;
	SetInputMode(save_mode); // update tool bar
	return 1;
}

bool HeeksCADapp::PickPosition(const wxChar* str, double* pos, void(*callback)(const double*))
{
	CInputMode* save_mode = input_mode_object;
	m_digitizing->m_prompt_when_doing_a_main_loop.assign(str);
	m_digitizing->m_doing_a_main_loop = true;
	m_digitizing->m_callback = callback;
	SetInputMode(m_digitizing);

	OnRun();

	bool return_found = false;
	if(m_digitizing->digitized_point.m_type != DigitizeNoItemType){
		extract(m_digitizing->digitized_point.m_point, pos);
		return_found = true;
	}

	m_digitizing->m_doing_a_main_loop = false;
	m_digitizing->m_callback = NULL;
	SetInputMode(save_mode);
	return return_found;
}

static int sphere_display_list = 0;

void HeeksCADapp::glSphere(double radius, const double* pos)
{
	glPushMatrix();

	if(pos)
	{
		glTranslated(pos[0], pos[1], pos[2]);
	}

	glScaled(radius, radius, radius);

	if(sphere_display_list)
	{
		glCallList(sphere_display_list);
	}
	else{
		sphere_display_list = glGenLists(1);
		glNewList(sphere_display_list, GL_COMPILE_AND_EXECUTE);

		glBegin(GL_QUADS);

		double v_ang = 0.0;
		double pcosv = 0.0;
		double psinv = 0.0;

		for(int i = 0; i<11; i++, v_ang += 0.15707963267948966)
		{
			double cosv = cos(v_ang);
			double sinv = sin(v_ang);

			if(i > 0)
			{
				double h_ang = 0.0;
				double pcosh = 0.0;
				double psinh = 0.0;

				for(int j = 0; j<21; j++, h_ang += 0.314159265358979323)
				{
					double cosh = cos(h_ang);
					double sinh = sin(h_ang);

					if(j > 0)
					{
						// top quad
						glNormal3d(pcosh * pcosv, psinh * pcosv, psinv);
						glVertex3d(pcosh * pcosv, psinh * pcosv, psinv);
						glNormal3d(cosh * pcosv, sinh * pcosv, psinv);
						glVertex3d(cosh * pcosv, sinh * pcosv, psinv);
						glNormal3d(cosh * cosv, sinh * cosv, sinv);
						glVertex3d(cosh * cosv, sinh * cosv, sinv);
						glNormal3d(pcosh * cosv, psinh * cosv, sinv);
						glVertex3d(pcosh * cosv, psinh * cosv, sinv);

						// bottom quad
						glNormal3d(pcosh * pcosv, psinh * pcosv, -psinv);
						glVertex3d(pcosh * pcosv, psinh * pcosv, -psinv);
						glNormal3d(pcosh * cosv, psinh * cosv, -sinv);
						glVertex3d(pcosh * cosv, psinh * cosv, -sinv);
						glNormal3d(cosh * cosv, sinh * cosv, -sinv);
						glVertex3d(cosh * cosv, sinh * cosv, -sinv);
						glNormal3d(cosh * pcosv, sinh * pcosv, -psinv);
						glVertex3d(cosh * pcosv, sinh * pcosv, -psinv);
					}
					pcosh = cosh;
					psinh = sinh;
				}
			}
			pcosv = cosv;
			psinv = sinv;
		}

		glEnd();

		glEndList();
	}

	glPopMatrix();
}

void HeeksCADapp::OnNewOrOpen(bool open)
{
	for(std::list<wxDynamicLibrary*>::iterator It = m_loaded_libraries.begin(); It != m_loaded_libraries.end(); It++){
		wxDynamicLibrary* shared_library = *It;
		void(*fnOnNewOrOpen)(int) = (void(*)(int))(shared_library->GetSymbol(_T("OnNewOrOpen")));
		if(fnOnNewOrOpen){
			(*fnOnNewOrOpen)(open ? 1:0);
		}
	}
}

void HeeksCADapp::RegisterHideableWindow(wxWindow* w)
{
	m_hideable_windows.push_back(w);
}

void HeeksCADapp::GetRecentFilesProfileString()
{
	for(int i = 0; i < MAX_RECENT_FILES; i++)
	{
		wxChar key_name[1024];
		wsprintf(key_name, _T("RecentFilePath%d"), i);
		wxString filepath = m_config->Read(key_name);
		if(filepath.IsEmpty())break;
		m_recent_files.push_back(filepath);
	}
}

void HeeksCADapp::WriteRecentFilesProfileString()
{
	std::list< wxString >::iterator It = m_recent_files.begin();
	for(int i = 0; i < MAX_RECENT_FILES; i++)
	{
		wxChar key_name[1024];
		wsprintf(key_name, _T("RecentFilePath%d"), i);
		wxString filepath;
		if(It != m_recent_files.end())
		{
			filepath = *It;
			It++;
		}
		m_config->Write(key_name, filepath);
	}
}

void HeeksCADapp::InsertRecentFileItem(const wxChar* filepath)
{
	// add to recent files list
	m_recent_files.remove(filepath);
	m_recent_files.push_front( wxString( filepath ) );
	if(m_recent_files.size() > MAX_RECENT_FILES)m_recent_files.pop_back();
}

bool HeeksCADapp::CheckForModifiedDoc()
{
	// returns true if OK to continue opening file
	if(history->IsModified())
	{
		wxChar str[1024];
		wsprintf(str, _T("Save changes to %s"), m_filepath.c_str());
		int res = wxMessageBox(str, wxMessageBoxCaptionStr, wxCANCEL|wxYES_NO|wxCENTRE);
		if(res == wxCANCEL)return false;
		if(res == wxYES)
		{
			return SaveFile(m_filepath.c_str(), true);
		}
	}

	return true;
}

void HeeksCADapp::SetFrameTitle()
{
	wxChar str[1024];
	wxFileName f(m_filepath.c_str());
	wsprintf(str, _T("%s.%s - %s"), f.GetName(), f.GetExt(), m_filepath.c_str());
	m_frame->SetTitle(str);
}

HeeksObj* HeeksCADapp::GetIDObject(int type, int id)
{
	std::map< int, std::map<int, HeeksObj*> >::iterator FindIt1 = used_ids.find(type);
	if(FindIt1 == used_ids.end())return NULL;
	std::map<int, HeeksObj*> &map = FindIt1->second;
	std::map<int, HeeksObj*>::iterator FindIt2 = map.find(id);
	if(FindIt2 == map.end())return NULL;
	return FindIt2->second;
}

void HeeksCADapp::SetObjectID(HeeksObj* object, int id)
{
	int id_group_type = object->GetIDGroupType();

	std::map< int, std::map<int, HeeksObj*> >::iterator FindIt1 = used_ids.find(id_group_type);
	if(FindIt1 == used_ids.end())
	{
		// add a new map
		std::map<int, HeeksObj*> empty_map;
		FindIt1 = used_ids.insert( std::make_pair( id_group_type, empty_map )).first;		
	}
	std::map<int, HeeksObj*> &map = FindIt1->second;
	map.insert( std::pair<int, HeeksObj*> (id, object) );
	object->m_id = id;
}

int HeeksCADapp::GetNextID(int id_group_type)
{
	std::map< int, std::map<int, HeeksObj*> >::iterator FindIt1 = used_ids.find(id_group_type);
	if(FindIt1 == used_ids.end())return 1;
	std::map< int, int >::iterator FindIt2 = next_id_map.find(id_group_type);
	std::map<int, HeeksObj*> &map = FindIt1->second;

	if(FindIt2 == next_id_map.end())
	{
		// add a new int
		int next_id = map.begin()->first + 1;
		FindIt2 = next_id_map.insert( std::make_pair(id_group_type, next_id) ).first;
	}

	int &next_id = FindIt2->second;

	while(map.find(next_id) != map.end())next_id++;
	return next_id;
}

void HeeksCADapp::RemoveID(HeeksObj* object)
{
	int id_group_type = object->GetIDGroupType();
	std::map< int, std::map<int, HeeksObj*> >::iterator FindIt1 = used_ids.find(id_group_type);
	if(FindIt1 == used_ids.end())return;
	std::map< int, int >::iterator FindIt2 = next_id_map.find(id_group_type);
	std::map<int, HeeksObj*> &map = FindIt1->second;
	if(FindIt2 == next_id_map.end())
	{
		// add a new int
		int next_id = 0;
		FindIt2 = next_id_map.insert( std::make_pair(id_group_type, next_id) ).first;
	}
	int &next_id = FindIt2->second;
	next_id = object->m_id; // this id has now become available
	map.erase(next_id);
}

void HeeksCADapp::ResetIDs()
{
	used_ids.clear();
	next_id_map.clear();
}

void HeeksCADapp::WriteIDToXML(HeeksObj* object, TiXmlElement *element)
{
	element->SetAttribute("id", object->m_id);
}

void HeeksCADapp::ReadIDFromXML(HeeksObj* object, TiXmlElement *element)
{
	// get the attributes
	for(TiXmlAttribute* a = element->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "id"){object->SetID(a->IntValue());}
	}
}

static double* value_for_set_value = NULL;
static void set_value(double value, HeeksObj* object){*value_for_set_value = value;}

static bool *success_for_double_input = NULL;

class CInputApply:public Tool{
private:
	static wxBitmap* m_bitmap;

public:
	void Run(){
		*success_for_double_input = true;
		wxGetApp().ExitMainLoop();
	}
	const wxChar* GetTitle(){return _T("Apply");}
	wxBitmap* Bitmap(){if(m_bitmap == NULL){wxString exe_folder = wxGetApp().GetExeFolder();m_bitmap = new wxBitmap(exe_folder + _T("/bitmaps/apply.png"), wxBITMAP_TYPE_PNG);}	return m_bitmap;}
	const wxChar* GetToolTip(){return _T("Accept value and continue");}
};
wxBitmap* CInputApply::m_bitmap = NULL;

CInputApply input_apply;

class CInputCancel:public Tool{
private:
	static wxBitmap* m_bitmap;

public:
	void Run(){wxGetApp().ExitMainLoop();}
	const wxChar* GetTitle(){return _T("Cancel");}
	wxBitmap* Bitmap(){if(m_bitmap == NULL){wxString exe_folder = wxGetApp().GetExeFolder();m_bitmap = new wxBitmap(exe_folder + _T("/bitmaps/cancel.png"), wxBITMAP_TYPE_PNG);}return m_bitmap;}
	const wxChar* GetToolTip(){return _T("Cancel operation");}
};
wxBitmap* CInputCancel::m_bitmap = NULL;

CInputCancel input_cancel;

class CDoubleInput: public CInputMode, CLeftAndRight
{
public:
	wxString m_title;
	wxString m_value_title;
	double m_value;
	bool m_success;

	CDoubleInput(const wxChar* prompt, const wxChar* value_name, double initial_value)
	{
		m_title.assign(prompt);
		m_value_title.assign(value_name);
		m_value = initial_value;
		m_success = false;
	}
	virtual ~CDoubleInput(){}

	// virtual functions for InputMode
	const wxChar* GetTitle(){return m_title.c_str();}
	void OnMouse( wxMouseEvent& event ){
		bool event_used = false;
		if(LeftAndRightPressed(event, event_used))
			wxGetApp().ExitMainLoop();
	}
	void GetProperties(std::list<Property *> *list)
	{
		value_for_set_value = &m_value;
		list->push_back(new PropertyDouble(m_value_title.c_str(), m_value, NULL, set_value));
	}
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p)
	{
		// add a do it now button
		t_list->push_back(&input_apply);
		t_list->push_back(&input_cancel);
	}
};

bool HeeksCADapp::InputDouble(const wxChar* prompt, const wxChar* value_name, double &value)
{
	CInputMode* save_mode = input_mode_object;
	CDoubleInput double_input(prompt, value_name, value);
	success_for_double_input = &(double_input.m_success);
	SetInputMode(&double_input);

	OnRun();

	SetInputMode(save_mode);

	if(double_input.m_success)value = double_input.m_value;

	return double_input.m_success;
}

void HeeksCADapp::RegisterOnGLCommands( void(*callbackfunc)() )
{
	m_on_glCommands_list.push_back(callbackfunc);
}

void HeeksCADapp::RemoveOnGLCommands( void(*callbackfunc)() )
{
	m_on_glCommands_list.remove(callbackfunc);
}

void HeeksCADapp::RegisterOnGraphicsSize( void(*callbackfunc)(wxSizeEvent& evt) )
{
	m_on_graphics_size_list.push_back(callbackfunc);
}

void HeeksCADapp::RemoveOnGraphicsSize( void(*callbackfunc)(wxSizeEvent& evt) )
{
	m_on_graphics_size_list.remove(callbackfunc);
}

void HeeksCADapp::RegisterOnMouseFn( void(*callbackfunc)(wxMouseEvent&) )
{
	m_lbutton_up_callbacks.push_back(callbackfunc);
}

void HeeksCADapp::RemoveOnMouseFn( void(*callbackfunc)(wxMouseEvent&) )
{
	m_lbutton_up_callbacks.remove(callbackfunc);
}

void HeeksCADapp::CreateTransformGLList(bool show_grippers_on_drag){
	DestroyTransformGLList();
	m_transform_gl_list = glGenLists(1);
	glNewList(m_transform_gl_list, GL_COMPILE);
	std::list<HeeksObj *>::const_iterator It;
	for(It = m_marked_list->list().begin(); It != m_marked_list->list().end(); It++){
		(*It)->glCommands(false, true, false);
	}
	glDisable(GL_DEPTH_TEST);
	if(show_grippers_on_drag)m_marked_list->GrippersGLCommands(false, false);
	glEnable(GL_DEPTH_TEST);
	glEndList();
}

void HeeksCADapp::DestroyTransformGLList(){
	if (m_transform_gl_list)
	{
		glDeleteLists(m_transform_gl_list, 1);
	}
	m_transform_gl_list = 0;
}

bool HeeksCADapp::IsPasteReady()
{
	wxString fstr;

	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported( wxDF_TEXT ))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData( data );
			fstr = data.GetText();
		}  
		wxTheClipboard->Close();

		if(fstr.StartsWith(_T("<?xml version=\"1.0\" ?>\r\n<HeeksCAD_Document>")))return true;
	}

	return false;
}

void HeeksCADapp::Paste()
{
	// assume the text is the contents of a heeks file
	wxString fstr;

	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported( wxDF_TEXT ))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData( data );
			fstr = data.GetText();
		}  
		wxTheClipboard->Close();
	}

	// write a temporary file
	wxStandardPaths sp;
	sp.GetTempDir();
	wxString temp_file = sp.GetTempDir() + _T("/temp_Heeks_clipboard_file.heeks");

	{
#if wxUSE_UNICODE
		wofstream ofs(temp_file);
#else
		ofstream ofs(temp_file);
#endif
		ofs<<fstr.c_str();
	}

	m_marked_list->Clear();
	m_mark_newly_added_objects = true;
	wxGetApp().OpenFile(temp_file, true);
	m_mark_newly_added_objects = false;
}
