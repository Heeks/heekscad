// CuboidDlg.cpp
// Copyright (c) 2014, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Cuboid.h"
#include "CuboidDlg.h"

enum
{
};

BEGIN_EVENT_TABLE(CuboidDlg, HeeksObjDlg)
END_EVENT_TABLE()

CuboidDlg::CuboidDlg(wxWindow *parent, HeeksObj* object, const wxString& title, bool top_level)
             : HeeksObjDlg(parent, object, title, false)
{
	leftControls.push_back(HControl(m_xyzPos = new XYZBoxes(this, _("Position"), _("X position"), _("Y position"), _("Z position")), wxEXPAND | wxALL));
	leftControls.push_back(HControl(m_xyzSize = new XYZBoxes(this, _("Size"), _("X width"), _("Y depth"), _("Z depth")), wxEXPAND | wxALL));

	if(top_level)
	{
		HeeksObjDlg::AddControlsAndCreate();
	}
}

void CuboidDlg::GetDataRaw(HeeksObj* object)
{
}

void CuboidDlg::SetFromDataRaw(HeeksObj* object)
{
}

void CuboidDlg::SetPicture(const wxString& name)
{
	HeeksObjDlg::SetPicture(name, _T("cuboid"));
}

void CuboidDlg::SetPictureByWindow(wxWindow* w)
{
}
