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
    MyTreeCtrl() { }
    MyTreeCtrl(wxWindow *parent, long style);
    virtual ~MyTreeCtrl(){};

    void OnDeleteItem(wxTreeEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnItemMenu(wxTreeEvent& event);
    void OnMenuEvent(wxCommandEvent& event);
    void OnGetInfo(wxTreeEvent& event);
    void OnSetInfo(wxTreeEvent& event);
    void OnSelChanged(wxTreeEvent& event);
    void OnSelChanging(wxTreeEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
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

    int ImageSize(void) const { return m_imageSize; }

    void SetLastItem(wxTreeItemId id) { m_lastItem = id; }

    void AddIcon(wxIcon icon);
	bool After(const wxTreeItemId& id1, const wxTreeItemId& id2);

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

class CTreeCanvas: public wxScrolledWindow, public Observer
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
    const wxTreeItemId Add(HeeksObj* object, const wxTreeItemId &owner);
    void AddSubstitute(HeeksObj* object, const wxTreeItemId &item);
    void AddChildren(HeeksObj* object, const wxTreeItemId &item);
    void Remove(HeeksObj *object, const wxTreeItemId &item, bool set_not_marked);
    bool RemoveChildren(const wxTreeItemId &item);

    MyTreeCtrl *m_treeCtrl;
    std::map<HeeksObj*, wxTreeItemId> tree_map;
    wxTreeItemId m_root;

public:
    CTreeCanvas(wxWindow* parent);
    virtual ~CTreeCanvas();

    void OnSize(wxSizeEvent& event);
	void OnMouseWheel(wxMouseEvent& event);
    wxTreeItemId Find(HeeksObj *object);
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
 
    DECLARE_NO_COPY_CLASS(CTreeCanvas)
    DECLARE_EVENT_TABLE()
};
