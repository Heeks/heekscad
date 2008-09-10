// HeeksCAD.cpp

#include "stdafx.h"
#include "HeeksCAD.h"
#include <wx/filedlg.h>
#include <wx/clipbrd.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/cmdline.h>
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
#include "SelectMode.h"
#include "MagDragWindow.h"
#include "DigitizeMode.h"
#include "GripperMode.h"
#include "ConversionTools.h"
#include "Solid.h"
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
#include "RemoveOrAddTool.h"
#include "LineArcCollection.h"
#include "../tinyxml/tinyxml.h"
#include "BezierCurve.h"


const int ID_TOOLBAR = 500;
const int ID_SOLID_TOOLBAR = 501;
static const long TOOLBAR_STYLE = wxTB_FLAT | wxTB_DOCKABLE | wxTB_TEXT;

IMPLEMENT_APP(HeeksCADapp)

HeeksCADapp::HeeksCADapp(): ObjList()
{
	m_geom_tol = 0.001;
	background_color = HeeksColor(0, 0, 0);
	current_color = HeeksColor(0, 0, 0);
	construction_color = HeeksColor(0, 0, 255);
	input_mode_object = NULL;
	cur_mouse_pos.x = 0;
	cur_mouse_pos.y = 0;
	gripper_mode = new GripperMode;
	drag_gripper = NULL;
	cursor_gripper = NULL;
	magnification = new MagDragWindow();
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
	m_filepath.assign("Untitled.heeks");
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
	delete gripper_mode;
	delete m_config;
}

bool HeeksCADapp::OnInit()
{
	m_config = new wxConfig("HeeksCAD");

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
	m_config->Read("MainFrameWidth", &width);
	m_config->Read("MainFrameHeight", &height);
	m_config->Read("MainFramePosX", &posx);
	m_config->Read("MainFramePosY", &posy);
	if(posx < 0)posx = 0;
	if(posy < 0)posy = 0;
	m_config->Read("DrawEnd", &digitize_end, true);
	m_config->Read("DrawInters", &digitize_inters, false);
	m_config->Read("DrawCentre", &digitize_centre, false);
	m_config->Read("DrawMidpoint", &digitize_midpoint, false);
	m_config->Read("DrawNearest", &digitize_nearest, false);
	m_config->Read("DrawTangent", &digitize_tangent, false);
	m_config->Read("DrawCoords", &digitize_coords, true);
	m_config->Read("DrawScreen", &digitize_screen, false);
	m_config->Read("DrawToGrid", &draw_to_grid, false);
	m_config->Read("DrawGrid", &digitizing_grid);
	m_config->Read("DrawRadius", &digitizing_radius);
	{
		wxString str;
		m_config->Read("BackgroundColor", &str, "242 204 162");
		int r = 0, g = 0, b = 0;
		sscanf(str, "%d %d %d", &r, &g, &b);
		background_color = HeeksColor(r, g, b);
	}
	{
		wxString str;
		m_config->Read("CurrentColor", &str, "0 0 0");
		int r = 0, g = 0, b = 0;
		sscanf(str, "%d %d %d", &r, &g, &b);
		current_color = HeeksColor(r, g, b);
	}
	{
		wxString str;
		m_config->Read("ConstructionColor", &str, "0 0 255");
		int r = 0, g = 0, b = 255;
		sscanf(str, "%d %d %d", &r, &g, &b);
		construction_color = HeeksColor(r, g, b);
	}
	m_config->Read("RotateMode", &m_rotate_mode);
	m_config->Read("Antialiasing", &m_antialiasing);
	m_config->Read("GridMode", &grid_mode);
	m_config->Read("m_light_push_matrix", &m_light_push_matrix);
	m_config->Read("WheelForwardAway", &mouse_wheel_forward_away);
	m_config->Read("CtrlDoesRotate", &ctrl_does_rotate);
	gripper_mode->GetProfileStrings();

	GetRecentFilesProfileString();

	wxImage::AddHandler(new wxPNGHandler);
	m_frame = new CHeeksFrame( wxT( "HeeksCAD free Solid Modelling software based on Open CASCADE" ), wxPoint(posx, posy), wxSize(width, height));
	m_frame->SetIcon(wxICON(HeeksCAD));
	SetInputMode(m_select_mode);
	m_frame->Show(TRUE);
	SetTopWindow(m_frame);

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
	m_config->Write("DrawEnd", digitize_end);
	m_config->Write("DrawInters", digitize_inters);
	m_config->Write("DrawCentre", digitize_centre);
	m_config->Write("DrawMidpoint", digitize_midpoint);
	m_config->Write("DrawNearest", digitize_nearest);
	m_config->Write("DrawTangent", digitize_tangent);
	m_config->Write("DrawCoords", digitize_coords);
	m_config->Write("DrawScreen", digitize_screen);
	m_config->Write("DrawToGrid", draw_to_grid);
	m_config->Write("DrawGrid", digitizing_grid);
	m_config->Write("DrawRadius", digitizing_radius);
	{
		char str[1024];
		sprintf(str, "%d %d %d", background_color.red, background_color.green, background_color.blue);
		m_config->Write("BackgroundColor", str);
	}
	{
		char str[1024];
		sprintf(str, "%d %d %d", current_color.red, current_color.green, current_color.blue);
		m_config->Write("CurrentColor", str);
	}
	{
		char str[1024];
		sprintf(str, "%d %d %d", construction_color.red, construction_color.green, construction_color.blue);
		m_config->Write("ConstructionColor", str);
	}
	m_config->Write("RotateMode", m_rotate_mode);	
	m_config->Write("Antialiasing", m_antialiasing);	
	m_config->Write("GridMode", grid_mode);
	m_config->Write("m_light_push_matrix", m_light_push_matrix);
	m_config->Write("WheelForwardAway", mouse_wheel_forward_away);
	m_config->Write("CtrlDoesRotate", ctrl_does_rotate);

	WriteRecentFilesProfileString();

	return result;
}

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
	m_frame->m_graphics->DrawFront();
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

class CFullScreenTool : public Tool  
{
public:
	// Tool's virtual functions
	const char* CFullScreenTool::GetTitle(){
		if (wxGetApp().m_frame->IsFullScreen()) return "Exit Full Screen Mode";
		else return "Show Full Screen";
	}
	void CFullScreenTool::Run(){
		wxGetApp().m_frame->ShowFullScreen(!wxGetApp().m_frame->IsFullScreen());
	}
};

void HeeksCADapp::GetTools(std::list<Tool*>* t_list, const wxPoint* point, MarkedObject* marked_object)
{
	if(point->x>=0 && point->y>=0)t_list->push_back(new CFullScreenTool);
	GetConversionMenuTools(t_list, point, marked_object);
}

void HeeksCADapp::Reset(){
	m_marked_list->Clear();
	std::set<Observer*>::iterator It;
	for(It = observers.begin(); It != observers.end(); It++){
		Observer *ov = *It;
		ov->Clear();
	}
	ClearUndoably();
	EndHistory();
	delete history;
	history = new MainHistory;
	m_doing_rollback = false;
	m_frame->m_graphics->m_view_point.SetView(gp_Vec(0, 1, 0), gp_Vec(0, 0, 1));
	m_filepath.assign("Untitled.heeks");
}

static std::map< std::string, HeeksObj*(*)(TiXmlElement* pElem) > *xml_read_fn_map = NULL;

static HeeksObj* ReadSTEPFileFromXMLElement(TiXmlElement* pElem)
{
	std::map<int, int> index_map;

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
					int id = -1, index = -1;

					// get the attributes
					for(TiXmlAttribute* a = subsubElem->FirstAttribute(); a; a = a->Next())
					{
						wxString attr_name(a->Name());
						if(attr_name == "index"){index = a->IntValue();}
						else if(attr_name == "id"){id = a->IntValue();}
					}
					if(id != -1 && index != -1)index_map.insert(std::pair<int, int>(index, id));
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
				wxString temp_file = sp.GetTempDir() + "/temp_HeeksCAD_STEP_file.step";
				{
					ofstream ofs(temp_file);
					ofs<<file_text;
				}
				CShape::ImportSolidsFile(temp_file, false, &index_map);
			}
		}
	}

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		wxString name(a->Name());
		if(name == "text")
		{
			wxStandardPaths sp;
			sp.GetTempDir();
			wxString temp_file = sp.GetTempDir() + "/temp_HeeksCAD_STEP_file.step";
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
		xml_read_fn_map->insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Image", HImage::ReadFromXMLElement ) );
		xml_read_fn_map->insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "LineArcCollection", CLineArcCollection::ReadFromXMLElement ) );
		xml_read_fn_map->insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "STEP_file", ReadSTEPFileFromXMLElement ) );
	}
}

void HeeksCADapp::RegisterReadXMLfunction(const char* type_name, HeeksObj*(*read_xml_function)(TiXmlElement* pElem))
{
	if(xml_read_fn_map->find(type_name) != xml_read_fn_map->end()){
		char str[1024];
		sprintf(str, "Error - trying to register an XML read function for an exisiting type - %s", type_name);
		wxMessageBox(str);
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

void HeeksCADapp::OpenXMLFile(const char *filepath, bool update_recent_file_list, bool set_app_caption)
{
	TiXmlDocument doc(filepath);
	if (!doc.LoadFile())
	{
		if(doc.Error())
		{
			wxMessageBox(doc.ErrorDesc());
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
		const char* name=pElem->Value();

		if(wxString(name) != wxString("HeeksCAD_Document"))
		{
			wxMessageBox("This is not a HeeksCAD document!");
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

	WereAdded(m_objects);

	m_filepath.assign(filepath);
	if(update_recent_file_list)InsertRecentFileItem(filepath);
	if(set_app_caption)SetFrameTitle();
}

static CLineArcCollection* line_arc_collection_for_callback = NULL;
static void add_line_from_bezier_curve(const gp_Pnt& vt0, const gp_Pnt& vt1)
{
	HLine* new_object = new HLine(vt0, vt1, &(wxGetApp().current_color));
	line_arc_collection_for_callback->Add(new_object, NULL);
}

void HeeksCADapp::ReadSVGElement(TiXmlElement* pElem)
{
	std::string name(pElem->Value());

	if(name == std::string("g"))
	{
		// loop through all the child elements, looking for path
		for(pElem = TiXmlHandle(pElem).FirstChildElement().Element(); pElem; pElem = pElem->NextSiblingElement())
		{
			ReadSVGElement(pElem);
		}
		return;
	}

	if(name == std::string("path"))
	{
		// get the attributes
		for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
		{
			wxString name(a->Name());
			if(name == "d")
			{
				// add lines and arcs and bezier curves
				const char* d = a->Value();

				double sx, sy;
				double px, py;
				CLineArcCollection* line_arc_collection = NULL;

				int pos = 0;
				while(1){
					if(d[pos] == 'M'){
						// make a line arc collection
						pos++;
						sscanf(&d[pos], "%lf,%lf", &sx, &sy);
						sy = -sy;
						px = sx; py = sy;
						line_arc_collection = new CLineArcCollection;
						Add(line_arc_collection, NULL);
					}
					else if(d[pos] == 'L'){
						// add a line
						pos++;
						double x, y;
						sscanf(&d[pos], "%lf,%lf", &x, &y);
						y = -y;
						HLine* new_object = new HLine(gp_Pnt(px, py, 0), gp_Pnt(x, y, 0), &current_color);
						px = x; py = y;
						line_arc_collection->Add(new_object, NULL);
					}
					else if(d[pos] == 'C'){
						// add a bezier curve ( just split into lines for now )
						pos++;
						double x1, y1, x2, y2, x3, y3;
						sscanf(&d[pos], "%lf,%lf %lf,%lf %lf,%lf", &x1, &y1, &x2, &y2, &x3, &y3);
						y1 = -y1; y2 = -y2; y3 = -y3;
						line_arc_collection_for_callback = line_arc_collection;
						split_bezier_curve(3, gp_Pnt(px, py,  0), gp_Pnt(x3, y3, 0), gp_Pnt(x1, y1, 0), gp_Pnt(x2, y2, 0), add_line_from_bezier_curve);
						px = x3; py = y3;
					}
					else if(toupper(d[pos]) == 'Z'){
						// join to end
						pos++;
						HLine* new_object = new HLine(gp_Pnt(px, py, 0), gp_Pnt(sx, sy, 0), &current_color);
						px = sx; py = sy;
						line_arc_collection->Add(new_object, NULL);
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

void HeeksCADapp::OpenSVGFile(const char *filepath, bool update_recent_file_list, bool set_app_caption)
{
	TiXmlDocument doc(filepath);
	if (!doc.LoadFile())
	{
		if(doc.Error())
		{
			wxMessageBox(doc.ErrorDesc());
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
		const char* name=pElem->Value();

		if(wxString(name) != wxString("svg"))
		{
			wxMessageBox("This is not an SVG document!");
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

	WereAdded(m_objects);

	m_filepath.assign(filepath);
	if(update_recent_file_list)InsertRecentFileItem(filepath);
	if(set_app_caption)SetFrameTitle();
}

bool HeeksCADapp::OpenFile(const char *filepath, bool update_recent_file_list, bool set_app_caption)
{
	// i'm not sure what return value means if anything

	wxString wf(filepath);

	if(wf.EndsWith(".heeks") || wf.EndsWith(".HEEKS"))
	{
		OpenXMLFile(filepath, update_recent_file_list, set_app_caption);
		return true;
	}

	if(wf.EndsWith(".svg") || wf.EndsWith(".SVG"))
	{
		OpenSVGFile(filepath, update_recent_file_list, set_app_caption);
		return true;
	}

	if(wf.EndsWith(".stl") || wf.EndsWith(".STL"))
	{
		// open stl file
		// to do
		return true;
	}

	// check for solid files
	if(CShape::ImportSolidsFile(filepath, false))
	{
		WereAdded(m_objects);
		m_filepath.assign(filepath);
		if(update_recent_file_list)InsertRecentFileItem(filepath);
		if(set_app_caption)SetFrameTitle();
		return true;
	}

	// error
	char mess[1024];
	sprintf(mess, "Invalid file type chosen ( expecting file with %s suffix )", wxGetApp().GetKnownFilesCommaSeparatedList());
	wxMessageBox(mess);

	return false;
}

void HeeksCADapp::SaveXMLFile(const char *filepath, bool update_recent_file_list, bool set_app_caption)
{
	// write an xml file
	TiXmlDocument doc;  
	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  

	TiXmlElement * root = new TiXmlElement( "HeeksCAD_Document" );  
	doc.LinkEndChild( root );  

	// loop through all the objects writing them
	CShape::m_solids_found = false;
	for(std::list<HeeksObj*>::iterator It = m_objects.begin(); It != m_objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->WriteXML(root);
	}

	// write a step file for all the solids
	if(CShape::m_solids_found){
		wxStandardPaths sp;
		sp.GetTempDir();
		wxString temp_file = sp.GetTempDir() + "/temp_HeeksCAD_STEP_file.step";
		std::map<int, int> index_map;
		CShape::ExportSolidsFile(temp_file, &index_map);

		TiXmlElement *step_file_element = new TiXmlElement( "STEP_file" );
		root->LinkEndChild( step_file_element );  

		// write the index map as a child of step_file
		{
			TiXmlElement *index_map_element = new TiXmlElement( "index_map" );
			step_file_element->LinkEndChild( index_map_element );
			for(std::map<int, int>::iterator It = index_map.begin(); It != index_map.end(); It++)
			{
				TiXmlElement *index_pair_element = new TiXmlElement( "index_pair" );
				index_map_element->LinkEndChild( index_pair_element );
				index_pair_element->SetAttribute("index", It->first);
				index_pair_element->SetAttribute("id", It->second);
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

	doc.SaveFile( filepath );  

	m_filepath.assign(filepath);
	if(update_recent_file_list)InsertRecentFileItem(filepath);
	if(set_app_caption)SetFrameTitle();
	SetLikeNewFile();
}

bool HeeksCADapp::SaveFile(const char *filepath, bool use_dialog, bool update_recent_file_list, bool set_app_caption)
{
	if(use_dialog){
		wxFileDialog fd(m_frame, _T("Save graphical data file"), wxEmptyString, filepath, GetKnownFilesWildCardString(), wxSAVE|wxOVERWRITE_PROMPT);
		fd.SetFilterIndex(1);
		if (fd.ShowModal() == wxID_CANCEL)return false;
		return SaveFile( fd.GetPath().c_str(), false, update_recent_file_list );
	}

	wxString wf(filepath);

	if(wf.EndsWith(".heeks") || wf.EndsWith(".HEEKS"))
	{
		SaveXMLFile(filepath, update_recent_file_list, set_app_caption);
		return true;
	}

	if(CShape::ExportSolidsFile(filepath))
	{
		m_filepath.assign(filepath);
		if(update_recent_file_list)InsertRecentFileItem(filepath);
		if(set_app_caption)SetFrameTitle();
		SetLikeNewFile();
		return true;
	}
	else
	{
		char mess[1024];
		sprintf(mess, "Invalid file type chosen ( expecting file with %s suffix )", wxGetApp().GetKnownFilesCommaSeparatedList());
		wxMessageBox(mess);
	}

	return false;
}


void HeeksCADapp::Repaint(bool soon)
{
	if(soon){
#ifdef __WXMSW__
		::SendMessage((HWND)(m_frame->m_graphics->GetHandle()), WM_PAINT, 0, 0);
#else
		m_frame->m_graphics->Update();
#endif
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
	glDepthMask(1);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glShadeModel(GL_FLAT);
	m_frame->m_graphics->m_view_point.SetPolygonOffset();
	RenderGrid(&view_point);
	RenderRuler();
	glCommands(select, false, false);
	input_mode_object->OnRender();
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

struct ToolIndex{
	Tool *m_tool;
	int m_index;
};

static void AddToolToListAndMenu(Tool *t, std::vector<ToolIndex> &tool_index_list, wxMenu *menu)
{
	if (t == NULL)
		menu->AppendSeparator();
	else if (t->IsAToolList())
	{
		wxMenu *menu2 = new wxMenu;
		std::list<Tool*>& tool_list = ((ToolList*)t)->m_tool_list;
		std::list<Tool*>::iterator It;
		for (It=tool_list.begin();It!=tool_list.end();It++)
		{
			AddToolToListAndMenu(*It, tool_index_list, menu2);
		}
		menu->Append(0, t->GetTitle(), menu2);
	}
	else
	{
		ToolIndex ti;
		ti.m_tool = t;
		ti.m_index = tool_index_list.size();
		tool_index_list.push_back(ti);
		menu->Append(ti.m_index+1, t->GetTitle());
		if(t->Disabled())menu->Enable(ti.m_index+1, false);
		if(t->Checked ())menu->Check(ti.m_index+1, true);
	}
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

class DeleteMarkedListTool : public Tool
{
protected:
	std::string m_text;

public:
	DeleteMarkedListTool(void) {m_text.assign("Delete");}
	DeleteMarkedListTool(const char* text) {m_text.assign(text);}
	
	const char* GetTitle() {return m_text.c_str();}
	void Run() {wxGetApp().DeleteMarkedItems();}
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
	{
		if (m_marked_list->size() > 1)
		{
			f_list.push_back(new DeleteMarkedListTool("Delete Marked Items"));
			f_list.push_back(NULL);
		}
		else if (marked_object->GetFirstOfEverything())
		{
			AddMenusToToolList(marked_object, f_list, new_point, from_graphics_canvas, control_pressed);
		}
	}
	temp_f_list.clear();
	if(input_mode_object)input_mode_object->GetTools(&temp_f_list, &new_point);
	AddToolListWithSeparator(f_list, temp_f_list);
	temp_f_list.clear();
	GetTools(&temp_f_list, &new_point, marked_object);
	AddToolListWithSeparator(f_list, temp_f_list);
	std::list<Tool*>::iterator FIt;
	for (FIt = f_list.begin(); FIt != f_list.end(); FIt++)
		AddToolToListAndMenu(*FIt, tool_index_list, &menu);
	wnd->PopupMenu(&menu, point);
}

void HeeksCADapp::on_menu_event(wxCommandEvent& event)
{
	int id = event.GetId();
	if(id){
		Tool *t = tool_index_list[id - 1].m_tool;
		if(t->Undoable())DoToolUndoably(t);
		else{
			t->Run();
			delete t;
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

void HeeksCADapp::StartHistory(const char* str)
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
	return true;
}

void HeeksCADapp::DeleteUndoably(HeeksObj *object){
	if(object == NULL)return;
	RemoveObjectTool *tool = new RemoveObjectTool(object);
	char str[1024];
	sprintf(str, "Deleting %s", object->GetShortStringOrTypeString());
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

void HeeksCADapp::SetDrawMatrix(const gp_Trsf& mat)
{
	digitizing_matrix = mat;
}

gp_Trsf HeeksCADapp::GetDrawMatrix(bool get_the_appropriate_orthogonal)
{
	if(get_the_appropriate_orthogonal){
		gp_Vec vx, vy;
		m_frame->m_graphics->m_view_point.GetTwoAxes(vx, vy, false, 0);
		{
			return make_matrix(gp_Pnt(0, 0, 0).Transformed(digitizing_matrix), vx, vy);
		}
	}
	return digitizing_matrix;
}

void on_set_background_color(HeeksColor value)
{
	wxGetApp().background_color = value;
	wxGetApp().Repaint();
}

void on_set_current_color(HeeksColor value)
{
	wxGetApp().current_color = value;
}

void on_set_construction_color(HeeksColor value)
{
	wxGetApp().construction_color = value;
}

void on_set_grid_mode(int value)
{
	wxGetApp().grid_mode = value;
	wxGetApp().Repaint();
}

void on_grid(bool onoff)
{
	wxGetApp().draw_to_grid = onoff;
	wxGetApp().Repaint();
}

void on_grid_edit(double grid_value)
{
	wxGetApp().digitizing_grid = grid_value;
	wxGetApp().Repaint();
}

void on_set_geom_tol(double value)
{
	wxGetApp().m_geom_tol = value;
	wxGetApp().Repaint();
}

static std::list<Property *> *list_for_GetOptions = NULL;

static void AddPropertyCallBack(Property* p)
{
	list_for_GetOptions->push_back(p);
}

void HeeksCADapp::GetOptions(std::list<Property *> *list)
{
	list->push_back ( new PropertyColor ( "background color",  background_color, on_set_background_color ) );
	list->push_back ( new PropertyColor ( "current color",  current_color, on_set_current_color ) );
	list->push_back ( new PropertyColor ( "construction color",  construction_color, on_set_construction_color ) );
	{
		std::list< std::string > choices;
		choices.push_back ( std::string ( "no grid" ) );
		choices.push_back ( std::string ( "faint color" ) );
		choices.push_back ( std::string ( "alpha blending" ) );
		choices.push_back ( std::string ( "colored alpha blending" ) );
		list->push_back ( new PropertyChoice ( "grid mode",  choices, grid_mode, on_set_grid_mode ) );
	}
	list->push_back(new PropertyDouble("grid size", digitizing_grid, on_grid_edit));
	list->push_back(new PropertyCheck("grid", draw_to_grid, on_grid));
	list->push_back(new PropertyDouble("geometry tolerance", m_geom_tol, on_set_geom_tol));
	for(std::list<wxDynamicLibrary*>::iterator It = m_loaded_libraries.begin(); It != m_loaded_libraries.end(); It++){
		wxDynamicLibrary* shared_library = *It;
		list_for_GetOptions = list;
		void(*GetOptions)(void(*)(Property*)) = (void(*)(void(*)(Property*)))(shared_library->GetSymbol("GetOptions"));
		(*GetOptions)(AddPropertyCallBack);
	}
}

void HeeksCADapp::DeleteMarkedItems()
{
	if(m_marked_list->size() == 1){
		DeleteUndoably(*(m_marked_list->list().begin()));
	}
	else if(m_marked_list->size()>1){
		StartHistory("Delete Marked Items");
		DeleteUndoably(m_marked_list->list());
		EndHistory();
	}
	Repaint(0);
}

void HeeksCADapp::AddUndoably(HeeksObj *object, HeeksObj* owner, HeeksObj* prev_object)
{
	if(object == NULL)return;
	if(owner == NULL)owner = this;
	AddObjectTool *tool = new AddObjectTool(object, owner, prev_object);
	char str[1024];
	sprintf(str, "*Adding %s", object->GetShortStringOrTypeString());
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

const char* HeeksCADapp::GetKnownFilesWildCardString()const
{
	return "Known Files |*.heeks;*.igs;*.iges;*.stp;*.step;*.stl;*.svg|Heeks files (*.heeks)|*.heeks|IGES files (*.igs *.iges)|*.igs;*.iges|STEP files (*.stp *.step)|*.stp;*.step|STL files (*.stl)|*.stl|Scalar Vector Graphics files (*.svg)|*.svg";
}

const char* HeeksCADapp::GetKnownFilesCommaSeparatedList()const
{
	return "heeks, igs, iges, stp, step, stl, svg";
}

class MarkObjectTool:public Tool{
private:
	MarkedObject *m_marked_object;
	wxPoint m_point;
	bool m_xor_marked_list;

public:
	MarkObjectTool(MarkedObject *o, const wxPoint& point, bool xor_marked_list){m_marked_object = o; m_point = point; m_xor_marked_list = xor_marked_list;}

	// Tool's virtual functions
	const char* GetTitle(){return "Mark";}
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

void HeeksCADapp::AddMenusToToolList(MarkedObject* marked_object, std::list<Tool*>& t_list, const wxPoint& point, bool from_graphics_canvas, bool control_pressed)
{
	std::map<HeeksObj*, MarkedObject*>::iterator It;
	for (It = marked_object->m_map.begin(); It != marked_object->m_map.end(); It++)
	{
		MarkedObject* object = It->second;
		AddMenusToToolList(object, t_list, point, from_graphics_canvas, control_pressed);
	}
	if (marked_object->GetObject())
	{
		std::list<Tool*> tools;

		tools.push_back(new RemoveObjectTool(marked_object->GetObject()));
		tools.push_back(NULL);
		if(from_graphics_canvas){
			if (!m_marked_list->ObjectMarked(marked_object->GetObject()))
			{
				tools.push_back(new MarkObjectTool(marked_object, point, control_pressed));
			}
		}
		unsigned int s = tools.size();
		marked_object->GetObject()->GetTools(&tools, &point);
		if (tools.size()>s) tools.push_back(NULL);

		if (tools.size()>0)
		{
			if (from_graphics_canvas)
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

int HeeksCADapp::PickObjects(const char* str)
{
	CInputMode* save_mode = input_mode_object;
	m_select_mode->m_prompt_when_doing_a_main_loop.assign(str);
	m_select_mode->m_doing_a_main_loop = true;
	SetInputMode(m_select_mode);

	OnRun();

	m_select_mode->m_doing_a_main_loop = false;
	SetInputMode(save_mode); // update tool bar
	return 1;
}

bool HeeksCADapp::PickPosition(const char* str, double* pos)
{
	CInputMode* save_mode = input_mode_object;
	m_digitizing->m_prompt_when_doing_a_main_loop.assign(str);
	m_digitizing->m_doing_a_main_loop = true;
	SetInputMode(m_digitizing);

	OnRun();

	bool return_found = false;
	if(m_digitizing->digitized_point.m_type != DigitizeNoItemType){
		extract(m_digitizing->digitized_point.m_point, pos);
		return_found = true;
	}

	m_digitizing->m_doing_a_main_loop = false;
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
		void(*fnOnNewOrOpen)(int) = (void(*)(int))(shared_library->GetSymbol("OnNewOrOpen"));
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
		char key_name[1024];
		sprintf(key_name, "RecentFilePath%d", i);
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
		char key_name[1024];
		sprintf(key_name, "RecentFilePath%d", i);
		wxString filepath;
		if(It != m_recent_files.end())
		{
			filepath = *It;
			It++;
		}
		m_config->Write(key_name, filepath);
	}
}

void HeeksCADapp::InsertRecentFileItem(const char* filepath)
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
		char str[1024];
		sprintf(str, "Save changes to %s", m_filepath.c_str());
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
	char str[1024];
	wxFileName f(m_filepath.c_str());
	sprintf(str, "%s.%s - %s", f.GetName(), f.GetExt(), m_filepath.c_str());
	m_frame->SetTitle(str);
}