// NiceTextCtrl.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

// a dialog control to enter a number
class CDoubleCtrl: public wxTextCtrl
{
	wxString DoubleToString(double value);
	double m_factor;
public:
	CDoubleCtrl(wxWindow* parent, wxWindowID id = wxID_ANY, double factor = 1.0);
	double GetValue();
	void SetValue(double value);
};

// a dialog control to enter a number in the current units
class CLengthCtrl: public CDoubleCtrl
{
public:
	CLengthCtrl(wxWindow* parent, wxWindowID id = wxID_ANY);
};

// a dialog control to enter a list of ids, for sketches or solids
class CObjectIdsCtrl: public wxTextCtrl
{
public:
	CObjectIdsCtrl(wxWindow* parent, wxWindowID id = wxID_ANY);
#ifdef OP_SKETCHES_AS_CHILDREN
	void GetAddChildren(HeeksObj* object, int group_type);
	void SetFromChildren(HeeksObj* object, int group_type);
#else
	void GetIDList(std::list<int> &id_list);
	void SetFromIDList(std::list<int> &id_list);
#endif
};

