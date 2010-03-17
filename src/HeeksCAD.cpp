// HeeksCAD.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "HeeksCAD.h"
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
#include "../interface/DoubleInput.h"
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
#include "UndoEngine.h"
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
#include "Sketch.h"
#include "BezierCurve.h"
#include "StlSolid.h"
#include "dxf.h"
#include "svg.h"
#include "CoordinateSystem.h"
#include "RegularShapesDrawing.h"
#include "HeeksPrintout.h"
#include "HeeksConfig.h"
#include "Group.h"
#include "RS274X.h"
#include "CxfFont.h"
#include "AutoSave.h"

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

	m_version_number = _T("0 11 1");
	m_geom_tol = 0.000001;
	m_view_units = 1.0;
	for(int i = 0; i<NUM_BACKGROUND_COLORS; i++)background_color[i] = HeeksColor(0, 0, 0);
	m_background_mode = BackgroundModeOneColor;
	current_color = HeeksColor(0, 0, 0);
	construction_color = HeeksColor(0, 0, 255);
	face_selection_color = HeeksColor(0, 0, 0);
	input_mode_object = NULL;
	cur_mouse_pos.x = 0;
	cur_mouse_pos.y = 0;
	drag_gripper = NULL;
	cursor_gripper = NULL;
	magnification = new MagDragWindow();
	viewrotating = new ViewRotating;
	viewzooming = new ViewZooming;
	m_select_mode = new CSelectMode(false);
	// Set this to 'true' for 'select similar' mode.  I'm not there yet.
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
	autosolve_constraints = false;
	digitizing_grid = 1.0;
	grid_mode = 3;
	m_rotate_mode = 0;
	m_antialiasing = false;
	m_light_push_matrix = true;
	m_marked_list = new MarkedList;
	history = new UndoEngine(this);
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
	m_file_open_matrix = NULL;
	m_min_correlation_factor = 0.75;
	m_max_scale_threshold = 1.5;
	m_number_of_sample_points = 10;
	m_property_grid_validation = false;

	m_font_paths = _T("/usr/share/qcad/fonts");
	m_stl_facet_tolerance = 0.1;
	// GetAvailableFonts();

	m_pVectorFont = NULL;	// Default to internal (OpenGL) font.
	m_icon_texture_number = 0;
	m_extrude_to_solid = true;
	m_revolve_angle = 360.0;


    {
        std::list<wxString> extensions;
        extensions.push_back(_T("svg"));
        RegisterFileOpenHandler( extensions, OpenSVGFile );
    }
    {
        std::list<wxString> extensions;
        extensions.push_back(_T("stl"));
        RegisterFileOpenHandler( extensions, OpenSTLFile );
    }
    {
        std::list<wxString> extensions;
        extensions.push_back(_T("gbr"));
        extensions.push_back(_T("rs274x"));
        extensions.push_back(_T("pho"));
        RegisterFileOpenHandler( extensions, OpenRS274XFile );
    }

}

HeeksCADapp::~HeeksCADapp()
{
	delete m_marked_list;
	m_marked_list = NULL;
	observers.clear();
	delete history;
	delete magnification;
	delete m_select_mode;
	delete m_digitizing;
	delete viewrotating;
	delete viewzooming;
	delete m_ruler;
	if(m_printData)delete m_printData;
	if(m_pageSetupData)delete m_pageSetupData;

	m_pVectorFont = NULL;	// Don't free this here.  This memory will be released via ~CxfFonts() instead.
	if (m_pVectorFonts.get() != NULL) delete m_pVectorFonts.release();
}

bool HeeksCADapp::OnInit()
{
	m_gl_font_initialized = false;
	m_sketch_mode = false;

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
	config.Read(_T("AutoSolveConstraints"), &autosolve_constraints, false);
	config.Read(_T("UseOldFuse"), &useOldFuse, true);
	config.Read(_T("DrawGrid"), &digitizing_grid);
	config.Read(_T("DrawRadius"), &digitizing_radius);
	{
		int default_color[NUM_BACKGROUND_COLORS] = {
			HeeksColor(255, 175, 96).COLORREF_color(),
			HeeksColor(198, 217, 119).COLORREF_color(),
			HeeksColor(247, 198, 243).COLORREF_color(),
			HeeksColor(193, 235, 236).COLORREF_color(),
			HeeksColor(255, 255, 255).COLORREF_color(),
			HeeksColor(255, 255, 255).COLORREF_color(),
			HeeksColor(255, 255, 255).COLORREF_color(),
			HeeksColor(255, 255, 255).COLORREF_color(),
			HeeksColor(255, 255, 255).COLORREF_color(),
			HeeksColor(255, 255, 255).COLORREF_color()
		};

		for(int i = 0; i<NUM_BACKGROUND_COLORS; i++)
		{
			wxString key = wxString::Format(_T("BackgroundColor%d"), i);
			int color = default_color[i];
			config.Read(key, &color);
			background_color[i] = HeeksColor(color);
		}
		int mode = (int)BackgroundModeTwoColors;
		config.Read(_T("BackgroundMode"), &mode);
		m_background_mode = (BackgroundMode)mode;
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
	{
		int color = HeeksColor(0, 255, 0).COLORREF_color();
		config.Read(_T("FaceSelectionColor"), &color);
		face_selection_color = HeeksColor(color);
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
	config.Read(_T("AllowOpenGLStippling"), &m_allow_opengl_stippling, true);

	config.Read(_T("DxfMakeSketch"), &HeeksDxfRead::m_make_as_sketch, true);
	config.Read(_T("ViewUnits"), &m_view_units);
	config.Read(_T("FaceToSketchDeviation"), &(FaceToSketchTool::deviation));

	config.Read(_T("MinCorrelationFactor"), &m_min_correlation_factor);
	config.Read(_T("MaxScaleThreshold"), &m_max_scale_threshold);
	config.Read(_T("NumberOfSamplePoints"), &m_number_of_sample_points);

	config.Read(_T("FontPaths"), &m_font_paths, _T("/usr/share/qcad/fonts"));
	config.Read(_T("STLFacetTolerance"), &m_stl_facet_tolerance, 0.1);

	config.Read(_T("AutoSaveInterval"), (int *) &m_auto_save_interval, 0);
	if (m_auto_save_interval > 0)
	{
		m_pAutoSave = std::auto_ptr<CAutoSave>(new CAutoSave(m_auto_save_interval));
	} // End if - then
	config.Read(_T("ExtrudeToSolid"), &m_extrude_to_solid);
	config.Read(_T("RevolveAngle"), &m_revolve_angle);

	HDimension::ReadFromConfig(config);

	m_ruler->ReadFromConfig(config);

	GetRecentFilesProfileString();
	GetAvailableFonts();

	wxImage::AddHandler(new wxPNGHandler);
	m_frame = new CHeeksFrame( wxT( "HeeksCAD free Solid Modelling software based on Open CASCADE" ), wxPoint(posx, posy), wxSize(width, height));

#ifdef __WXMSW__
	// to do, make this compile in Linux
	m_frame->SetIcon(wxICON(HeeksCAD));
#endif

	// NOTE: A side-effect of calling the SetInputMode() method is
	// that the GetOptions() method is called.  To that end, all
	// configuration settings should be read BEFORE this point.
	SetInputMode(m_select_mode);
	m_frame->Show(TRUE);
	SetTopWindow(m_frame);

	OnNewOrOpen(false);
	SetLikeNewFile();
	SetFrameTitle();
	SetStatusText();

	if ((m_pAutoSave.get() != NULL) && (m_pAutoSave->AutoRecoverRequested()))
	{
		m_pAutoSave->Recover();
	}
	else
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
				wxString param = parser.GetParam(0);
				if(!(param.Lower().EndsWith(_T(".dll"))))
				{
					OpenFile(parser.GetParam(0));
				}
			}
		}
	}


	return TRUE;
}

int HeeksCADapp::OnExit(){

	if (m_pAutoSave.get() != NULL)
	{
		delete m_pAutoSave.release();
		m_pAutoSave = std::auto_ptr<CAutoSave>(NULL);
	}

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
	config.Write(_T("AutoSolveConstraints"), autosolve_constraints);
	config.Write(_T("UseOldFuse"), useOldFuse);
	config.Write(_T("DrawGrid"), digitizing_grid);
	config.Write(_T("DrawRadius"), digitizing_radius);
	for(int i = 0; i<NUM_BACKGROUND_COLORS; i++)
	{
		wxString key = wxString::Format(_T("BackgroundColor%d"), i);
		config.Write(key, background_color[i].COLORREF_color());
	}
	config.Write(_T("BackgroundMode"), (int)m_background_mode);
	config.Write(_T("FaceSelectionColor"), face_selection_color.COLORREF_color());
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
	config.Write(_T("AllowOpenGLStippling"), m_allow_opengl_stippling);
	config.Write(_T("DxfMakeSketch"), HeeksDxfRead::m_make_as_sketch);
	config.Write(_T("FaceToSketchDeviation"), FaceToSketchTool::deviation);

	config.Write(_T("MinCorrelationFactor"), m_min_correlation_factor);
	config.Write(_T("MaxScaleThreshold"), m_max_scale_threshold);
	config.Write(_T("NumberOfSamplePoints"), m_number_of_sample_points);
	config.Write(_T("FontPaths"), m_font_paths);
	config.Write(_T("STLFacetTolerance"), m_stl_facet_tolerance);
	config.Write(_T("AutoSaveInterval"), m_auto_save_interval);
	config.Write(_T("ExtrudeToSolid"), m_extrude_to_solid);
	config.Write(_T("RevolveAngle"), m_revolve_angle);

	HDimension::WriteToConfig(config);

	m_ruler->WriteToConfig(config);

	WriteRecentFilesProfileString(config);

	delete history;
	history = NULL;

	for(std::list<wxDynamicLibrary*>::iterator It = m_loaded_libraries.begin(); It != m_loaded_libraries.end(); It++){
		wxDynamicLibrary* shared_library = *It;
		delete shared_library;
	}
	m_loaded_libraries.clear();

	int result = wxApp::OnExit();
	return result;
}

void HeeksCADapp::SetStatusText()
{
	wxString status_text;

	status_text.Append(_("units"));
	status_text.Append(_T(" = "));
	if(fabs(m_view_units - 1.0) < 0.0000000001)status_text.Append(_("mm"));
	else if(fabs(m_view_units - 25.4) < 0.000000001)status_text.Append(_("inch"));
	else status_text.Append(wxString::Format(_T("%g"), m_view_units));

	m_frame->SetStatusText( status_text );
}

bool HeeksCADapp::EndSketchMode()
{
	if(m_sketch_mode  && input_mode_object == m_select_mode)
	{
		m_sketch_mode = false;
		Repaint();
		return true;
	}
	return false;
}

void HeeksCADapp::EnterSketchMode(CSketch* sketch)
{
	m_sketch_mode = true;
	m_sketch = sketch;
	if(sketch->m_coordinate_system)
		m_current_coordinate_system = sketch->m_coordinate_system;
}

CSketch* HeeksCADapp::GetContainer()
{
	if(m_sketch_mode)
		return m_sketch;

	m_sketch = new CSketch();
	Add(m_sketch,NULL);
	return m_sketch;
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
	delete history;
	history = new UndoEngine(this);
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
				CShape::ImportSolidsFile(temp_file, &index_map, paste_into_for_ReadSTEPFileFromXMLElement);
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
			CShape::ImportSolidsFile(temp_file,&index_map, paste_into_for_ReadSTEPFileFromXMLElement);
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
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Constraint", Constraint::ReadFromXMLElement ) );
	}
}

void HeeksCADapp::RegisterReadXMLfunction(const char* type_name, HeeksObj*(*read_xml_function)(TiXmlElement* pElem))
{
	if(xml_read_fn_map.find(type_name) != xml_read_fn_map.end()){
		wxMessageBox(_T("Error - trying to register an XML read function for an existing type"));
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


/**
	This method traverses the list of HeeksObj pointers recursively and looks for
	duplicate objects based on the type/id pairs.  When a duplicate is found, the
	duplicate is deleted and both pointers are reset to point at a common object.
	The object's owner pointers are also updated appropriately.
 */

HeeksObj *HeeksCADapp::MergeCommonObjects( ObjectReferences_t & unique_set, HeeksObj *object ) const
{
	if (object->GetFirstChild() != NULL)
	{
		// This is also an ObjList pointer.  Recursively check the children for duplicates.
		for (HeeksObj *child = object->GetFirstChild(); child != NULL; child = object->GetNextChild())
		{
			HeeksObj * replacement = MergeCommonObjects( unique_set, child );
			if (replacement != child)
			{
				object->Remove(child);
				object->Add( replacement, NULL );
			}
		}
	}

	HeeksObj *unique_reference = object;
	ObjectReference_t object_reference(object->GetType(),object->m_id);

	if (unique_set.find(object_reference) == unique_set.end())
	{
		unique_set.insert( std::make_pair( object_reference, object ) );
		unique_reference = object;
	}
	else
	{
		// We've seen an object like this one before.  Use the old one.
		unique_reference = unique_set[ object_reference ];
		std::list<HeeksObj *> owners = object->Owners();
		for (std::list<HeeksObj *>::iterator itOwner = owners.begin(); itOwner != owners.end(); itOwner++)
		{
			(*itOwner)->Remove(object);
			(*itOwner)->Add( unique_reference, NULL );
		}

		unique_set[ object_reference ] = unique_reference;
		object = unique_reference;
	}

	return(object);
}


void HeeksCADapp::OpenXMLFile(const wxChar *filepath, HeeksObj* paste_into)
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

	ObjectReferences_t unique_set;

	std::list<HeeksObj*> objects;
	for(pElem = root->FirstChildElement(); pElem;	pElem = pElem->NextSiblingElement())
	{
		HeeksObj* object = ReadXMLElement(pElem);
		if(object)
		{
			objects.push_back(object);
		}
	}

	for (std::list<HeeksObj *>::iterator itObject = objects.begin(); itObject != objects.end(); itObject++)
	{
		*itObject = MergeCommonObjects( unique_set, *itObject );
	}

	if(objects.size() > 0)
	{
		HeeksObj* add_to = this;
		if(paste_into)add_to = paste_into;
		for(std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
		{
			HeeksObj* object = *It;
			object->ReloadPointers();

			if(add_to->CanAdd(object) && object->CanAddTo(add_to))
			{
				if(object->OneOfAKind())
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
					if(!one_found)
					{
						add_to->Add(object, NULL);
					}
				}
				else
				{
					add_to->Add(object, NULL);
				}
			}
		}
	}

	CGroup::MoveSolidsToGroupsById(this);
}

/* static */ void HeeksCADapp::OpenSVGFile(const wxChar *filepath)
{
	HeeksSvgRead svgread(filepath,true);
}

/* static */ void HeeksCADapp::OpenSTLFile(const wxChar *filepath)
{
	CStlSolid* new_object = new CStlSolid(filepath, &(wxGetApp().current_color));
	wxGetApp().Add(new_object, NULL);
}

/* static */ void HeeksCADapp::OpenDXFFile(const wxChar *filepath )
{
	HeeksDxfRead dxf_file(filepath);
	dxf_file.DoRead();
}

/* static */ void HeeksCADapp::OpenRS274XFile(const wxChar *filepath)
{
    wxString message(_("Select how the file is to be interpreted"));
    wxString caption(_("RS274X file interpretation"));

    wxArrayString choices;

    choices.Add(_("Produce trace isolation sketches"));
    choices.Add(_("Produce trace centre-line sketches"));

    wxString choice = ::wxGetSingleChoice( message, caption, choices );

    RS274X gerber;

    if (choice == choices[0])
    {
        gerber.Read(wxString(filepath).mb_str(), RS274X::IsolationRouting );
    }

    if (choice == choices[1])
    {
        gerber.Read(wxString(filepath).mb_str(), RS274X::CentreLines );
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
			HImage* new_object = new HImage(filepath);
			Add(new_object, NULL);
			Repaint();
			return true;
		}
	}

	return false;
}

bool HeeksCADapp::OpenFile(const wxChar *filepath, bool import_not_open, HeeksObj* paste_into, bool retain_filename /* = true */ )
{
	CreateUndoPoint();

	m_in_OpenFile = true;
	m_file_open_or_import_type = FileOpenOrImportTypeOther;
	double file_open_matrix[16];
	if(import_not_open && wxGetApp().m_current_coordinate_system)
	{
		extract(wxGetApp().m_current_coordinate_system->GetMatrix(), file_open_matrix);
		m_file_open_matrix = file_open_matrix;
	}

	// returns true if file open was successful
	wxString wf(filepath);
	wf.LowerCase();

	wxString extension(filepath);
	int offset = extension.Find('.',true);
	if (offset > 0) extension.Remove(0, offset+1);
	extension.LowerCase();

	bool open_succeeded = true;

	if(wf.EndsWith(_T(".heeks")))
	{
		m_file_open_or_import_type = FileOpenTypeHeeks;
		if(import_not_open)
			m_file_open_or_import_type = FileImportTypeHeeks;
		OpenXMLFile(filepath, paste_into);
	}
	else if(m_fileopen_handlers.find(extension) != m_fileopen_handlers.end())
	{
		(m_fileopen_handlers[extension])(filepath);
	}
	else if(wf.EndsWith(_T(".dxf")))
	{
		m_file_open_or_import_type = FileOpenOrImportTypeDxf;
		OpenDXFFile(filepath);
	}
	// check for images
	else if(OpenImageFile(filepath))
	{
	}

	// check for solid files
	else if(CShape::ImportSolidsFile(filepath))
	{
	}
	else if(wf.EndsWith(_T(".dll")))
	{
		// add a plugin
	}
	else
	{
		// error
		wxString str = wxString(_("Invalid file type chosen")) + _T("  ") + _("expecting") + _T(" ") + GetKnownFilesCommaSeparatedList();
		wxMessageBox(str);
		open_succeeded = false;
	}

	if(open_succeeded)
	{
	    InsertRecentFileItem(filepath);

		if((!import_not_open) && (retain_filename))
		{
			m_filepath.assign(filepath);
			SetFrameTitle();
			SetLikeNewFile();
		}
		Changed();
	}

	m_file_open_matrix = NULL;
	m_in_OpenFile = false;

	return open_succeeded;
}

static void WriteDXFEntity(HeeksObj* object, CDxfWrite& dxf_file, const wxString parent_layer_name)
{
    wxString layer_name;

    if (parent_layer_name.Len() == 0)
    {
        layer_name << object->m_id;
    }
    else
    {
        layer_name = parent_layer_name;
    }

	switch(object->GetType())
	{
	case LineType:
		{
			HLine* l = (HLine*)object;
			double s[3], e[3];
			extract(l->A->m_p, s);
			extract(l->B->m_p, e);
			dxf_file.WriteLine(s, e, layer_name);
		}
		break;
	case ArcType:
		{
			HArc* a = (HArc*)object;
			double s[3], e[3], c[3];
			extract(a->A->m_p, s);
			extract(a->B->m_p, e);
			extract(a->C->m_p, c);
			bool dir = a->m_axis.Direction().Z() > 0;
			dxf_file.WriteArc(s, e, c, dir, layer_name);
		}
		break;
      case EllipseType:
                {
			HEllipse* e = (HEllipse*)object;
			double c[3];
			extract(e->C->m_p, c);
			bool dir = e->m_zdir.Z() > 0;
			double maj_r = e->m_majr;
			double min_r = e->m_minr;
			double rot = e->GetRotation();
			dxf_file.WriteEllipse(c, maj_r, min_r, rot, 0, 2 * Pi, dir, layer_name);
                }
		break;
        case CircleType:
                {
			HCircle* cir = (HCircle*)object;
			double c[3];
			extract(cir->C->m_p, c);
			double radius = cir->m_radius;
			dxf_file.WriteCircle(c, radius, layer_name);
                }
		break;
	default:
		{
		    if (parent_layer_name.Len() == 0)
		    {
		        layer_name.Clear();
                if ((object->GetShortString() != NULL) && (wxString(object->GetTypeString()) != wxString(object->GetShortString())))
                {
                    layer_name << object->GetShortString();
                }
                else
                {
                    layer_name << object->m_id;   // Use the ID as a layer name so that it's unique.
                }
		    }
		    else
		    {
		        layer_name = parent_layer_name;
		    }

			for(HeeksObj* child = object->GetFirstChild(); child; child = object->GetNextChild())
			{

				// recursive
				WriteDXFEntity(child, dxf_file, layer_name);
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
		// At this level, don't assign each element to its own layer.  We only want sketch objects
		// to be located on their own layer.  This will be done from within the WriteDXFEntity() method.
		WriteDXFEntity(object, dxf_file, _T(""));
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
		object->GetTriangles(write_stl_triangle, m_stl_facet_tolerance);
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
	Constraint::BeginSave();
	for(std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->WriteXML(root);
	}

	// write the constraints to the root
	Constraint::EndSave(root);

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
	if(soon)m_frame->m_graphics->RefreshSoon();
	else m_frame->m_graphics->Refresh();
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

		if(m_sketch_mode)
			screen_text1.Append(_T("Sketch Mode:\n"));

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

	// for sketch mode, only allow items in the sketch to be selected
	if(m_sketch_mode)
	{
		m_sketch->glCommands(select, marked || wxGetApp().m_marked_list->ObjectMarked(m_sketch), no_color);
	}
	else
	{
		std::list<HeeksObj*>::iterator It;
		for(It=m_objects.begin(); It!=m_objects.end() ;It++)
		{
			HeeksObj* object = *It;
			if(object->OnVisibleLayer() && object->m_visible)
			{
				if(select)glPushName((unsigned long)object);
				object->glCommands(select, marked || wxGetApp().m_marked_list->ObjectMarked(object), no_color);
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

void HeeksCADapp::SetLikeNewFile(void){
	history->SetLikeNewFile();
}

void HeeksCADapp::ClearHistory(void){
	history->ClearHistory();
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

	m_marked_list->GetTools(marked_object, f_list, &new_point, true);

	temp_f_list.clear();
	if(input_mode_object)input_mode_object->GetTools(&temp_f_list, &new_point);
	AddToolListWithSeparator(f_list, temp_f_list);
	temp_f_list.clear();

	// exit full screen
	if(wxGetApp().m_frame->IsFullScreen() && point.x>=0 && point.y>=0)temp_f_list.push_back(new CFullScreenTool);

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
		CreateUndoPoint();
		t->Run();
		if(t->CallChangedOnRun())
		{
			//TODO: this should be handled better. While TreeView is parsing the tree it should be able to remove
			//non existant items from MarkedList.
			m_marked_list->Clear(false);
			Changed();
			Repaint();
		}
	}
}

void HeeksCADapp::DoToolUndoably(Tool *t)
{
	CreateUndoPoint();
	t->Run();
	Changed();
}

void HeeksCADapp::Undo(void)
{
	history->Undo();
	Changed();
	m_marked_list->Clear(true);
	Repaint();
}

void HeeksCADapp::Redo(void)
{
	history->Redo();
	Changed();
	m_marked_list->Clear(true);
	Repaint();
}

void HeeksCADapp::WentTransient(HeeksObj* obj, TransientObject *tobj)
{
	m_transient_objects[(HeeksObj*)tobj].push_back(obj);
}

void HeeksCADapp::ClearTransients()
{
	m_transient_objects.clear();
}

std::map<HeeksObj*,std::list<HeeksObj*> >& HeeksCADapp::GetTransients()
{
	return m_transient_objects;
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

void HeeksCADapp::CreateUndoPoint()
{
	history->CreateUndoPoint();
}

void HeeksCADapp::Changed()
{
	ObserversOnChange(NULL,NULL,NULL);
}

void HeeksCADapp::ObserversMarkedListChanged(bool selection_cleared, const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed){
	std::set<Observer*>::iterator It;
	for(It = observers.begin(); It != observers.end(); It++){
		Observer *ov = *It;
		ov->WhenMarkedListChanges(selection_cleared, added, removed);
	}
}

void HeeksCADapp::ObserversFreeze()
{
	std::set<Observer*>::iterator It;
	for(It = observers.begin(); It != observers.end(); It++){
		Observer *ov = *It;
		ov->Freeze();
	}
}

void HeeksCADapp::ObserversThaw()
{
	std::set<Observer*>::iterator It;
	for(It = observers.begin(); It != observers.end(); It++){
		Observer *ov = *It;
		ov->Thaw();
	}
}

bool HeeksCADapp::Add(HeeksObj *object, HeeksObj* prev_object)
{
	if (!ObjList::Add(object, prev_object)) return false;

	if(object->GetType() == CoordinateSystemType && (!m_in_OpenFile || (m_file_open_or_import_type !=FileOpenTypeHeeks && m_file_open_or_import_type  != FileImportTypeHeeks)))
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
	HeeksObj* owner = object->GetFirstOwner();
	while(owner)
	{
		if(owner != this)
		{
			owner->Remove(object);
			owner->ReloadPointers();
		}
		else
			ObjList::Remove(object);
		owner = object->GetNextOwner();
	}
	if(object == m_current_coordinate_system)m_current_coordinate_system = NULL;
}

void HeeksCADapp::Remove(std::list<HeeksObj*> objects)
{
	ObjList::Remove(objects);
}

void HeeksCADapp::Transform(std::list<HeeksObj*> objects,double *m)
{
	std::list<HeeksObj*>::iterator it;
	for(it = objects.begin(); it!= objects.end(); ++it)
	{
		(*it)->ModifyByMatrix(m);
	}
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

void on_set_background_color0(HeeksColor value, HeeksObj* object)
{
	wxGetApp().background_color[0] = value;
	wxGetApp().Repaint();
}

void on_set_background_color1(HeeksColor value, HeeksObj* object)
{
	wxGetApp().background_color[1] = value;
	wxGetApp().Repaint();
}

void on_set_background_color2(HeeksColor value, HeeksObj* object)
{
	wxGetApp().background_color[2] = value;
	wxGetApp().Repaint();
}

void on_set_background_color3(HeeksColor value, HeeksObj* object)
{
	wxGetApp().background_color[3] = value;
	wxGetApp().Repaint();
}

void on_set_background_color4(HeeksColor value, HeeksObj* object)
{
	wxGetApp().background_color[4] = value;
	wxGetApp().Repaint();
}

void on_set_background_color5(HeeksColor value, HeeksObj* object)
{
	wxGetApp().background_color[5] = value;
	wxGetApp().Repaint();
}

void on_set_background_color6(HeeksColor value, HeeksObj* object)
{
	wxGetApp().background_color[6] = value;
	wxGetApp().Repaint();
}

void on_set_background_color7(HeeksColor value, HeeksObj* object)
{
	wxGetApp().background_color[7] = value;
	wxGetApp().Repaint();
}

void on_set_background_color8(HeeksColor value, HeeksObj* object)
{
	wxGetApp().background_color[8] = value;
	wxGetApp().Repaint();
}

void on_set_background_color9(HeeksColor value, HeeksObj* object)
{
	wxGetApp().background_color[9] = value;
	wxGetApp().Repaint();
}

void on_set_background_mode(int value, HeeksObj* object)
{
	wxGetApp().m_background_mode = (BackgroundMode)value;
	wxGetApp().Repaint();
	wxGetApp().m_frame->m_options->RefreshByRemovingAndAddingAll();
}

void on_set_face_color(HeeksColor value, HeeksObj* object)
{
	wxGetApp().face_selection_color = value;
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

void on_autosolve(bool onoff, HeeksObj* object)
{
	wxGetApp().autosolve_constraints = onoff;
	wxGetApp().Repaint();
}

void on_useOldFuse(bool onoff, HeeksObj* object)
{
	wxGetApp().useOldFuse = onoff;
}

static void on_extrude_to_solid(bool onoff, HeeksObj* object)
{
	wxGetApp().m_extrude_to_solid = onoff;
}

static void on_revolve_angle(double value, HeeksObj* object)
{
	wxGetApp().m_revolve_angle = value;
}

void on_set_min_correlation_factor(double value, HeeksObj* object)
{
	wxGetApp().m_min_correlation_factor = value;
	wxGetApp().Repaint();
}

void on_set_max_scale_threshold(double value, HeeksObj* object)
{
	wxGetApp().m_max_scale_threshold = value;
	wxGetApp().Repaint();
}

void on_set_number_of_sample_points(int value, HeeksObj* object)
{
	wxGetApp().m_number_of_sample_points = value;
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
void on_sel_filter_pad(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_PAD;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_PAD;
}

void on_sel_filter_part(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_PART;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_PART;
}

void on_sel_filter_pocket(bool value, HeeksObj* object){
	if(value)wxGetApp().m_marked_list->m_filter |= MARKING_FILTER_POCKETSOLID;
	else wxGetApp().m_marked_list->m_filter &= ~MARKING_FILTER_POCKETSOLID;
}

void on_dxf_make_sketch(bool value, HeeksObj* object){
	HeeksDxfRead::m_make_as_sketch = value;
}

void on_stl_facet_tolerance(double value, HeeksObj* object){
	wxGetApp().m_stl_facet_tolerance = value;
}

void on_set_auto_save_interval(int value, HeeksObj* object){
	wxGetApp().m_auto_save_interval = value;

	if (wxGetApp().m_auto_save_interval > 0)
	{
		if (wxGetApp().m_pAutoSave.get() == NULL)
		{
			wxGetApp().m_pAutoSave = std::auto_ptr<CAutoSave>(new CAutoSave( wxGetApp().m_auto_save_interval, true ));
		}
		else
		{
			wxGetApp().m_pAutoSave->Start( wxGetApp().m_auto_save_interval * 60 * 1000, false );
		}
	}
	else
	{
		if (wxGetApp().m_pAutoSave.get() != NULL)
		{
			delete wxGetApp().m_pAutoSave.release();
			wxGetApp().m_pAutoSave = std::auto_ptr<CAutoSave>(NULL);
		}
	}
	HeeksConfig config;
	config.Write(_T("AutoSaveInterval"), wxGetApp().m_auto_save_interval);
}

static void on_set_units(int value, HeeksObj* object)
{
	wxGetApp().m_view_units = (value == 0) ? 1.0:25.4;
	HeeksConfig config;
	config.Write(_T("ViewUnits"), wxGetApp().m_view_units);
	wxGetApp().m_frame->m_properties->RefreshByRemovingAndAddingAll();
	wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
	wxGetApp().m_ruler->KillGLLists();
	wxGetApp().SetStatusText();
	wxGetApp().Repaint();
}

static void on_dimension_draw_flat(bool value, HeeksObj* object)
{
	HDimension::DrawFlat = value;
	wxGetApp().Repaint();
}

static void on_set_font(int zero_based_choice, HeeksObj *obj)
{
	if (zero_based_choice == 0)
	{
		wxGetApp().m_pVectorFont = NULL;
		return;
	} // End if - then

	std::set<wxString> names = wxGetApp().GetAvailableFonts()->FontNames();
	std::vector<wxString> vector_names;
	vector_names.push_back(_T("OpenGL"));	// Keep the zero-based offset.
	std::copy( names.begin(), names.end(), std::inserter( vector_names, vector_names.end() ) );
	if (zero_based_choice < int(vector_names.size()))
	{
		wxGetApp().m_pVectorFont = wxGetApp().GetAvailableFonts()->Font( VectorFont::Name_t(vector_names[zero_based_choice].c_str()) );
	}
}


static void on_edit_font_paths(const wxChar* value, HeeksObj* object)
{
	wxGetApp().m_font_paths.assign(value);
	if (wxGetApp().m_pVectorFonts.get()) delete wxGetApp().m_pVectorFonts.release();
	wxGetApp().GetAvailableFonts();

	HeeksConfig config;
	config.Write(_T("FontPaths"), wxGetApp().m_font_paths);
}


void on_set_allow_opengl_stippling(bool value, HeeksObj* object)
{
	wxGetApp().m_allow_opengl_stippling = value;
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
	{
		std::list< wxString > choices;
		choices.push_back ( wxString ( _("single color") ) );
		choices.push_back ( wxString ( _("top color and bottom color") ) );
		choices.push_back ( wxString ( _("left color and right color") ) );
		choices.push_back ( wxString ( _("four corner colors") ) );
		choices.push_back ( wxString ( _("sky and ground") ) );
		view_options->m_list.push_back ( new PropertyChoice ( _("background mode"),  choices, (int)m_background_mode, NULL, on_set_background_mode ) );
	}
	switch(m_background_mode)
	{
	case BackgroundModeOneColor:
		view_options->m_list.push_back ( new PropertyColor ( _("background color"),  background_color[0], NULL, on_set_background_color0 ) );
		break;

	case BackgroundModeTwoColors:
		view_options->m_list.push_back ( new PropertyColor ( _("top background color"),  background_color[0], NULL, on_set_background_color0 ) );
		view_options->m_list.push_back ( new PropertyColor ( _("bottom background color"),  background_color[1], NULL, on_set_background_color1 ) );
		break;

	case BackgroundModeTwoColorsLeftToRight:
		view_options->m_list.push_back ( new PropertyColor ( _("left background color"),  background_color[0], NULL, on_set_background_color0 ) );
		view_options->m_list.push_back ( new PropertyColor ( _("right background color"),  background_color[2], NULL, on_set_background_color2 ) );
		break;

	case BackgroundModeFourColors:
		view_options->m_list.push_back ( new PropertyColor ( _("top left background color"),  background_color[0], NULL, on_set_background_color0 ) );
		view_options->m_list.push_back ( new PropertyColor ( _("bottom left background color"),  background_color[1], NULL, on_set_background_color1 ) );
		view_options->m_list.push_back ( new PropertyColor ( _("top right background color"),  background_color[2], NULL, on_set_background_color2 ) );
		view_options->m_list.push_back ( new PropertyColor ( _("bottom right background color"),  background_color[3], NULL, on_set_background_color3 ) );
		break;

	case BackgroundModeSkyDome:
		view_options->m_list.push_back ( new PropertyColor ( _("sky top color"),  background_color[4], NULL, on_set_background_color4 ) );
		view_options->m_list.push_back ( new PropertyColor ( _("sky middle color"),  background_color[5], NULL, on_set_background_color5 ) );
		view_options->m_list.push_back ( new PropertyColor ( _("sky bottom color"),  background_color[6], NULL, on_set_background_color6 ) );
		view_options->m_list.push_back ( new PropertyColor ( _("ground top color"),  background_color[7], NULL, on_set_background_color7 ) );
		view_options->m_list.push_back ( new PropertyColor ( _("ground middle color"),  background_color[8], NULL, on_set_background_color8 ) );
		view_options->m_list.push_back ( new PropertyColor ( _("ground bottom color"),  background_color[9], NULL, on_set_background_color9 ) );
		break;

	}
	{
		std::list< wxString > choices;
		choices.push_back ( wxString ( _("no grid") ) );
		choices.push_back ( wxString ( _("faint color") ) );
		choices.push_back ( wxString ( _("alpha blending") ) );
		choices.push_back ( wxString ( _("colored alpha blending") ) );
		view_options->m_list.push_back ( new PropertyChoice ( _("grid mode"),  choices, grid_mode, NULL, on_set_grid_mode ) );
	}
	view_options->m_list.push_back ( new PropertyColor ( _("face selection color"), face_selection_color, NULL, on_set_face_color ) );
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
	view_options->m_list.push_back(new PropertyCheck(_("flat on screen dimension text"), HDimension::DrawFlat, NULL, on_dimension_draw_flat));
	view_options->m_list.push_back(new PropertyCheck(_("Allow OpenGL stippling"), m_allow_opengl_stippling, NULL, on_set_allow_opengl_stippling));

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
	digitizing->m_list.push_back(new PropertyCheck(_("autosolve constraints"), autosolve_constraints, NULL, on_autosolve));
	list->push_back(digitizing);

	PropertyList* correlation_properties = new PropertyList(_("correlation"));
	correlation_properties->m_list.push_back(new PropertyDouble(_("Minimum correlation factor (0.0 (nothing like it) -> 1.0 (perfect match))"), m_min_correlation_factor, NULL, on_set_min_correlation_factor));
	correlation_properties->m_list.push_back(new PropertyDouble(_("Maximum scale threshold (1.0 - must be same size, 1.5 (can be half as big again or 2/3 size)"), m_max_scale_threshold, NULL, on_set_max_scale_threshold));
	correlation_properties->m_list.push_back(new PropertyInt(_("Number of sample points"), m_number_of_sample_points, NULL, on_set_number_of_sample_points));
	list->push_back(correlation_properties);

	PropertyList* drawing = new PropertyList(_("drawing"));
	drawing->m_list.push_back ( new PropertyColor ( _("current color"),  current_color, NULL, on_set_current_color ) );
	drawing->m_list.push_back ( new PropertyColor ( _("construction color"),  construction_color, NULL, on_set_construction_color ) );
	drawing->m_list.push_back(new PropertyLength(_("geometry tolerance"), m_geom_tol, NULL, on_set_geom_tol));
	drawing->m_list.push_back(new PropertyLength(_("face to sketch deviaton"), FaceToSketchTool::deviation, NULL, on_set_face_to_sketch_deviation));
	drawing->m_list.push_back(new PropertyCheck(_("Use old solid fuse ( to prevent coplanar faces )"), useOldFuse, NULL, on_useOldFuse));
	drawing->m_list.push_back(new PropertyCheck(_("Extrude makes a solid"), m_extrude_to_solid, NULL, on_extrude_to_solid));
	drawing->m_list.push_back(new PropertyDouble(_("Solid revolution angle"), m_revolve_angle, NULL, on_revolve_angle));
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
	selection_filter->m_list.push_back(new PropertyCheck(_("pad"), (m_marked_list->m_filter & MARKING_FILTER_PAD) != 0, NULL, on_sel_filter_pad));
	selection_filter->m_list.push_back(new PropertyCheck(_("part"), (m_marked_list->m_filter & MARKING_FILTER_PART) != 0, NULL, on_sel_filter_part));
	selection_filter->m_list.push_back(new PropertyCheck(_("pocket"), (m_marked_list->m_filter & MARKING_FILTER_POCKETSOLID) != 0, NULL, on_sel_filter_pocket));
	list->push_back(selection_filter);

	PropertyList* file_options = new PropertyList(_("file options"));
	PropertyList* dxf_options = new PropertyList(_("DXF"));
	dxf_options->m_list.push_back(new PropertyCheck(_("make sketch"), HeeksDxfRead::m_make_as_sketch, NULL, on_dxf_make_sketch));
	file_options->m_list.push_back(dxf_options);
	PropertyList* stl_options = new PropertyList(_("STL"));
	stl_options->m_list.push_back(new PropertyDouble(_("stl save facet tolerance"), m_stl_facet_tolerance, NULL, on_stl_facet_tolerance));
	file_options->m_list.push_back(stl_options);
	file_options->m_list.push_back(new PropertyInt(_("auto save interval (in minutes)"), m_auto_save_interval, NULL, on_set_auto_save_interval));
	list->push_back(file_options);

	// Font options
	PropertyList* font_options = new PropertyList(_("font options"));
	if (m_pVectorFonts.get() != NULL)
	{
		std::list<wxString> choices;

		choices.push_back( wxString(_("OpenGL (default) font")) );
		int choice = 0;

		int option = 0;
		std::set<VectorFont::Name_t> font_names = m_pVectorFonts->FontNames();
		for (std::set<VectorFont::Name_t>::const_iterator l_itFontName = font_names.begin();
			l_itFontName != font_names.end(); l_itFontName++)
		{
			option++;
			choices.push_back( *l_itFontName );
			if ((m_pVectorFont != NULL) && (m_pVectorFont->Name() == *l_itFontName)) choice = option;
		} // End for
		font_options->m_list.push_back ( new PropertyChoice ( _("Active font"),  choices, choice, this, on_set_font ) );
	}

	font_options->m_list.push_back( new PropertyString(_("Paths (semicolon delimited)"), m_font_paths, this, on_edit_font_paths));
	list->push_back(font_options);
}

void HeeksCADapp::DeleteMarkedItems()
{
	std::list<HeeksObj *> list = m_marked_list->list();

	// clear first, so properties cancel happens first
	m_marked_list->Clear(false);

	if(list.size() == 1){
		Remove(*(list.begin()));
	}
	else if(list.size()>1){
		Remove(list);
	}
	Repaint(0);
}

void HeeksCADapp::glColorEnsuringContrast(const HeeksColor &c)
{
	if(c == background_color[0])background_color[0].best_black_or_white().glColor();
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

		wxString registeredExtensions;
		for (FileOpenHandlers_t::const_iterator itHandler = m_fileopen_handlers.begin(); itHandler != m_fileopen_handlers.end(); itHandler++)
		{
		    registeredExtensions << _T(";*.") << itHandler->first;
		}

		known_file_ext = wxString(_("Known Files")) + _T(" |*.heeks;*.HEEKS;*.igs;*.IGS;*.iges;*.IGES;*.stp;*.STP;*.step;*.STEP;*.dxf;*.DXF") + imageExtStr + registeredExtensions + _T("|") + _("Heeks files") + _T(" (*.heeks)|*.heeks;*.HEEKS|") + _("IGES files") + _T(" (*.igs *.iges)|*.igs;*.IGS;*.iges;*.IGES|") + _("STEP files") + _T(" (*.stp *.step)|*.stp;*.STP;*.step;*.STEP|") + _("STL files") + _T(" (*.stl)|*.stl;*.STL|") + _("Scalar Vector Graphics files") + _T(" (*.svg)|*.svg;*.SVG|") + _("DXF files") + _T(" (*.dxf)|*.dxf;*.DXF|") + _("RX274X/Gerber files") + _T(" (*.gbr,*.rs274x)|*.gbr;*.GBR;*.rs274x;*.RS274X;*.pho;*.PHO|") + _("Picture files") + _T(" (") + imageExtStr2 + _T(")|") + imageExtStr;
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
		wxString known_ext_str = _T("heeks, HEEKS, igs, iges, stp, step, dxf");
		for(wxList::iterator It = handlers.begin(); It != handlers.end(); It++)
		{
			wxImageHandler* handler = (wxImageHandler*)(*It);
			wxString ext = handler->GetExtension();
			known_ext_str.Append(_T(", "));
			known_ext_str.Append(ext);
		}

		for (FileOpenHandlers_t::const_iterator itHandler = m_fileopen_handlers.begin(); itHandler != m_fileopen_handlers.end(); itHandler++)
		{
		    known_ext_str << _T(", ") << itHandler->first;
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
	bool CallChangedOnRun(){return false;}
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

		//TODO: this is the only useful thing removeobjecttool ever did
		//if(marked_object->GetObject()->CanBeRemoved())tools.push_back(new RemoveObjectTool(marked_object->GetObject()));

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

wxString HeeksCADapp::GetResFolder()const
{
#ifdef WIN32
	return GetExeFolder();
#else
#ifdef CODEBLOCKS
	return (GetExeFolder() + _T("/../.."));
#else
#ifdef RUNINPLACE
	return GetExeFolder();
#else
	return (GetExeFolder() + _T("/../share/heekscad"));
#endif
#endif
#endif
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
	bool save_just_one = m_select_mode->m_just_one;
	m_select_mode->m_just_one = just_one;
	SetInputMode(m_select_mode);

	// set marking filter
	long save_filter = m_marked_list->m_filter;
	m_marked_list->m_filter = marking_filter;

	// stay in an input loop until finished picking
	OnRun();

	// restore marking filter
	m_marked_list->m_filter = save_filter;

	m_select_mode->m_just_one = save_just_one;
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
	UsedIds_t::iterator FindIt1 = used_ids.find(type);
	if (FindIt1 == used_ids.end()) return(NULL);

	IdsToObjects_t &ids = FindIt1->second;
	if (ids.find(id) == ids.end()) return(NULL);
	else return(ids.lower_bound(id)->second);

}

void HeeksCADapp::SetObjectID(HeeksObj* object, int id)
{
	if(object->UsesID())
	{
		GroupId_t id_group_type = object->GetIDGroupType();

		UsedIds_t::iterator FindIt1 = used_ids.find(id_group_type);
		if(FindIt1 == used_ids.end())
		{
			// add a new map
			object->m_id = id;
			IdsToObjects_t empty_map;
			empty_map.insert( std::make_pair( id, object ) );
			FindIt1 = used_ids.insert( std::make_pair( id_group_type, empty_map )).first;
			return;
		}

		IdsToObjects_t &map = FindIt1->second;
		bool found = false;
		for (IdsToObjects_t::iterator itIdsToObjects = map.lower_bound( id ); itIdsToObjects != map.upper_bound( id ); itIdsToObjects++)
		{
			if (itIdsToObjects->second == object)
			{
				found = true;
				break;
			}
		} // End for

		if (found)
		{
			// It's already there.
			object->m_id = id;
			return;
		}

		// It's not there yet.
		map.insert( std::make_pair(id, object) );
		object->m_id = id;
		return;
	}
}

int HeeksCADapp::GetNextID(int id_group_type)
{
	UsedIds_t::iterator FindIt1 = used_ids.find(id_group_type);
	if(FindIt1 == used_ids.end())return 1;

	std::map< int, int >::iterator FindIt2 = next_id_map.find(id_group_type);

	IdsToObjects_t &map = FindIt1->second;

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

	UsedIds_t::iterator FindIt1 = used_ids.find(id_group_type);
	if(FindIt1 == used_ids.end())return;
	std::map< int, int >::iterator FindIt2 = next_id_map.find(id_group_type);

	IdsToObjects_t &map = FindIt1->second;
	if (FindIt2 != next_id_map.end())
	{
		for ( IdsToObjects_t::iterator it = map.lower_bound( object->m_id ); it != map.upper_bound( object->m_id ); /* increment within loop */)
		{
			if (it->second == object)
			{
				map.erase(it);
				return;
			}
			else
			{
				it++;
			}
		} // End for
	} // End if - then
}

void HeeksCADapp::ResetIDs()
{
	used_ids.clear();
	next_id_map.clear();
}

bool HeeksCADapp::InputDouble(const wxChar* prompt, const wxChar* value_name, double &value)
{
	CInputMode* save_mode = input_mode_object;
	CDoubleInput double_input(prompt, value_name, value);
	SetInputMode(&double_input);

	OnRun();

	SetInputMode(save_mode);

	if(CDoubleInput::m_success)value = double_input.m_value;

	return CDoubleInput::m_success;
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

bool HeeksCADapp::CheckForNOrMore(const std::list<HeeksObj*> &list, int min_num, int type1, int type2, const wxString& msg, const wxString& caption)
{
	int num_of_type = 0;
	for(std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == type1 || object->GetType() == type2)num_of_type++;
	}

	if(num_of_type < min_num)
	{
		wxMessageBox(msg, caption);
		return false;
	}

	return true;
}

bool HeeksCADapp::CheckForNOrMore(const std::list<HeeksObj*> &list, int min_num, int type1, int type2, int type3, const wxString& msg, const wxString& caption)
{
	int num_of_type = 0;
	for(std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == type1 || object->GetType() == type2 || object->GetType() == type3)num_of_type++;
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
	if(m_gl_font_initialized)
		return;
	wxString fstr = GetResFolder() + _T("/bitmaps/font.glf");
	glGenTextures( 1, &m_font_tex_number );

	//Create our glFont from verdana.glf, using texture 1
	m_gl_font.Create((char*)Ttc(fstr.c_str()), m_font_tex_number);
	m_gl_font_initialized = true;
}

void HeeksCADapp::render_text(const wxChar* str)
{
	//Needs to be called before text output
	create_font();
	//glColor4ub(0, 0, 0, 255);
	EnableBlend();
	glEnable(GL_TEXTURE_2D);
	glDepthMask(0);
	glDisable(GL_POLYGON_OFFSET_FILL);
	m_gl_font.Begin();

	//Draws text with a glFont
	m_gl_font.DrawString(str, 0.08f, 0.0f, 0.0f);

	glDepthMask(1);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_TEXTURE_2D);
	DisableBlend();
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
	background_color[0].best_black_or_white().glColor();
	int w, h;
	m_frame->m_graphics->GetClientSize(&w, &h);
	glTranslated(2.0, h - 1.0, 0.0);

	glScaled(10.0, 10.0, 0);
	render_screen_text2(str1);

	glScaled(0.612, 0.612, 0);
	render_screen_text2(str2);

//Even though this is in reverse order, the different matrices have different stacks, and we want to exit here in the modelview
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void HeeksCADapp::EnableBlend()
{
	if(!m_antialiasing)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

void HeeksCADapp::DisableBlend()
{
	if(!m_antialiasing)glDisable(GL_BLEND);
}

void HeeksCADapp::render_screen_text_at(const wxChar* str1, double scale, double x, double y, double theta)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	m_frame->m_graphics->SetIdentityProjection();
	background_color[0].best_black_or_white().glColor();
	int w, h;
	m_frame->m_graphics->GetClientSize(&w, &h);
	glTranslated(x,y, 0.0);

	glScaled(scale, scale, 0);
	glRotated(theta,0,0,1);
	render_screen_text2(str1);

//Even though this is in reverse order, the different matrices have different stacks, and we want to exit here in the modelview
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
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


std::auto_ptr<VectorFonts>	& HeeksCADapp::GetAvailableFonts()
{
	if (m_pVectorFonts.get() == NULL)
	{
		std::vector<wxString> paths = Tokens( m_font_paths, _T(";") );
		for (std::vector<wxString>::const_iterator l_itPath = paths.begin(); l_itPath != paths.end(); l_itPath++)
		{
			if (m_pVectorFonts.get() == NULL)
			{
				m_pVectorFonts = std::auto_ptr<VectorFonts>(new VectorFonts(*l_itPath));
			} // End if - then
			else
			{
				m_pVectorFonts->Add( *l_itPath );
			} // End if - else
		} // End for
	} // End if - then

	return(m_pVectorFonts);
} // End GetAvailableFonts() method

void HeeksCADapp::GetPluginsFromCommandLineParams(std::list<wxString> &plugins)
{
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
				wxString param = parser.GetParam(0);
				if(param.Lower().EndsWith(_T(".dll")))
				{
					plugins.push_back(param);
				}
			}
		}
	}
#endif
}

void HeeksCADapp::RegisterOnBuildTexture(void(*callbackfunc)())
{
	m_on_build_texture_callbacks.push_back(callbackfunc);
}


bool HeeksCADapp::RegisterFileOpenHandler( const std::list<wxString> file_extensions, FileOpenHandler_t fileopen_handler )
{
    std::set<wxString> valid_extensions;

    // For Linux, where the file system supports case-sensitive file names, we should expand
    // the extensions to include uppercase and lowercase and add them to our set.

    for (std::list<wxString>::const_iterator l_itExtension = file_extensions.begin(); l_itExtension != file_extensions.end(); l_itExtension++)
    {
        wxString extension(*l_itExtension);

        // Make sure the calling routine didn't add the '.'
        if (extension.StartsWith(_T(".")))
        {
            extension.Remove(0,1);
        }

        #ifndef WIN32
            extension.LowerCase();
            valid_extensions.insert( extension );

            extension.UpperCase();
            valid_extensions.insert( extension );
        #else
            extension.LowerCase();
            valid_extensions.insert( extension );
        #endif // WIN32
    } // End for

    for (std::set<wxString>::iterator itExtension = valid_extensions.begin(); itExtension != valid_extensions.end(); itExtension++)
    {
        if (m_fileopen_handlers.find( *itExtension ) != m_fileopen_handlers.end())
        {
            printf("Aborting file-open handler registration for extension %s as it has already been registered\n", Ttc( *itExtension ));
            return(false);
        }
    }

    // We must not have seen these extensions before.  Go ahead and register them.
    for (std::set<wxString>::iterator itExtension = valid_extensions.begin(); itExtension != valid_extensions.end(); itExtension++)
    {
        m_fileopen_handlers.insert( std::make_pair( *itExtension, fileopen_handler ) );
    }

    return(true);
}

bool HeeksCADapp::UnregisterFileOpenHandler( void (*fileopen_handler)(const wxChar *path) )
{
    std::list<FileOpenHandlers_t::iterator> remove;
    for (FileOpenHandlers_t::iterator itHandler = m_fileopen_handlers.begin(); itHandler != m_fileopen_handlers.end(); itHandler++)
    {
        if (itHandler->second == fileopen_handler) remove.push_back(itHandler);
    }

    for (std::list<FileOpenHandlers_t::iterator>::iterator itRemove = remove.begin(); itRemove != remove.end(); itRemove++)
    {
        m_fileopen_handlers.erase( *itRemove );
    }

    return(remove.size() > 0);
}
