#include "stdafx.h"
#include "HeeksFrame.h"
#include "HeeksRibbon.h"
#include "GraphicsCanvas.h"

#ifdef USING_RIBBON

BEGIN_EVENT_TABLE(HeeksRibbon, wxRibbonBar)

#define EVT_RIBBONBUTTONBAR_CLICK_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_RIBBONBUTTONBAR_CLICKED, id1, id2, wxRibbonButtonBarEventHandler(func))
#define EVT_RIBBONTOOLBAR_CLICK_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_RIBBONTOOLBAR_CLICKED, id1, id2, wxRibbonToolBarEventHandler(func))
#define EVT_RIBBONBUTTONBAR_DROPDOWN_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_RIBBONBUTTONBAR_DROPDOWN_CLICKED, id1, id2, wxRibbonButtonBarEventHandler(func))
#define EVT_RIBBONTOOLBAR_DROPDOWN_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_RIBBONTOOLBAR_DROPDOWN_CLICKED, id1, id2, wxRibbonToolBarEventHandler(func))
EVT_RIBBONBUTTONBAR_CLICK_RANGE(ID_FIRST_EXTERNAL_BUTTON, ID_FIRST_POP_UP_MENU_TOOL + 1000, HeeksRibbon::OnExternalBarButton)
//EVT_RIBBONTOOLBAR_CLICK_RANGE(ID_FIRST_EXTERNAL_BUTTON, ID_FIRST_POP_UP_MENU_TOOL + 1000, HeeksRibbon::OnExternalToolButton)
EVT_RIBBONBUTTONBAR_DROPDOWN_RANGE(ID_FIRST_EXTERNAL_BUTTON, ID_FIRST_POP_UP_MENU_TOOL + 1000, HeeksRibbon::OnBarButtonDropDown)
EVT_UPDATE_UI_RANGE(ID_FIRST_EXTERNAL_BUTTON, ID_FIRST_POP_UP_MENU_TOOL + 1000, HeeksRibbon::OnUpdateExternalButton)

END_EVENT_TABLE()

void test(std::initializer_list <int> t)
{
	int tt = 0;
	for (auto i : t) {
		wxMessageBox(wxString::Format(_T("%d"), i));

		tt += i;
	}
}

static void OnOpenDropdown(wxRibbonButtonBarEvent& evt)
{
	wxMenu menu;

	int recent_id = ID_RECENT_FIRST;
	for (std::list< wxString >::iterator It = wxGetApp().m_recent_files.begin(); It != wxGetApp().m_recent_files.end() && recent_id < ID_RECENT_FIRST + MAX_RECENT_FILES; It++, recent_id++)
	{
		wxString& filepath = *It;
		menu.Append(recent_id, filepath, filepath);
	}

	evt.PopupMenu(&menu);
}

static void OnEndofButton(wxCommandEvent& event)
{
	wxGetApp().digitize_end = !wxGetApp().digitize_end;
}

static void OnUpdateEndof(wxUpdateUIEvent& event)
{
	event.Check(wxGetApp().digitize_end);
}

static void OnIntersButton(wxCommandEvent& event)
{
	wxGetApp().digitize_inters = !wxGetApp().digitize_inters;
}

static void OnUpdateInters(wxUpdateUIEvent& event)
{
	event.Check(wxGetApp().digitize_inters);
}

static void OnCentreButton(wxCommandEvent& event)
{
	wxGetApp().digitize_centre = !wxGetApp().digitize_centre;
}

static void OnUpdateCentre(wxUpdateUIEvent& event)
{
	event.Check(wxGetApp().digitize_centre);
}

static void OnMidpointButton(wxCommandEvent& event)
{
	wxGetApp().digitize_midpoint = !wxGetApp().digitize_midpoint;
}

static void OnUpdateMidpoint(wxUpdateUIEvent& event)
{
	event.Check(wxGetApp().digitize_midpoint);
}

static void OnSnapButton(wxCommandEvent& event)
{
	wxGetApp().draw_to_grid = !wxGetApp().draw_to_grid;
}

HeeksRibbon::HeeksRibbon(wxWindow* parent) :wxRibbonBar(parent, -1, wxDefaultPosition, wxDefaultSize,
wxRIBBON_BAR_FLOW_HORIZONTAL
	| wxRIBBON_BAR_SHOW_PAGE_LABELS
//	| wxRIBBON_BAR_SHOW_PANEL_EXT_BUTTONS
//	| wxRIBBON_BAR_SHOW_TOGGLE_BUTTON
//	| wxRIBBON_BAR_SHOW_HELP_BUTTON
	)
{
	//wxAcceleratorEntry[] = 
	//{
	//	wxAcceleratorEntry(int flags = 0, int keyCode = 0, int cmd = 0, wxMenuItem *item = NULL);

	//wxAcceleratorTable()

	m_next_id_for_button = ID_FIRST_EXTERNAL_BUTTON;
	wxRibbonPage* home = new wxRibbonPage(this, wxID_ANY, wxT("Page"), ToolImage(_T("new")));
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("File"), ToolImage(_T("new")), wxDefaultPosition, wxDefaultSize);
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("New"), ToolImage(_T("new")), _("New file"), OnNewButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Open"), ToolImage(_T("open")), _("Open file"), OnOpenButton, NULL, OnOpenDropdown));
		AddToolBarTool(toolbar, RibbonButtonData(_("Import"), ToolImage(_T("import")), _("Import file"), OnImportButton));
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("Save"), ToolImage(_T("save")), _("Save file"), OnSaveButton, OnUpdateSave));
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("Save As"), ToolImage(_T("saveas")), _("Save file with given name"), OnSaveAsButton, OnUpdateSave));
		AddToolBarTool(toolbar, RibbonButtonData(_("Plugins"), ToolImage(_T("plugin")), _("Edit plugins"), OnPlugins));
		AddToolBarTool(toolbar, RibbonButtonData(_("Restore Defaults"), ToolImage(_T("restore")), _("Restore all defaults"), OnResetDefaultsButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("About"), ToolImage(_T("about")), _("Software Information"), OnAbout));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("Print"), ToolImage(_T("print")), wxDefaultPosition, wxDefaultSize);
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		AddToolBarTool(toolbar, RibbonButtonData(_("Print"), ToolImage(_T("print")), _("Print the view to a printer"), OnPrint));
		AddToolBarTool(toolbar, RibbonButtonData(_("Page Setup"), ToolImage(_T("psetup")), _("Setup the printer layout"), OnPageSetup));
		AddToolBarTool(toolbar, RibbonButtonData(_("Print Preview"), ToolImage(_T("ppreview")), _("Show a preview of the print view"), OnPrintPreview));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("Edit"), ToolImage(_T("cut")), wxDefaultPosition, wxDefaultSize);
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("Cut"), ToolImage(_T("cut")), _("Cut selected items to the clipboard"), OnCutButton, OnUpdateCut));
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("Copy"), ToolImage(_T("copy")), _("Copy selected items to the clipboard"), OnCopyButton, OnUpdateCopy));
		if (!wxGetApp().m_no_creation_mode)AddToolBarTool(toolbar, RibbonButtonData(_("Paste"), ToolImage(_T("paste")), _("Paste items from the clipboard"), OnPasteButton, OnUpdatePaste));
		AddToolBarTool(toolbar, RibbonButtonData(_("Undo"), ToolImage(_T("undo")), _("Undo the previous command"), OnUndoButton, OnUpdateUndo));
		AddToolBarTool(toolbar, RibbonButtonData(_("Redo"), ToolImage(_T("redo")), _("Redo the next command"), OnRedoButton, OnUpdateRedo));
		AddToolBarTool(toolbar, RibbonButtonData(_("Select"), ToolImage(_T("select")), _("Select Mode"), OnSelectModeButton));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("Settings"), ToolImage(_T("gear")));
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("Sketches"), ToolImage(_T("lines")));
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		AddToolBarTool(toolbar, RibbonButtonData(_("Lines"), ToolImage(_T("lines")), _("Draw a sketch with lines and arcs"), OnLinesButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Rectangles"), ToolImage(_T("rect")), _("Draw rectangles"), OnRectanglesButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Obrounds"), ToolImage(_T("obround")), _("Draw obrounds"), OnObroundsButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Polygons"), ToolImage(_T("pentagon")), _("Draw polygons"), OnPolygonsButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Gear"), ToolImage(_T("gear")), _("Add a gear"), OnGearButton));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("Circles"), ToolImage(_T("circ3p")));
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		AddToolBarTool(toolbar, RibbonButtonData(_("3 Points"), ToolImage(_T("circ3p")), _("Draw circles through 3 points"), OnCircles3pButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("2 Points"), ToolImage(_T("circ2p")), _("Draw circles, centre and point"), OnCircles2pButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Radius"), ToolImage(_T("circpr")), _("Draw circles, centre and radius"), OnCirclesprButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Ellipses"), ToolImage(_T("ellipse")), _("Draw ellipses"), OnEllipseButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Gear"), ToolImage(_T("gear")), _("Add a gear"), OnGearButton));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("OtherDrawing"), ToolImage(_T("point")));
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		AddToolBarTool(toolbar, RibbonButtonData(_("Infinite Line"), ToolImage(_T("iline")), _("Drawing Infinite Lines"), OnILineButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Points"), ToolImage(_T("point")), _("Drawing Points"), OnPointsButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Spline"), ToolImage(_T("splpts")), _("Spline Through Points"), OnSplinePointsButton));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("Text"), ToolImage(_T("text")));
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		AddToolBarTool(toolbar, RibbonButtonData(_("Text"), ToolImage(_T("text")), _("Add a text object"), OnTextButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Dimensioning"), ToolImage(_T("dimension")), _("Add a dimension"), OnDimensioningButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Coordinate System"), ToolImage(_T("coordsys")), _("Add a coordinate system"), OnCoordinateSystem));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("Transformations"), ToolImage(_T("movet")));
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		AddToolBarTool(toolbar, RibbonButtonData(_("Move Translate"), ToolImage(_T("movet")), _("Translate selected items"), OnMoveTranslateButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Copy Translate"), ToolImage(_T("copyt")), _("Copy and translate selected items"), OnCopyTranslateButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Move Rotate"), ToolImage(_T("mover")), _("Rotate selected items"), OnMoveRotateButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Copy Rotate"), ToolImage(_T("copyr")), _("Copy and rotate selected items"), OnCopyRotateButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Move Mirror"), ToolImage(_T("movem")), _("Mirror selected items"), OnMoveMirrorButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Copy Mirror"), ToolImage(_T("copym")), _("Copy and mirror selected items"), OnCopyMirrorButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Move Scale"), ToolImage(_T("moves")), _("Scale selected items"), OnMoveScaleButton));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("Digitizing"), ToolImage(_T("endof")));
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		AddToolBarTool(toolbar, RibbonButtonData(_("Endof"), ToolImage(_T("endof")), _("End point"), OnEndofButton, OnUpdateEndof));
		AddToolBarTool(toolbar, RibbonButtonData(_("Inters"), ToolImage(_T("inters")), _("Intersection"), OnIntersButton, OnUpdateInters));
		AddToolBarTool(toolbar, RibbonButtonData(_("Centre"), ToolImage(_T("centre")), _("Centre"), OnCentreButton, OnUpdateCentre));
		AddToolBarTool(toolbar, RibbonButtonData(_("Midpoint"), ToolImage(_T("midpoint")), _("Midpoint"), OnMidpointButton, OnUpdateMidpoint));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("SolidPrimitives"), ToolImage(_T("sphere")));
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		AddToolBarTool(toolbar, RibbonButtonData(_("Sphere"), ToolImage(_T("sphere")), _("Add a sphere"), OnSphereButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Cube"), ToolImage(_T("cube")), _("Add a cube"), OnCubeButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Cylinder"), ToolImage(_T("cyl")), _("Add a cylinder"), OnCylButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Cone"), ToolImage(_T("cone")), _("Add a cone"), OnConeButton));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("SolidMake"), ToolImage(_T("ruled")));
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		AddToolBarTool(toolbar, RibbonButtonData(_("Ruled"), ToolImage(_T("ruled")), _("Create a lofted face"), OnRuledSurfaceButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Extrude"), ToolImage(_T("extrude")), _("Extrude a wire or face"), OnExtrudeButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Revolve"), ToolImage(_T("revolve")), _("Revolve a wire or face"), OnRevolveButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Sweep"), ToolImage(_T("sweep")), _("Sweep a wire or face"), OnSweepButton));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("SolidBooleans"), ToolImage(_T("subtract")));
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		AddToolBarTool(toolbar, RibbonButtonData(_("Cut"), ToolImage(_T("subtract")), _("Cut one solid from another"), OnSubtractButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Fuse"), ToolImage(_T("fuse")), _("Fuse one solid to another"), OnFuseButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Common"), ToolImage(_T("common")), _("Find common solid between two solids"), OnCommonButton));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("SolidChamfers"), ToolImage(_T("fillet")));
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		AddToolBarTool(toolbar, RibbonButtonData(_("Fillet"), ToolImage(_T("fillet")), _("Make a fillet on selected edges"), OnFilletButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Chamfer"), ToolImage(_T("chamfer")), _("Make a chamfer on selected edges"), OnChamferButton));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("ViewMag"), ToolImage(_T("magextents")));
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		AddToolBarTool(toolbar, RibbonButtonData(_("Mag Extents"), ToolImage(_T("magextents")), _("Zoom in to fit the extents of the drawing into the graphics window"), OnMagExtentsButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Mag No Rotation"), ToolImage(_T("magnorot")), _("Zoom in to fit the extents of the drawing into the graphics window, but without rotating the view"), OnMagNoRotButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Zoom Window"), ToolImage(_T("mag")), _("Zoom in to a dragged window"), OnMagButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("View Back"), ToolImage(_T("magprev")), _("Go back to previous view"), OnMagPreviousButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("FullScreen"), ToolImage(_T("fullscreen")), _("Switch to full screen view ( press escape to return )"), OnFullScreenButton));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("ViewSpecific"), ToolImage(_T("magxy")));
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		AddToolBarTool(toolbar, RibbonButtonData(_("View XY Front"), ToolImage(_T("magxy")), _("View XY Front"), OnMagXYButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("View XY Back"), ToolImage(_T("magxym")), _("View XY Back"), OnMagXYMButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("View XZ Top"), ToolImage(_T("magxz")), _("View XZ Top"), OnMagXZButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("View XZ Bottom"), ToolImage(_T("magxzm")), _("View XZ Bottom"), OnMagXZMButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("View YZ Right"), ToolImage(_T("magyz")), _("View YZ Right"), OnMagYZButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("View YZ Left"), ToolImage(_T("magyzm")), _("View YZ Left"), OnMagYZMButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("View XY Isometric"), ToolImage(_T("magxyz")), _("View XY Isometric"), OnMagXYZButton));
	}
	{
		wxRibbonPanel *panel = new wxRibbonPanel(home, wxID_ANY, wxT("ViewMag"), ToolImage(_T("viewrot")));
		wxRibbonButtonBar *toolbar = new wxRibbonButtonBar(panel);
		AddToolBarTool(toolbar, RibbonButtonData(_("View Rotate"), ToolImage(_T("viewrot")), _("Enter view rotating mode"), OnViewRotateButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("View Zoom"), ToolImage(_T("zoom")), _("Drag to zoom in and out"), OnViewZoomButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("View Pan"), ToolImage(_T("pan")), _("Drag to move view"), OnViewPanButton));
		AddToolBarTool(toolbar, RibbonButtonData(_("Redraw"), ToolImage(_T("redraw")), _("Redraw"), OnRedrawButton));
	}

	this->AddPageHighlight(this->GetPageCount() - 1);

	this->Realize();

}

std::map<int, RibbonButtonData > m_ribbon_button_map;

int HeeksRibbon::MakeNextIDForTool(const RibbonButtonData& data)
{
	while (m_ribbon_button_map.find(m_next_id_for_button) != m_ribbon_button_map.end())
	{
		// already used
		m_next_id_for_button++;
	}

	if (m_next_id_for_button >= ID_FIRST_POP_UP_MENU_TOOL)
	{
		// too many button IDs!
		wxMessageBox(_T("too many button IDs!, see CHeeksFrame::GetNextIDForTool"));
	}

	int id_to_use = m_next_id_for_button;

	m_ribbon_button_map.insert(std::pair<int, RibbonButtonData>(id_to_use, data));
	m_next_id_for_button++;
	return id_to_use;
}

RibbonButtonData* HeeksRibbon::GetRibbonButtonData(int id)
{
	std::map<int, RibbonButtonData>::iterator FindIt = m_ribbon_button_map.find(id);
	if (FindIt == m_ribbon_button_map.end())return NULL;
	return &(FindIt->second);
}

wxRibbonButtonBarButtonBase* HeeksRibbon::AddToolBarTool(wxRibbonButtonBar* toolbar, const RibbonButtonData& data)
{
	int id_to_use = MakeNextIDForTool(data);
	if (data.m_on_dropdown != NULL)
	{
		return toolbar->AddHybridButton(id_to_use, data.m_title, data.m_bitmap, data.m_caption);
	}
	else
	{
		return toolbar->AddButton(id_to_use, data.m_title, data.m_bitmap, data.m_caption);
	}
}

wxRibbonToolBarToolBase* HeeksRibbon::AddToolBarTool(wxRibbonToolBar* toolbar, const RibbonButtonData& data)
{
	int id_to_use = MakeNextIDForTool(data);
	return toolbar->AddDropdownTool(id_to_use, data.m_bitmap, data.m_caption);
}

void HeeksRibbon::OnExternalBarButton(wxRibbonButtonBarEvent& event)
{
	RibbonButtonData* data = GetRibbonButtonData(event.GetId());
	if (data)
	{
		(*(data->m_on_button))(event);
		event.Skip();
	}
}

void HeeksRibbon::OnBarButtonDropDown(wxRibbonButtonBarEvent& event)
{
	RibbonButtonData* data = GetRibbonButtonData(event.GetId());

	if (data && data->m_on_dropdown)
	{
		(*(data->m_on_dropdown))(event);
	}
}

void HeeksRibbon::OnExternalToolButton(wxRibbonToolBarEvent& event)
{
	RibbonButtonData* data = GetRibbonButtonData(event.GetId());
	if (data)
	{
		(*(data->m_on_button))(event);
	}
}

void HeeksRibbon::OnUpdateExternalButton(wxUpdateUIEvent& event)
{
	RibbonButtonData* data = GetRibbonButtonData(event.GetId());
	if (data)
	{
		if(data->m_on_update_button)
			(*(data->m_on_update_button))(event);
	}
}

#endif