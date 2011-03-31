
#ifndef __Sectioning__
#define __Sectioning__

///////////////////////////////////////////////////////////////////////////

#include "../interface/HDialogs.h"
#include "../interface/NiceTextCtrl.h"

///////////////////////////////////////////////////////////////////////////////
/// Class SectioningDlg
///////////////////////////////////////////////////////////////////////////////

class SectioningData
{
public:
	double m_pos1[3];
	double m_pos2[3];
	double m_angle;

	void ReadFromConfig();
	void WriteConfig();
	int GetAxisType();
	bool Validate();
	void GetOrigin(gp_Trsf &trsf);
};

class SectioningDlg : public HDialog 
{
	protected:
		wxStaticText* m_staticText1;
		wxRadioButton* m_radioBtn1;
		wxRadioButton* m_radioBtn2;
		wxRadioButton* m_radioBtn3;
		wxRadioButton* m_radioBtn4;
		wxStaticText* m_staticText3;
		wxButton* m_button1;
		wxButton* m_button_zero;
		wxStaticText* m_staticText2;
		wxStaticText* m_staticText4;
		wxStaticText* m_staticText5;
		CLengthCtrl* m_textCtrl1;
		CLengthCtrl* m_textCtrl2;
		CLengthCtrl* m_textCtrl3;
		wxStaticText* m_staticText12;
		wxButton* m_button2;
		wxStaticText* m_staticText9;
		wxStaticText* m_staticText10;
		wxStaticText* m_staticText11;
		CLengthCtrl* m_textCtrl4;
		CLengthCtrl* m_textCtrl5;
		CLengthCtrl* m_textCtrl6;
		wxStaticText* m_staticText13;
		CDoubleCtrl* m_textCtrl7;
	
	public:
		SectioningDlg( wxWindow* parent, SectioningData &data );
		~SectioningDlg();
	
		void OnRadioButtonX(wxCommandEvent& event);
		void OnRadioButtonY(wxCommandEvent& event);
		void OnRadioButtonZ(wxCommandEvent& event);
		void OnRadioButtonSpecify(wxCommandEvent& event);
		void OnPickPos1(wxCommandEvent& event);
		void OnButtonZero(wxCommandEvent& event);
		void OnPickPos2(wxCommandEvent& event);
		void OnOK(wxCommandEvent& event);

		void EnableSecondaryPointControls(bool bEnable);
		void UpdateSecondaryPoint();
		void SetFromData(SectioningData &data);
		void SetPos1(const double* pos);
		void GetData(SectioningData &data);

		DECLARE_EVENT_TABLE()
};

#endif //__Sectioning__
