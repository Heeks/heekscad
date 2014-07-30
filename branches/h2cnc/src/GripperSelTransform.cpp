// GripperSelTransform.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "GripperSelTransform.h"
#include "../interface/Tool.h"
#include "MarkedList.h"
#include "DigitizeMode.h"
#include "StretchTool.h"
#include "HeeksFrame.h"
#include "GraphicsCanvas.h"
#include "ObjPropsCanvas.h"
#include "Sketch.h"
#include "EndedObject.h"

GripperSelTransform::GripperSelTransform(const GripData& data, HeeksObj* parent):Gripper(data, parent){
}

bool GripperSelTransform::OnGripperGrabbed(const std::list<HeeksObj*>& list, bool show_grippers_on_drag, double* from){
	wxGetApp().CreateUndoPoint();

	m_initial_grip_pos[0] = m_data.m_x;
	m_initial_grip_pos[1] = m_data.m_y;
	m_initial_grip_pos[2] = m_data.m_z;

	memcpy(m_from, from, 3*sizeof(double));
	memcpy(m_last_from, from, 3*sizeof(double));
	wxGetApp().m_marked_list->gripping = true;
	m_items_marked_at_grab.clear();
	std::list<HeeksObj *>::const_iterator It;
	for(It = list.begin(); It != list.end(); It++){
		m_items_marked_at_grab.push_back(*It);
		wxGetApp().m_marked_list->set_ignore_onoff(*It, true);
	}
	if ( m_data.m_type <= GripperTypeObjectScaleXY )
	{
		wxGetApp().CreateTransformGLList(list, show_grippers_on_drag);
		wxGetApp().m_drag_matrix = gp_Trsf();
		for(It = list.begin(); It != list.end(); It++){
			HeeksObj* object = *It;
			if(object->m_visible)wxGetApp().m_hidden_for_drag.push_back(object);
			object->m_visible = false;
		}
	}
	return true;
}


void GripperSelTransform::OnGripperMoved( double* from, const double* to ){
	if ( m_data.m_type == GripperTypeStretch)
	{
		bool stretch_done = false;

		double shift[3];
		if(m_data.m_move_relative){
			shift[0] = to[0] - from[0];
			shift[1] = to[1] - from[1];
			shift[2] = to[2] - from[2];
		}
		else{
			shift[0] = to[0] - m_initial_grip_pos[0];
			shift[1] = to[1] - m_initial_grip_pos[1];
			shift[2] = to[2] - m_initial_grip_pos[2];
		}

		{
			std::list<HeeksObj *>::iterator It;
			for ( It = m_items_marked_at_grab.begin(); It != m_items_marked_at_grab.end(); It++ )
			{
				HeeksObj* object = *It;
				if(object)
				{
					double p[3] = {m_data.m_x, m_data.m_y, m_data.m_z};
					stretch_done = object->StretchTemporaryTransformed(p, shift,m_data.m_data);
				}
			}
		}
		
		if(stretch_done)
		{
			m_data.m_x += shift[0];
			m_data.m_y += shift[1];
			m_data.m_z += shift[2];
			from[0] += shift[0];
			from[1] += shift[1];
			from[2] += shift[2];
			m_initial_grip_pos[0] += shift[0];
			m_initial_grip_pos[1] += shift[1];
			m_initial_grip_pos[2] += shift[2];
		}

		wxGetApp().Repaint(true);
		return;
	}

	double object_m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

	if(m_items_marked_at_grab.size() > 0)m_items_marked_at_grab.front()->GetScaleAboutMatrix(object_m);

	MakeMatrix ( from, to, object_m, wxGetApp().m_drag_matrix );

	if(wxGetApp().m_sketch_mode && wxGetApp().m_sketch->m_coordinate_system)
	{
		wxGetApp().m_drag_matrix = wxGetApp().m_sketch->m_coordinate_system->GetMatrix() * wxGetApp().m_drag_matrix;
	}

	wxGetApp().Repaint();
}

void GripperSelTransform::OnGripperReleased ( const double* from, const double* to )
{
	wxGetApp().DestroyTransformGLList();

	for ( std::list<HeeksObj *>::iterator It = m_items_marked_at_grab.begin(); It != m_items_marked_at_grab.end(); It++ )
	{
		HeeksObj* object = *It;
		if ( object == m_gripper_parent && m_data.m_type > GripperTypeScale )
		{
			double shift[3];
			if(m_data.m_move_relative){
				shift[0] = to[0] - from[0];
				shift[1] = to[1] - from[1];
				shift[2] = to[2] - from[2];
			}
			else{
				shift[0] = to[0] - m_initial_grip_pos[0];
				shift[1] = to[1] - m_initial_grip_pos[1];
				shift[2] = to[2] - m_initial_grip_pos[2];
			}

			{
				if(object)wxGetApp().DoToolUndoably(new StretchTool(object, m_initial_grip_pos, shift, m_data.m_data));
			}
			m_data.m_x += shift[0];
			m_data.m_y += shift[1];
			m_data.m_z += shift[2];
		}
		else
		{
			gp_Trsf mat;
			double object_m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
			if(m_items_marked_at_grab.size() > 0)m_items_marked_at_grab.front()->GetScaleAboutMatrix(object_m);
			MakeMatrix ( from, to, object_m, mat );
			double m[16];
			extract(mat, m );
			object->ModifyByMatrix(m);
		}
	}

	m_items_marked_at_grab.clear();

	if ( m_data.m_type <= GripperTypeObjectScaleXY )
	{
		for(std::list<HeeksObj*>::iterator It = wxGetApp().m_hidden_for_drag.begin(); It != wxGetApp().m_hidden_for_drag.end(); It++)
		{
			HeeksObj* object = *It;
			object->m_visible = true;
		}
		wxGetApp().m_hidden_for_drag.clear();
	}

	{
		std::list<HeeksObj *>::iterator It;
		for ( It = m_items_marked_at_grab.begin(); It != m_items_marked_at_grab.end(); It++ )
		{
			wxGetApp().m_marked_list->set_ignore_onoff ( *It, false );
		}
	}
	wxGetApp().m_marked_list->gripping = false;
	wxGetApp().Changed();
}

void GripperSelTransform::MakeMatrix ( const double* from, const double* to, const double* object_m, gp_Trsf& mat )
{
	mat = gp_Trsf();
	switch ( m_data.m_type )
	{
	case GripperTypeTranslate:
		mat.SetTranslation ( gp_Vec ( make_point ( from ), make_point ( to ) ) );
		break;
	case GripperTypeScale:
		{
			gp_Trsf object_mat = make_matrix(object_m);
			gp_Pnt scale_centre_point = gp_Pnt(0, 0, 0).Transformed(object_mat);
			double dist = make_point ( from ).Distance ( scale_centre_point );
			if ( dist<0.00000001 )
			{
				return;
			}
			double scale = make_point ( to ).Distance ( scale_centre_point ) /dist;
			mat.SetScale( scale_centre_point, scale );
		}
		break;
	case GripperTypeObjectScaleX:
		{
			gp_Trsf object_mat = make_matrix(object_m);
			gp_Vec object_x = gp_Vec(1, 0, 0).Transformed(object_mat).Normalized();
			gp_Pnt scale_centre_point = gp_Pnt(0, 0, 0).Transformed(object_mat);
			double old_x = make_vector(from) * object_x - gp_Vec(scale_centre_point.XYZ()) * object_x;
			double new_x = make_vector(to) * object_x - gp_Vec(scale_centre_point.XYZ()) * object_x;
			if(fabs(old_x) < 0.000000001)return;
			double scale = new_x/old_x;
			double m[16] = {scale, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
			mat = object_mat * make_matrix(m) * object_mat.Inverted();
		}
		break;
	case GripperTypeObjectScaleY:
		{
			gp_Trsf object_mat = make_matrix(object_m);
			gp_Vec object_y = gp_Vec(0, 1, 0).Transformed(object_mat).Normalized();
			gp_Pnt scale_centre_point = gp_Pnt(0, 0, 0).Transformed(object_mat);
			double old_y = make_vector(from) * object_y - gp_Vec(scale_centre_point.XYZ()) * object_y;
			double new_y = make_vector(to) * object_y - gp_Vec(scale_centre_point.XYZ()) * object_y;
			if(fabs(old_y) < 0.000000001)return;
			double scale = new_y/old_y;
			double m[16] = {1, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
			mat = object_mat * make_matrix(m) * object_mat.Inverted();
		}
		break;
	case GripperTypeObjectScaleZ:
		{
			gp_Trsf object_mat = make_matrix(object_m);
			gp_Vec object_z = gp_Vec(0, 0, 1).Transformed(object_mat).Normalized();
			gp_Pnt scale_centre_point = gp_Pnt(0, 0, 0).Transformed(object_mat);
			double old_z = make_vector(from) * object_z - gp_Vec(scale_centre_point.XYZ()) * object_z;
			double new_z = make_vector(to) * object_z - gp_Vec(scale_centre_point.XYZ()) * object_z;
			if(fabs(old_z) < 0.000000001)return;
			double scale = new_z/old_z;
			double m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1};
			mat = object_mat * make_matrix(m) * object_mat.Inverted();
		}
		break;
	case GripperTypeObjectScaleXY:
		{
			gp_Trsf object_mat = make_matrix(object_m);
			gp_Vec object_x = gp_Vec(1, 0, 0).Transformed(object_mat).Normalized();
			gp_Pnt scale_centre_point = gp_Pnt(0, 0, 0).Transformed(object_mat);
			double old_x = make_vector(from) * object_x - gp_Vec(scale_centre_point.XYZ()) * object_x;
			double new_x = make_vector(to) * object_x - gp_Vec(scale_centre_point.XYZ()) * object_x;
			if(fabs(old_x) < 0.000000001)return;
			double scale = new_x/old_x;
			double m[16] = {scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, scale};
			mat = object_mat * make_matrix(m) * object_mat.Inverted();
		}
		break;
	case GripperTypeRotate:
	case GripperTypeRotateObject:
	case GripperTypeRotateObjectXY:
	case GripperTypeRotateObjectXZ:
	case GripperTypeRotateObjectYZ:
		{
			gp_Trsf object_mat = make_matrix(object_m);
			gp_Pnt rotate_centre_point = gp_Pnt(0, 0, 0).Transformed(object_mat);
			gp_Vec start_to_end_vector(make_point(from), make_point(to));
			if ( start_to_end_vector.Magnitude() <0.000001 ) return;
			gp_Vec start_vector ( rotate_centre_point, make_point ( from ) );
			gp_Vec end_vector ( rotate_centre_point, make_point ( to ) );
			if ( start_vector.Magnitude() <0.000001 ) return;
			if ( end_vector.Magnitude() <0.000001 ) return;
			mat.SetTranslation ( -gp_Vec ( rotate_centre_point.XYZ() ) );

			gp_Vec vx, vy;
			wxGetApp().m_current_viewport->m_view_point.GetTwoAxes(vx, vy, false, 0);			
			gp_Vec rot_dir = vx ^ vy;
			rot_dir.Normalize();

			if(m_data.m_type == GripperTypeRotateObjectXY){
				// use object z axis
				gp_Vec object_x = gp_Vec(1, 0, 0).Transformed(object_mat).Normalized();
				gp_Vec object_y = gp_Vec(0, 1, 0).Transformed(object_mat).Normalized();
				gp_Vec object_z = gp_Vec(0, 0, 1).Transformed(object_mat).Normalized();
				rot_dir = object_z;
				vx = object_x;
				vy = object_y;
			}

			else if(m_data.m_type == GripperTypeRotateObjectXZ){
				// use object y axis
				gp_Vec object_x = gp_Vec(1, 0, 0).Transformed(object_mat).Normalized();
				gp_Vec object_y = gp_Vec(0, 1, 0).Transformed(object_mat).Normalized();
				gp_Vec object_z = gp_Vec(0, 0, 1).Transformed(object_mat).Normalized();
				rot_dir = object_y;
				vx = object_z;
				vy = object_x;
			}

			else if(m_data.m_type == GripperTypeRotateObjectYZ){
				// use object x axis
				gp_Vec object_x = gp_Vec(1, 0, 0).Transformed(object_mat).Normalized();
				gp_Vec object_y = gp_Vec(0, 1, 0).Transformed(object_mat).Normalized();
				gp_Vec object_z = gp_Vec(0, 0, 1).Transformed(object_mat).Normalized();
				rot_dir = object_x;
				vx = object_y;
				vy = object_z;
			}

			else if(m_data.m_type == GripperTypeRotateObject){
				// choose the closest object axis to use
				gp_Vec object_x = gp_Vec(1, 0, 0).Transformed(object_mat).Normalized();
				gp_Vec object_y = gp_Vec(0, 1, 0).Transformed(object_mat).Normalized();
				gp_Vec object_z = gp_Vec(0, 0, 1).Transformed(object_mat).Normalized();

				double dpx = fabs(rot_dir * object_x);
				double dpy = fabs(rot_dir * object_y);
				double dpz = fabs(rot_dir * object_z);
				if(dpx > dpy && dpx > dpz){
					// use object x axis
					rot_dir = object_x;
					vx = object_y;
					vy = object_z;
				}
				else if(dpy > dpz){
					// use object y axis
					rot_dir = object_y;
					vx = object_z;
					vy = object_x;
				}
				else{
					// use object z axis
					rot_dir = object_z;
					vx = object_x;
					vy = object_y;
				}
			}

			gp_Ax1 rot_axis(rotate_centre_point, rot_dir);
			double sx = start_vector * vx;
			double sy = start_vector * vy;
			double ex = end_vector * vx;
			double ey = end_vector * vy;
			double angle = gp_Vec(sx, sy, 0).AngleWithRef(gp_Vec(ex, ey, 0), gp_Vec(0,0,1));
			mat.SetRotation(rot_axis, angle);
		}
		break;
	default:
		break;
	}
}
