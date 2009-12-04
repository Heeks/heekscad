// TreeCanvas.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "TreeCanvas.h"
#include "HeeksFrame.h"
#include "wxImageLoader.h"
#include "../interface/MarkedObject.h"

BEGIN_EVENT_TABLE(CTreeCanvas, wxGLCanvas)
    EVT_SIZE(CTreeCanvas::OnSize)
	EVT_ERASE_BACKGROUND(CTreeCanvas::OnEraseBackground)
    EVT_PAINT(CTreeCanvas::OnPaint)
    EVT_MOUSE_EVENTS(CTreeCanvas::OnMouse)
    EVT_MENU_RANGE(ID_FIRST_POP_UP_MENU_TOOL, ID_FIRST_POP_UP_MENU_TOOL + 1000, CTreeCanvas::OnMenuEvent)
	EVT_KEY_DOWN(CTreeCanvas::OnKeyDown)
	EVT_KEY_UP(CTreeCanvas::OnKeyUp)
	EVT_CHAR(CTreeCanvas::OnCharEvent)
END_EVENT_TABLE()

CTreeCanvas::CTreeCanvas(wxWindow* parent, wxGLCanvas* shared, int *attribList)
        : wxGLCanvas(parent, shared, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, _T("some text"), attribList),m_frozen(false), m_refresh_wanted_on_thaw(false),
		width(0), height(0), textureWidth(0), textureHeight(0), m_texture_built(false), m_xpos(0), m_ypos(0), scroll_y_pos(0)
{
	wxGetApp().RegisterObserver(this);
}

void CTreeCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    /* must always be here */
    wxPaintDC dc(this);

#ifndef __WXMOTIF__
    if (!GetContext()) return;
#endif

    SetCurrent();

	glDrawBuffer(GL_BACK);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// clear the back buffer
	HeeksColor(255, 255, 255).glClearColor(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int w, h;
    GetClientSize(&w, &h);
	glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(- 0.5, w - 0.5, -0.5, h - 0.5, 0,10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// render everything
	Render(w, h);

    SwapBuffers();
}

void CTreeCanvas::OnSize(wxSizeEvent& event)
{
    // this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize(event);
	Refresh();
}

void CTreeCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
	// Do nothing, to avoid flashing on MSW
}

const CTreeCanvas::CTreeButton* CTreeCanvas::HitTest( const wxPoint& pt )
{
	for(std::list<CTreeButton>::iterator It = m_tree_buttons.begin(); It != m_tree_buttons.end(); It++)
	{
		CTreeButton& b = *It;
		if(b.rect.Inside(pt))
		{
			return &b;
		}
	}
	return NULL;
}

static HeeksObj* clicked_object = NULL;

void CTreeCanvas::OnMouse( wxMouseEvent& event )
{
	if(wxGetApp().m_property_grid_validation)return;

	if(event.Entering()){
	    SetCurrent();
		SetFocus(); // so middle wheel works
	}

	if(event.LeftDown())
	{
		const CTreeButton* button = HitTest(event.GetPosition());

		if(button)
		{
			switch(button->type)
			{
			case ButtonTypePlus:
			case ButtonTypeMinus:
				SetExpanded(button->obj, button->type == 0);
				this->Refresh();
				break;

			case ButtonTypeLabel:
			default:
				OnLabelLeftDown(button->obj, event);
				clicked_object = button->obj;
				break;
			}
		}
		else
		{
			wxGetApp().m_marked_list->Clear(true);
		}
	}

	if(event.RightDown())
	{
		const CTreeButton* button = HitTest(event.GetPosition());
		clicked_object = NULL;
		if(button && button->type == ButtonTypeLabel)
		{
			clicked_object = button->obj;
			OnLabelRightDown(button->obj, event);
		}
	}

	if(event.RightUp())
	{
		// do a context menu
		MarkedObjectOneOfEach marked_object(0, clicked_object, 1);
		wxGetApp().DoDropDownMenu(this, event.GetPosition(), &marked_object, true, false, false);
	}

	if(event.GetWheelRotation() != 0)
	{
		double wheel_value = (double)(event.GetWheelRotation());
		scroll_y_pos -= (wheel_value / 100);
		if(scroll_y_pos < 0)scroll_y_pos = 0;
		Refresh();
	}

	event.Skip();
}

void CTreeCanvas::OnKeyDown(wxKeyEvent& event)
{
	if(event.GetKeyCode() == WXK_ESCAPE && wxGetApp().EndSketchMode())
	{}
	else wxGetApp().input_mode_object->OnKeyDown(event);

	event.Skip();
}

void CTreeCanvas::OnCharEvent(wxKeyEvent& event)
{
	const int ControlA = 1;
	const int ControlC = 3;
	const int ControlV = 22;

	// printf("Key event is '%d'\n", event.GetKeyCode());
	switch (event.GetKeyCode())
	{
		case ControlA:
			{
				// Select all
				std::list<HeeksObj*> obj_list;
				for(HeeksObj* object = wxGetApp().GetFirstChild(); object != NULL; object = wxGetApp().GetNextChild())
				{
					if(object->GetType() != GripperType)
					{
						obj_list.push_back(object);
					} // End if - then
				} // End for
				wxGetApp().m_marked_list->Add(obj_list, true);
				wxGetApp().Repaint();
				event.Skip();
				break;
			} // End ControlA scope

		case ControlC:
			// Copy
			wxGetApp().m_marked_list->CopySelectedItems();
			wxGetApp().Repaint();
			event.Skip();
			break;

		case ControlV:
			// Paste
			wxGetApp().Paste(NULL);
			wxGetApp().Repaint();
			event.Skip();
			break;

		default:
			break;
	} // End switch
} // End OnCharEvent() method


void CTreeCanvas::OnKeyUp(wxKeyEvent& event)
{
	wxGetApp().input_mode_object->OnKeyUp(event);
	event.Skip();
}

void CTreeCanvas::OnMenuEvent(wxCommandEvent& event)
{
	wxGetApp().on_menu_event(event);
}

void CTreeCanvas::OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified)
{
	Refresh();
}

void CTreeCanvas::WhenMarkedListChanges(bool selection_cleared, const std::list<HeeksObj *>* added_list, const std::list<HeeksObj *>* removed_list)
{
	Refresh();
}

void CTreeCanvas::Clear()
{
	Refresh();
}

void CTreeCanvas::Freeze()
{
	m_frozen = true;
}

void CTreeCanvas::Thaw()
{
	m_frozen = false;
	if(m_refresh_wanted_on_thaw)
	{
		Refresh();
		m_refresh_wanted_on_thaw = false;
	}
}

void CTreeCanvas::Refresh()
{
	if(m_frozen)
	{
		m_refresh_wanted_on_thaw = true;
	}
	else
	{
		wxGLCanvas::Refresh(false);
	}
}

void CTreeCanvas::RefreshSoon()
{
	if(m_frozen)
	{
		m_refresh_wanted_on_thaw = true;
	}
	else if(wxGetApp().m_frame->IsShown())
	{
		wxGLCanvas::Refresh(false);
		Update();
	}
}

void CTreeCanvas::BuildTexture()
{
	wxString filepath = wxGetApp().GetResFolder() + _T("/icons/iconimage.png");
	unsigned int* t = loadImage(filepath.c_str(), &width, &height, &textureWidth, &textureHeight);
	if(t)wxGetApp().m_icon_texture_number= *t;

	// build external textures
	for(std::list< void(*)() >::iterator It = wxGetApp().m_on_build_texture_callbacks.begin(); It != wxGetApp().m_on_build_texture_callbacks.end(); It++)
	{
		void(*callbackfunc)() = *It;
		(*callbackfunc)();
	}

	m_texture_built = true;
}

bool CTreeCanvas::IsExpanded(HeeksObj* object)
{
	return m_expanded.find(object) != m_expanded.end();
}

void CTreeCanvas::SetExpanded(HeeksObj* object, bool bExpanded)
{
	if(bExpanded)
	{
		m_expanded.insert(object);
	}
	else
	{
		m_expanded.erase(object);
	}
}

static int render_width = 0, render_height = 0;
static bool render_marked = false;

void CTreeCanvas::RenderIcon(int texture_number, int x, int y)
{
	unsigned int icon_x = x * 16;
	unsigned int icon_y = y * 16;

	if(texture_number){
		int xpos16 = m_xpos * 16;
		int ypos16 = render_height - m_ypos * 18;

#ifdef WIN32
		float texture_left = ((float)(icon_x) + 0.5f)/256;
		float texture_right = ((float)(icon_x) + 16.5f)/256;
		float texture_top = ((float)(icon_y) - 0.5f)/256;
		float texture_bottom = ((float)(icon_y) + 15.5f)/256;
#else
		float texture_left = ((float)(icon_x))/256;
		float texture_right = ((float)(icon_x) + 16.0f)/256;
		float texture_top = ((float)(icon_y))/256;
		float texture_bottom = ((float)(icon_y) + 16.0f)/256;
#endif

		glColor4ub(255, 255, 255, 255);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture_number);
		glBegin(GL_QUADS);
		glTexCoord2f(texture_left, texture_top);
		glVertex2i(xpos16, ypos16);
		glTexCoord2f(texture_left, texture_bottom);
		glVertex2i(xpos16, ypos16 - 16);
		glTexCoord2f(texture_right, texture_bottom);
		glVertex2i(xpos16 + 16, ypos16 - 16);
		glTexCoord2f(texture_right, texture_top);
		glVertex2i(xpos16 + 16, ypos16);
		glEnd();

		glDisable(GL_TEXTURE_2D);
	}
}

void CTreeCanvas::AddPlusOrMinusButton(HeeksObj* object, bool plus)
{
	CTreeButton b;
	b.type = plus ? ButtonTypePlus : ButtonTypeMinus;
	b.rect.x = m_xpos * 16;
	b.rect.y = m_ypos * 18;
	b.rect.width = 16;
	b.rect.height = 18;
	b.obj = object;
	m_tree_buttons.push_back(b);
}

void CTreeCanvas::AddLabelButton(HeeksObj* object, int label_start_x, int label_end_x)
{
	CTreeButton b;
	b.type = ButtonTypeLabel;
	b.rect.x = label_start_x;
	b.rect.y = m_ypos * 18;
	b.rect.width = label_end_x - label_start_x;
	b.rect.height = 18;
	b.obj = object;
	m_tree_buttons.push_back(b);
}

void CTreeCanvas::OnLabelLeftDown(HeeksObj* object, wxMouseEvent& event)
{
	if(event.ShiftDown())
	{
		// mark a list of siblings
		HeeksObj* parent = object->Owner();
		std::set<HeeksObj*> sibling_set;
		std::list<HeeksObj*> sibling_list;
		for(HeeksObj* sibling = parent->GetFirstChild(); sibling; sibling = parent->GetNextChild())
		{
			sibling_set.insert(sibling);
			sibling_list.push_back(sibling);
		}
		// find most recently marked sibling
		std::list<HeeksObj*> &marked = wxGetApp().m_marked_list->list();
		HeeksObj* recently_marked_sibling = NULL;
		bool recent_first = false;
		for(std::list<HeeksObj*>::reverse_iterator It = marked.rbegin(); It != marked.rend(); It++)
		{
			if(*It == object)recent_first = true;
			if(sibling_set.find(*It) != sibling_set.end())
			{
				recently_marked_sibling = *It;
				break;
			}
		}

		if(recently_marked_sibling)
		{
			if(!event.ControlDown())
			{
				wxGetApp().m_marked_list->Clear(false);
			}

			bool marking = false;
			std::list<HeeksObj*> list_to_mark;
			bool finish_marking = false;
			for(std::list<HeeksObj*>::iterator It = sibling_list.begin(); !finish_marking && It != sibling_list.end(); It++)
			{
				HeeksObj* sibling = *It;
				if(sibling == object || sibling == recently_marked_sibling)
				{
					if(marking)finish_marking = true;
					else marking = true;
				}

				if(marking)
				{
					list_to_mark.push_back(sibling);
				}
			}

			wxGetApp().m_marked_list->Add(list_to_mark, true);
		}
		else
		{
			if(event.ControlDown())
			{
				if(wxGetApp().m_marked_list->ObjectMarked(object))
				{
					wxGetApp().m_marked_list->Remove(object, true);
				}
				else{
					wxGetApp().m_marked_list->Add(object, true);
				}
			}
			else
			{
				wxGetApp().m_marked_list->Clear(false);
				wxGetApp().m_marked_list->Add(object, true);
			}
		}
	}
	else
	{
		// shift not down
		if(event.ControlDown())
		{
			if(wxGetApp().m_marked_list->ObjectMarked(object))
			{
				wxGetApp().m_marked_list->Remove(object, true);
			}
			else{
				wxGetApp().m_marked_list->Add(object, true);
			}
		}
		else
		{
			wxGetApp().m_marked_list->Clear(false);
			wxGetApp().m_marked_list->Add(object, true);
		}
	}
}

void CTreeCanvas::OnLabelRightDown(HeeksObj* object, wxMouseEvent& event)
{
	if(!wxGetApp().m_marked_list->ObjectMarked(object))
	{
		wxGetApp().m_marked_list->Clear(false);
		wxGetApp().m_marked_list->Add(object, true);
	}
}

static int text_start_posx[127] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0  ,6  ,11 ,19 ,34 ,45 ,62 ,75 ,80 ,86 ,93 ,103,117,123,131,135,142,154,164,175,186,197,208,219,230,241,253,0  ,7  ,22 ,37 ,51 ,61 ,78 ,92 ,103,117,131,142,151,166,180,183,192,204,214,230,243,0  ,10 ,25 ,37 ,47 ,61 ,73 ,86 ,105,117,130,143,149,157,164,177,188,196,207,217,226,236,247,2  ,14 ,25 ,28 ,35 ,45 ,50 ,67 ,77 ,89 ,99 ,111,118,126,134,144,156,171,182,193,203,214,220,231};
static int text_start_posy[127] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,184,202,202,202,202,202,202,202,202,202,202,202,202,202,202,202,202,202,202,202,202,202,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,238,238,238,238,238,238,238,238,238,238,238,238,238,238,238,238,238,238,238,238,238,238,238,238};
static int text_start_posd[127] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6  ,5  ,8  ,15 ,11 ,17 ,13 ,5  ,6  ,7  ,10 ,14 ,16 ,8  ,4  ,7  ,12 ,10 ,11 ,11 ,11 ,11 ,11 ,11 ,11 ,12 ,5  ,7  ,15 ,15 ,14 ,10 ,17 ,14 ,11 ,14 ,14 ,11 ,9  ,15 ,14 ,3  ,9  ,12 ,10 ,16 ,13 ,15 ,10 ,15 ,12 ,10 ,14 ,12 ,13 ,19 ,12 ,13 ,13 ,6  ,8  ,7  ,13 ,10 ,9  ,11 ,10 ,9  ,10 ,11 ,7  ,12 ,11 ,3  ,7  ,10 ,5  ,17 ,10 ,12 ,10 ,12 ,7  ,8  ,8  ,10 ,12 ,15 ,11 ,11 ,10 ,11 ,6  ,11 ,15 };
static int text_pos = 0;

int CTreeCanvas::RenderChar(char c)
{
	// renders the character at the current xpos * 16 + text_pos, ypos
	if(c<32 || c>126)return 0;

	int posx = text_start_posx[c];
	int posy = text_start_posy[c];
	int shift = text_start_posd[c];

#ifdef WIN32
	float texture_left = ((float)(posx) + 0.5f)/256;
	float texture_right = ((float)(posx) + 0.5f + shift)/256;
	float texture_top = ((float)(posy) - 0.5f)/256;
	float texture_bottom = ((float)(posy) + 17.5f)/256;
#else
	float texture_left = ((float)(posx))/256;
	float texture_right = ((float)(posx) + shift)/256;
	float texture_top = ((float)(posy))/256;
	float texture_bottom = ((float)(posy) + 18.0f)/256;
#endif

	if(wxGetApp().m_icon_texture_number){
		int xpos16 = m_xpos * 16 + text_pos;
		int ypos16 = render_height - m_ypos * 18;
		if(render_marked)glColor4ub(128, 128, 255, 255);
		else glColor4ub(255, 255, 255, 255);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, wxGetApp().m_icon_texture_number);
		glBegin(GL_QUADS);
		glTexCoord2f(texture_left, texture_top);
		glVertex2i(xpos16, ypos16);
		glTexCoord2f(texture_left, texture_bottom);
		glVertex2i(xpos16, ypos16 - 18);
		glTexCoord2f(texture_right, texture_bottom);
		glVertex2i(xpos16 + shift, ypos16 - 18);
		glTexCoord2f(texture_right, texture_top);
		glVertex2i(xpos16 + shift, ypos16);
		glEnd();

		glDisable(GL_TEXTURE_2D);
	}

	return shift;
}

int CTreeCanvas::RenderText(const char* str)
{
	int l = strlen(str);
	for(int i = 0; i<l; i++)
	{
		text_pos += RenderChar(str[i]);
	}

	return text_pos;
}

int GetCharLength(char c)
{
	if(c<32 || c>126)return 0;
	return text_start_posd[c];
}

int GetTextLength(const char* str)
{
	int text_length = 0;
	int l = strlen(str);
	for(int i = 0; i<l; i++)
	{
		text_pos += GetCharLength(str[i]);
	}

	return text_length;
}

void CTreeCanvas::RenderBranchIcon(HeeksObj* object, HeeksObj* next_object, bool expanded, int level)
{
	int num_children = object->GetNumChildren();
	if(num_children > 0)
	{
		if(level)
		{
			// with branches
			if(expanded)
			{
				RenderIcon(wxGetApp().m_icon_texture_number, 2, 0); // -
				AddPlusOrMinusButton(object, false);
			}
			else
			{
				RenderIcon(wxGetApp().m_icon_texture_number, 1, 0);// +
				AddPlusOrMinusButton(object, true);
			}
		}
		else
		{
			// without branches
			if(expanded)
			{
				RenderIcon(wxGetApp().m_icon_texture_number, 6, 0); // -
				AddPlusOrMinusButton(object, false);
			}
			else
			{
				RenderIcon(wxGetApp().m_icon_texture_number, 5, 0); // +
				AddPlusOrMinusButton(object, true);
			}
		}
	}
	else
	{
		if(level)
		{
			// just branches
			if(next_object)RenderIcon(wxGetApp().m_icon_texture_number, 3, 0);
			else RenderIcon(wxGetApp().m_icon_texture_number, 4, 0);
		}
	}
}

void CTreeCanvas::RenderBranchIcons(HeeksObj* object, HeeksObj* next_object, bool expanded, int level)
{
	// render initial branches
	for(int i = 0; i<level; i++)
	{
		if(i > 0)RenderIcon(wxGetApp().m_icon_texture_number, 7, 0);
		m_xpos++;
	}

	// render + or -
	RenderBranchIcon(object, next_object, expanded, level);
	m_xpos++;
}

void CTreeCanvas::RenderObject(HeeksObj* object, HeeksObj* next_object, int level)
{
	bool expanded = IsExpanded(object);

	m_xpos = 0;
	RenderBranchIcons(object, next_object, expanded, level);

	int label_start_x = m_xpos * 16;
	// find icon info
	int texture_number = wxGetApp().m_icon_texture_number;
	int x = 9; // unknown icon
	int y = 0;
	object->GetIcon(texture_number, x, y);
	RenderIcon(texture_number, x, y);
	m_xpos++;

	text_pos = 8; // leave a gap before the text
	const char* str = Ttc(object->GetShortStringOrTypeString());
	render_marked = wxGetApp().m_marked_list->ObjectMarked(object);
	RenderText(str);
	int label_end_x = m_xpos * 16 + text_pos;
	AddLabelButton(object, label_start_x, label_end_x);
	m_xpos--;
	m_ypos++;

	if(expanded)
	{
		HeeksObj* child = object->GetFirstChild();

		while(child)
		{
			HeeksObj* next_child = object->GetNextChild();
			RenderObject(child, next_child, level + 1);
			child = next_child;
		}
	}

	m_xpos--;
}

void CTreeCanvas::Render(int width, int height)
{
	if(!m_texture_built)BuildTexture();

	render_width = width;
	render_height = height;

	m_xpos = 0; // start at the left
	m_ypos = -scroll_y_pos; // start at the top
	m_tree_buttons.clear();

	HeeksObj* object = wxGetApp().GetFirstChild();

	while(object)
	{
		HeeksObj* next_object = wxGetApp().GetNextChild();
		RenderObject(object, next_object, 0);
		object = next_object;
	}
}
