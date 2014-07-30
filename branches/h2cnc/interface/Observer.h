// Observer.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class HeeksObj;

class Observer{
public:
	virtual ~Observer(){}

	virtual void OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified){}
	virtual void WhenMarkedListChanges(bool selection_cleared, const std::list<HeeksObj*>* added_list, const std::list<HeeksObj*>* removed_list){}
	virtual void Clear(){}
	virtual void Freeze(){}
	virtual void Thaw(){}
};
