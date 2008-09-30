// HeeksCAD.h

#pragma once

#include "../interface/HeeksColor.h"
#include "../interface/ObjList.h"

class Tool;
class MagDragWindow;
class CInputMode;
class CSelectMode;
class DigitizeMode;
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

#define MAX_RECENT_FILES 20

class HeeksCADapp : public wxApp, public ObjList
{
private:
	std::set<Observer*> observers;
	MainHistory *history;
	std::map< int, std::map<int, HeeksObj*> > used_ids; // map of group type ( usually same as object type ) to "map of ID to object"
	std::map< int, int > next_id_map;

public:
	HeeksCADapp();
	~HeeksCADapp();

	wxPoint cur_mouse_pos;
	HeeksColor current_color;
	HeeksColor construction_color;
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
	bool digitize_tangent;
	double digitizing_radius; // for ambiguous arcs and circles
	bool draw_to_grid;
	double digitizing_grid;
	bool mouse_wheel_forward_away; // true for forwards/backwards = zoom out / zoom in, false for reverse
	bool ctrl_does_rotate; // true - rotate on Ctrl, pan when not Ctrl      false - rotate when not Ctrl, pan when Ctrl
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
	std::list<wxWindow*> m_hideable_windows;
	bool m_show_ruler;
	std::list< wxString > m_recent_files;
	bool m_disable_SetObjectID_on_Add;

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
	void GetTools(std::list<Tool*>* t_list, const wxPoint* point, MarkedObject* marked_object);
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
	HeeksObj* ReadXMLElement(TiXmlElement* pElem);
	void InitializeXMLFunctions();
	void OpenXMLFile(const char *filepath);
	void ReadSVGElement(TiXmlElement* pElem);
	void OpenSVGFile(const char *filepath);
	void OpenSTLFile(const char *filepath);
	void OpenDXFFile(const char *filepath);
	bool OpenImageFile(const char *filepath);
	bool OpenFile(const char *filepath, bool update_recent_file_list = true, bool set_app_caption = true);
	void SaveDXFFile(const char *filepath);
	void SaveSTLFile(const char *filepath);
	void SaveXMLFile(const char *filepath);
	bool SaveFile(const char *filepath, bool use_dialog = false, bool update_recent_file_list = true, bool set_app_caption = true);
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
	void GetOptions(std::list<Property *> *list);
	void DeleteMarkedItems();
	void AddUndoably(HeeksObj *object, HeeksObj* owner, HeeksObj* prev_object);
	void AddUndoably(const std::list<HeeksObj*>& list, HeeksObj* owner);
	void glColorEnsuringContrast(const HeeksColor &c);
	void HideMarkedList(){m_hide_marked_list_stack++;}
	void UnHideMarkedList(){if(m_hide_marked_list_stack>0)m_hide_marked_list_stack--;}
	void RegisterObserver(Observer* observer);
	void RemoveObserver(Observer* observer);
	void ObserversOnChange(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified);
	void ObserversMarkedListChanged(bool all_marked, bool none_marked, const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed);
	const char* GetKnownFilesWildCardString(bool open = true)const;
	const char* GetKnownFilesCommaSeparatedList(bool open = true)const;
	void AddMenusToToolList(MarkedObject* marked_object, std::list<Tool*>& t_list, const wxPoint& point, bool from_graphics_canvas, bool control_pressed);
	wxString GetExeFolder()const;
	void get_2d_arc_segments(double xs, double ys, double xe, double ye, double xc, double yc, bool dir, bool want_start, double pixels_per_mm, void(*callbackfunc)(const double* xy));
	void PassMouseWheelToGraphics(wxMouseEvent& event);
	int PickObjects(const char* str);
	bool PickPosition(const char* str, double* pos);
	void glSphere(double radius, const double* pos = NULL);
	void OnNewOrOpen(bool open);
	void RegisterHideableWindow(wxWindow* w);
	void RegisterReadXMLfunction(const char* type_name, HeeksObj*(*read_xml_function)(TiXmlElement* pElem));
	void GetRecentFilesProfileString();
	void WriteRecentFilesProfileString();
	void InsertRecentFileItem(const char* filepath);
	bool CheckForModifiedDoc(); // returns true, if OK to continue with file open etc.
	void SetFrameTitle();
	HeeksObj* GetIDObject(int type, int id);
	void SetObjectID(HeeksObj* object, int id);
	int GetNextID(int type);
	void RemoveID(HeeksObj* object); // only call this from ObjList::Remove()
	void ResetIDs();
	void WriteIDToXML(HeeksObj* object, TiXmlElement *element);
	void ReadIDFromXML(HeeksObj* object, TiXmlElement *element);
	bool InputDouble(const char* prompt, const char* value_name, double &value);
};

DECLARE_APP(HeeksCADapp)

