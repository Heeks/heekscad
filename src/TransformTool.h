// TransformTool.h

#pragma once
#include "../interface/Tool.h"

class TransformTool: public Tool{
private:
	double modify_matrix[16];
	double revert_matrix[16];
	HeeksObj *object;
	bool m_done_with_add_and_remove;

public:
	TransformTool(HeeksObj *o, const gp_Trsf &t, const gp_Trsf &i);
	~TransformTool(void);

	// Tool's virtual functions
	const wxChar* GetTitle();
	void Run();
	void RollBack();
	bool Undoable(){return true;}
};

class TransformObjectsTool: public Tool{
private:
	double modify_matrix[16];
	double revert_matrix[16];
	std::list<HeeksObj*> m_list;
	std::set<HeeksObj*> m_done_with_add_and_remove;

public:
	TransformObjectsTool(const std::list<HeeksObj*>& list, const gp_Trsf &t, const gp_Trsf &i);
	~TransformObjectsTool(void);

	// Tool's virtual functions
	const wxChar* GetTitle();
	void Run();
	void RollBack();
	bool Undoable(){return true;}
};
