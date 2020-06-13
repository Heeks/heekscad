// MarkedList.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "MarkedList.h"
#include "../interface/HeeksObj.h"
#include "../interface/MarkedObject.h"
#include "../interface/PropertyInt.h"
#include "DigitizeMode.h"
#include "SelectMode.h"
#include "PointOrWindow.h"
#include "GripperSelTransform.h"
#include "GraphicsCanvas.h"
#include "HeeksFrame.h"
#include "ConversionTools.h"
#include "SolidTools.h"
#include "MenuSeparator.h"
#include "../interface/Picking.h"
using namespace std;

MarkedList::MarkedList(){
	gripping = false;
	point_or_window = new PointOrWindow(true);
	gripper_marked_list_changed = false;
	ignore_coords_only = false;
	m_filter = -1;
}

MarkedList::~MarkedList(void){
	delete point_or_window;
	std::list<Gripper*>::iterator It;
	for(It = move_grips.begin(); It != move_grips.end(); It++){
		Gripper* gripper = *It;
		gripper->m_index = 0;
	}
	delete_move_grips(false);
}

void MarkedList::delete_move_grips(bool check_app_grippers){
	std::list<Gripper*>::iterator It;
	for(It = move_grips.begin(); It != move_grips.end(); It++){
		Gripper* gripper = *It;
		if(check_app_grippers){
			if(gripper == wxGetApp().cursor_gripper)wxGetApp().cursor_gripper = NULL;
			if(gripper == wxGetApp().drag_gripper)wxGetApp().drag_gripper = NULL;
		}
		delete gripper;
	}
	move_grips.clear();
}

void MarkedList::create_move_grips(){
	delete_move_grips(true);
	int number_of_grips_made = 0;
	std::list<HeeksObj*>::iterator Iter ;
	for(Iter = m_list.begin(); Iter != m_list.end() && number_of_grips_made<100; Iter++){
		HeeksObj* object = *Iter;
		if(object->GetType() == GripperType)continue;
		std::list<GripData> vl;
		std::list<GripData>::iterator It;
		object->GetGripperPositionsTransformed(&vl, false);
		for(It = vl.begin(); It != vl.end() && number_of_grips_made<100; It++)
		{
			move_grips.push_back(new GripperSelTransform(*It, object));
			number_of_grips_made++;
		}
	}
}

void MarkedList::update_move_grips(){
	if(gripping)return;
	std::list<HeeksObj*>::iterator Iter ;
	std::list<Gripper*>::iterator Iter2;
	Iter2 = move_grips.begin();
	for(Iter = m_list.begin(); Iter != m_list.end(); Iter++){
		if(Iter2 == move_grips.end())break;
		HeeksObj* object = *Iter;
		if(object->GetType() == GripperType)continue;
		std::list<GripData> vl;
		std::list<GripData>::iterator It;
		object->GetGripperPositionsTransformed(&vl, false);
		for(It = vl.begin(); It != vl.end(); It++){
			Gripper* gripper = *Iter2;
			gripper->m_data = *It;
			Iter2++;
			if(Iter2 == move_grips.end())break;
		}
	}
}

void MarkedList::render_move_grips(bool select, bool no_color){
	std::list<Gripper*>::iterator It;
	for(It = move_grips.begin(); It != move_grips.end(); It++){
		if (select)SetPickingColor((*It)->GetIndex());
		(*It)->glCommands(select, false, no_color);
	}
}

void MarkedList::create_grippers(){
	if(gripping)return;
	if(gripper_marked_list_changed)create_move_grips();
	else update_move_grips();
	gripper_marked_list_changed = false;
}

void MarkedList::GrippersGLCommands(bool select, bool no_color){
	if(size()>0){
		create_grippers();
		render_move_grips(select, no_color);
	}
}

int MarkedList::GetObjectListFromColor(unsigned int color, std::list< HeeksObj* > &object_list)
{
	bool ignore_coords_only_found = false;
	HeeksObj *object = m_name_index.find(color);

	std::list<HeeksObj*> owner_list;
	while (object && (object != &(wxGetApp())))
	{
		owner_list.push_front(object);
		object = object->m_owner;
	}

	int highest_pick_priority = 0;

	for (std::list<HeeksObj*>::iterator It = owner_list.begin(); It != owner_list.end(); It++)
	{
		object = *It;

		if (!ignore_coords_only_found){
			if (ignore_coords_only && wxGetApp().m_digitizing->OnlyCoords(object)){
				ignore_coords_only_found = true;
			}
			else{
				if ((object->GetType() == GripperType) || ((object->GetMarkingMask() & m_filter) && (object->GetMarkingMask() != 0))){
					object_list.push_back(object);
					if(object->PickPriority() > highest_pick_priority)highest_pick_priority = object->PickPriority();
				}
			}
		}
	}

	return highest_pick_priority;
}

int MarkedList::GetObjectsFromColors(const std::set<unsigned int> &color_list, std::multimap< int, std::list< HeeksObj* > > &objects)
{
	int objects_highest_pick_priority = 0;

	for (std::set<unsigned int>::const_iterator It = color_list.begin(); It != color_list.end(); It++)
	{
		unsigned int color = *It;
		std::list< HeeksObj* > object_list;
		int highest_pick_priority = GetObjectListFromColor(color, object_list);

		if(object_list.size() > 0)
		{
			objects.insert(std::make_pair(highest_pick_priority, object_list));
			if(highest_pick_priority > objects_highest_pick_priority)objects_highest_pick_priority = highest_pick_priority;
		}
	}

	return objects_highest_pick_priority;
}

void MarkedList::ObjectsInWindow( wxRect window, MarkedObject* marked_object, bool single_picking){
	// render everything with unique colors

	wxGetApp().m_frame->m_graphics->SetCurrent();

	glDrawBuffer(GL_BACK);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	wxGetApp().m_frame->m_graphics->SetViewport();
	wxGetApp().m_frame->m_graphics->m_view_point.SetProjection(true);
	wxGetApp().m_frame->m_graphics->m_view_point.SetModelview();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1);
	glDepthMask(1);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glShadeModel(GL_FLAT);
	wxGetApp().m_frame->m_graphics->m_view_point.SetPolygonOffset();

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	wxGetApp().glCommands(true, false, true);

	glDisable(GL_DEPTH_TEST);

	GrippersGLCommands(true, true);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_COLOR_MATERIAL);

	if (window.width < 0)
	{
		window.x += window.width;
		window.width = abs(window.width);
	}
	if (window.height < 0)
	{
		window.y += window.height;
		window.height = abs(window.height);
	}

	unsigned int pixel_size = 4 * window.width * window.height;
	unsigned char* pixels = (unsigned char*)malloc(pixel_size);
	memset((void*)pixels, 0, pixel_size);
	glReadPixels(window.x, window.y, window.width, window.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// check all the pixels in the box
	std::set<unsigned int> color_list;
	for (unsigned int i = 0; i < pixel_size; i += 4)
	{
		unsigned int color = GetPickingName(pixels[i], pixels[i + 1], pixels[i + 2]);
		if (color != 0)color_list.insert(color);
	}
	std::multimap< int, std::list< HeeksObj* > > objects;
	int objects_highest_pick_priority = GetObjectsFromColors(color_list, objects);

	if (single_picking){
		// check centre pixel of box
		int half_window_width = (window.width) / 2;
		unsigned char* pixel_pos = pixels + 4 * ( half_window_width * window.width + half_window_width );
		unsigned int color = GetPickingName(pixel_pos[0], pixel_pos[1], pixel_pos[2]);
		std::list< HeeksObj* > object_list;
		if (color != 0)
		{
			int highest_pick_priority = GetObjectListFromColor(color, object_list);
			if(highest_pick_priority >= objects_highest_pick_priority)
			{
				objects.clear();
				objects.insert(std::make_pair(highest_pick_priority, object_list));
			}
		}

		while(objects.size() > 1)
		{
			objects.erase(objects.begin());
		}
	}

	for (std::multimap< int, std::list< HeeksObj* > >::iterator It = objects.begin(); It != objects.end(); It++)
	{
		std::list< HeeksObj* > &object_list = It->second;
		MarkedObject* current_found_object = marked_object;

		for (std::list<HeeksObj*>::iterator It2 = object_list.begin(); (It2 != object_list.end()) && (current_found_object != NULL); It2++)
		{
			HeeksObj* object = *It2;
			int window_size = window.width;
			current_found_object = current_found_object->Add(object, 0, window_size, 0, NULL);
		}
	}

	free(pixels);
}

void MarkedList::Add(std::list<HeeksObj *> &list, bool call_OnChanged){
	std::list<HeeksObj *>::iterator It;
	for(It = list.begin(); It != list.end(); It++){
		HeeksObj *object = *It;
		m_list.push_back(object);
		m_set.insert(object);
	}
	if(call_OnChanged)OnChanged(false, &list, NULL);
}

void MarkedList::Remove(HeeksObj *object, bool call_OnChanged){
	if (!object) return;
	std::list<HeeksObj *> list;
	list.push_back(object);
	Remove(list, call_OnChanged);
}

void MarkedList::Add(HeeksObj *object, bool call_OnChanged){
	if (!object) return;
	std::list<HeeksObj *> list;
	list.push_back(object);
	Add(list, call_OnChanged);
}

void MarkedList::Remove(const std::list<HeeksObj *> &obj_list, bool call_OnChanged){
	std::list<HeeksObj *>::const_iterator It;
	for(It = obj_list.begin(); It != obj_list.end(); It++){
		HeeksObj *object = *It;
		if(m_set.find(object) != m_set.end()){
			m_list.remove(object);
		}
		m_set.erase(object);
	}
	if(call_OnChanged)OnChanged(false, NULL, &obj_list);
}

void MarkedList::Clear(bool call_OnChanged){
	m_list.clear();
	m_set.clear();
	if(call_OnChanged)OnChanged(true, NULL, NULL);
}

void MarkedList::FindMarkedObject(const wxPoint &point, MarkedObject* marked_object){
	if(marked_object == NULL)return;
	point_or_window->SetWithPoint(point);
	ObjectsInWindow(point_or_window->box_chosen, marked_object);
}

bool MarkedList::ObjectMarked(HeeksObj *object){
	return m_set.find(object) != m_set.end();
}

void MarkedList::OnChanged(bool selection_cleared, const std::list<HeeksObj *>* added, const std::list<HeeksObj *>* removed){
	gripper_marked_list_changed = true;
	wxGetApp().ObserversMarkedListChanged(selection_cleared, added, removed);
}

void MarkedList::OnChangedAdded(HeeksObj* object)
{
	std::list<HeeksObj *> added;
	added.push_back(object);
	OnChanged(false, &added, NULL);
}

void MarkedList::OnChangedRemoved(HeeksObj* object)
{
	std::list<HeeksObj *> removed;
	removed.push_back(object);
	OnChanged(false, NULL, &removed);
}

void MarkedList::set_ignore_onoff(HeeksObj* object, bool b){
	if(b)m_ignore_set.insert(object);
	else m_ignore_set.erase(object);
}

bool MarkedList::get_ignore(HeeksObj* object){
	if(m_ignore_set.find(object) != m_ignore_set.end())return true;
	return false;
}

void MarkedList::GetProperties(std::list<Property *> *list){
	if(m_list.size() == 1)
	{
		m_list.front()->GetProperties(list);
	}
	else
	{
		// multiple selection
		//list->push_back(new PropertyInt(_("Number of items selected"), m_list.size(), NULL));
		for(std::list<HeeksObj*>::iterator It = m_list.begin(); It != m_list.end(); It++)
		{
			HeeksObj* obj = *It;
			obj->GetProperties(list);
		}
	}
}

class DeleteMarkedListTool : public Tool
{
public:
	const wxChar* GetTitle() {return _("Delete Marked Items");}
	void Run() {wxGetApp().DeleteMarkedItems();}
	wxString BitmapPath(){return _T("delete");}
} delete_marked_list_tool;

class CopyMarkedList: public Tool
{
public:
	void Run();
	const wxChar* GetTitle(){return _("Copy");}
	wxString BitmapPath(){return _T("copy");}
	const wxChar* GetToolTip(){return _("Copies the selected items to the clipboard");}
} copy_marked_list;

void CopyMarkedList::Run()
{
	wxGetApp().m_marked_list->CopySelectedItems();
}

class PasteTool: public Tool
{
public:
	HeeksObj* m_paste_into;

	PasteTool():m_paste_into(NULL){}
	void Run();
	const wxChar* GetTitle(){return m_paste_into ? _("Paste Into") : _("Paste");}
	wxString BitmapPath(){return _T("paste");}
	const wxChar* GetToolTip(){return _("Paste items from the clipboard to the drawing");}
} paste_tool;

void PasteTool::Run()
{
	wxGetApp().Paste(m_paste_into, NULL);
}

void MarkedList::GetTools(MarkedObject* clicked_object, std::list<Tool*>& t_list, const wxPoint* p, bool copy_and_paste_tools){
	if(wxGetApp().m_no_creation_mode)return;

	if (m_list.size() > 0)
	{
		t_list.push_back(&delete_marked_list_tool);
		t_list.push_back(new MenuSeparator);
	}

	wxGetApp().GetExternalMarkedListTools(t_list);
	GetConversionMenuTools(&t_list);
	GetSolidMenuTools(&t_list);

	if(copy_and_paste_tools)
	{
		// cut and copy tools
		for(std::list<HeeksObj*>::iterator It = m_list.begin(); It != m_list.end(); It++)
		{
			HeeksObj* object = *It;
			if(object->CanBeCopied())
			{
				t_list.push_back(&copy_marked_list);
				break;
			}
		}

		// paste
		if (wxGetApp().IsPasteReady())
		{
			paste_tool.m_paste_into = clicked_object->GetObject();
			t_list.push_back(&paste_tool);
		}
	}
}

void MarkedList::CutSelectedItems()
{
	CopySelectedItems();
	wxGetApp().Remove(m_list);
}

void MarkedList::CopySelectedItems()
{
#if wxCHECK_VERSION(3, 0, 0)
	wxStandardPaths& sp = wxStandardPaths::Get();
#else
	wxStandardPaths sp;
#endif
	sp.GetTempDir();
	wxFileName temp_file(sp.GetTempDir().c_str(), _T("temp_Heeks_clipboard_file.xml"));

	wxGetApp().SaveXMLFile(m_list, temp_file.GetFullPath().c_str(), true);

#if wxUSE_UNICODE
	wifstream ifs(Ttc(temp_file.GetFullPath().c_str()));
#else
	ifstream ifs(temp_file.GetFullPath());
#endif
	if(!ifs)return;

	wxString fstr;
	wxChar str[1024];
	while(!(ifs.eof())){
		ifs.getline(str, 1022);
		fstr.append(str);
		fstr.append(_T("\r\n"));
		if(!ifs)break;
	}

	if (wxTheClipboard->Open())
	{
		// This data object is held by the clipboard,
		// so do not delete them in the app.
		wxTheClipboard->SetData( new wxTextDataObject(fstr));
		wxTheClipboard->Close();
	}
}

void MarkedList::Reset()
{
	delete_move_grips(true);
	m_name_index.clear();
}

unsigned int MarkedList::GetIndex(HeeksObj *object) {
	return m_name_index.insert(object);
}

void MarkedList::ReleaseIndex(unsigned int index) {
	return m_name_index.erase(index);
}
