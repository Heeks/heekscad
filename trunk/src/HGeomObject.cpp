// HGeomObject.cpp

#include "stdafx.h"

#include "HGeomObject.h"
#include "icons/line.xpm"
#include "icons/arc.xpm"

wxIcon* line_icon = NULL;
wxIcon* arc_icon = NULL;

wxIcon* HGeomObject::GetIcon(){

	switch(GetType()){
	case LineType:
		{
			if(line_icon == NULL)line_icon = new wxIcon(line_xpm);
			return line_icon;
		}
	case ArcType:
		{
			if(arc_icon == NULL)arc_icon = new wxIcon(arc_xpm);
			return arc_icon;
		}
	}
	return NULL;
}

