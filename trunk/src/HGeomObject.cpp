// HGeomObject.cpp

#include "stdafx.h"

#include "HGeomObject.h"

wxIcon* line_icon = NULL;
wxIcon* arc_icon = NULL;

wxIcon* HGeomObject::GetIcon(){
	switch(GetType()){
	case LineType:
		{
			if(line_icon == NULL)
			{
				wxString exe_folder = wxGetApp().GetExeFolder();
				line_icon = new wxIcon(exe_folder + "/icons/line.png", wxBITMAP_TYPE_PNG);
			}
			return line_icon;
		}
	case ArcType:
		{
			if(arc_icon == NULL)
			{
				wxString exe_folder = wxGetApp().GetExeFolder();
				arc_icon = new wxIcon(exe_folder + "/icons/arc.png", wxBITMAP_TYPE_PNG);
			}
			return arc_icon;
		}
	}
	return NULL;
}

