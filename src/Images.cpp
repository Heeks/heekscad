// Images.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "Images.h"
#include "../interface/HeeksObj.h"

Images::Images(){
	m_image_list = NULL;
}

int Images::GetImage(HeeksObj *object)
{
	int image_index = -1;

	if(m_image_list)
	{
		std::map<wxString, int>::iterator FindIt;

		wxString icon_str = object->GetIcon();

		if (image_map.size()>0) FindIt = image_map.find(icon_str);
		if (image_map.size() == 0 || FindIt == image_map.end())
		{
			image_index = m_image_list->Add(wxIcon(icon_str + _T(".png"), wxBITMAP_TYPE_PNG));
			FindIt = image_map.insert(std::pair<wxString, int>(icon_str, image_index)).first;
			image_index = FindIt->second;
		}
		else image_index = FindIt->second;
	}
	return image_index;
}

bool Images::InitializeImageList(int width, int height){
	if(m_image_list == NULL){
		m_image_list = new wxImageList(width, height);
		return true;
	}
	return false;
}
