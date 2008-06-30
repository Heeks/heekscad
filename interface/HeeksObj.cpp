// HeeksObj.cpp

#include "stdafx.h"
#include "HeeksObj.h"
#include "PropertyString.h"

static HeeksObj* object_for_edit_string = NULL;

void on_edit_string(const char* value)
{
	object_for_edit_string->OnEditString(value);

	// to do , reconnect these two
//	wxGetApp().WasModified(object_for_edit_string);
//	wxGetApp().Repaint();
}

void HeeksObj::GetProperties(std::list<Property *> *list)
{
	bool editable = CanEditString();
	object_for_edit_string = this;
	list->push_back(new PropertyString("object type", GetTypeString(), NULL));
	if(GetShortString())list->push_back(new PropertyString("object title", GetShortString(), editable ? on_edit_string : NULL));
}
