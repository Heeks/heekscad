#include "stdafx.h"

#include "Sectioning.h"
#include "HeeksConfig.h"
#include "HeeksFrame.h"

void SectioningData::ReadFromConfig()
{
	HeeksConfig config;
	config.Read(_T("SectioningPos1X"), &m_pos1[0], 0.0);
	config.Read(_T("SectioningPos1Y"), &m_pos1[1], 0.0);
	config.Read(_T("SectioningPos1Z"), &m_pos1[2], 0.0);
	config.Read(_T("SectioningPos2X"), &m_pos2[0], 100.0);
	config.Read(_T("SectioningPos2Y"), &m_pos2[1], 0.0);
	config.Read(_T("SectioningPos2Z"), &m_pos2[2], 0.0);
	config.Read(_T("SectioningPlaneAngle"), &m_angle, 0.0);
}

void SectioningData::WriteConfig()
{
	HeeksConfig config;
	config.Write(_T("SectioningPos1X"), m_pos1[0]);
	config.Write(_T("SectioningPos1Y"), m_pos1[1]);
	config.Write(_T("SectioningPos1Z"), m_pos1[2]);
	config.Write(_T("SectioningPos2X"), m_pos2[0]);
	config.Write(_T("SectioningPos2Y"), m_pos2[1]);
	config.Write(_T("SectioningPos2Z"), m_pos2[2]);
	config.Write(_T("SectioningPlaneAngle"), m_angle);
}

int SectioningData::GetAxisType()
{
	gp_Pnt p1 = make_point(m_pos1);
	gp_Pnt p2 = make_point(m_pos2);
	if(p1.IsEqual(p2, 0.0000000001))
	{
		return 0;
	}
	gp_Dir dir(gp_Vec(p1, p2));
	int axis_type = 0;
	gp_Dir x_axis(1.0, 0.0, 0.0);
	gp_Dir y_axis(0.0, 1.0, 0.0);
	gp_Dir z_axis(0.0, 0.0, 1.0);
	if(dir.IsEqual(x_axis, 0.0000001))axis_type = 1;
	else if(dir.IsEqual(y_axis, 0.0000001))axis_type = 2;
	else if(dir.IsEqual(z_axis, 0.0000001))axis_type = 3;

	return axis_type;
}

bool SectioningData::Validate()
{
	gp_Pnt p1 = make_point(m_pos1);
	gp_Pnt p2 = make_point(m_pos2);
	if(p1.IsEqual(p2, 0.0000000001))
	{
		wxMessageBox(_("Points must not be the same!"));
		return false;
	}

	return true;
}

static gp_Dir GetReferenceNormal(const gp_Dir &d)
{
	if(fabs(d.Dot(gp_Dir(0, 0, 1))) > 0.707)
	{
		// z axis, use x axis as zero angle normal
		return gp_Dir(1, 0, 0);
	}
	else
	{
		// x axis, use z axis as zero angle normal
		return gp_Dir(0, 0, 1);
	}
}

void SectioningData::GetOrigin(gp_Trsf &trsf)
{
	gp_Dir axis_dir(gp_Vec(make_point(m_pos1), make_point(m_pos2)));
	gp_Dir ref = GetReferenceNormal(axis_dir);
	gp_Dir vec_in_plane = axis_dir ^ ref;
	gp_Dir plane_normal = axis_dir ^ vec_in_plane;
	double angle_radians = 0.0174532925199432957 * m_angle;
	gp_Dir new_vec_in_plane = gp_Vec(vec_in_plane) * cos(angle_radians) + gp_Vec(plane_normal) * sin(angle_radians);

	trsf = make_matrix(make_point(m_pos1), new_vec_in_plane, axis_dir);
}

///////////////////////////////////////////////////////////////////////////
enum
{
    ID_RADIO_X = 100,
	ID_RADIO_Y,
	ID_RADIO_Z,
	ID_RADIO_SPECIFY,
	ID_BUTTON_PICK_POS1,
	ID_BUTTON_ZERO,
	ID_POS_X,
	ID_POS_Y,
	ID_POS_Z,
	ID_BUTTON_PICK_POS2,
	ID_POS2_X,
	ID_POS2_Y,
	ID_POS2_Z,
	ID_ANGLE,
};

SectioningDlg::SectioningDlg( wxWindow* parent, SectioningData &data ) : HDialog( parent, wxID_ANY, wxT("Solid Sectioning"), wxDefaultPosition, wxSize(-1, -1))
{
	m_ignore_event_functions = true;

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("Axis"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bSizer5->Add( m_staticText1, 0, wxALL, 5 );
	
	m_radioBtn1 = new wxRadioButton( this, ID_RADIO_X, wxT("X axis"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	bSizer5->Add( m_radioBtn1, 0, wxALL, 5 );
	
	m_radioBtn2 = new wxRadioButton( this, ID_RADIO_Y, wxT("Y axis"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( m_radioBtn2, 0, wxALL, 5 );
	
	m_radioBtn3 = new wxRadioButton( this, ID_RADIO_Z, wxT("Z axis"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( m_radioBtn3, 0, wxALL, 5 );
	
	m_radioBtn4 = new wxRadioButton( this, ID_RADIO_SPECIFY, wxT("Specify"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( m_radioBtn4, 0, wxALL, 5 );

	bSizer2->Add( bSizer5, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer101;
	bSizer101 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText3 = new wxStaticText( this, wxID_ANY, wxT("Point on axis"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	bSizer101->Add( m_staticText3, 0, wxALL, 5 );
	
	m_button1 = new wxButton( this, ID_BUTTON_PICK_POS1, wxT("Select"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer101->Add( m_button1, 0, wxALL, 5 );

	m_button_zero = new wxButton( this, ID_BUTTON_ZERO, wxT("0, 0, 0"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer101->Add( m_button_zero, 0, wxALL, 5 );
	
	bSizer6->Add( bSizer101, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("X"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	bSizer7->Add( m_staticText2, 1, wxLEFT|wxRIGHT, 5 );
	
	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("Y"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	bSizer7->Add( m_staticText4, 1, wxLEFT|wxRIGHT, 5 );
	
	m_staticText5 = new wxStaticText( this, wxID_ANY, wxT("Z"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap( -1 );
	bSizer7->Add( m_staticText5, 1, wxLEFT|wxRIGHT, 5 );
	
	bSizer6->Add( bSizer7, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxHORIZONTAL );
	
	m_textCtrl1 = new CLengthCtrl( this, ID_POS_X );
	bSizer8->Add( m_textCtrl1, 0, wxALL, 0 );
	
	m_textCtrl2 = new CLengthCtrl( this, ID_POS_Y );
	bSizer8->Add( m_textCtrl2, 0, wxALL, 0 );
	
	m_textCtrl3 = new CLengthCtrl( this, ID_POS_Z );
	bSizer8->Add( m_textCtrl3, 0, wxALL, 0 );
	
	bSizer6->Add( bSizer8, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText12 = new wxStaticText( this, wxID_ANY, wxT("Secondary point on axis"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	bSizer11->Add( m_staticText12, 0, wxALL, 5 );
	
	m_button2 = new wxButton( this, ID_BUTTON_PICK_POS2, wxT("Select"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_button2, 0, wxALL, 5 );
	
	bSizer6->Add( bSizer11, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText9 = new wxStaticText( this, wxID_ANY, wxT("X"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	bSizer9->Add( m_staticText9, 1, wxLEFT|wxRIGHT, 5 );
	
	m_staticText10 = new wxStaticText( this, wxID_ANY, wxT("Y"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	bSizer9->Add( m_staticText10, 1, wxLEFT|wxRIGHT, 5 );
	
	m_staticText11 = new wxStaticText( this, wxID_ANY, wxT("Z"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	bSizer9->Add( m_staticText11, 1, wxLEFT|wxRIGHT, 5 );
	
	bSizer6->Add( bSizer9, 0, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );
	
	m_textCtrl4 = new CLengthCtrl( this, ID_POS2_X );
	bSizer10->Add( m_textCtrl4, 0, wxALL, 0 );
	
	m_textCtrl5 = new CLengthCtrl( this, ID_POS2_Y );
	bSizer10->Add( m_textCtrl5, 0, wxALL, 0 );
	
	m_textCtrl6 = new CLengthCtrl( this, ID_POS2_Z );
	bSizer10->Add( m_textCtrl6, 0, wxALL, 0 );
	
	bSizer6->Add( bSizer10, 0, wxEXPAND, 0 );
	
	bSizer2->Add( bSizer6, 1, wxEXPAND, 5 );
	
	bSizer1->Add( bSizer2, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText13 = new wxStaticText( this, wxID_ANY, wxT("Angle of plane"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13->Wrap( -1 );
	bSizer3->Add( m_staticText13, 0, wxALL, 5 );
	
	m_textCtrl7 = new CDoubleCtrl( this, ID_ANGLE );
	bSizer3->Add( m_textCtrl7, 0, wxALL, 5 );
	
	bSizer1->Add( bSizer3, 0, wxEXPAND, 5 );

	// add OK and Cancel
	wxBoxSizer *sizerOKCancel = MakeOkAndCancel(wxHORIZONTAL);
	bSizer1->Add( sizerOKCancel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, control_border );
	
	this->SetSizer( bSizer1 );
	this->Layout();
	bSizer1->Fit( this );

	SetFromData(data);

	m_ignore_event_functions = false;
}

BEGIN_EVENT_TABLE(SectioningDlg, HDialog)
    EVT_RADIOBUTTON(ID_RADIO_X,SectioningDlg::OnRadioButtonX)
    EVT_RADIOBUTTON(ID_RADIO_Y,SectioningDlg::OnRadioButtonY)
    EVT_RADIOBUTTON(ID_RADIO_Z,SectioningDlg::OnRadioButtonZ)
    EVT_RADIOBUTTON(ID_RADIO_SPECIFY,SectioningDlg::OnRadioButtonSpecify)
    EVT_BUTTON(ID_BUTTON_PICK_POS1,SectioningDlg::OnPickPos1)
    EVT_BUTTON(ID_BUTTON_ZERO,SectioningDlg::OnButtonZero)
    EVT_BUTTON(ID_BUTTON_PICK_POS2,SectioningDlg::OnPickPos2)
    EVT_BUTTON(wxID_OK,SectioningDlg::OnOK)
END_EVENT_TABLE()


SectioningDlg::~SectioningDlg()
{
}

void SectioningDlg::SetPos1(const double* pos)
{
	m_textCtrl1->SetValue( pos[0] );
	m_textCtrl2->SetValue( pos[1] );
	m_textCtrl3->SetValue( pos[2] );
}

void SectioningDlg::EnableSecondaryPointControls(bool bEnable)
{
	m_textCtrl4->Enable( bEnable );
	m_textCtrl5->Enable( bEnable );
	m_textCtrl6->Enable( bEnable );
	m_button2->Enable( bEnable );
	m_staticText12->Enable( bEnable );
}

void SectioningDlg::SetFromData(SectioningData &data)
{
	int axis_type = data.GetAxisType();
	m_radioBtn1->SetValue( axis_type == 1 );
	m_radioBtn2->SetValue( axis_type == 2 );
	m_radioBtn3->SetValue( axis_type == 3 );
	m_radioBtn4->SetValue( axis_type == 0 );

	SetPos1(data.m_pos1);

	m_textCtrl4->SetValue( data.m_pos2[0] );
	m_textCtrl5->SetValue( data.m_pos2[1] );
	m_textCtrl6->SetValue( data.m_pos2[2] );

	EnableSecondaryPointControls( axis_type == 0 );

	m_textCtrl7->SetValue( data.m_angle );
}

void SectioningDlg::GetData(SectioningData &data)
{
	data.m_pos1[0] = m_textCtrl1->GetValue();
	data.m_pos1[1] = m_textCtrl2->GetValue();
	data.m_pos1[2] = m_textCtrl3->GetValue();
	data.m_pos2[0] = m_textCtrl4->GetValue();
	data.m_pos2[1] = m_textCtrl5->GetValue();
	data.m_pos2[2] = m_textCtrl6->GetValue();
	data.m_angle = m_textCtrl7->GetValue();
}

void SectioningDlg::UpdateSecondaryPoint()
{
	if(m_radioBtn1->GetValue())
	{
		m_textCtrl4->SetValue( m_textCtrl1->GetValue() + 100.0 );
		m_textCtrl5->SetValue( m_textCtrl2->GetValue() );
		m_textCtrl6->SetValue( m_textCtrl3->GetValue() );
		EnableSecondaryPointControls( false );
	}
	else if(m_radioBtn2->GetValue())
	{
		m_textCtrl4->SetValue( m_textCtrl1->GetValue() );
		m_textCtrl5->SetValue( m_textCtrl2->GetValue() + 100.0 );
		m_textCtrl6->SetValue( m_textCtrl3->GetValue() );
		EnableSecondaryPointControls( false );
	}
	else if(m_radioBtn3->GetValue())
	{
		m_textCtrl4->SetValue( m_textCtrl1->GetValue() );
		m_textCtrl5->SetValue( m_textCtrl2->GetValue() );
		m_textCtrl6->SetValue( m_textCtrl3->GetValue() + 100.0 );
		EnableSecondaryPointControls( false );
	}
	else
	{
		EnableSecondaryPointControls( true );
	}
}

void SectioningDlg::OnRadioButtonX(wxCommandEvent& event)
{
	if(m_ignore_event_functions)return;
	UpdateSecondaryPoint();
}

void SectioningDlg::OnRadioButtonY(wxCommandEvent& event)
{
	if(m_ignore_event_functions)return;
	UpdateSecondaryPoint();
}

void SectioningDlg::OnRadioButtonZ(wxCommandEvent& event)
{
	if(m_ignore_event_functions)return;
	UpdateSecondaryPoint();
}

void SectioningDlg::OnRadioButtonSpecify(wxCommandEvent& event)
{
	if(m_ignore_event_functions)return;
	UpdateSecondaryPoint();
}

void SectioningDlg::OnPickPos1(wxCommandEvent& event)
{
	if(m_ignore_event_functions)return;
	this->EndDialog(ID_BUTTON_PICK_POS1);
}

void SectioningDlg::OnPickPos2(wxCommandEvent& event)
{
	if(m_ignore_event_functions)return;
	this->EndDialog(ID_BUTTON_PICK_POS2);
}

void SectioningDlg::OnButtonZero(wxCommandEvent& event)
{
	if(m_ignore_event_functions)return;
	m_textCtrl1->SetValue( 0.0 );
	m_textCtrl2->SetValue( 0.0 );
	m_textCtrl3->SetValue( 0.0 );
	UpdateSecondaryPoint();
}

void SectioningDlg::OnOK(wxCommandEvent& event)
{
	if(m_ignore_event_functions)return;

	SectioningData data;
	this->GetData(data);
	if(data.Validate())
	{
		EndModal(wxID_OK);
	}
}

static void SectionObjectsWithDialog(std::list<HeeksObj*> list)
{
	SectioningData data;
	data.ReadFromConfig();

	while(1)
	{
		SectioningDlg dlg(wxGetApp().m_frame, data);
		int ret = dlg.ShowModal();

		if(ret == wxID_OK)
		{
			// do the sectioning
			dlg.GetData(data);
			data.WriteConfig();

			gp_Trsf trsf;
			data.GetOrigin(trsf);
			gp_Pnt p1(-1000.0, -1000.0, 0.0);
			p1.Transform(trsf);
			gp_Ax2 axis(p1, gp_Dir(0, 0, 1).Transformed(trsf), gp_Dir(1, 0, 0).Transformed(trsf));

			wxGetApp().CreateUndoPoint();

			try{

			TopoDS_Solid cuboid = BRepPrimAPI_MakeBox(axis, 2000.0, 2000.0, 1000.0);

			wxGetApp().m_marked_list->Clear(false);

			for(std::list<HeeksObj*>::iterator It = list.begin(); It != list.end(); It++)
			{
				HeeksObj* object = *It;
				if(CShape::IsTypeAShape(object->GetType()) == false)continue;
				TopoDS_Shape new_shape = BRepAlgoAPI_Cut(((CShape*)object)->Shape(), cuboid);
				if(new_shape.IsNull())continue;
				HeeksObj* new_object = CShape::MakeObject(new_shape, _("Sectioned Solid"), SOLID_TYPE_UNKNOWN, ((CShape*)object)->m_color, ((CShape*)object)->GetOpacity());
				wxGetApp().Add(new_object, NULL);
				wxGetApp().Remove(object);
				wxGetApp().m_marked_list->Add(new_object, false);
			}
			}
			catch(...)
			{
				::wxMessageBox(_("Sectioning failed!"));
			}

			wxGetApp().Changed();

			break;
		}
		else if(ret == wxID_CANCEL)
		{
			break;
		}
		else if(ret == ID_BUTTON_PICK_POS1)
		{
			dlg.GetData(data);
			wxGetApp().PickPosition(_("Pick point on axis"), data.m_pos1);
			dlg.SetPos1(data.m_pos1);
			dlg.UpdateSecondaryPoint();
			dlg.GetData(data);
		}
		else if(ret == ID_BUTTON_PICK_POS2)
		{
			dlg.GetData(data);
			wxGetApp().PickPosition(_("Pick secondary point on axis"), data.m_pos2);
		}
	}
}

void HeeksCADapp::SectioningDialog()
{
	// pick items
	if(wxGetApp().m_marked_list->size() == 0){
		wxGetApp().PickObjects(_("Pick objects to section"));
	}
	if(wxGetApp().m_marked_list->size() == 0)return;

	SectionObjectsWithDialog(wxGetApp().m_marked_list->list());
}
