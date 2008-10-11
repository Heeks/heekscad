// GripperSelTransform.h

#if !defined GripperSelTransform_HEADER
#define GripperSelTransform_HEADER

#include "Gripper.h"

class MarkedList;

class GripperSelTransform:public Gripper{
public:
	MarkedList* m_marked_list;
	double m_from[3];
	double m_last_from[3];
	double m_initial_grip_pos[3];
	std::list<HeeksObj *> m_items_marked_at_grab;
	int m_transform_gl_list;
	gp_Trsf m_drag_matrix;

	GripperSelTransform(MarkedList* m, const gp_Pnt& pos, EnumGripperType gripper_type);

	// HeeksObj's virtual functions
	HeeksObj *MakeACopy(void)const{ return new GripperSelTransform(*this);}

	// virtual functions
	void MakeMatrix(const double* from, const double* to, const double* about, const double* x_axis, const double* y_axis, gp_Trsf& mat);

	//Gripper's virtual functions
	void OnGripperMoved( const double* from, const double* to );
	bool OnGripperGrabbed(double* from);
	void OnGripperReleased(const double* from, const double* to);
	void OnRender();

	void DestroyGLList();
	void CreateGLList();
};

#endif
