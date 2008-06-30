// Images.cpp

#include "stdafx.h"

#include "Images.h"
#include "../interface/HeeksObj.h"

Images::Images(){
	m_image_list = NULL;
}

int Images::Add(wxIcon* hicon)
{
	if(hicon == NULL)return -1;
	std::map<wxIcon*, int>::iterator FindIt;
	if (image_map.size()>0) FindIt = image_map.find(hicon);
	int image_index = 2;
	if (image_map.size() == 0 || FindIt == image_map.end())
	{
		image_index = m_image_list->Add(*hicon);
		FindIt = image_map.insert(std::pair<wxIcon*, int>(hicon, image_index)).first;
		image_index = FindIt->second;
	}
	else image_index = FindIt->second;
	return image_index;
}

int Images::GetImage(HeeksObj *object)
{
	if(m_image_list == NULL)
	{
		return -1;
	}
	return Add(object->GetIcon());
}

bool Images::InitializeImageList(int width, int height){
	if(m_image_list == NULL){
		m_image_list = new wxImageList(width, height);
		return true;
	}
	return false;
}
