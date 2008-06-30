// HeeksCAD.cpp

#include "stdafx.h"
#include "HeeksCAD.h"
#include <wx/filedlg.h>
#include <wx/clipbrd.h>
#include <wx/stdpaths.h>
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
#include "LeftCanvas.h"
#include "SelectMode.h"
#include "MagDragWindow.h"
#include "Window.h"
#include "DigitizeMode.h"
#include "GripperMode.h"
#include "ConversionTools.h"
#include "Solid.h"
#include "ViewPoint.h"
#include "MarkedList.h"
#include "History.h"
#include "Observer.h"
#include "TransformTool.h"
#include "Grid.h"
#include "StretchTool.h"
#include "HLine.h"
#include "RemoveOrAddTool.h"

const int ID_TOOLBAR = 500;
const int ID_SOLID_TOOLBAR = 501;
static const long TOOLBAR_STYLE = wxTB_FLAT | wxTB_DOCKABLE | wxTB_TEXT;

IMPLEMENT_APP(HeeksCADapp)

HeeksCADapp::HeeksCADapp(): ObjList()
{
	m_geom_tol = 0.001;
	background_color = HeeksColor(0, 0, 0);
	current_color = HeeksColor(0, 0, 0);
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
	digitize_coords = true;
	digitize_screen = false;
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
	m_config->Read("DrawEnd", &digitize_end, true);
	m_config->Read("DrawInters", &digitize_inters, false);
	m_config->Read("DrawCentre", &digitize_centre, false);
	m_config->Read("DrawMidpoint", &digitize_midpoint, false);
	m_config->Read("DrawNearest", &digitize_nearest, false);
	m_config->Read("DrawCoords", &digitize_coords, true);
	m_config->Read("DrawScreen", &digitize_screen, false);
	m_config->Read("DrawToGrid", &draw_to_grid, false);
	m_config->Read("DrawGrid", &digitizing_grid);
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
	m_config->Read("RotateMode", &m_rotate_mode);
	m_config->Read("Antialiasing", &m_antialiasing);
	m_config->Read("GridMode", &grid_mode);
	m_config->Read("m_light_push_matrix", &m_light_push_matrix);
	m_config->Read("WheelForwardAway", &mouse_wheel_forward_away);
	gripper_mode->GetProfileStrings();
	m_frame = new CHeeksFrame( wxT( "HeeksCAD free Solid Modelling software based on Open CASCADE" ), wxPoint(posx, posy), wxSize(width, height));
	SetInputMode(m_select_mode);
	m_frame->Show(TRUE);
	SetTopWindow(m_frame);
	return TRUE;
} 

int HeeksCADapp::OnExit(){
	int result = wxApp::OnExit();
	m_config->Write("DrawEnd", digitize_end);
	m_config->Write("DrawInters", digitize_inters);
	m_config->Write("DrawCentre", digitize_centre);
	m_config->Write("DrawMidpoint", digitize_midpoint);
	m_config->Write("DrawNearest", digitize_nearest);
	m_config->Write("DrawCoords", digitize_coords);
	m_config->Write("DrawScreen", digitize_screen);
	m_config->Write("DrawToGrid", draw_to_grid);
	m_config->Write("DrawGrid", digitizing_grid);
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
	m_config->Write("RotateMode", m_rotate_mode);	
	m_config->Write("Antialiasing", m_antialiasing);	
	m_config->Write("GridMode", grid_mode);
	m_config->Write("m_light_push_matrix", m_light_push_matrix);
	m_config->Write("WheelForwardAway", mouse_wheel_forward_away);
	
	delete m_config;

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
}

bool HeeksCADapp::OpenFile(const char *filepath){
	// i'm not sure what return value means if anything

	wxString wf(filepath);

	// check for solid files
	if(CShape::ImportSolidsFile(filepath)){
		return true;
	}

	// error
	else{
		char mess[1024];
		sprintf(mess, "Invalid file type chosen ( expecting file with %s suffix )", wxGetApp().GetKnownFilesCommaSeparatedList());
		wxMessageBox(mess);
	}

	return false;
}

bool HeeksCADapp::SaveFile(const char *filepath){
	// i'm not sure what return value means, if anything

	wxString wf(filepath);

	if(CShape::ExportSolidsFile(filepath)){
		return true;
	}
	else{
		char mess[1024];
		sprintf(mess, "Invalid file type chosen ( expecting file with %s suffix )", wxGetApp().GetKnownFilesCommaSeparatedList());
		wxMessageBox(mess);
	}

	return false;
}


void HeeksCADapp::Repaint(bool soon){
	m_frame->m_graphics->Refresh(0);
}

void HeeksCADapp::RecalculateGLLists(){
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
	glEnable(GL_POLYGON_OFFSET_FILL);
	m_frame->m_graphics->m_view_point.SetPolygonOffset();
	glCommands(select, false, false);
	input_mode_object->OnRender();
	DestroyLights();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL );
	RenderGrid(&view_point);
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
	wxString title(wxT("menu"));
	wxMenu menu(title);
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

void HeeksCADapp::GetProperties(std::list<Property *> *list)
{
	list->push_back ( new PropertyColor ( "background color",  background_color, on_set_background_color ) );
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
	for(std::list<wxDynamicLibrary*>::iterator It = m_loaded_libraries.begin(); It != m_loaded_libraries.end(); It++){
		wxDynamicLibrary* shared_library = *It;
		void(*GetProperties)(std::list<Property *> *) = (void (*)(std::list<Property *> *))(shared_library->GetSymbol("GetProperties"));
		(*GetProperties)(list);
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
	return "Known Files |*.igs;*.iges;*.stp;*.step|IGES files (*.igs *.iges)|*.igs;*.iges|STEP files (*.stp *.step)|*.stp;*.step|STL files (*.stl)|*.stl";
}

const char* HeeksCADapp::GetKnownFilesCommaSeparatedList()const
{
	return "igs, iges, stp, step, stl";
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
			if (!wxGetApp().m_marked_list->ObjectMarked(marked_object->GetObject()))
			{
				tools.push_back(new MarkObjectTool(marked_object, point, control_pressed));
			}
		}
		unsigned int s = tools.size();
		marked_object->GetObject()->GetTools(&tools, &point);
		if (tools.size()>s) tools.push_back(NULL);
		if (marked_object->GetObject()->m_owner)
		{
			s = tools.size();
			marked_object->GetObject()->m_owner->GetToolsForChild(marked_object->GetObject(), &tools, &point);
			if (tools.size()>s) tools.push_back(NULL);
		}
		if (wxGetApp().input_mode_object)
		{
			s = tools.size();
			wxGetApp().input_mode_object->GetObjectGetTools(&tools, marked_object->GetObject(), &point);
			if (tools.size()>s) tools.push_back(NULL);
		}
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
