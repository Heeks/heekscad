// StretchTool.h

#if !defined StretchTool_HEADER
#define StretchTool_HEADER

#include "../interface/Tool.h"

class StretchTool: public Tool{
private:
	double m_pos[3];
	double m_shift[3];
	double m_new_pos[3];
	HeeksObj *m_object;
	bool m_undo_uses_add;

public:
	StretchTool(HeeksObj *object, const double *p, const double* shift);
	~StretchTool(void);

	// Tool's virtual functions
	const wxChar* GetTitle();
	void Run();
	void RollBack();
	bool Undoable(){return true;}
};

#endif
