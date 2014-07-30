// StretchTool.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#if !defined StretchTool_HEADER
#define StretchTool_HEADER

#include "../interface/Tool.h"

class StretchTool: public Tool{
private:
	double m_pos[3];
	double m_shift[3];
	double m_new_pos[3];
	void* m_data;
	HeeksObj *m_object;
	bool m_undo_uses_add;

public:
	StretchTool(HeeksObj *object, const double *p, const double* shift, void* data);
	~StretchTool(void);

	// Tool's virtual functions
	const wxChar* GetTitle();
	void Run();
	void RollBack();
	bool Undoable(){return true;}
};

#endif
