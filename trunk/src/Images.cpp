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

	if(m_image_list && object->GetType() != UnknownType)
	{
		std::map<int, int>::iterator FindIt;
		if (image_map.size()>0) FindIt = image_map.find(object->GetType());
		if (image_map.size() == 0 || FindIt == image_map.end())
		{
			image_index = m_image_list->Add(wxIcon(wxGetApp().GetExeFolder() + _T("/icons/") + object->GetIcon() + _T(".png"), wxBITMAP_TYPE_PNG));
			FindIt = image_map.insert(std::pair<int, int>(object->GetType(), image_index)).first;
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
