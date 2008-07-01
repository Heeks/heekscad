// HeeksCAD.h

#pragma once

#include "../interface/HeeksObj.h"
#include "../interface/HeeksColor.h"
#include "../interface/InputMode.h"
#include "ObjList.h"

class CLeftCanvas;
class CGraphicsCanvas;
class CPropertiesCanvas;
class COptionsCanvas;
class Tool;
class MagDragWindow;
class CInputMode;
class CSelectMode;
class DigitizeMode;
class Window;
class MarkedList;
class GripperMode;
class HeeksObj;
class MarkedObject;
class Gripper;
class CViewPoint;
class MainHistory;
class Observer;
class CHeeksFrame;
class wxDynamicLibrary;

class HeeksCADapp : public wxApp, public ObjList
{
private:
	std::set<Observer*> observers;
	MainHistory *history;

public:
	HeeksCADapp();
	~HeeksCADapp();

	wxPoint cur_mouse_pos;
	HeeksColor current_color;
	HeeksColor background_color;
	int m_rotate_mode;
	bool m_antialiasing;
	bool digitize_end;
	bool digitize_inters;
	bool digitize_centre;
	bool digitize_midpoint;
	bool digitize_nearest;
	bool digitize_coords;
	bool digitize_screen;
	bool draw_to_grid;
	double digitizing_grid;
	bool mouse_wheel_forward_away; // true for forwards/backwards = zoom out / zoom in, false for reverse
	gp_Trsf digitizing_matrix;
	CInputMode *input_mode_object;
	MagDragWindow *magnification;
	CSelectMode *m_select_mode;
	DigitizeMode *m_digitizing;
	GripperMode* gripper_mode;
	int grid_mode;
	Gripper *drag_gripper;
	gp_Pnt grip_from, grip_to;
	Gripper *cursor_gripper;
	CHeeksFrame *m_frame;
	wxConfig* m_config;
	MarkedList *m_marked_list;
	bool m_doing_rollback;
	std::string m_filepath;
	bool m_light_push_matrix;
	int m_hide_marked_list_stack;
	double m_geom_tol;
	std::list<wxDynamicLibrary*> m_loaded_libraries;

	// HeeksObj's virtual functions
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	bool CanAdd(HeeksObj* object){return true;}

	virtual bool OnInit();
	int OnExit();
	void CreateLights(void);
	void DestroyLights(void);
	void FindMarkedObject(const wxPoint &point, MarkedObject* marked_object);
	void clear_marked_list(void);
	void SetInputMode(CInputMode *i);
	void GetTools(std::list<Tool*>* f_list, const wxPoint* point, MarkedObject* marked_object);
	void Repaint(bool soon = false);
	void RecalculateGLLists();
	void SetLikeNewFile(void);
	bool IsModified(void);
	void SetAsModified();
	void ClearHistory(void);
	void glCommandsAll(bool select, const CViewPoint &view_point);
	double GetPixelScale(void);
	void DoDropDownMenu(wxWindow *wnd, const wxPoint &point, MarkedObject* marked_object, bool dont_use_point_for_functions, bool from_graphics_canvas, bool control_pressed);
	void on_menu_event(wxCommandEvent& event);
	void DoToolUndoably(Tool *);
	bool RollBack(void);
	bool RollForward(void);
	void StartHistory(const char* str);
	void EndHistory(void);
	void ClearRollingForward(void);
	bool Add(HeeksObj* object, HeeksObj* prev_object);
	void Reset();
	bool OpenFile(const char *filepath);
	bool SaveFile(const char *filepath);
	void DeleteUndoably(HeeksObj* object);
	void DeleteUndoably(const std::list<HeeksObj*>& list);
	void TransformUndoably(HeeksObj *object, double *m);
	void TransformUndoably(const std::list<HeeksObj*>& list, double* m);
	void WasModified(HeeksObj *object);
	void WasAdded(HeeksObj *object);
	void WasRemoved(HeeksObj *object);
	void WereModified(const std::list<HeeksObj*>& list);
	void WereAdded(const std::list<HeeksObj*>& list);
	void WereRemoved(const std::list<HeeksObj*>& list);
	void SetDrawMatrix(const gp_Trsf& mat);
	gp_Trsf GetDrawMatrix(bool get_the_appropriate_orthogonal);
	void GetProperties(std::list<Property *> *list);
	void DeleteMarkedItems();
	void AddUndoably(HeeksObj *object, HeeksObj* parent, HeeksObj* prev_object);
	void AddUndoably(const std::list<HeeksObj*>& list, HeeksObj* parent);
	void glColorEnsuringContrast(const HeeksColor &c);
	void HideMarkedList(){m_hide_marked_list_stack++;}
	void UnHideMarkedList(){if(m_hide_marked_list_stack>0)m_hide_marked_list_stack--;}
	void RegisterObserver(Observer* observer);
	void RemoveObserver(Observer* observer);
	void ObserversOnChange(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified);
	void ObserversMarkedListChanged(bool all_marked, bool none_marked, const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed);
	const char* GetKnownFilesWildCardString()const;
	const char* GetKnownFilesCommaSeparatedList()const;
	void AddMenusToToolList(MarkedObject* marked_object, std::list<Tool*>& t_list, const wxPoint& point, bool from_graphics_canvas, bool control_pressed);
	wxString GetExeFolder()const;
	void get_2d_arc_segments(double xs, double ys, double xe, double ye, double xc, double yc, bool dir, bool want_start, double pixels_per_mm, void(*callbackfunc)(const double* xy));
};

DECLARE_APP(HeeksCADapp)

