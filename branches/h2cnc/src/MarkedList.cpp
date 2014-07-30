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
#include "SketchTools.h"
#include "SolidTools.h"
#include "MenuSeparator.h"
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
		if(select)glPushName((*It)->GetIndex());
		(*It)->glCommands(select, false, no_color);
		if(select)glPopName();
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

void MarkedList::ObjectsInWindow( wxRect window, MarkedObject* marked_object, bool single_picking){
	int buffer_length = 16384;
	GLuint *data = (GLuint *)malloc( buffer_length * sizeof(GLuint) );
	if(data == NULL)return;
	int half_window_width = 0;
	wxPoint window_centre;
	if(single_picking){
		half_window_width = (window.width)/2;
		window_centre.x = window.x + window.width/2;
		window_centre.y = window.y + window.height/2;
	}
	int window_mode = 0;
	while(1){
		if(single_picking){
			int window_size = half_window_width;
			if(window_mode == 0)window_size = 0;
			if(window_mode == 1)window_size = half_window_width/2;
			window.x = window_centre.x - window_size;
			window.y = window_centre.y - window_size;
			window.width = window_size * 2;
			window.height = window_size * 2;
		}
	    GLint num_hits = -1;
		while(num_hits < 0){
			glSelectBuffer(buffer_length, data);
			glRenderMode(GL_SELECT);
			glInitNames();
			wxGetApp().m_current_viewport->SetViewport();
			wxGetApp().m_current_viewport->m_view_point.SetPickProjection(window);
			wxGetApp().m_current_viewport->m_view_point.SetModelview();
			wxGetApp().glCommands(true, false, false);
			GrippersGLCommands(true, false);
			glFlush();
			num_hits = glRenderMode(GL_RENDER);
			if(num_hits<0){
				free(data);
				buffer_length *= 10;
				data = (GLuint *)malloc( buffer_length * sizeof(GLuint) );
				if(data == NULL)return;
			}
		}
		int pos = 0;
		bool added = false;
		for(unsigned i=0; i<(unsigned int)num_hits; i++)
		{
			unsigned int names = data[pos];
			if(names == 0)break;
			pos++;
			unsigned int min_depth = data[pos];
			pos+=2;
			MarkedObject* current_found_object = marked_object;
			bool ignore_coords_only_found = false;
			for(unsigned int j=0; j<names; j++, pos++){
				HeeksObj *object = m_name_index.find(data[pos]);
				bool custom_names = object->UsesCustomSubNames();
				if(!ignore_coords_only_found && current_found_object != NULL){


					if(ignore_coords_only && wxGetApp().m_digitizing->OnlyCoords(object)){
						ignore_coords_only_found = true;
					}
					else{
						if((object->GetType() == GripperType) || ((object->GetMarkingMask() & m_filter) && (object->GetMarkingMask() != 0))){
							int window_size = window.width;
							current_found_object = current_found_object->Add(object, min_depth, window_size, custom_names ? (names - 1 - j) : 0, custom_names ? (&data[pos+1]):NULL);
							added = true;
						}
					}

				}
				if(custom_names)
				{
					pos+=(names-j);
					break;
				}
			}
		}
		window_mode++;
		if(!single_picking)break;
		if(window_mode > 2)break;
	}

	free(data);
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
		list->push_back(new PropertyInt(_("Number of items selected"), m_list.size(), NULL));
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

	if(m_list.size() == 1)
	{
		m_list.front()->GetTools(&t_list, p);
	}

	GetConversionMenuTools(&t_list);
	GetSketchMenuTools(&t_list);
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
	wxStandardPaths sp;
	sp.GetTempDir();
	wxFileName temp_file(sp.GetTempDir().c_str(), _T("temp_Heeks_clipboard_file.xml"));

	wxGetApp().SaveXMLFile(m_list, temp_file.GetFullPath().c_str(), true);

#if wxUSE_UNICODE
#ifdef __WXMSW__
	wifstream ifs(temp_file.GetFullPath());
#else
	wifstream ifs(Ttc(temp_file.GetFullPath().c_str()));
#endif
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
}

unsigned int MarkedList::GetIndex(HeeksObj *object) {
	return m_name_index.insert(object);
}

void MarkedList::ReleaseIndex(unsigned int index) {
	return m_name_index.erase(index);
}
