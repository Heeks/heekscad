// HeeksCAD.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "HeeksCAD.h"
#include <wx/filedlg.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/cmdline.h>
#include <wx/clipbrd.h>
#include <wx/aui/aui.h>
#include <wx/toolbar.h>
#include "../interface/Tool.h"
#include "../interface/Material.h"
#include "../interface/ToolList.h"
#include "../interface/HeeksObj.h"
#include "../interface/MarkedObject.h"
#include "../interface/PropertyColor.h"
#include "../interface/PropertyChoice.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "../interface/PropertyInt.h"
#include "../interface/PropertyCheck.h"
#include "../interface/PropertyString.h"
#include "../interface/PropertyList.h"
#include "HeeksFrame.h"
#include "GraphicsCanvas.h"
#include "OptionsCanvas.h"
#include "InputModeCanvas.h"
#include "TreeCanvas.h"
#include "ObjPropsCanvas.h"
#include "SelectMode.h"
#include "MagDragWindow.h"
#include "ViewRotating.h"
#include "ViewZooming.h"
#include "DigitizeMode.h"
#include "Shape.h"
#include "Face.h"
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
#include "HEllipse.h"
#include "HSpline.h"
#include "HText.h"
#include "HDimension.h"
#include "HXml.h"
#include "RemoveOrAddTool.h"
#include "Sketch.h"
#include "../tinyxml/tinyxml.h"
#include "BezierCurve.h"
#include "StlSolid.h"
#include "dxf.h"
#include "svg.h"
#include "CoordinateSystem.h"
#include "RegularShapesDrawing.h"
#include "HeeksPrintout.h"
#include "HeeksConfig.h"
#include "Group.h"
#include <fstream>
using namespace std;

IMPLEMENT_APP(HeeksCADapp)

#if 0
int MyAllocHook( int allocType, void *userData, size_t size, int blockType, long requestNumber, const unsigned char *filename, int lineNumber)
{
	if (size==24 && requestNumber > 14000 && requestNumber < 18000)
	{
		int here = 0;
		here = 3;
	}
	return TRUE;
}
#endif

HeeksCADapp::HeeksCADapp(): ObjList()
{
#if 0
	_CrtSetAllocHook(MyAllocHook);
#endif

	m_version_number = _T("0 7 0");
	m_geom_tol = 0.001;
	m_view_units = 1.0;
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
	viewzooming = new ViewZooming;
	m_select_mode = new CSelectMode();
	m_digitizing = new DigitizeMode();
	digitize_end = false;
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
	m_marked_list = new MarkedList;
	history = new MainHistory;
	m_doing_rollback = false;
	mouse_wheel_forward_away = true;
	ctrl_does_rotate = false;
	m_ruler = new HRuler();
	m_show_ruler = false;
	m_show_datum_coords_system = true;
	m_datum_coords_system_solid_arrows = true;
	m_filepath = wxString(_("Untitled")) + _T(".heeks");
	m_in_OpenFile = false;
	m_transform_gl_list = 0;
	m_current_coordinate_system = NULL;
	m_mark_newly_added_objects = false;
	m_show_grippers_on_drag = true;
	m_extrude_removes_sketches = false;
	m_loft_removes_sketches = false;
	m_font_tex_number = 0;
	m_graphics_text_mode = GraphicsTextModeNone;
	m_locale_initialised = false;
	m_printData = NULL;
	m_pageSetupData = NULL;
	m_file_open_or_import_type = FileOpenOrImportTypeOther;
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
	delete viewrotating;
	delete viewzooming;
	delete m_ruler;
	if(m_printData)delete m_printData;
	if(m_pageSetupData)delete m_pageSetupData;
}

bool HeeksCADapp::OnInit()
{
	wxInitAllImageHandlers();

	InitialiseLocale();

	ClearHistory();

    m_printData = new wxPrintData;
    m_pageSetupData = new wxPageSetupDialogData;
    // copy over initial paper size from print record
    (*m_pageSetupData) = *m_printData;
    // Set some initial page margins in mm. 
    m_pageSetupData->SetMarginTopLeft(wxPoint(15, 15));
    m_pageSetupData->SetMarginBottomRight(wxPoint(15, 15));

	int width = 600;
	int height = 400;
	int posx = 200;
	int posy = 200;
	HeeksConfig config;
	config.Read(_T("MainFrameWidth"), &width);
	config.Read(_T("MainFrameHeight"), &height);
	config.Read(_T("MainFramePosX"), &posx);
	config.Read(_T("MainFramePosY"), &posy);
	if(posx < 0)posx = 0;
	if(posy < 0)posy = 0;
	config.Read(_T("DrawEnd"), &digitize_end, false);
	config.Read(_T("DrawInters"), &digitize_inters, false);
	config.Read(_T("DrawCentre"), &digitize_centre, false);
	config.Read(_T("DrawMidpoint"), &digitize_midpoint, false);
	config.Read(_T("DrawNearest"), &digitize_nearest, false);
	config.Read(_T("DrawTangent"), &digitize_tangent, false);
	config.Read(_T("DrawCoords"), &digitize_coords, true);
	config.Read(_T("DrawScreen"), &digitize_screen, false);
	config.Read(_T("DrawToGrid"), &draw_to_grid, true);
	config.Read(_T("DrawGrid"), &digitizing_grid);
	config.Read(_T("DrawRadius"), &digitizing_radius);
	{
		wxString str;
		config.Read(_T("BackgroundColor"), &str, _T("242 204 162"));
		int r = 0, g = 0, b = 0;
#if wxUSE_UNICODE
		swscanf(str, _T("%d %d %d"), &r, &g, &b);
#else
		sscanf(str, _T("%d %d %d"), &r, &g, &b);
#endif
		background_color = HeeksColor((unsigned char)r, (unsigned char)g, (unsigned char)b);
	}
	{
		wxString str;
		config.Read(_T("CurrentColor"), &str, _T("0 0 0"));
		int r = 0, g = 0, b = 0;
#if wxUSE_UNICODE
		swscanf(str, _T("%d %d %d"), &r, &g, &b);
#else
		sscanf(str, _T("%d %d %d"), &r, &g, &b);
#endif
		current_color = HeeksColor((unsigned char)r, (unsigned char)g, (unsigned char)b);
	}
	{
		wxString str;
		config.Read(_T("ConstructionColor"), &str, _T("0 0 255"));
		int r = 0, g = 0, b = 255;
#if wxUSE_UNICODE
		swscanf(str, _T("%d %d %d"), &r, &g, &b);
#else
		sscanf(str, _T("%d %d %d"), &r, &g, &b);
#endif
		construction_color = HeeksColor((unsigned char)r, (unsigned char)g, (unsigned char)b);
	}
	config.Read(_T("RotateMode"), &m_rotate_mode);
	config.Read(_T("Antialiasing"), &m_antialiasing);
	config.Read(_T("GridMode"), &grid_mode);
	config.Read(_T("m_light_push_matrix"), &m_light_push_matrix);
	config.Read(_T("WheelForwardAway"), &mouse_wheel_forward_away);
	config.Read(_T("ZoomingReversed"), &ViewZooming::m_reversed);
	config.Read(_T("CtrlDoesRotate"), &ctrl_does_rotate);
	config.Read(_T("DrawDatum"), &m_show_datum_coords_system, true);
	config.Read(_T("DrawDatumSolid"), &m_datum_coords_system_solid_arrows, true);
	config.Read(_T("DatumSize"), &CoordinateSystem::size, 30);
	config.Read(_T("DatumSizeIsPixels"), &CoordinateSystem::size_is_pixels, true);
	config.Read(_T("DrawRuler"), &m_show_ruler, false);
	config.Read(_T("RegularShapesMode"), (int*)(&(regular_shapes_drawing.m_mode)));
	config.Read(_T("RegularShapesNSides"), &(regular_shapes_drawing.m_number_of_side_for_polygon));
	config.Read(_T("RegularShapesRectRad"), &(regular_shapes_drawing.m_rect_radius));
	config.Read(_T("RegularShapesObRad"), &(regular_shapes_drawing.m_obround_radius));
	config.Read(_T("ExtrudeRemovesSketches"), &m_extrude_removes_sketches, true);
	config.Read(_T("LoftRemovesSketches"), &m_loft_removes_sketches, true);
	config.Read(_T("GraphicsTextMode"), (int*)(&m_graphics_text_mode), GraphicsTextModeWithHelp);

	config.Read(_T("DxfMakeSketch"), &HeeksDxfRead::m_make_as_sketch, true);	
	config.Read(_T("ViewUnits"), &m_view_units);
	config.Read(_T("FaceToSketchDeviation"), &(FaceToSketchTool::deviation));

	m_ruler->ReadFromConfig(config);

	GetRecentFilesProfileString();

	wxImage::AddHandler(new wxPNGHandler);
	m_frame = new CHeeksFrame( wxT( "HeeksCAD free Solid Modelling software based on Open CASCADE" ), wxPoint(posx, posy), wxSize(width, height));

#ifdef __WXMSW__
	// to do, make this compile in Linux
	m_frame->SetIcon(wxICON(HeeksCAD));
#endif
	SetInputMode(m_select_mode);
	m_frame->Show(TRUE);
	SetTopWindow(m_frame);

	OnNewOrOpen(false);
	SetLikeNewFile();
	SetFrameTitle();

#ifdef __WXMSW__
	// to do, make this compile in Linux
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
			if(parser.GetParamCount() > 0)
			{
				OpenFile(parser.GetParam(0));
			}
		}
	}
#endif

	return TRUE;
} 

int HeeksCADapp::OnExit(){
	HeeksConfig config;
	config.Write(_T("DrawEnd"), digitize_end);
	config.Write(_T("DrawInters"), digitize_inters);
	config.Write(_T("DrawCentre"), digitize_centre);
	config.Write(_T("DrawMidpoint"), digitize_midpoint);
	config.Write(_T("DrawNearest"), digitize_nearest);
	config.Write(_T("DrawTangent"), digitize_tangent);
	config.Write(_T("DrawCoords"), digitize_coords);
	config.Write(_T("DrawScreen"), digitize_screen);
	config.Write(_T("DrawToGrid"), draw_to_grid);
	config.Write(_T("DrawGrid"), digitizing_grid);
	config.Write(_T("DrawRadius"), digitizing_radius);
	config.Write(_T("BackgroundColor"), wxString::Format(_T("%d %d %d"), background_color.red, background_color.green, background_color.blue));
	config.Write(_T("CurrentColor"), wxString::Format( _T("%d %d %d"), current_color.red, current_color.green, current_color.blue));
	config.Write(_T("ConstructionColor"), wxString::Format(_T("%d %d %d"), construction_color.red, construction_color.green, construction_color.blue));
	config.Write(_T("RotateMode"), m_rotate_mode);	
	config.Write(_T("Antialiasing"), m_antialiasing);	
	config.Write(_T("GridMode"), grid_mode);
	config.Write(_T("m_light_push_matrix"), m_light_push_matrix);
	config.Write(_T("WheelForwardAway"), mouse_wheel_forward_away);
	config.Write(_T("ZoomingReversed"), ViewZooming::m_reversed);
	config.Write(_T("CtrlDoesRotate"), ctrl_does_rotate);
	config.Write(_T("DrawDatum"), m_show_datum_coords_system);
	config.Write(_T("DrawDatumSolid"), m_show_datum_coords_system);
	config.Write(_T("DatumSize"), CoordinateSystem::size);
	config.Write(_T("DatumSizeIsPixels"), CoordinateSystem::size_is_pixels);
	config.Write(_T("DrawRuler"), m_show_ruler);
	config.Write(_T("RegularShapesMode"), regular_shapes_drawing.m_mode);
	config.Write(_T("RegularShapesNSides"), regular_shapes_drawing.m_number_of_side_for_polygon);
	config.Write(_T("RegularShapesRectRad"), regular_shapes_drawing.m_rect_radius);
	config.Write(_T("RegularShapesObRad"), regular_shapes_drawing.m_obround_radius);
	config.Write(_T("ExtrudeRemovesSketches"), m_extrude_removes_sketches);
	config.Write(_T("LoftRemovesSketches"), m_loft_removes_sketches);
	config.Write(_T("GraphicsTextMode"), m_graphics_text_mode);
	config.Write(_T("DxfMakeSketch"), HeeksDxfRead::m_make_as_sketch);
	config.Write(_T("FaceToSketchDeviation"), FaceToSketchTool::deviation);

	m_ruler->WriteToConfig(config);

	WriteRecentFilesProfileString(config);

	for(std::list<wxDynamicLibrary*>::iterator It = m_loaded_libraries.begin(); It != m_loaded_libraries.end(); It++){
		wxDynamicLibrary* shared_library = *It;
		delete shared_library;
	}
	m_loaded_libraries.clear();

	int result = wxApp::OnExit();
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
	if(m_graphics_text_mode != GraphicsTextModeNone)Repaint();
}

void HeeksCADapp::FindMarkedObject(const wxPoint &point, MarkedObject* marked_object){
	m_frame->m_graphics->FindMarkedObject(point, marked_object);
}

void HeeksCADapp::CreateLights(void)
{
	GLfloat amb[4] =  {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat dif[4] =  {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat spec[4] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat pos[4] = {0.5f, 0.5f, 0.5f, 0.0f};
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
	glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,pos);
	if(m_light_push_matrix){
		glPopMatrix();
	}
    glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	glDisable(GL_LIGHT3);
	glDisable(GL_LIGHT4);
	glDisable(GL_LIGHT5);
	glDisable(GL_LIGHT6);
	glDisable(GL_LIGHT7);
}

void HeeksCADapp::DestroyLights(void)
{
    glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
    glDisable(GL_AUTO_NORMAL);
    glDisable(GL_NORMALIZE);
}

void HeeksCADapp::Reset(){
	m_marked_list->Clear(true);
	m_marked_list->Reset();
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
	gp_Vec vy(0, 1, 0), vz(0, 0, 1);
	m_frame->m_graphics->m_view_point.SetView(vy, vz);
	m_filepath = wxString(_("Untitled")) + _T(".heeks");
	m_hidden_for_drag.clear();
	m_show_grippers_on_drag = true;
	*m_ruler = HRuler();
	SetInputMode(m_select_mode);

	ResetIDs();
}

static bool undoably_for_ReadSTEPFileFromXMLElement = false;
static HeeksObj* paste_into_for_ReadSTEPFileFromXMLElement = NULL;

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
						else if(attr_name == std::string("vis")){shape_data.m_visible = (a->IntValue() != 0);}
						else shape_data.m_xml_element.SetAttribute(a->Name(), a->Value());
					}

					// get face ids
					for(TiXmlElement* faceElem = TiXmlHandle(subsubElem).FirstChildElement("face").Element(); faceElem; faceElem = faceElem->NextSiblingElement("face"))
					{
						int id = 0;
						faceElem->Attribute("id", &id);
						shape_data.m_face_ids.push_back(id);
					}

					// get edge ids
					for(TiXmlElement* edgeElem = TiXmlHandle(subsubElem).FirstChildElement("edge").Element(); edgeElem; edgeElem = edgeElem->NextSiblingElement("edge"))
					{
						int id = 0;
						edgeElem->Attribute("id", &id);
						shape_data.m_edge_ids.push_back(id);
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
#ifdef __WXMSW__
					wofstream ofs(temp_file);
#else
					wofstream ofs(Ttc(temp_file.c_str()));
#endif
#else
					ofstream ofs(temp_file);
#endif
					ofs<<file_text;
				}
				CShape::ImportSolidsFile(temp_file, undoably_for_ReadSTEPFileFromXMLElement, &index_map, paste_into_for_ReadSTEPFileFromXMLElement);
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
#ifdef __WXMSW__
				ofstream ofs(temp_file);
#else
				ofstream ofs(Ttc(temp_file.c_str()));
#endif
				ofs<<a->Value();
			}
			CShape::ImportSolidsFile(temp_file, undoably_for_ReadSTEPFileFromXMLElement, &index_map, paste_into_for_ReadSTEPFileFromXMLElement);
		}
	}

	return NULL;
}

void HeeksCADapp::InitializeXMLFunctions()
{
	// set up function map
	if(xml_read_fn_map.size() == 0)
	{
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Line", HLine::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Arc", HArc::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "InfiniteLine", HILine::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Circle", HCircle::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Point", HPoint::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Image", HImage::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Sketch", CSketch::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "STEP_file", ReadSTEPFileFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "STLSolid", CStlSolid::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "CoordinateSystem", CoordinateSystem::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Text", HText::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Dimension", HDimension::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Ellipse", HEllipse::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Spline", HSpline::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Group", CGroup::ReadFromXMLElement ) );
	}
}

void HeeksCADapp::RegisterReadXMLfunction(const char* type_name, HeeksObj*(*read_xml_function)(TiXmlElement* pElem))
{
	if(xml_read_fn_map.find(type_name) != xml_read_fn_map.end()){
		wxMessageBox(_T("Error - trying to register an XML read function for an exisiting type"));
		return;
	}
	xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( type_name, read_xml_function ) );
}

HeeksObj* HeeksCADapp::ReadXMLElement(TiXmlElement* pElem)
{
	std::string name(pElem->Value());

	std::map< std::string, HeeksObj*(*)(TiXmlElement* pElem) >::iterator FindIt = xml_read_fn_map.find( name );
	HeeksObj* object = NULL;
	if(FindIt != xml_read_fn_map.end())
	{
		object = (*(FindIt->second))(pElem);
	}
	else
	{
		object = HXml::ReadFromXMLElement(pElem);
	}

	return object;
}

void HeeksCADapp::ObjectWriteBaseXML(HeeksObj *object, TiXmlElement *element)
{
	if(object->UsesID())element->SetAttribute("id", object->m_id);
	if(!object->m_visible)element->SetAttribute("vis", 0);
}

void HeeksCADapp::ObjectReadBaseXML(HeeksObj *object, TiXmlElement* element)
{
	// get the attributes
	for(TiXmlAttribute* a = element->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(object->UsesID() && name == "id"){object->SetID(a->IntValue());}
		if(name == "vis"){object->m_visible = (a->IntValue() != 0);}
	}
}

void HeeksCADapp::OpenXMLFile(const wxChar *filepath, bool undoably, HeeksObj* paste_into)
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

	undoably_for_ReadSTEPFileFromXMLElement = undoably;
	paste_into_for_ReadSTEPFileFromXMLElement = paste_into;

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlNode* root = &doc;

	pElem=hDoc.FirstChildElement().Element();
	if (!pElem) return;
	std::string name(pElem->Value());
	if(name == "HeeksCAD_Document")
	{
		root = pElem;
	}

	std::list<HeeksObj*> objects;
	for(pElem = root->FirstChildElement(); pElem;	pElem = pElem->NextSiblingElement())
	{
		HeeksObj* object = ReadXMLElement(pElem);
		if(object)objects.push_back(object);
	}

	if(objects.size() > 0)
	{
		HeeksObj* add_to = this;
		if(paste_into)add_to = paste_into;
		for(std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
		{
			HeeksObj* object = *It;
			if(add_to->CanAdd(object) && object->CanAddTo(add_to))
			{
				if(undoably && object->OneOfAKind())
				{
					bool one_found = false;
					for(HeeksObj* child = add_to->GetFirstChild(); child; child = add_to->GetNextChild())
					{
						if(child->GetType() == object->GetType())
						{
							child->CopyFrom(object);
							one_found = true;
							break;
						}
					}
					if(!one_found)AddUndoably(object, add_to, NULL);
				}
				else
				{
					if(undoably)AddUndoably(object, add_to, NULL);
					else add_to->Add(object, NULL);
				}
			}
		}
	}

	CGroup::MoveSolidsToGroupsById(this);
}

void HeeksCADapp::OpenSVGFile(const wxChar *filepath, bool undoably)
{
	HeeksSvgRead svgread(filepath,undoably,true);
}

void HeeksCADapp::OpenSTLFile(const wxChar *filepath, bool undoably)
{
	CStlSolid* new_object = new CStlSolid(filepath, &current_color);
	if(undoably)AddUndoably(new_object, this, NULL);
	else Add(new_object, NULL);
}

void HeeksCADapp::OpenDXFFile(const wxChar *filepath, bool undoably)
{
	HeeksDxfRead dxf_file(filepath);
	dxf_file.DoRead(undoably);
}

bool HeeksCADapp::OpenImageFile(const wxChar *filepath, bool undoably)
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
			HImage* new_object = new HImage(filepath);
			if(undoably)AddUndoably(new_object, NULL, NULL);
			else Add(new_object, NULL);
			Repaint();
			return true;
		}
	}

	return false;
}

bool HeeksCADapp::OpenFile(const wxChar *filepath, bool import_not_open, HeeksObj* paste_into)
{
	m_in_OpenFile = true;
	m_file_open_or_import_type = FileOpenOrImportTypeOther;

	// returns true if file open was successful
	wxString wf(filepath);
	wf.LowerCase();

	bool open_failed = false;

	if(import_not_open)StartHistory();

	if(wf.EndsWith(_T(".heeks")) || wf.EndsWith(_T(".HEEKS")))
	{
		m_file_open_or_import_type = FileOpenOrImportTypeHeeks;
		OpenXMLFile(filepath, import_not_open, paste_into);
	}
	else if(wf.EndsWith(_T(".svg")) || wf.EndsWith(_T(".SVG")))
	{
		OpenSVGFile(filepath, import_not_open);
	}
	else if(wf.EndsWith(_T(".stl")) || wf.EndsWith(_T(".STL")))
	{
		OpenSTLFile(filepath, import_not_open);
	}
	else if(wf.EndsWith(_T(".dxf")) || wf.EndsWith(_T(".DXF")))
	{
		m_file_open_or_import_type = FileOpenOrImportTypeDxf;
		OpenDXFFile(filepath, import_not_open);
	}

	// check for images
	else if(OpenImageFile(filepath, import_not_open))
	{
	}

	// check for solid files
	else if(CShape::ImportSolidsFile(filepath, import_not_open))
	{
	}
	else
	{
		// error
		wxString str = wxString(_("Invalid file type chosen")) + _T("  ") + _("expecting") + _T(" ") + GetKnownFilesCommaSeparatedList();
		wxMessageBox(str);
		open_failed = true;
	}

	if(import_not_open)EndHistory();

	if(!open_failed)
	{
		if(!import_not_open)
		{
			WereAdded(m_objects);
			m_filepath.assign(filepath);
			InsertRecentFileItem(filepath);
			SetFrameTitle();
		}
	}

	m_in_OpenFile = false;

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
      case EllipseType:
                {
			HEllipse* e = (HEllipse*)object;
			double c[3];
			extract(e->m_ellipse.Location(), c);
			bool dir = e->m_ellipse.Axis().Direction().Z() > 0;
			double maj_r = e->m_ellipse.MajorRadius();
			double min_r = e->m_ellipse.MinorRadius();
			double rot = e->GetRotation();
			dxf_file.WriteEllipse(c, maj_r, min_r, rot, 0, 2 * Pi, dir);
                }
		break;
        case CircleType:
                {
			HCircle* cir = (HCircle*)object;
			double c[3];
			extract(cir->m_circle.Location(), c);
			double radius = cir->m_circle.Radius();
			dxf_file.WriteCircle(c, radius);
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
		wxString str = wxString(_("couldn't open file")) + filepath;
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
	(*ofs_for_write_stl_triangle) << " facet normal " << n[0] << " " << n[1] << " " << n[2] << endl;
	(*ofs_for_write_stl_triangle) << "   outer loop" << endl;
	(*ofs_for_write_stl_triangle) << "     vertex " << x[0] << " " << x[1] << " " << x[2] << endl;
	(*ofs_for_write_stl_triangle) << "     vertex " << x[3] << " " << x[4] << " " << x[5] << endl;
	(*ofs_for_write_stl_triangle) << "     vertex " << x[6] << " " << x[7] << " " << x[8] << endl;
	(*ofs_for_write_stl_triangle) << "   endloop" << endl;
	(*ofs_for_write_stl_triangle) << " endfacet" << endl;
}

static void write_cpp_triangle(const double* x, const double* n)
{
	for(int i = 0; i<3; i++)
	{
		(*ofs_for_write_stl_triangle) << "glNormal3d(" << n[i*3 + 0] << ", " << n[i*3 + 1] << ", " << n[i*3 + 2] << ");" << endl;
		(*ofs_for_write_stl_triangle) << "glVertex3d(" << x[i*3 + 0] << ", " << x[i*3 + 1] << ", " << x[i*3 + 2] << ");" << endl;
	}
}

void HeeksCADapp::SaveSTLFile(const std::list<HeeksObj*>& objects, const wxChar *filepath)
{
#ifdef __WXMSW__
	ofstream ofs(filepath);
#else
	ofstream ofs(Ttc(filepath));
#endif
	if(!ofs)
	{
		wxString str = wxString(_("couldn't open file")) + _T(" - ") + filepath;
		wxMessageBox(str);
		return;
	}
	ofs.imbue(std::locale("C"));

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

void HeeksCADapp::SaveCPPFile(const std::list<HeeksObj*>& objects, const wxChar *filepath)
{
#ifdef __WXMSW__
	ofstream ofs(filepath);
#else
	ofstream ofs(Ttc(filepath));
#endif
	if(!ofs)
	{
		wxString str = wxString(_("couldn't open file")) + _T(" - ") + filepath;
		wxMessageBox(str);
		return;
	}
	ofs.imbue(std::locale("C"));

	ofs<<"glBegin(GL_TRIANGLES);"<<endl;

	// write all the objects
	ofs_for_write_stl_triangle = &ofs;
	for(std::list<HeeksObj*>::iterator It = m_objects.begin(); It != m_objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->GetTriangles(write_cpp_triangle, 0.1, false);
	}

	ofs<<"glEnd();"<<endl;
}

void HeeksCADapp::SaveXMLFile(const std::list<HeeksObj*>& objects, const wxChar *filepath, bool for_clipboard)
{
	// write an xml file
	TiXmlDocument doc;  
	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  

	TiXmlNode* root = &doc;
	if(!for_clipboard)
	{
		root = new TiXmlElement( "HeeksCAD_Document" );
		doc.LinkEndChild( root );
	}

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
				index_pair_element->SetAttribute("vis", shape_data.m_visible ? 1:0);
				if(shape_data.m_solid_type != SOLID_TYPE_UNKNOWN)index_pair_element->SetAttribute("solid_type", shape_data.m_solid_type);
				// get the CShapeData attributes
				for(TiXmlAttribute* a = shape_data.m_xml_element.FirstAttribute(); a; a = a->Next())
				{
					index_pair_element->SetAttribute(a->Name(), a->Value());
				}

				// write the face ids
				for(std::list<int>::iterator It = shape_data.m_face_ids.begin(); It != shape_data.m_face_ids.end(); It++)
				{
					int id = *It;
					TiXmlElement *face_id_element = new TiXmlElement( "face" );
					index_pair_element->LinkEndChild( face_id_element );
					face_id_element->SetAttribute("id", id);
				}

				// write the edge ids
				for(std::list<int>::iterator It = shape_data.m_edge_ids.begin(); It != shape_data.m_edge_ids.end(); It++)
				{
					int id = *It;
					TiXmlElement *edge_id_element = new TiXmlElement( "edge" );
					index_pair_element->LinkEndChild( edge_id_element );
					edge_id_element->SetAttribute("id", id);
				}
			}
		}

		// write the step file as a string attribute of step_file
#ifdef __WXMSW__
		ifstream ifs(temp_file);
#else
		ifstream ifs(Ttc(temp_file.c_str()));
#endif
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
		wxFileDialog fd(m_frame, _("Save graphical data file"), wxEmptyString, filepath, GetKnownFilesWildCardString(false), wxSAVE|wxOVERWRITE_PROMPT);
		fd.SetFilterIndex(1);
		if (fd.ShowModal() == wxID_CANCEL)return false;
		return SaveFile( fd.GetPath().c_str(), false, update_recent_file_list );
	}

	wxString wf(filepath);
	wf.LowerCase();

	if(wf.EndsWith(_T(".heeks")))
	{
		// call external OnSave functions
		for(std::list< void(*)(bool) >::iterator It = wxGetApp().m_on_save_callbacks.begin(); It != wxGetApp().m_on_save_callbacks.end(); It++)
		{
			void(*callbackfunc)(bool) = *It;
			(*callbackfunc)(false);
		}

		SaveXMLFile(filepath);
	}
	else if(wf.EndsWith(_T(".dxf")))
	{
		SaveDXFFile(filepath);
	}
	else if(wf.EndsWith(_T(".stl")))
	{
		SaveSTLFile(m_objects, filepath);
	}
	else if(wf.EndsWith(_T(".cpp")))
	{
		SaveCPPFile(m_objects, filepath);
	}
	else if(CShape::ExportSolidsFile(m_objects, filepath))
	{
	}
	else
	{
		wxString str = wxString(_("Invalid file type chosen")) + _T("  ") + _("expecting") + _T(" ") + GetKnownFilesCommaSeparatedList();
		wxMessageBox(str);
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

void HeeksCADapp::RenderDatumOrCurrentCoordSys(bool select)
{
	if(m_show_datum_coords_system || m_current_coordinate_system)
	{
		bool bright_datum = (m_current_coordinate_system == NULL);
		if(m_datum_coords_system_solid_arrows)
		{
			// make the datum appear at the front of everything, by clearing the depth buffer
			if(m_show_datum_coords_system)
			{
				glClear(GL_DEPTH_BUFFER_BIT);
				CoordinateSystem::RenderDatum(bright_datum, true);
			}
			if(m_current_coordinate_system)
			{
				glClear(GL_DEPTH_BUFFER_BIT);
				CoordinateSystem::rendering_current = true;
				m_current_coordinate_system->glCommands(select, wxGetApp().m_marked_list->ObjectMarked(m_current_coordinate_system), false);
				CoordinateSystem::rendering_current = false;
			}
		}
		else
		{
			// make the datum appear at the front of everything, by setting the depth range
			GLfloat save_depth_range[2];
			glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
			glDepthRange(0, 0);

			if(m_current_coordinate_system)
			{
				CoordinateSystem::rendering_current = true;
				m_current_coordinate_system->glCommands(select, wxGetApp().m_marked_list->ObjectMarked(m_current_coordinate_system), false);
				CoordinateSystem::rendering_current = false;
			}
			if(m_show_datum_coords_system)
			{
				CoordinateSystem::RenderDatum(bright_datum, false);
			}

			// restore the depth range
			glDepthRange(save_depth_range[0], save_depth_range[1]);
		}
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

	std::list<HeeksObj*> after_others_objects;

	for(std::list<HeeksObj*>::iterator It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
		if(object->OnVisibleLayer() && object->m_visible)
		{
			if(object->DrawAfterOthers())after_others_objects.push_back(object);
			else
			{
				if(select)glPushName((unsigned long)object);
				object->glCommands(select, m_marked_list->ObjectMarked(object), false);
				if(select)glPopName();
			}
		}
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
	for(std::list< void(*)() >::iterator It = m_on_glCommands_list.begin(); It != m_on_glCommands_list.end(); It++)
	{
		void(*callbackfunc)() = *It;
		(*callbackfunc)();
	}
	glEnable(GL_POLYGON_OFFSET_FILL);

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

	// draw any last_objects
	for(std::list<HeeksObj*>::iterator It = after_others_objects.begin(); It != after_others_objects.end(); It++)
	{
		HeeksObj* object = *It;
		if(select)glPushName((unsigned long)object);
		object->glCommands(select, m_marked_list->ObjectMarked(object), false);
		if(select)glPopName();
	}

	// draw the ruler
	if(m_show_ruler && m_ruler->m_visible)
	{
		if(select)glPushName((unsigned long)m_ruler);
		m_ruler->glCommands(select, false, false);
		if(select)glPopName();
	}

	// draw the grid
	glDepthFunc(GL_LESS);
	RenderGrid(&view_point);
	glDepthFunc(GL_LEQUAL);

	// draw the datum
	RenderDatumOrCurrentCoordSys(select);

	DestroyLights();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL );
	if(m_hidden_for_drag.size() == 0 || !m_show_grippers_on_drag)m_marked_list->GrippersGLCommands(select, false);

	// draw the input mode text on the top
	if(m_graphics_text_mode != GraphicsTextModeNone)
	{
		wxString screen_text1, screen_text2;

		if(input_mode_object && input_mode_object->GetTitle())
		{
			screen_text1.Append(input_mode_object->GetTitle());
			screen_text1.Append(_T("\n"));
		}
		if(m_graphics_text_mode == GraphicsTextModeWithHelp && input_mode_object)
		{
			const wxChar* help_str = input_mode_object->GetHelpText();
			if(help_str)
			{
				screen_text2.Append(help_str);
			}
		}
		render_screen_text(screen_text1, screen_text2);
	}
}

void HeeksCADapp::OnInputModeTitleChanged()
{
	if(m_graphics_text_mode != GraphicsTextModeNone)
	{
		Repaint();
	}
}

void HeeksCADapp::OnInputModeHelpTextChanged()
{
	if(m_graphics_text_mode == GraphicsTextModeWithHelp)
	{
		Repaint();
	}
}

void HeeksCADapp::glCommands(bool select, bool marked, bool no_color)
{
	// this is called when select is true
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
		if(object->OnVisibleLayer() && object->m_visible)
		{
			if(select)glPushName((unsigned long)object);
			(*It)->glCommands(select, marked || wxGetApp().m_marked_list->ObjectMarked(object), no_color);
			if(select)glPopName();
		}
	}

	// draw the ruler
	if(m_show_ruler)
	{
		if(select)glPushName((unsigned long)m_ruler);
		m_ruler->glCommands(select, false, false);
		if(select)glPopName();
	}
}

void HeeksCADapp::GetBox(CBox &box){
	CBox temp_box;
	ObjList::GetBox(temp_box);
	if(temp_box.m_valid && temp_box.Radius() > 0.000001)
		box.Insert(temp_box);
}

double HeeksCADapp::GetPixelScale(void){
	return m_frame->m_graphics->m_view_point.m_pixel_scale;
}

bool HeeksCADapp::IsModified(void){
	for(std::list< bool(*)() >::iterator It = wxGetApp().m_is_modified_callbacks.begin(); It != wxGetApp().m_is_modified_callbacks.end(); It++)
	{
		bool(*callbackfunc)() = *It;
		bool is_modified = (*callbackfunc)();
		if(is_modified)return true;
	}

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
	const wxChar* GetTitle(){
		if (wxGetApp().m_frame->IsFullScreen()) return _("Exit Full Screen Mode");
		else return _("Show Full Screen");
	}
	void Run(){
		wxGetApp().m_frame->ShowFullScreen(!wxGetApp().m_frame->IsFullScreen());
	}
	wxString BitmapPath(){return _T("fullscreen");}
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

	m_marked_list->GetTools(marked_object, f_list, &new_point);

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

void HeeksCADapp::StartHistory()
{
	history->StartHistory();
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

	if(object->GetType() == CoordinateSystemType && (!m_in_OpenFile || m_file_open_or_import_type !=FileOpenOrImportTypeHeeks))
	{
		m_current_coordinate_system = (CoordinateSystem*)object;
	}

	if(m_mark_newly_added_objects)
	{
		m_marked_list->Add(object, true);
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
	if(!object->CanBeRemoved())return;
	RemoveObjectTool *tool = new RemoveObjectTool(object);
	StartHistory();
	DoToolUndoably(tool);
	EndHistory();
}

void HeeksCADapp::DeleteUndoably(const std::list<HeeksObj*>& list)
{
	if(list.size() == 0)return;
	std::list<HeeksObj*> list2;
	for(std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++)
	{
		HeeksObj* object = *It;
		if(object->CanBeRemoved())list2.push_back(object);
	}
	if(list2.size() == 0)return;
	RemoveObjectsTool *tool = new RemoveObjectsTool(list2, list2.front()->m_owner);
	DoToolUndoably(tool);
}

void HeeksCADapp::TransformUndoably(HeeksObj *object, double *m)
{
	if(!object)return;
	gp_Trsf mat = make_matrix(m);
	gp_Trsf im = mat;
	im.Invert();
	StartHistory(); // the following action might do add and remove, so needs to be in a group
	TransformTool *tool = new TransformTool(object, mat, im);
	DoToolUndoably(tool);
	EndHistory();
}

void HeeksCADapp::TransformUndoably(const std::list<HeeksObj*> &list, double *m)
{
	if(list.size() == 0)return;
	gp_Trsf mat = make_matrix(m);
	gp_Trsf im = mat;
	im.Invert();
	StartHistory(); // the following action might do add and remove, so needs to be in a group
	TransformObjectsTool *tool = new TransformObjectsTool(list, mat, im);
	DoToolUndoably(tool);
	EndHistory();
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

	std::list<HeeksObj*> marked_remove;
	for(std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++)
	{
		object = *It;
		if(m_marked_list->ObjectMarked(object))marked_remove.push_back(object);
	}
	if(marked_remove.size() > 0)m_marked_list->Remove(marked_remove, false);

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
			if(m_current_coordinate_system)o.Transform(m_current_coordinate_system->GetMatrix());
			return make_matrix(o, vx, vy);
		}
	}

	gp_Trsf mat;
	if(m_current_coordinate_system)mat = m_current_coordinate_system->GetMatrix();
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

void on_set_perspective(bool value, HeeksObj* object)
{
	wxGetApp().m_frame->m_graphics->m_view_point.SetPerspective(value);
	wxGetApp().Repaint();
}

void on_set_tool_icon_size(int value, HeeksObj* object)
{
	int size = 16;
	switch(value)
	{
	case 0:
		size = 16;
		break;
	case 1:
		size = 24;
		break;
	case 2:
		size = 32;
		break;
	case 3:
		size = 48;
		break;
	case 4:
		size = 64;
		break;
	case 5:
		size = 96;
		break;
	}

	ToolImage::SetBitmapSize(size);

	wxGetApp().m_frame->OnChangeBitmapSize();
	wxGetApp().m_frame->m_aui_manager->Update();
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

void on_set_face_to_sketch_deviation(double value, HeeksObj* object)
{
	FaceToSketchTool::deviation = value;
}

void on_set_show_datum(bool onoff, HeeksObj* object)
{
	wxGetApp().m_show_datum_coords_system = onoff;
	wxGetApp().Repaint();
}

void on_set_solid_datum(bool onoff, HeeksObj* object)
{
	wxGetApp().m_datum_coords_system_solid_arrows = onoff;
	wxGetApp().Repaint();
}

void on_set_show_ruler(bool onoff, HeeksObj* object)
{
	wxGetApp().m_show_ruler = onoff;
	wxGetApp().Repaint();
}

void on_set_rotate_mode(int value, HeeksObj* object)
{
	wxGetApp().m_rotate_mode = value;
	if(!wxGetApp().m_rotate_mode)
	{
		gp_Vec vy(0, 1, 0), vz(0, 0, 1);
		wxGetApp().m_frame->m_graphics->m_view_point.SetView(vy, vz);
		wxGetApp().m_frame->m_graphics->StoreViewPoint();
		wxGetApp().Repaint();
	}
}

void on_set_screen_text(int value, HeeksObj* object)
{
	wxGetApp().m_graphics_text_mode = (GraphicsTextMode)value;
	wxGetApp().Repaint();
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

void on_set_reverse_zooming(bool value, HeeksObj* object)
{
	ViewZooming::m_reversed = value;
	wxGetApp().OnInputModeHelpTextChanged();
}

void on_set_ctrl_does_rotate(bool value, HeeksObj* object)
{
	wxGetApp().ctrl_does_rotate = value;
	wxGetApp().OnInputModeHelpTextChanged();
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
	wxGetApp().Repaint();
}

void on_set_size_is_pixels(bool value, HeeksObj* object){
	CoordinateSystem::size_is_pixels = value;
	wxGetApp().Repaint();
}

void on_sel_filter_line(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_LINE;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_LINE;
}

void on_sel_filter_arc(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_ARC;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_ARC;
}

void on_sel_filter_iline(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_ILINE;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_ILINE;
}

void on_sel_filter_circle(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_CIRCLE;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_CIRCLE;
}

void on_sel_filter_point(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_POINT;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_POINT;
}

void on_sel_filter_solid(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_SOLID;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_SOLID;
}

void on_sel_filter_stl_solid(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_STL_SOLID;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_STL_SOLID;
}

void on_sel_filter_wire(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_WIRE;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_WIRE;
}

void on_sel_filter_face(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_FACE;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_FACE;
}

void on_sel_filter_vertex(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_VERTEX;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_VERTEX;
}

void on_sel_filter_edge(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_EDGE;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_EDGE;
}

void on_sel_filter_sketch(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_SKETCH;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_SKETCH;
}

void on_sel_filter_image(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_IMAGE;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_IMAGE;
}

void on_sel_filter_coordinate_sys(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_COORDINATE_SYSTEM;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_COORDINATE_SYSTEM;
}

void on_sel_filter_text(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_TEXT;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_TEXT;
}

void on_sel_filter_dimension(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_DIMENSION;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_DIMENSION;
}

void on_sel_filter_ruler(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_RULER;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_RULER;
}

void on_dxf_make_sketch(bool value, HeeksObj* object){
	HeeksDxfRead::m_make_as_sketch = value;
}

static void on_set_units(int value, HeeksObj* object)
{
	wxGetApp().m_view_units = (value == 0) ? 1.0:25.4;
	HeeksConfig config;
	config.Write(_T("ViewUnits"), wxGetApp().m_view_units);
	wxGetApp().m_frame->m_properties->RefreshByRemovingAndAddingAll(false);
	wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
	wxGetApp().m_ruler->KillGLLists();
	wxGetApp().Repaint();
}

void HeeksCADapp::GetOptions(std::list<Property *> *list)
{
	PropertyList* view_options = new PropertyList(_("view options"));

	{
		std::list< wxString > choices;
		choices.push_back ( wxString ( _("stay upright") ) );
		choices.push_back ( wxString ( _("free") ) );
		view_options->m_list.push_back ( new PropertyChoice ( _("rotate mode"),  choices, m_rotate_mode, NULL, on_set_rotate_mode ) );
	}
	{
		std::list< wxString > choices;
		choices.push_back ( wxString ( _("none") ) );
		choices.push_back ( wxString ( _("input mode title") ) );
		choices.push_back ( wxString ( _("full help") ) );
		view_options->m_list.push_back ( new PropertyChoice ( _("screen text"),  choices, m_graphics_text_mode, NULL, on_set_screen_text ) );
	}
	view_options->m_list.push_back( new PropertyCheck(_("antialiasing"), m_antialiasing, NULL, on_set_antialiasing));
#if _DEBUG
	view_options->m_list.push_back( new PropertyCheck(_("fixed light"), m_light_push_matrix, NULL, on_set_light_push_matrix));
#endif
	view_options->m_list.push_back( new PropertyCheck(_("reverse mouse wheel"), !(mouse_wheel_forward_away), NULL, on_set_reverse_mouse_wheel));
	view_options->m_list.push_back( new PropertyCheck(_("reverse zooming mode"), ViewZooming::m_reversed, NULL, on_set_reverse_zooming));
	view_options->m_list.push_back( new PropertyCheck(_("Ctrl key does rotate"), ctrl_does_rotate, NULL, on_set_ctrl_does_rotate));
	view_options->m_list.push_back(new PropertyCheck(_("show datum"), m_show_datum_coords_system, NULL, on_set_show_datum));
	view_options->m_list.push_back(new PropertyCheck(_("datum is solid"), m_datum_coords_system_solid_arrows, NULL, on_set_solid_datum));
	view_options->m_list.push_back(new PropertyDouble(_("datum size"), CoordinateSystem::size, NULL, on_set_datum_size));
	view_options->m_list.push_back(new PropertyCheck(_("datum size is pixels not mm"), CoordinateSystem::size_is_pixels, NULL, on_set_size_is_pixels));
	view_options->m_list.push_back(new PropertyCheck(_("show ruler"), m_show_ruler, NULL, on_set_show_ruler));
	view_options->m_list.push_back ( new PropertyColor ( _("background color"),  background_color, NULL, on_set_background_color ) );
	{
		std::list< wxString > choices;
		choices.push_back ( wxString ( _("no grid") ) );
		choices.push_back ( wxString ( _("faint color") ) );
		choices.push_back ( wxString ( _("alpha blending") ) );
		choices.push_back ( wxString ( _("colored alpha blending") ) );
		view_options->m_list.push_back ( new PropertyChoice ( _("grid mode"),  choices, grid_mode, NULL, on_set_grid_mode ) );
	}
	view_options->m_list.push_back( new PropertyCheck(_("perspective"), m_frame->m_graphics->m_view_point.GetPerspective(), NULL, on_set_perspective));

	{
		std::list< wxString > choices;
		choices.push_back ( wxString ( _T("16") ) );
		choices.push_back ( wxString ( _T("24") ) );
		choices.push_back ( wxString ( _T("32") ) );
		choices.push_back ( wxString ( _T("48") ) );
		choices.push_back ( wxString ( _T("64") ) );
		choices.push_back ( wxString ( _T("96") ) );
		int s = ToolImage::GetBitmapSize();
		int choice = 0;
		if(s > 70)choice = 5;
		else if(s > 60)choice = 4;
		else if(s > 40)choice = 3;
		else if(s > 30)choice = 2;
		else if(s > 20)choice = 1;
		view_options->m_list.push_back ( new PropertyChoice ( _("tool icon size"),  choices, choice, NULL, on_set_tool_icon_size ) );
	}

	{
		std::list< wxString > choices;
		choices.push_back ( wxString ( _("mm") ) );
		choices.push_back ( wxString ( _("inch") ) );
		int choice = 0;
		if(m_view_units > 25.0)choice = 1;
		view_options->m_list.push_back ( new PropertyChoice ( _("units"),  choices, choice, this, on_set_units ) );
	}
	list->push_back(view_options);

	PropertyList* digitizing = new PropertyList(_("digitizing"));
	digitizing->m_list.push_back(new PropertyCheck(_("end"), digitize_end, NULL, on_end_of));
	digitizing->m_list.push_back(new PropertyCheck(_("intersection"), digitize_inters, NULL, on_intersection));
	digitizing->m_list.push_back(new PropertyCheck(_("centre"), digitize_centre, NULL, on_centre));
	digitizing->m_list.push_back(new PropertyCheck(_("midpoint"), digitize_midpoint, NULL, on_mid_point));
	digitizing->m_list.push_back(new PropertyCheck(_("nearest"), digitize_nearest, NULL, on_nearest));
	digitizing->m_list.push_back(new PropertyCheck(_("tangent"), digitize_tangent, NULL, on_tangent));
	digitizing->m_list.push_back(new PropertyLength(_("radius for undefined circles"), digitizing_radius, NULL, on_radius));
	digitizing->m_list.push_back(new PropertyCheck(_("coordinates"), digitize_coords, NULL, on_coords));
	digitizing->m_list.push_back(new PropertyCheck(_("screen"), digitize_screen, NULL, on_relative));
	digitizing->m_list.push_back(new PropertyLength(_("grid size"), digitizing_grid, NULL, on_grid_edit));
	digitizing->m_list.push_back(new PropertyCheck(_("snap to grid"), draw_to_grid, NULL, on_grid));
	list->push_back(digitizing);

	PropertyList* drawing = new PropertyList(_("drawing"));
	drawing->m_list.push_back ( new PropertyColor ( _("current color"),  current_color, NULL, on_set_current_color ) );
	drawing->m_list.push_back ( new PropertyColor ( _("construction color"),  construction_color, NULL, on_set_construction_color ) );
	drawing->m_list.push_back(new PropertyLength(_("geometry tolerance"), m_geom_tol, NULL, on_set_geom_tol));
	drawing->m_list.push_back(new PropertyLength(_("face to sketch deviaton"), FaceToSketchTool::deviation, NULL, on_set_face_to_sketch_deviation));
	list->push_back(drawing);

	for(std::list<wxDynamicLibrary*>::iterator It = m_loaded_libraries.begin(); It != m_loaded_libraries.end(); It++){
		wxDynamicLibrary* shared_library = *It;
		list_for_GetOptions = list;
		bool success;
		void(*GetOptions)(void(*)(Property*)) = (void(*)(void(*)(Property*)))(shared_library->GetSymbol(_T("GetOptions"), &success));
		if(GetOptions)(*GetOptions)(AddPropertyCallBack);
	}

	PropertyList* selection_filter = new PropertyList(_("selection filter"));
	selection_filter->m_list.push_back(new PropertyCheck(_("point"), (m_marked_list->m_filter & MARKING_FILTER_POINT) != 0, NULL, on_sel_filter_point));
	selection_filter->m_list.push_back(new PropertyCheck(_("line"), (m_marked_list->m_filter & MARKING_FILTER_LINE) != 0, NULL, on_sel_filter_line));
	selection_filter->m_list.push_back(new PropertyCheck(_("arc"), (m_marked_list->m_filter & MARKING_FILTER_ARC) != 0, NULL, on_sel_filter_arc));
	selection_filter->m_list.push_back(new PropertyCheck(_("infinite line"), (m_marked_list->m_filter & MARKING_FILTER_ILINE) != 0, NULL, on_sel_filter_iline));
	selection_filter->m_list.push_back(new PropertyCheck(_("circle"), (m_marked_list->m_filter & MARKING_FILTER_CIRCLE) != 0, NULL, on_sel_filter_circle));
	selection_filter->m_list.push_back(new PropertyCheck(_("edge"), (m_marked_list->m_filter & MARKING_FILTER_EDGE) != 0, NULL, on_sel_filter_edge));
	selection_filter->m_list.push_back(new PropertyCheck(_("face"), (m_marked_list->m_filter & MARKING_FILTER_FACE) != 0, NULL, on_sel_filter_face));
	selection_filter->m_list.push_back(new PropertyCheck(_("vertex"), (m_marked_list->m_filter & MARKING_FILTER_VERTEX) != 0, NULL, on_sel_filter_vertex));
	selection_filter->m_list.push_back(new PropertyCheck(_("solid"), (m_marked_list->m_filter & MARKING_FILTER_SOLID) != 0, NULL, on_sel_filter_solid));
	selection_filter->m_list.push_back(new PropertyCheck(_("stl_solid"), (m_marked_list->m_filter & MARKING_FILTER_STL_SOLID) != 0, NULL, on_sel_filter_stl_solid));
	selection_filter->m_list.push_back(new PropertyCheck(_("wire"), (m_marked_list->m_filter & MARKING_FILTER_WIRE) != 0, NULL, on_sel_filter_wire));
	selection_filter->m_list.push_back(new PropertyCheck(_("sketch"), (m_marked_list->m_filter & MARKING_FILTER_SKETCH) != 0, NULL, on_sel_filter_sketch));
	selection_filter->m_list.push_back(new PropertyCheck(_("image"), (m_marked_list->m_filter & MARKING_FILTER_IMAGE) != 0, NULL, on_sel_filter_image));
	selection_filter->m_list.push_back(new PropertyCheck(_("coordinate system"), (m_marked_list->m_filter & MARKING_FILTER_COORDINATE_SYSTEM) != 0, NULL, on_sel_filter_coordinate_sys));
	selection_filter->m_list.push_back(new PropertyCheck(_("text"), (m_marked_list->m_filter & MARKING_FILTER_TEXT) != 0, NULL, on_sel_filter_text));
	selection_filter->m_list.push_back(new PropertyCheck(_("dimension"), (m_marked_list->m_filter & MARKING_FILTER_DIMENSION) != 0, NULL, on_sel_filter_dimension));
	selection_filter->m_list.push_back(new PropertyCheck(_("ruler"), (m_marked_list->m_filter & MARKING_FILTER_RULER) != 0, NULL, on_sel_filter_ruler));
	list->push_back(selection_filter);

	PropertyList* file_options = new PropertyList(_("file options"));
	PropertyList* dxf_options = new PropertyList(_("DXF"));
	dxf_options->m_list.push_back(new PropertyCheck(_("make sketch"), HeeksDxfRead::m_make_as_sketch, NULL, on_dxf_make_sketch));
	file_options->m_list.push_back(dxf_options);
	list->push_back(file_options);
}

void HeeksCADapp::DeleteMarkedItems()
{
	std::list<HeeksObj *> list = m_marked_list->list();

	// clear first, so properties cancel happens first
	m_marked_list->Clear(false);

	if(list.size() == 1){
		DeleteUndoably(*(list.begin()));
	}
	else if(list.size()>1){
		StartHistory();
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
	StartHistory();
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
		known_file_ext = wxString(_("Known Files")) + _T(" |*.heeks;*.HEEKS;*.igs;*.IGS;*.iges;*.IGES;*.stp;*.STP;*.step;*.STEP;*.stl;*.STL;*.svg;*.SVG;*.dxf;*.DXF;") + imageExtStr + _T("|") + _("Heeks files") + _T(" (*.heeks)|*.heeks;*.HEEKS|") + _("IGES files") + _T(" (*.igs *.iges)|*.igs;*.IGS;*.iges;*.IGES|") + _("STEP files") + _T(" (*.stp *.step)|*.stp;*.STP;*.step;*.STEP|") + _("STL files") + _T(" (*.stl)|*.stl;*.STL|") + _("Scalar Vector Graphics files") + _T(" (*.svg)|*.svg;*.SVG|") + _("DXF files") + _T(" (*.dxf)|*.dxf;*.DXF|") + _("Picture files") + _T(" (") + imageExtStr2 + _T(")|") + imageExtStr;
		return known_file_ext.c_str();
	}
	else{
		// file save
		known_file_ext = wxString(_("Known Files")) + _T(" |*.heeks;*.igs;*.iges;*.stp;*.step;*.stl;*.dxf|") + _("Heeks files") + _T(" (*.heeks)|*.heeks|") + _("IGES files") + _T(" (*.igs *.iges)|*.igs;*.iges|") + _("STEP files") + _T(" (*.stp *.step)|*.stp;*.step|") + _("STL files") + _T(" (*.stl)|*.stl|") + _("DXF files") + _T(" (*.dxf)|*.dxf");
		return known_file_ext.c_str();
	}
}

const wxChar* HeeksCADapp::GetKnownFilesCommaSeparatedList(bool open)const
{
	if(open){
		wxList handlers = wxImage::GetHandlers();
		wxString known_ext_str = _T("heeks, HEEKS, igs, iges, stp, step, stl, svg, dxf");
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

	MarkObjectTool(MarkedObject *marked_object, const wxPoint& point, bool xor_marked_list):m_marked_object(marked_object), m_point(point), m_xor_marked_list(xor_marked_list){}

	// Tool's virtual functions
	const wxChar* GetTitle(){
		if(m_xor_marked_list){
			if(wxGetApp().m_marked_list->ObjectMarked(m_marked_object->GetObject())){
				return _("Unmark");
			}
			else{
				return _("Mark");
			}
		}
		return _("Properties");
	}
	void Run(){
		if(m_marked_object == NULL)return;
		if(m_marked_object->GetObject() == NULL)return;
		if(m_xor_marked_list){
			if(wxGetApp().m_marked_list->ObjectMarked(m_marked_object->GetObject())){
				wxGetApp().m_marked_list->Remove(m_marked_object->GetObject(), true);
			}
			else{
				wxGetApp().m_marked_list->Add(m_marked_object->GetObject(), true);
			}
		}
		else{
			wxGetApp().m_marked_list->Clear(true);
			wxGetApp().m_marked_list->Add(m_marked_object->GetObject(), true);
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

		if(marked_object->GetObject()->CanBeRemoved())tools.push_back(new RemoveObjectTool(marked_object->GetObject()));

		tools.push_back(NULL);

		tools.push_back(new MarkObjectTool(marked_object, point, control_pressed));

		if (tools.size()>0)
		{
			ToolList *function_list = new ToolList(marked_object->GetObject()->GetShortStringOrTypeString());
			function_list->Add(tools);
			t_list.push_back(function_list);
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
	long save_filter = m_marked_list->m_filter;
	m_marked_list->m_filter = marking_filter;

	// stay in an input loop until finished picking
	OnRun();

	// restore marking filter
	m_marked_list->m_filter = save_filter;

	m_select_mode->m_doing_a_main_loop = false;
	SetInputMode(save_mode); // update tool bar
	return 1;
}

int HeeksCADapp::OnRun()
{
	try
	{
		return wxApp::OnRun();
	}
	catch(...)
	{
		return 0;
	}
}

bool HeeksCADapp::OnExceptionInMainLoop()
{
	throw;
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
		bool success;
		void(*fnOnNewOrOpen)(int) = (void(*)(int))(shared_library->GetSymbol(_T("OnNewOrOpen"), &success));
		if(fnOnNewOrOpen){
			(*fnOnNewOrOpen)(open ? 1:0);
		}
	}
}

void HeeksCADapp::RegisterHideableWindow(wxWindow* w)
{
	m_hideable_windows.push_back(w);
}

void HeeksCADapp::RemoveHideableWindow(wxWindow* w)
{
	m_hideable_windows.remove(w);
}

void HeeksCADapp::GetRecentFilesProfileString()
{
	HeeksConfig config;
	for(int i = 0; i < MAX_RECENT_FILES; i++)
	{
		wxString key_name = wxString::Format(_T("RecentFilePath%d"), i);
		wxString filepath = config.Read(key_name);
		if(filepath.IsEmpty())break;
		m_recent_files.push_back(filepath);
	}
}

void HeeksCADapp::WriteRecentFilesProfileString(wxConfigBase &config)
{
	std::list< wxString >::iterator It = m_recent_files.begin();
	for(int i = 0; i < MAX_RECENT_FILES; i++)
	{
		wxString key_name = wxString::Format(_T("RecentFilePath%d"), i);
		wxString filepath;
		if(It != m_recent_files.end())
		{
			filepath = *It;
			It++;
		}
		config.Write(key_name, filepath);
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
	if(IsModified())
	{
		wxString str = wxString(_("Save changes to file")) + _T(" ") + m_filepath;
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
	wxFileName f(m_filepath.c_str());
	wxString str = wxString::Format(_T("%s.%s - %s"), f.GetName().c_str(), f.GetExt().c_str(), m_filepath.c_str());
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
	if(object->UsesID())
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
		map.erase(id);
		map.insert( std::pair<int, HeeksObj*> (id, object) );
		object->m_id = id;
	}
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
	if(FindIt2 != next_id_map.end())
	{
		int &next_id = FindIt2->second;
		next_id = object->m_id; // this id has now become available
		map.erase(next_id);
	}
}

void HeeksCADapp::ResetIDs()
{
	used_ids.clear();
	next_id_map.clear();
}

static double* value_for_set_value = NULL;
static void set_value(double value, HeeksObj* object){*value_for_set_value = value;}

static bool *success_for_double_input = NULL;

class CInputApply:public Tool{
public:
	void Run(){
		*success_for_double_input = true;
		wxGetApp().ExitMainLoop();
	}
	const wxChar* GetTitle(){return _("Apply");}
	wxString BitmapPath(){return _T("apply");}
	const wxChar* GetToolTip(){return _("Accept value and continue");}
};

CInputApply input_apply;

class CInputCancel:public Tool{
public:
	void Run(){wxGetApp().ExitMainLoop();}
	const wxChar* GetTitle(){return _("Cancel");}
	wxString BitmapPath(){return _T("cancel");}
	const wxChar* GetToolTip(){return _("Cancel operation");}
};

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

void HeeksCADapp::RegisterOnSaveFn( void(*callbackfunc)(bool from_changed_prompt) )
{
	m_on_save_callbacks.push_back(callbackfunc);
}

void HeeksCADapp::RegisterIsModifiedFn( bool(*callbackfunc)() )
{
	m_is_modified_callbacks.push_back(callbackfunc);
}

void HeeksCADapp::CreateTransformGLList(const std::list<HeeksObj*>& list, bool show_grippers_on_drag){
	DestroyTransformGLList();
	m_transform_gl_list = glGenLists(1);
	glNewList(m_transform_gl_list, GL_COMPILE);
	std::list<HeeksObj *>::const_iterator It;
	for(It = list.begin(); It != list.end(); It++){
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

#ifndef WIN32
	if(wxTheClipboard->m_open)return false;
#endif

	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported( wxDF_TEXT ))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData( data );
			fstr = data.GetText();
		}  
		wxTheClipboard->Close();

		if(fstr.StartsWith(_T("<?xml version=\"1.0\" ?>")))return true;
	}

	return false;
}

void HeeksCADapp::Paste(HeeksObj* paste_into)
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
#ifdef __WXMSW__
		wofstream ofs(temp_file);
#else
		wofstream ofs(Ttc(temp_file.c_str()));
#endif
#else
		ofstream ofs(temp_file);
#endif
		ofs<<fstr.c_str();
	}

	m_marked_list->Clear(true);
	m_mark_newly_added_objects = true;
	OpenFile(temp_file, true, paste_into);
	m_mark_newly_added_objects = false;
}

bool HeeksCADapp::CheckForNOrMore(const std::list<HeeksObj*> &list, int min_num, int type, const wxString& msg, const wxString& caption)
{
	int num_of_type = 0;
	for(std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == type)num_of_type++;
	}

	if(num_of_type < min_num)
	{
		wxMessageBox(msg, caption);
		return false;
	}

	return true;
}

void HeeksCADapp::create_font()
{
#ifdef WIN32
	wxString fstr = GetExeFolder() + _T("/bitmaps/font.glf");
#else
	wxString fstr = GetExeFolder() + _T("/../share/heekscad/bitmaps/font.glf");
#endif
	glGenTextures( 1, &m_font_tex_number );

	//Create our glFont from verdana.glf, using texture 1
	m_gl_font.Create((char*)Ttc(fstr.c_str()), m_font_tex_number);
}

void HeeksCADapp::render_text(const wxChar* str)
{
	create_font();

	//Needs to be called before text output
	//glColor4ub(0, 0, 0, 255);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable(GL_TEXTURE_2D);
	glDepthMask(0);
	glDisable(GL_POLYGON_OFFSET_FILL);
	m_gl_font.Begin();

	//Draws text with a glFont
	m_gl_font.DrawString(str, 0.08f, 0.0f, 0.0f);

	glDepthMask(1);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

bool HeeksCADapp::get_text_size(const wxChar* str, float* width, float* height)
{
	create_font();

	std::pair<int, int> size;
	m_gl_font.GetStringSize(str, &size);
	*width = (float)(size.first) * 0.08f;
	*height = (float)(size.second) * 0.08f;

	return true;
}

void HeeksCADapp::render_screen_text2(const wxChar* str)
{
#if wxUSE_UNICODE
	size_t n = wcslen(str);
#else
	size_t n = strlen(str);
#endif
	wxChar buffer[1024];

	int j = 0;
	const wxChar* newlinestr = _T("\n");
	wxChar newline = newlinestr[0];

	for(size_t i = 0; i<n; i++)
	{
		buffer[j] = str[i];
		j++;
		if(str[i] == newline || i == n-1 || j == 1023){
			buffer[j] = 0;
			render_text(buffer);
			if(str[i] == newline)glTranslated(0.0, -2.2, 0.0);
			j = 0;
		}
	}
}

void HeeksCADapp::render_screen_text(const wxChar* str1, const wxChar* str2)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	m_frame->m_graphics->SetIdentityProjection();
	background_color.best_black_or_white().glColor();
	int w, h;
	m_frame->m_graphics->GetClientSize(&w, &h);
	glTranslated(2.0, h - 1.0, 0.0);

	glScaled(10.0, 10.0, 0);
	render_screen_text2(str1);

	glScaled(0.612, 0.612, 0);
	render_screen_text2(str2);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void HeeksCADapp::PlotSetColor(const HeeksColor &c)
{
	m_frame->m_printout->SetColor(c);
}

void HeeksCADapp::PlotLine(const double* s, const double* e)
{
	m_frame->m_printout->DrawLine(s, e);
}

void HeeksCADapp::PlotArc(const double* s, const double* e, const double* c)
{
	m_frame->m_printout->DrawArc(s, e, c);
}

void HeeksCADapp::InitialiseLocale()
{
	if(!m_locale_initialised)
	{
		m_locale_initialised = true;

		int language = wxLANGUAGE_DEFAULT;
		{
			HeeksConfig config;
			config.Read(_T("Language"), &language);
		}

		// Initialize the catalogs we'll be using
		if ( !m_locale.Init(language, wxLOCALE_CONV_ENCODING) )
		{
			wxLogError(_T("This language is not supported by the system."));
			return;
		}

		wxLocale::AddCatalogLookupPathPrefix(wxT("."));

		m_locale.AddCatalog(m_locale.GetCanonicalName());
		m_locale.AddCatalog(wxT("HeeksCAD"));
	}
}
