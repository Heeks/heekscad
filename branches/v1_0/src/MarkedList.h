// MarkedList.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "ViewPoint.h"
#include "Index.h"


class Gripper;
class PointOrWindow;

class MarkedList{
private:
	std::list<HeeksObj*> m_list;
	std::set<HeeksObj*> m_set;
	std::set<HeeksObj*> m_ignore_set;
	Index<unsigned, HeeksObj*> m_name_index;

	void delete_move_grips(bool check_app_grippers = false);
	void create_move_grips();
	void update_move_grips();
	void render_move_grips(bool select, bool no_color);
	void OnChangedAdded(HeeksObj* object);
	void OnChangedRemoved(HeeksObj* object);

public:
	PointOrWindow *point_or_window;
	bool gripping;
	std::list<Gripper*> move_grips;
	bool gripper_marked_list_changed;
	bool ignore_coords_only;
	long m_filter;

	MarkedList();
	virtual ~MarkedList(void);

	void create_grippers();
	void Add(std::list<HeeksObj *> &obj_list, bool call_OnChanged);
	void Add(HeeksObj *object, bool call_OnChanged);
	void Remove(const std::list<HeeksObj *> &obj_list, bool call_OnChanged);
	void Remove(HeeksObj *object, bool call_OnChanged);
	bool ObjectMarked(HeeksObj *object);
	void Clear(bool call_OnChanged);
	int size(void){return m_list.size();}
	std::list<HeeksObj *> &list(void){return m_list;}
	void FindMarkedObject(const wxPoint &point, MarkedObject* marked_object);
	void ObjectsInWindow( wxRect box, MarkedObject* marked_object, bool single_picking = true);
	void GrippersGLCommands(bool select, bool no_color);
	void OnChanged(bool selection_cleared, const std::list<HeeksObj *>* added, const std::list<HeeksObj *>* removed);
	void set_ignore_onoff(HeeksObj* object, bool b);
	bool get_ignore(HeeksObj* object);
	void GetProperties(std::list<Property *> *list);
	void GetTools(MarkedObject* clicked_object, std::list<Tool*>& t_list, const wxPoint* p, bool copy_and_paste_tools);
	void CutSelectedItems();
	void CopySelectedItems();
	void Reset();
	unsigned int GetIndex(HeeksObj *object);
	void ReleaseIndex(unsigned int index);
};
