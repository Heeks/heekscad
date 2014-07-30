// SolidTools.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "SolidTools.h"
#include "MarkedList.h"
#include "HeeksConfig.h"
#include "HeeksFrame.h"

void GetSolidMenuTools(std::list<Tool*>* t_list){
	// Tools for multiple selected items.
	bool solids_in_marked_list = false;

	// check to see what types have been marked
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case SolidType:
			case StlSolidType:
				solids_in_marked_list = true;
		}
	}

	if(solids_in_marked_list)
	{
		t_list->push_back(new SaveSolids);
	}
}

void SaveSolids::Run(){
	std::list<HeeksObj*> objects;

	for(std::list<HeeksObj*>::const_iterator It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType())
		{
		case SolidType:
		case StlSolidType:
			{
				objects.push_back(object);
			}
			break;
		}
	}

	if(objects.size() > 0)
	{
		wxString filepath(_T(""));
		{
			// get last used filepath
			HeeksConfig config;
			config.Read(_T("SolidExportFilepath"), &filepath, _T(""));
		}

		wxFileDialog fd(wxGetApp().m_frame, _("Save solid file"), wxEmptyString, filepath, wxString(_("Solid Files")) + _T(" |*.igs;*.iges;*.stp;*.step;*.stl;*.cpp;*.py|") + _("IGES files") + _T(" (*.igs *.iges)|*.igs;*.iges|") + _("STEP files") + _T(" (*.stp *.step)|*.stp;*.step|") + _("STL files") + _T(" (*.stl)|*.stl|") + _("CPP files") + _T(" (*.cpp)|*.cpp|") + _("OpenCAMLib python files") + _T(" (*.py)|*.py"), wxSAVE|wxOVERWRITE_PROMPT);
		fd.SetFilterIndex(0);
		if (fd.ShowModal() == wxID_CANCEL)return;
		filepath = fd.GetPath();

		wxString wf(filepath);
		wf.LowerCase();

		if(wf.EndsWith(_T(".stl")))
		{
			wxGetApp().SaveSTLFile(objects, filepath, -1.0, NULL, wxGetApp().m_stl_save_as_binary);
		}
		else if(wf.EndsWith(_T(".cpp")))
		{
			wxGetApp().SaveCPPFile(objects, filepath);
		}
		else if(wf.EndsWith(_T(".py")))
		{
			wxGetApp().SavePyFile(objects, filepath);
		}
		else if(CShape::ExportSolidsFile(objects, filepath))
		{
		}
		else
		{
			wxMessageBox(_("Invalid solid file type chosen"));
			return;
		}

		{
			// save last used filepath
			HeeksConfig config;
			config.Write(_T("SolidExportFilepath"), filepath);
		}
	}
}

