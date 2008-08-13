// MarkedList.cpp
#include "stdafx.h"

#include "MarkedList.h"
#include "../interface/HeeksObj.h"
#include "../interface/MarkedObject.h"
#include "DigitizeMode.h"
#include "SelectMode.h"
#include "PointOrWindow.h"
#include "GripperSelTransform.h"
#include "GripperMode.h"
#include "GraphicsCanvas.h"
#include "HeeksFrame.h"

MarkedList::MarkedList(){
	gripping = false;
	point_or_window = new PointOrWindow(true);
	gripper_marked_list_changed = false;
	ignore_coords_only = false;
}

MarkedList::~MarkedList(void){
	delete point_or_window;
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
	delete_move_grips();
	double pos[3];
	int number_of_grips_made = 0;
	std::list<HeeksObj*>::iterator Iter ;
	for(Iter = m_list.begin(); Iter != m_list.end() && number_of_grips_made<100; Iter++){
		HeeksObj* object = *Iter;
		if(object->GetType() == GripperType)continue;
		std::list<double> vl;
		std::list<double>::iterator It;
		object->GetGripperPositions(&vl, false);
		for(It = vl.begin(); It != vl.end() && number_of_grips_made<100; It++){
			It++;
			pos[0] = *It;
			It++;
			pos[1] = *It;
			It++;
			pos[2] = *It;
			move_grips.push_back(new GripperSelTransform(this, make_point(pos)));
			number_of_grips_made++;
		}
	}
}

void MarkedList::update_move_grips(){
	if(gripping)return;
	double pos[3];
	std::list<HeeksObj*>::iterator Iter ;
	std::list<Gripper*>::iterator Iter2;
	Iter2 = move_grips.begin();
	for(Iter = m_list.begin(); Iter != m_list.end(); Iter++){
		if(Iter2 == move_grips.end())break;
		HeeksObj* object = *Iter;
		if(object->GetType() == GripperType)continue;
		std::list<double> vl;
		std::list<double>::iterator It;
		object->GetGripperPositions(&vl, false);
		for(It = vl.begin(); It != vl.end(); It++){
			It++;
			pos[0] = *It;
			It++;
			pos[1] = *It;
			It++;
			pos[2] = *It;
			Gripper* gripper = *Iter2;
			gripper->position = make_point(pos);
			Iter2++;
			if(Iter2 == move_grips.end())break;
		}
	}
}

void MarkedList::render_move_grips(bool select, bool no_color){
	std::list<Gripper*>::iterator It;
	for(It = move_grips.begin(); It != move_grips.end(); It++){
		if(select)glPushName((unsigned int)(*It));
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
	int i, j;
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
			wxGetApp().m_frame->m_graphics->m_view_point.SetViewport();
			wxGetApp().m_frame->m_graphics->m_view_point.SetPickProjection(window);
			wxGetApp().m_frame->m_graphics->m_view_point.SetModelview();
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
		for(i=0; i<num_hits; i++)
		{
			unsigned int names = data[pos];
			if(names == 0)break;
			pos++;
			unsigned int min_depth = data[pos];
			pos+=2;
			MarkedObject* current_found_object = marked_object;
			bool ignore_coords_only_found = false;
			for(j=0; j<(int)names; j++, pos++){
				if(!ignore_coords_only_found && current_found_object != NULL){
					HeeksObj *object = (HeeksObj *)(data[pos]);
					if(ignore_coords_only && wxGetApp().m_digitizing->OnlyCoords(object)){
						ignore_coords_only_found = true;
					}
					else{
						int window_size = window.width;
						current_found_object = current_found_object->Add(object, min_depth, window_size);
					}
				}
			}
		}
		window_mode++;
		if(!single_picking)break;
		if(window_mode > 2)break;
	}
	
	free(data);
}

void MarkedList::glCommands(){
	std::list<HeeksObj*>::iterator Iter ;
	int object_count = 0;
	for(Iter = m_list.begin(); Iter != m_list.end(); Iter++, object_count++){
		HeeksObj *object = (*Iter);
		object->glCommands(true, false, false);
	}
}

void MarkedList::Add(std::list<HeeksObj *> &list){
	std::list<HeeksObj *>::iterator It;
	for(It = list.begin(); It != list.end(); It++){
		HeeksObj *object = *It;
		m_list.push_back(object);
		m_set.insert(object);
	}
	OnChanged(false, false, &list, NULL);
}

void MarkedList::Remove(HeeksObj *object){
	std::list<HeeksObj *> list;
	list.push_back(object);
	Remove(list);
}

void MarkedList::Add(HeeksObj *object){
	std::list<HeeksObj *> list;
	list.push_back(object);
	Add(list);
}

void MarkedList::Remove(const std::list<HeeksObj *> &obj_list){
	std::list<HeeksObj *>::const_iterator It;
	for(It = obj_list.begin(); It != obj_list.end(); It++){
		HeeksObj *object = *It;
		if(m_set.find(object) != m_set.end()){
			m_list.remove(object);
		}
		m_set.erase(object);
	}
	OnChanged(false, false, NULL, &obj_list);
}

void MarkedList::Clear(void){ 
	m_list.clear();
	m_set.clear();
	OnChanged(false, true, NULL, NULL);
}

void MarkedList::FindMarkedObject(const wxPoint &point, MarkedObject* marked_object){
	if(marked_object == NULL)return;
	point_or_window->SetWithPoint(point);
	ObjectsInWindow(point_or_window->box_chosen, marked_object);
}

bool MarkedList::ObjectMarked(HeeksObj *object){
	return m_set.find(object) != m_set.end();
}

void MarkedList::OnChanged(bool all_marked, bool none_marked, const std::list<HeeksObj *>* added, const std::list<HeeksObj *>* removed){
	gripper_marked_list_changed = true;
	wxGetApp().ObserversMarkedListChanged(all_marked, none_marked, added, removed);
}

void MarkedList::set_ignore_onoff(HeeksObj* object, bool b){
	if(b)m_ignore_set.insert(object);
	else m_ignore_set.erase(object);
}

bool MarkedList::get_ignore(HeeksObj* object){
	if(m_ignore_set.find(object) != m_ignore_set.end())return true;
	return false;
}
