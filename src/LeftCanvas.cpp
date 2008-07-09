#include "stdafx.h"
#include "wx/dcmirror.h"
#include "LeftCanvas.h"
#include "../interface/MarkedObject.h"
#include "MarkedList.h"

class DanObjectTreeData : public wxTreeItemData{
	public:
	HeeksObj* m_object;
	DanObjectTreeData(HeeksObj* object) : wxTreeItemData(){ m_object = object; }
	virtual ~DanObjectTreeData(){}
};


BEGIN_EVENT_TABLE(CLeftCanvas, wxScrolledWindow)
    EVT_SIZE(CLeftCanvas::OnSize)
	EVT_MOUSEWHEEL(CLeftCanvas::OnMouseWheel)
END_EVENT_TABLE()


CLeftCanvas::CLeftCanvas(wxWindow* parent)
        : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                           wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE), m_treeCtrl(NULL)
{
    CreateTreeWithDefStyle();
}

CLeftCanvas::~CLeftCanvas()
{
}

void CLeftCanvas::CreateTreeWithDefStyle()
{
    long style = wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT |
#ifndef NO_VARIABLE_HEIGHT
                 wxTR_HAS_VARIABLE_ROW_HEIGHT |
#endif
                 wxTR_MULTIPLE;

    CreateTree(style | wxSUNKEN_BORDER);

}

void CLeftCanvas::OnSize(wxSizeEvent& event)
{
    if ( m_treeCtrl  )
    {
        Resize();
    }

    event.Skip();
}

void CLeftCanvas::OnMouseWheel(wxMouseEvent& event)
{
	wxGetApp().PassMouseWheelToGraphics(event);
}

void CLeftCanvas::Resize()
{
    wxSize size = GetClientSize();
    m_treeCtrl->SetSize(0, 0, size.x, size.y );
}

void CLeftCanvas::CreateTree(long style)
{
    m_treeCtrl = new MyTreeCtrl(this, TreeTest_Ctrl,
                                wxDefaultPosition, wxDefaultSize,
                                style);

	m_root = m_treeCtrl->AddRoot("root", -1, -1, NULL);

    wxGetApp().RegisterObserver(this);
    Resize();
}

void CLeftCanvas::OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified)
{
	if(added){
		std::list<HeeksObj*>::const_iterator It;
		for(It = added->begin(); It != added->end(); It++){
			HeeksObj* object = *It;
			bool can_add = false;
			if(object->m_owner == NULL || object->m_owner == &wxGetApp()){
				can_add = true;
			}
			else{
				can_add = (Find(object->m_owner) != wxTreeItemId());
			}
			if(can_add){
				HeeksObj* owner_object = object->m_owner;
				wxTreeItemId owner = Find(owner_object);
				if(owner && owner_object){
					int count = 0;
					for(HeeksObj* temp_child = owner_object->GetFirstChild(); temp_child; temp_child = owner_object->GetNextChild()){
						count++;
					}

					bool is_expanded = m_treeCtrl->IsExpanded(owner);

						int child_count = 0;
						if(m_treeCtrl->ItemHasChildren(owner)){
							wxTreeItemIdValue cookie;
							wxTreeItemId hNextItem;
							wxTreeItemId hChildItem = m_treeCtrl->GetFirstChild(owner, cookie);
							while (hChildItem != wxTreeItemId())
							{
								hNextItem = m_treeCtrl->GetNextChild(owner, cookie);
								child_count++;
								hChildItem = hNextItem;
							}
						}
						if(count > child_count){					
							Add(object, owner, false);
						}
				}
				else{
					wxTreeItemId item = Add(object, m_root, false);
				}
			}
		}
	}
	if(removed){
		std::list<HeeksObj*>::const_iterator It;
		for(It = removed->begin(); It != removed->end(); It++){
			HeeksObj* object = *It;
			wxTreeItemId item = Find(object);
			if(item)Remove(object, item, false);
		}
	}
	if(modified){
		std::list<HeeksObj*>::const_iterator It;
		for(It = modified->begin(); It != modified->end(); It++){
			HeeksObj* object = *It;
			wxTreeItemId item = Find(object);
			if(item)
			{
				m_treeCtrl->SetItemText(item, object->GetShortStringOrTypeString());
				int image = m_treeCtrl->GetImage(object);
				m_treeCtrl->SetItemImage(item, image);
			}
		}
	}
}

void CLeftCanvas::WhenMarkedListChanges(bool all_added, bool all_removed, const std::list<HeeksObj *>* added_list, const std::list<HeeksObj *>* removed_list)
{
	if(all_added){
		wxTreeItemId item = m_treeCtrl->GetFirstVisibleItem();
		while(item.m_pItem != NULL){
			m_treeCtrl->SelectItem(item);
			item = m_treeCtrl->GetNextVisible(item);
		}
	}
	else if(all_removed){
		wxTreeItemId item = m_treeCtrl->GetFirstVisibleItem();
		while(item.m_pItem != NULL){
			m_treeCtrl->SelectItem(item, false);
			item = m_treeCtrl->GetNextVisible(item);
		}
	}
	else{
		if(added_list){
			std::list<HeeksObj *>::const_iterator It;
			for(It = added_list->begin(); It != added_list->end(); It++){
				HeeksObj* object = *It;
				wxTreeItemId item = Find(object);
				m_treeCtrl->SelectItem(item);
			}
		}
		if(removed_list){
			std::list<HeeksObj *>::const_iterator It;
			for(It = removed_list->begin(); It != removed_list->end(); It++){
				HeeksObj* object = *It;
				wxTreeItemId item = Find(object);
				m_treeCtrl->SelectItem(item, false);
			}
		}
	}
	Refresh();
}

void CLeftCanvas::Clear()
{
	m_treeCtrl->DeleteAllItems();
}

bool CLeftCanvas::CanAdd(HeeksObj* object)
{
	if (object == NULL) return false;
	if (tree_map.find(object) != tree_map.end()) return false;

	return true;
}

const wxTreeItemId CLeftCanvas::Add(HeeksObj* object, const wxTreeItemId &owner, bool expand)
{
	if (!CanAdd(object)) return wxTreeItemId();
	int image = m_treeCtrl->GetImage(object);
	wxTreeItemId item = m_treeCtrl->AppendItem(owner, object->GetShortStringOrTypeString(), image, -1, new DanObjectTreeData(object));
	tree_map.insert(std::pair<HeeksObj*, wxTreeItemId>(object, item));

	m_treeCtrl->Expand(item);
	AddChildren(object, item);

	return item;
}

void CLeftCanvas::AddChildren(HeeksObj* object, const wxTreeItemId &item)
{
	std::set<HeeksObj*> expand_set;
	{
		for(HeeksObj* child = object->GetFirstAutoExpandChild(); child; child = object->GetNextAutoExpandChild()){
			expand_set.insert(child);
		}
	}
	{
		for(HeeksObj* child = object->GetFirstChild(); child; child = object->GetNextChild()){
			bool expand = false;
			if(expand_set.find(child) != expand_set.end())expand = true;
			Add(child, item, expand);
		}
	}
}

void CLeftCanvas::Remove(HeeksObj *object, const wxTreeItemId &item, bool set_not_marked){
	if(object == NULL)return;
	if(tree_map.find(object) == tree_map.end())return;
	if(item == wxTreeItemId())return;
	RemoveChildren(item);
	if(item){
		m_treeCtrl->Delete(item);
		tree_map.erase(object);
	}
}

bool CLeftCanvas::RemoveChildren(const wxTreeItemId &item)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId child = m_treeCtrl->GetFirstChild(item, cookie);
	if(child){
		std::list<wxTreeItemId> child_list;
		while(child){
			child_list.push_back(child);
			child = m_treeCtrl->GetNextChild(item, cookie);
		}
		std::list<wxTreeItemId>::iterator It;
		for(It = child_list.begin(); It != child_list.end(); It++){
			HeeksObj *child_object = (HeeksObj *)(m_treeCtrl->GetItemData( *It ));
			Remove(child_object, *It, false);
		}
	}
	return false;
}

wxTreeItemId CLeftCanvas::Find(HeeksObj *object){
	std::map<HeeksObj*, wxTreeItemId>::iterator FindIt = tree_map.find(object);
	if(FindIt == tree_map.end())return wxTreeItemId();
	return FindIt->second;
}

#if USE_GENERIC_TREECTRL
BEGIN_EVENT_TABLE(MyTreeCtrl, wxGenericTreeCtrl)
#else
BEGIN_EVENT_TABLE(MyTreeCtrl, wxTreeCtrl)
#endif
    EVT_TREE_BEGIN_DRAG(TreeTest_Ctrl, MyTreeCtrl::OnBeginDrag)
    EVT_TREE_BEGIN_RDRAG(TreeTest_Ctrl, MyTreeCtrl::OnBeginRDrag)
    EVT_TREE_END_DRAG(TreeTest_Ctrl, MyTreeCtrl::OnEndDrag)
    EVT_TREE_BEGIN_LABEL_EDIT(TreeTest_Ctrl, MyTreeCtrl::OnBeginLabelEdit)
    EVT_TREE_END_LABEL_EDIT(TreeTest_Ctrl, MyTreeCtrl::OnEndLabelEdit)
    EVT_TREE_DELETE_ITEM(TreeTest_Ctrl, MyTreeCtrl::OnDeleteItem)
    EVT_TREE_SET_INFO(TreeTest_Ctrl, MyTreeCtrl::OnSetInfo)
    EVT_TREE_ITEM_EXPANDED(TreeTest_Ctrl, MyTreeCtrl::OnItemExpanded)
    EVT_TREE_ITEM_EXPANDING(TreeTest_Ctrl, MyTreeCtrl::OnItemExpanding)
    EVT_TREE_ITEM_COLLAPSED(TreeTest_Ctrl, MyTreeCtrl::OnItemCollapsed)
    EVT_TREE_ITEM_COLLAPSING(TreeTest_Ctrl, MyTreeCtrl::OnItemCollapsing)

    EVT_TREE_SEL_CHANGED(TreeTest_Ctrl, MyTreeCtrl::OnSelChanged)
    EVT_TREE_SEL_CHANGING(TreeTest_Ctrl, MyTreeCtrl::OnSelChanging)
    EVT_TREE_KEY_DOWN(TreeTest_Ctrl, MyTreeCtrl::OnTreeKeyDown)
    EVT_TREE_ITEM_ACTIVATED(TreeTest_Ctrl, MyTreeCtrl::OnItemActivated)

    // so many differents ways to handle right mouse button clicks...
    EVT_CONTEXT_MENU(MyTreeCtrl::OnContextMenu)
    // EVT_TREE_ITEM_MENU is the preferred event for creating context menus
    // on a tree control, because it includes the point of the click or item,
    // meaning that no additional placement calculations are required.
//    EVT_TREE_ITEM_MENU(TreeTest_Ctrl, MyTreeCtrl::OnItemMenu)
//    EVT_TREE_ITEm_graphics_CLICK(TreeTest_Ctrl, MyTreeCtrl::OnItemRClick)
   EVT_MENU_RANGE(1, 1000, MyTreeCtrl::OnMenuEvent)

    EVT_LEFT_DOWN(MyTreeCtrl::OnLMouseDown)
    EVT_LEFT_UP(MyTreeCtrl::OnLMouseUp)
    EVT_LEFT_DCLICK(MyTreeCtrl::OnLMouseDClick)
//    EVT_RIGHT_DOWN(MyTreeCtrl::OnRMouseDown)
//    EVT_RIGHT_UP(MyTreeCtrl::OnRMouseUp)
//    EVT_RIGHT_DCLICK(MyTreeCtrl::OnRMouseDClick)
END_EVENT_TABLE()

// MyTreeCtrl implementation
#if USE_GENERIC_TREECTRL
IMPLEMENT_DYNAMIC_CLASS(MyTreeCtrl, wxGenericTreeCtrl)
#else
IMPLEMENT_DYNAMIC_CLASS(MyTreeCtrl, wxTreeCtrl)
#endif

MyTreeCtrl::MyTreeCtrl(wxWindow *parent, const wxWindowID id,
                       const wxPoint& pos, const wxSize& size,
                       long style)
          : wxTreeCtrl(parent, id, pos, size, style)
{
    m_reverseSort = false;
    CreateImageList();
}

void MyTreeCtrl::CreateImageList(int size)
{
    if ( size == -1 )
    {
        SetImageList(NULL);
        return;
    }
    if ( size == 0 )
        size = m_imageSize;
    else
        m_imageSize = size;

	InitializeImageList(size, size);
    AssignImageList(m_image_list);
}

void MyTreeCtrl::CreateButtonsImageList(int WXUNUSED(size))
{
}

int MyTreeCtrl::OnCompareItems(const wxTreeItemId& item1,
                               const wxTreeItemId& item2)
{
    if ( m_reverseSort )
    {
        return wxTreeCtrl::OnCompareItems(item2, item1);
    }
    else
    {
        return wxTreeCtrl::OnCompareItems(item1, item2);
    }
}

// avoid repetition
#define TREE_EVENT_HANDLER(name)                                 \
void MyTreeCtrl::name(wxTreeEvent& event)                        \
{                                                                \
    event.Skip();                                                \
}

TREE_EVENT_HANDLER(OnBeginRDrag)
TREE_EVENT_HANDLER(OnDeleteItem)
TREE_EVENT_HANDLER(OnGetInfo)
TREE_EVENT_HANDLER(OnSetInfo)
TREE_EVENT_HANDLER(OnItemExpanded)
TREE_EVENT_HANDLER(OnItemExpanding)
TREE_EVENT_HANDLER(OnItemCollapsed)

#undef TREE_EVENT_HANDLER

void MyTreeCtrl::OnSelChanged(wxTreeEvent& event) 
{ 
}

void MyTreeCtrl::OnSelChanging(wxTreeEvent& event) 
{ 
}

void MyTreeCtrl::OnTreeKeyDown(wxTreeEvent& event)
{
    event.Skip();
}

void MyTreeCtrl::OnBeginDrag(wxTreeEvent& event)
{
    // need to explicitly allow drag
    if ( event.GetItem() != GetRootItem() )
    {
        m_draggedItem = event.GetItem();
        event.Allow();
    }
    else
    {
        wxLogMessage(wxT("OnBeginDrag: this item can't be dragged."));
    }
}

void MyTreeCtrl::OnEndDrag(wxTreeEvent& event)
{
    wxTreeItemId itemSrc = m_draggedItem,
                 itemDst = event.GetItem();
    m_draggedItem = (wxTreeItemId)0l;

    if ( itemDst.IsOk() && !ItemHasChildren(itemDst) )
    {
        itemDst = GetItemParent(itemDst);
    }

    if ( !itemDst.IsOk() )
    {
        wxLogMessage(wxT("OnEndDrag: can't drop here."));

        return;
    }

    wxString text = GetItemText(itemSrc);
    wxLogMessage(wxT("OnEndDrag: '%s' copied to '%s'."),
                 text.c_str(), GetItemText(itemDst).c_str());

    // just do append here - we could also insert it just before/after the item
    // on which it was dropped, but this requires slightly more work... we also
    // completely ignore the client data and icon of the old item but could
    // copy them as well.
    //
    // Finally, we only copy one item here but we might copy the entire tree if
    // we were dragging a folder.
    int image = TreeCtrlIcon_File;
    AppendItem(itemDst, text, image);
}

void MyTreeCtrl::OnBeginLabelEdit(wxTreeEvent& event)
{
    event.Skip();
}

void MyTreeCtrl::OnEndLabelEdit(wxTreeEvent& event)
{
    event.Skip();
}

void MyTreeCtrl::OnItemCollapsing(wxTreeEvent& event)
{
    event.Skip();
}

void MyTreeCtrl::OnItemActivated(wxTreeEvent& event)
{
    // show some info about this item
    wxTreeItemId itemId = event.GetItem();
    MyTreeItemData *item = (MyTreeItemData *)GetItemData(itemId);
}

void MyTreeCtrl::OnItemMenu(wxTreeEvent& event)
{
    event.Skip();
}

void MyTreeCtrl::OnMenuEvent(wxCommandEvent& event)
{
	wxGetApp().on_menu_event(event);
    event.Skip();
}

void MyTreeCtrl::OnContextMenu(wxContextMenuEvent& event)
{
    wxPoint pt = event.GetPosition();
	pt = ScreenToClient(pt);
	int flags;
    wxTreeItemId item = HitTest(pt, flags);

	if(item)ShowMenu(item, pt);
}

void MyTreeCtrl::ShowMenu(wxTreeItemId itemId, const wxPoint& point)
{
     MyTreeItemData *item = itemId.IsOk() ? (MyTreeItemData *)GetItemData(itemId)
                                         : NULL;
    MarkedObjectOneOfEach marked_object(0, item->m_object, 1);
		wxGetApp().DoDropDownMenu(this, point, &marked_object, false, false, false);
}

void MyTreeCtrl::OnItemRClick(wxTreeEvent& event)
{
    wxTreeItemId itemId = event.GetItem();
    MyTreeItemData *item = itemId.IsOk() ? (MyTreeItemData *)GetItemData(itemId)
                                         : NULL;
    event.Skip();
}

void MyTreeCtrl::OnLMouseDown(wxMouseEvent& event)
{
    //event.Skip();
}

void MyTreeCtrl::OnLMouseUp(wxMouseEvent& event)
{
    wxTreeItemId id = HitTest(event.GetPosition());
    if ( !id ){
	}
    else
    {
        MyTreeItemData *item = (MyTreeItemData *)GetItemData(id);
		if(event.ControlDown()){
			if(wxGetApp().m_marked_list->ObjectMarked(item->m_object))wxGetApp().m_marked_list->Remove(item->m_object);
			else wxGetApp().m_marked_list->Add(item->m_object);
			wxGetApp().Repaint();
		}
		else{
			wxGetApp().m_marked_list->Clear();
			wxGetApp().m_marked_list->Add(item->m_object);
			wxGetApp().Repaint();
		}
    }

	event.Skip();
}

void MyTreeCtrl::OnLMouseDClick(wxMouseEvent& event)
{
    wxTreeItemId id = HitTest(event.GetPosition());
    if ( !id ){
	}
    else
    {
        MyTreeItemData *item = (MyTreeItemData *)GetItemData(id);
    }

    event.Skip();
}

void MyTreeCtrl::OnRMouseDown(wxMouseEvent& event)
{
    event.Skip();
}

void MyTreeCtrl::OnRMouseUp(wxMouseEvent& event)
{
    event.Skip();
}

void MyTreeCtrl::OnRMouseDClick(wxMouseEvent& event)
{
    wxTreeItemId id = HitTest(event.GetPosition());
    if ( id )
    {
        MyTreeItemData *item = (MyTreeItemData *)GetItemData(id);
    }

    event.Skip();
}

void MyTreeCtrl::AddIcon(wxIcon icon)
{
	GetImageList()->Add(icon);
}

static inline const wxChar *Bool2String(bool b)
{
    return b ? wxT("") : wxT("not ");
}
