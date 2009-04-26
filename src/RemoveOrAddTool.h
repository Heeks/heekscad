// RemoveOrAddTool.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "../interface/Tool.h"

class RemoveOrAddTool:public Tool{
protected:
	HeeksObj* m_prev_object;
	bool m_belongs_to_owner;

	void Add();
	void Remove();

public:
	HeeksObj* m_owner;
	HeeksObj* m_object;

	RemoveOrAddTool(HeeksObj *object, HeeksObj *owner, HeeksObj* prev_object);
	virtual ~RemoveOrAddTool();

	// Tool's virtual functions
	bool Undoable(){return true;}
};

class AddObjectTool:public RemoveOrAddTool{
private:

public:
	AddObjectTool(HeeksObj *object, HeeksObj* owner, HeeksObj* prev_object):RemoveOrAddTool(object, owner, prev_object){}

	// Tool's virtual functions
	const wxChar* GetTitle();
	void Run();
	void RollBack();
};

class RemoveObjectTool:public RemoveOrAddTool{
public:
	RemoveObjectTool(HeeksObj *object);

	// Tool's virtual functions
	const wxChar* GetTitle() {return _("Remove");}
	void Run();
	void RollBack();
	wxString BitmapPath(){return _T("delete");}
};

class ManyRemoveOrAddTool:public Tool{
protected:
	std::list<HeeksObj*> m_objects;
	HeeksObj* m_owner;
	bool m_belongs_to_owner;

	void Add();
	void Remove();

public:
	ManyRemoveOrAddTool(const std::list<HeeksObj*> &list, HeeksObj *owner): m_objects(list), m_owner(owner), m_belongs_to_owner(false){}
	virtual ~ManyRemoveOrAddTool();

	// Tool's virtual functions
	bool Undoable(){return true;}
};

class AddObjectsTool:public ManyRemoveOrAddTool{
public:
	AddObjectsTool(const std::list<HeeksObj*> &list, HeeksObj *owner):ManyRemoveOrAddTool(list, owner){}

	// Tool's virtual functions
	const wxChar* GetTitle();
	void Run();
	void RollBack();
};

class RemoveObjectsTool:public ManyRemoveOrAddTool{
public:
	RemoveObjectsTool(const std::list<HeeksObj*> &list, HeeksObj *owner):ManyRemoveOrAddTool(list, owner){}

	// Tool's virtual functions
	const wxChar* GetTitle();
	void Run();
	void RollBack();
};

class ChangeOwnerTool:public Tool{
protected:
	HeeksObj* m_prev_owner;
	HeeksObj* m_new_owner;
	HeeksObj* m_object;

public:
	ChangeOwnerTool(HeeksObj *object, HeeksObj *prev_owner, HeeksObj* new_owner):m_object(object), m_prev_owner(prev_owner), m_new_owner(new_owner){}

	// Tool's virtual functions
	const wxChar* GetTitle() {return _("ChangeOwner");}
	void Run();
	void RollBack();
	bool Undoable(){return true;}
};

class ManyChangeOwnerTool:public Tool{
protected:
	std::list<HeeksObj*> m_objects;
	std::list<HeeksObj*> m_prev_owners;
	HeeksObj* m_new_owner;

public:
	ManyChangeOwnerTool(const std::list<HeeksObj*> &list, HeeksObj* new_owner);

	// Tool's virtual functions
	const wxChar* GetTitle() {return _("ManyChangeOwner");}
	void Run();
	void RollBack();
	bool Undoable(){return true;}
};
