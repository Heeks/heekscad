// Observer.h

#pragma once

class HeeksObj;

class Observer{
public:
	virtual ~Observer(){}

	virtual void OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified){}
	virtual void WhenMarkedListChanges(bool all_added, bool all_removed, const std::list<HeeksObj*>* added_list, const std::list<HeeksObj*>* removed_list){}
	virtual void Clear(){}
};
