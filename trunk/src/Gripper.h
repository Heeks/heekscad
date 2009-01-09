// Gripper.h

#if !defined Gripper_HEADER
#define Gripper_HEADER

#include "../interface/HeeksObj.h"
#include "gp_Pnt.hxx"

enum EnumGripperType{
	GripperTypeTranslate,
	GripperTypeRotate,
	GripperTypeRotateObject,
	GripperTypeScale,
	GripperTypeObjectScaleX,
	GripperTypeObjectScaleY,
	GripperTypeObjectScaleZ,
	GripperTypeObjectScaleXY,
	GripperTypeAngle,
	GripperTypeStretch
};

class Gripper: public HeeksObj{
private:

public:
	gp_Pnt position;
	wxString prompt;
	EnumGripperType m_gripper_type;

	Gripper(const gp_Pnt& pos, const wxChar* str, EnumGripperType gripper_type);
	virtual ~Gripper(){}

	// HeeksObj's virtual functions
	int GetType()const{return GripperType;}
	void glCommands(bool select, bool marked, bool no_color);
	const wxChar* GetTypeString(void)const{return _("Gripper");}
	bool ModifyByMatrix(const double* m);

	//Gripper's virtual functions
	virtual wxCursor* get_gripper_cursor(){return NULL;}
	virtual void OnFrontRender(){}
	virtual void OnRender(){}
	virtual bool OnGripperGrabbed(const std::list<HeeksObj*>& list, bool show_grippers_on_drag, double* from){return false;}
	virtual void OnGripperMoved(double* from, const double* to){}
	virtual void OnGripperReleased(const double* from, const double* to){}

	// member functions
	const wxChar* get_gripper_prompt(){return prompt.c_str();}
};

#endif
