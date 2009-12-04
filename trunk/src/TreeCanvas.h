// TreeCanvas.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

// this will be renamed from TreeCanvas to TreeCanvas, when it is finished.

// this tree view is drawn with OpenGL.

#pragma once

#include "../interface/Observer.h"

class CTreeCanvas: public wxGLCanvas, Observer
{
private:
	bool m_LButton;
	wxPoint m_CurrentPoint;
	bool m_frozen;
	bool m_refresh_wanted_on_thaw;
    int width, height, textureWidth, textureHeight;
	bool m_texture_built;
	std::set<HeeksObj*> m_expanded;
	int scroll_y_pos;

	int m_xpos, m_ypos;
	enum TreeButtonType{ ButtonTypePlus, ButtonTypeMinus, ButtonTypeLabel };
	class CTreeButton{public:	TreeButtonType type; wxRect rect; HeeksObj* obj;};
	std::list<CTreeButton> m_tree_buttons;

	void BuildTexture();
	bool IsExpanded(HeeksObj* object);
	void SetExpanded(HeeksObj* object, bool bExpanded);
	void RenderIcon(int texture_number, int x, int y);
	int RenderChar(char c);
	int RenderText(const char* str);
	int GetCharLength(char c);
	int GetTextLength(const char* str);
	void RenderBranchIcon(HeeksObj* object, HeeksObj* next_object, bool expanded, int level);
	void RenderBranchIcons(HeeksObj* object, HeeksObj* next_object, bool expanded, int level);
	void RenderObject(HeeksObj* object, HeeksObj* next_object, int level);
	void AddPlusOrMinusButton(HeeksObj* object, bool plus);
	void AddLabelButton(HeeksObj* object, int label_start_x, int label_end_x);
	void OnLabelLeftDown(HeeksObj* object, wxMouseEvent& event);
	void OnLabelRightDown(HeeksObj* object, wxMouseEvent& event);

public:
	CTreeCanvas(wxWindow* parent, wxGLCanvas* shared, int *attribList = (int*) NULL);
    virtual ~CTreeCanvas(){};

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
    void OnMouse( wxMouseEvent& event );
	void OnMenuEvent(wxCommandEvent& WXUNUSED(event));
	void OnCharEvent(wxKeyEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void Render(int width, int height); // OpenGL commands for all the objects
	bool TextureBuilt(){return m_texture_built;}
	const CTreeButton* HitTest( const wxPoint& pt );

	// Observer's virtual functions
	void OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified);
	void WhenMarkedListChanges(bool selection_cleared, const std::list<HeeksObj*>* added_list, const std::list<HeeksObj*>* removed_list);
	void Clear();
	void Freeze();
	void Thaw();
	void Refresh();
	void RefreshSoon(); // for dragging the view, for example

private:
    DECLARE_EVENT_TABLE()
};
