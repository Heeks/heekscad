// HeeksCAD.h

#pragma once

#include "../interface/HeeksColor.h"
#include "../interface/ObjList.h"
#include "glfont.h"

class Tool;
class MagDragWindow;
class ViewRotating;
class ViewZooming;
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
class CoordinateSystem;
class HRuler;
class wxConfigBase;
class wxAuiManager;

#define MAX_RECENT_FILES 20

enum GraphicsTextMode
{
	GraphicsTextModeNone,
	GraphicsTextModeInputTitle,
	GraphicsTextModeWithHelp
};

class HeeksCADapp : public wxApp, public ObjList
{
private:
	std::set<Observer*> observers;
	MainHistory *history;
	std::map< int, std::map<int, HeeksObj*> > used_ids; // map of group type ( usually same as object type ) to "map of ID to object"
	std::map< int, int > next_id_map;
	std::map< std::string, HeeksObj*(*)(TiXmlElement* pElem) > xml_read_fn_map;

	void create_font();
	void render_screen_text2(const wxChar* str);

protected:
    wxLocale m_locale; // locale we'll be using
	bool m_locale_initialised;

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
	//gp_Trsf digitizing_matrix;
	CoordinateSystem *m_current_coordinate_system;
	CInputMode *input_mode_object;
	MagDragWindow *magnification;
	ViewRotating *viewrotating;
	ViewZooming *viewzooming;
	CSelectMode *m_select_mode;
	DigitizeMode *m_digitizing;
	GripperMode* gripper_mode;
	int grid_mode;
	Gripper *drag_gripper;
	gp_Pnt grip_from, grip_to;
	Gripper *cursor_gripper;
	CHeeksFrame *m_frame;
	MarkedList *m_marked_list;
	bool m_doing_rollback;
	wxString m_filepath;
	bool m_light_push_matrix;
	std::list<HeeksObj*> m_hidden_for_drag;
	bool m_show_grippers_on_drag;
	double m_geom_tol;
	std::list<wxDynamicLibrary*> m_loaded_libraries;
	std::list< void(*)() > m_on_glCommands_list;
	std::list< wxToolBarBase* > m_external_toolbars;
	std::list< void(*)() > m_AddToolBars_list;
	std::list<wxWindow*> m_hideable_windows;
	HRuler* m_ruler;
	bool m_show_ruler;
	bool m_show_datum_coords_system;
	std::list< wxString > m_recent_files;
	bool m_in_OpenFile;
	bool m_mark_newly_added_objects;
	wxString m_version_number;
	std::list< void(*)(wxSizeEvent&) > m_on_graphics_size_list;
	std::list< void(*)(wxMouseEvent&) > m_lbutton_up_callbacks;
	int m_transform_gl_list;
	gp_Trsf m_drag_matrix;
	bool m_extrude_removes_sketches;
	bool m_loft_removes_sketches;
	bool m_font_created;
	GLFONT *m_gl_font;
	unsigned int m_font_tex_number;
	GraphicsTextMode m_graphics_text_mode;
	bool m_print_scaled_to_page;
	wxPrintData *m_printData;
	wxPageSetupDialogData* m_pageSetupData;

	// HeeksObj's virtual functions
	void GetBox(CBox &box);
	void glCommands(bool select, bool marked, bool no_color);
	bool CanAdd(HeeksObj* object){return true;}
	int GetType()const{return DocumentType;}

	virtual bool OnInit();
	int OnExit();
	void CreateLights(void);
	void DestroyLights(void);
	void FindMarkedObject(const wxPoint &point, MarkedObject* marked_object);
	void SetInputMode(CInputMode *i);
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
	void StartHistory();
	void EndHistory(void);
	void ClearRollingForward(void);
	bool Add(HeeksObj* object, HeeksObj* prev_object);
	void Remove(HeeksObj* object);
	void Reset();
	HeeksObj* ReadXMLElement(TiXmlElement* pElem);
	void InitializeXMLFunctions();
	void OpenXMLFile(const wxChar *filepath, bool undoably = false, HeeksObj* paste_into = NULL);
	void ReadSVGElement(TiXmlElement* pElem, bool undoably = false);
	void OpenSVGFile(const wxChar *filepath, bool undoably = false);
	void OpenSTLFile(const wxChar *filepath, bool undoably = false);
	void OpenDXFFile(const wxChar *filepath, bool undoably = false);
	bool OpenImageFile(const wxChar *filepath, bool undoably = false);
	bool OpenFile(const wxChar *filepath, bool import_not_open = false, HeeksObj* paste_into = NULL);
	void SaveDXFFile(const wxChar *filepath);
	void SaveSTLFile(const std::list<HeeksObj*>& objects, const wxChar *filepath);
	void SaveXMLFile(const std::list<HeeksObj*>& objects, const wxChar *filepath, bool for_clipboard = false);
	void SaveXMLFile(const wxChar *filepath){SaveXMLFile(m_objects, filepath);}
	bool SaveFile(const wxChar *filepath, bool use_dialog = false, bool update_recent_file_list = true, bool set_app_caption = true);
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
	gp_Trsf GetDrawMatrix(bool get_the_appropriate_orthogonal);
	void GetOptions(std::list<Property *> *list);
	void DeleteMarkedItems();
	void AddUndoably(HeeksObj *object, HeeksObj* owner, HeeksObj* prev_object);
	void AddUndoably(const std::list<HeeksObj*>& list, HeeksObj* owner);
	void glColorEnsuringContrast(const HeeksColor &c);
	void RegisterObserver(Observer* observer);
	void RemoveObserver(Observer* observer);
	void ObserversOnChange(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified);
	void ObserversMarkedListChanged(bool all_marked, bool none_marked, const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed);
	const wxChar* GetKnownFilesWildCardString(bool open = true)const;
	const wxChar* GetKnownFilesCommaSeparatedList(bool open = true)const;
	void GetTools(MarkedObject* marked_object, std::list<Tool*>& t_list, const wxPoint& point, bool from_graphics_canvas, bool control_pressed);
	wxString GetExeFolder()const;
	void get_2d_arc_segments(double xs, double ys, double xe, double ye, double xc, double yc, bool dir, bool want_start, double pixels_per_mm, void(*callbackfunc)(const double* xy));
	void PassMouseWheelToGraphics(wxMouseEvent& event);
	int PickObjects(const wxChar* str, long marking_filter = -1, bool just_one = false);
	bool PickPosition(const wxChar* str, double* pos, void(*callback)(const double*) = NULL);
	void glSphere(double radius, const double* pos = NULL);
	void OnNewOrOpen(bool open);
	void RegisterHideableWindow(wxWindow* w);
	void RemoveHideableWindow(wxWindow* w);
	void RegisterReadXMLfunction(const char* type_name, HeeksObj*(*read_xml_function)(TiXmlElement* pElem));
	void GetRecentFilesProfileString();
	void WriteRecentFilesProfileString(wxConfigBase &config);
	void InsertRecentFileItem(const wxChar* filepath);
	bool CheckForModifiedDoc(); // returns true, if OK to continue with file open etc.
	void SetFrameTitle();
	HeeksObj* GetIDObject(int type, int id);
	void SetObjectID(HeeksObj* object, int id);
	int GetNextID(int type);
	void RemoveID(HeeksObj* object); // only call this from ObjList::Remove()
	void ResetIDs();
	void WriteIDToXML(HeeksObj* object, TiXmlElement *element);
	void ReadIDFromXML(HeeksObj* object, TiXmlElement *element);
	bool InputDouble(const wxChar* prompt, const wxChar* value_name, double &value);
	void RegisterOnGLCommands( void(*callbackfunc)() );
	void RemoveOnGLCommands( void(*callbackfunc)() );
	void RegisterOnGraphicsSize( void(*callbackfunc)(wxSizeEvent&) );
	void RemoveOnGraphicsSize( void(*callbackfunc)(wxSizeEvent&) );
	void RegisterOnMouseFn( void(*callbackfunc)(wxMouseEvent&) );
	void RemoveOnMouseFn( void(*callbackfunc)(wxMouseEvent&) );
	void CreateTransformGLList(const std::list<HeeksObj*>& list, bool show_grippers_on_drag);
	void DestroyTransformGLList();
	bool IsPasteReady();
	void Paste(HeeksObj* paste_into);
	bool CheckForNOrMore(const std::list<HeeksObj*> &list, int min_num, int type, const wxString& msg, const wxString& caption);
	void render_text(const wxChar* str);
	bool get_text_size(const wxChar* str, float* width, float* height);
	void render_screen_text(const wxChar* str1, const wxChar* str2);
	void OnInputModeTitleChanged();
	void OnInputModeHelpTextChanged();
	void PlotSetColor(const HeeksColor &c);
	void PlotLine(const double* s, const double* e);
	void PlotArc(const double* s, const double* e, const double* c);
	void InitialiseLocale();
};

DECLARE_APP(HeeksCADapp)

