// MarkedList.h

#pragma once

#include "ViewPoint.h"


class Gripper;
class PointOrWindow;

class MarkedList{
private:
	std::list<HeeksObj*> m_list;
	std::set<HeeksObj*> m_set;
	std::set<HeeksObj*> m_ignore_set;

	void delete_move_grips(bool check_app_grippers = false);
	void create_move_grips();
	void update_move_grips();
	void render_move_grips(bool select, bool no_color);

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
	void glCommands();
	void Add(std::list<HeeksObj *> &obj_list);
	void Add(HeeksObj *object);
	void Remove(const std::list<HeeksObj *> &obj_list);
	void Remove(HeeksObj *object);
	bool ObjectMarked(HeeksObj *object);
	void Clear(void);
	int size(void){return m_list.size();}
	const std::list<HeeksObj *> &list(void){return m_list;}
	void FindMarkedObject(const wxPoint &point, MarkedObject* marked_object);
	void ObjectsInWindow( wxRect box, MarkedObject* marked_object, bool single_picking = true);
	void GrippersGLCommands(bool select, bool no_color);
	void OnChanged(bool all_marked, bool none_marked, const std::list<HeeksObj *>* added, const std::list<HeeksObj *>* removed);
	void set_ignore_onoff(HeeksObj* object, bool b);
	bool get_ignore(HeeksObj* object);
	void GetProperties(std::list<Property *> *list);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	void CutSelectedItems();
	void CopySelectedItems();
	void Reset();
};
