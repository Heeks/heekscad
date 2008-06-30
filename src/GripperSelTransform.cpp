// GripperSelTransform.cpp

#include "stdafx.h"

#include "GripperSelTransform.h"
#include "../interface/Tool.h"
#include "MarkedList.h"
#include "DigitizeMode.h"
#include "GripperMode.h"
#include "StretchTool.h"

GripperSelTransform::GripperSelTransform(MarkedList* m, const gp_Pnt& pos):Gripper(pos, ""), m_marked_list(m), m_transform_gl_list(0){
}

bool GripperSelTransform::OnGripperGrabbed(double* from){
	extract(position, m_initial_grip_pos);
	memcpy(m_from, from, 3*sizeof(double));
	memcpy(m_last_from, from, 3*sizeof(double));
	m_marked_list->gripping = true;
	m_items_marked_at_grab.clear();
	std::list<HeeksObj *>::const_iterator It;
	for(It = m_marked_list->list().begin(); It != m_marked_list->list().end(); It++){
		m_items_marked_at_grab.push_back(*It);
		wxGetApp().m_marked_list->set_ignore_onoff(*It, true);
	}
	if ( wxGetApp().gripper_mode->m_mode != StretchMode )
	{
		CreateGLList();
		m_drag_matrix = gp_Trsf();
		wxGetApp().HideMarkedList();
	}
	return true;
}

void GripperSelTransform::DestroyGLList(){
	if (m_transform_gl_list)
	{
		glDeleteLists(m_transform_gl_list, 1);
	}
	m_transform_gl_list = 0;
}

void GripperSelTransform::CreateGLList(){
	DestroyGLList();
	m_transform_gl_list = glGenLists(1);
	glNewList(m_transform_gl_list, GL_COMPILE);
	std::list<HeeksObj *>::const_iterator It;
	for(It = m_marked_list->list().begin(); It != m_marked_list->list().end(); It++){
		(*It)->glCommands(false, true, false);
	}
	m_marked_list->GrippersGLCommands(false, false);
	glEndList();
}

void GripperSelTransform::OnGripperMoved( const double* from, const double* to ){
	if ( wxGetApp().gripper_mode->m_mode == StretchMode )
	{
		return;
	}
	MakeMatrix ( from, to, m_drag_matrix );
	wxGetApp().Repaint();
}

void GripperSelTransform::OnGripperReleased ( const double* from, const double* to )
{
	if ( wxGetApp().gripper_mode->m_mode == StretchMode )
	{
		wxGetApp().StartHistory ( "Stretch Marked List" );
		double shift[3] = {to[0] - from[0], to[1] - from[1], to[2] - from[2]};
		{
			std::list<HeeksObj *>::iterator It;
			for ( It = m_items_marked_at_grab.begin(); It != m_items_marked_at_grab.end(); It++ )
			{
				HeeksObj* object = *It;
				if(object)wxGetApp().DoToolUndoably(new StretchTool(object, m_initial_grip_pos, shift));
			}
		}
		wxGetApp().EndHistory();
		position = position.XYZ() + make_vector( shift ).XYZ();
	}
	else
	{
		gp_Trsf mat;
		MakeMatrix ( from, to, mat );
		double m[16];
		extract(mat, m );
		wxGetApp().StartHistory ( "Move Marked List" );
		std::list<HeeksObj*> marked_list = m_marked_list->list();
		std::list<HeeksObj *>::iterator It;
		for ( It = marked_list.begin(); It != marked_list.end(); It++ )
		{
			HeeksObj* object = *It;
			if ( wxGetApp().gripper_mode->m_make_copies )
			{
				HeeksObj *new_object = object;
				for ( int i = 0; i<wxGetApp().gripper_mode->m_number_of_copies; i++ )
				{
					new_object = new_object->MakeACopy();
					if ( new_object == NULL ) break;
					new_object->ModifyByMatrix ( m );
					wxGetApp().AddUndoably ( new_object, object->m_owner, NULL );
				}
			}
			else
			{
				wxGetApp().TransformUndoably( object, m );
			}
		}
		wxGetApp().EndHistory();
		wxGetApp().UnHideMarkedList();
	}
	{
		std::list<HeeksObj *>::iterator It;
		for ( It = m_items_marked_at_grab.begin(); It != m_items_marked_at_grab.end(); It++ )
		{
			wxGetApp().m_marked_list->set_ignore_onoff ( *It, false );
		}
	}
	m_items_marked_at_grab.clear();
	DestroyGLList();
	m_marked_list->gripping = false;
}

void GripperSelTransform::OnRender(){
	if(m_transform_gl_list)
	{
        glPushMatrix();
		double m[16];
		extract_transposed(m_drag_matrix, m);
		glMultMatrixd(m);
		glCallList(m_transform_gl_list);
		glPopMatrix();
	}
}

void GripperSelTransform::MakeMatrix ( const double* from, const double* to, gp_Trsf& mat )
{
	mat = gp_Trsf();
	switch ( wxGetApp().gripper_mode->m_mode )
	{
	case TranslationMode:
		mat.SetTranslation ( gp_Vec ( make_point ( from ), make_point ( to ) ) );
		break;
	case ScaleMode:
		{
			double dist = make_point ( from ).Distance ( wxGetApp().gripper_mode->m_centre_point );
			if ( dist<0.00000001 )
			{
				return;
			}
			double scale = make_point ( to ).Distance ( wxGetApp().gripper_mode->m_centre_point ) /dist;
			mat.SetScale( wxGetApp().gripper_mode->m_centre_point, scale );
		}
		break;
	case RotationMode:
		{
			gp_Vec start_to_end_vector(make_point(from), make_point(to));
			if ( start_to_end_vector.Magnitude() <0.000001 ) return;
			gp_Vec start_vector ( wxGetApp().gripper_mode->m_centre_point, make_point ( from ) );
			gp_Vec end_vector ( wxGetApp().gripper_mode->m_centre_point, make_point ( to ) );
			if ( start_vector.Magnitude() <0.000001 ) return;
			if ( end_vector.Magnitude() <0.000001 ) return;
			mat.SetTranslation ( -gp_Vec ( wxGetApp().gripper_mode->m_centre_point.XYZ() ) );

			gp_Vec rot_dir(0, 0, 1);
			rot_dir.Normalize();
			gp_Ax1 rot_axis(wxGetApp().gripper_mode->m_centre_point, rot_dir);
			double angle = start_vector.AngleWithRef(end_vector, rot_dir);
			mat.SetRotation(rot_axis, angle);
		}
		break;
	default:
		break;
	}
}
