#include "wx/treectrl.h"
#include "../interface/Observer.h"
#include "Images.h"

class HeeksObj;

class MyTreeItemData : public wxTreeItemData
{
public:
    HeeksObj* m_object;

    MyTreeItemData(const wxString& desc, HeeksObj* object) : m_object(object),  m_desc(desc){ }

private:
    wxString m_desc;
};

class MyTreeCtrl : public wxTreeCtrl, public Images
{
public:
    enum
    {
        TreeCtrlIcon_File,
        TreeCtrlIcon_FileSelected,
        TreeCtrlIcon_Folder,
        TreeCtrlIcon_FolderSelected,
        TreeCtrlIcon_FolderOpened
    };

    MyTreeCtrl() { }
    MyTreeCtrl(wxWindow *parent, const wxWindowID id,
               const wxPoint& pos, const wxSize& size,
               long style);
    virtual ~MyTreeCtrl(){};

    void OnBeginDrag(wxTreeEvent& event);
    void OnBeginRDrag(wxTreeEvent& event);
    void OnEndDrag(wxTreeEvent& event);
    void OnBeginLabelEdit(wxTreeEvent& event);
    void OnEndLabelEdit(wxTreeEvent& event);
    void OnDeleteItem(wxTreeEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnItemMenu(wxTreeEvent& event);
    void OnMenuEvent(wxCommandEvent& event);
    void OnGetInfo(wxTreeEvent& event);
    void OnSetInfo(wxTreeEvent& event);
    void OnItemExpanded(wxTreeEvent& event);
    void OnItemExpanding(wxTreeEvent& event);
    void OnItemCollapsed(wxTreeEvent& event);
    void OnItemCollapsing(wxTreeEvent& event);
    void OnSelChanged(wxTreeEvent& event);
    void OnSelChanging(wxTreeEvent& event);
    void OnTreeKeyDown(wxTreeEvent& event);
    void OnItemActivated(wxTreeEvent& event);
    void OnItemRClick(wxTreeEvent& event);
    void OnLMouseDown(wxMouseEvent& event);
    void OnLMouseUp(wxMouseEvent& event);
    void OnLMouseDClick(wxMouseEvent& event);
    void OnRMouseDown(wxMouseEvent& event);
    void OnRMouseUp(wxMouseEvent& event);
    void OnRMouseDClick(wxMouseEvent& event);
    void CreateImageList(int size = 16);
    void CreateButtonsImageList(int size = 11);

    void DoSortChildren(const wxTreeItemId& item, bool reverse = false)
        { m_reverseSort = reverse; wxTreeCtrl::SortChildren(item); }
    void DoEnsureVisible() { if (m_lastItem.IsOk()) EnsureVisible(m_lastItem); }

     void ShowMenu(wxTreeItemId id, const wxPoint& pt);

    int ImageSize(void) const { return m_imageSize; }

    void SetLastItem(wxTreeItemId id) { m_lastItem = id; }

    void AddIcon(wxIcon icon);

protected:
    virtual int OnCompareItems(const wxTreeItemId& i1, const wxTreeItemId& i2);

private:
    int          m_imageSize;               // current size of images
    bool         m_reverseSort;             // flag for OnCompareItems
    wxTreeItemId m_lastItem,                // for OnEnsureVisible()
                 m_draggedItem;             // item being dragged right now

    // NB: due to an ugly wxMSW hack you _must_ use DECLARE_DYNAMIC_CLASS()
    //     if you want your overloaded OnCompareItems() to be called.
    //     OTOH, if you don't want it you may omit the next line - this will
    //     make default (alphabetical) sorting much faster under wxMSW.
    DECLARE_DYNAMIC_CLASS(MyTreeCtrl)
    DECLARE_EVENT_TABLE()
};


// menu and control ids
enum
{
    TreeTest_Quit = wxID_EXIT,
    TreeTest_About = wxID_ABOUT,
    TreeTest_TogButtons = wxID_HIGHEST,
    TreeTest_TogTwist,
    TreeTest_TogLines,
    TreeTest_TogEdit,
    TreeTest_TogHideRoot,
    TreeTest_TogRootLines,
    TreeTest_TogBorder,
    TreeTest_TogFullHighlight,
    TreeTest_SetFgColour,
    TreeTest_SetBgColour,
    TreeTest_ResetStyle,
    TreeTest_Dump,
    TreeTest_DumpSelected,
    TreeTest_Count,
    TreeTest_CountRec,
    TreeTest_Sort,
    TreeTest_SortRev,
    TreeTest_SetBold,
    TreeTest_ClearBold,
    TreeTest_Rename,
    TreeTest_Delete,
    TreeTest_DeleteChildren,
    TreeTest_DeleteAll,
    TreeTest_Recreate,
    TreeTest_ToggleImages,
    TreeTest_ToggleButtons,
    TreeTest_SetImageSize,
    TreeTest_ToggleSel,
    TreeTest_CollapseAndReset,
    TreeTest_EnsureVisible,
    TreeTest_AddItem,
    TreeTest_InsertItem,
    TreeTest_IncIndent,
    TreeTest_DecIndent,
    TreeTest_IncSpacing,
    TreeTest_DecSpacing,
    TreeTest_ToggleIcon,
    TreeTest_Select,
    TreeTest_Unselect,
    TreeTest_SelectRoot,
    TreeTest_Ctrl = 1000
};

class CLeftCanvas: public wxScrolledWindow, public Observer
{
private:
// Observer's virtual functions
    void OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified);
    void WhenMarkedListChanges(bool all_added, bool all_removed, const std::list<HeeksObj *>* added_list, const std::list<HeeksObj *>* removed_list);
	void Clear();

    void Resize();
    void CreateTreeWithDefStyle();
    void CreateTree(long style);
    bool CanAdd(HeeksObj* object);
    const wxTreeItemId Add(HeeksObj* object, const wxTreeItemId &owner, bool expand);
    void AddSubstitute(HeeksObj* object, const wxTreeItemId &item);
    void AddChildren(HeeksObj* object, const wxTreeItemId &item);
    void Remove(HeeksObj *object, const wxTreeItemId &item, bool set_not_marked);
    bool RemoveChildren(const wxTreeItemId &item);

    MyTreeCtrl *m_treeCtrl;
    std::map<HeeksObj*, wxTreeItemId> tree_map;
    wxTreeItemId m_root;

public:
    CLeftCanvas(wxWindow* parent);
    virtual ~CLeftCanvas();

    void OnSize(wxSizeEvent& event);
	void OnMouseWheel(wxMouseEvent& event);
    wxTreeItemId Find(HeeksObj *object);
 
    DECLARE_NO_COPY_CLASS(CLeftCanvas)
    DECLARE_EVENT_TABLE()
};
