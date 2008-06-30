// Gripper.h

#if !defined Gripper_HEADER
#define Gripper_HEADER

#include "../interface/HeeksObj.h"

class Gripper: public HeeksObj{
private:

public:
	gp_Pnt position;
	std::string prompt;

	Gripper(const gp_Pnt& pos, const char* str);
	virtual ~Gripper(){}

	// HeeksObj's virtual functions
	int GetType()const{return GripperType;}
	void glCommands(bool select, bool marked, bool no_color);
	const char* GetTypeString(void)const{return "Gripper";}
	void ModifyByMatrix(const double* m);

	//Gripper's virtual functions
	virtual wxCursor* get_gripper_cursor(){return NULL;}
	virtual void OnFrontRender(){}
	virtual void OnRender(){}
	virtual bool OnGripperGrabbed(double* from){return false;}
	virtual void OnGripperMoved(const double* from, const double* to){}
	virtual void OnGripperReleased(const double* from, const double* to){}

	// member functions
	const char* get_gripper_prompt(){return prompt.c_str();}
};

#endif
