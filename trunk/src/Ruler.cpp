// Ruler.cpp

#include "stdafx.h"
#include "Ruler.h"
#include "Gripper.h"
#include "../interface/Tool.h"
#include "HeeksFrame.h"
#include "ObjPropsCanvas.h"
#include "ToolImage.h"

void RulerMark::glCommands()
{
	glDisable(GL_POLYGON_OFFSET_FILL);
	double half_width = width/2;
	double dpos = (double)pos;

	if(wxGetApp().GetPixelScale() < 10)
	{
		// draw a line
		glBegin(GL_LINES);
		glVertex2d(dpos, 0.0);
		glVertex2d(dpos, -length);
		glEnd();
	}
	else
	{
		// draw triangles
		double p[5][3] = {
			{dpos, 0.0, 0.0},
			{dpos - half_width, -half_width, 0.0},
			{dpos - half_width, -length, 0.0},
			{dpos + half_width, -length, 0.0},
			{dpos + half_width, -half_width, 0.0}
		};

		glBegin(GL_TRIANGLES);
		glVertex3dv(p[0]);
		glVertex3dv(p[1]);
		glVertex3dv(p[4]);
		glVertex3dv(p[1]);
		glVertex3dv(p[2]);
		glVertex3dv(p[4]);
		glVertex3dv(p[2]);
		glVertex3dv(p[3]);
		glVertex3dv(p[4]);
		glEnd();
	}

	// draw text
	if(pos == 0)
	{
		wxString str = _T("cm");
		glPushMatrix();
		glTranslated(dpos + half_width, -length + 2.05, 0.0);
		glColor4ub(0, 0, 0, 255);
		wxGetApp().render_text(str);
		glPopMatrix();
	}
	else if(pos % 10 == 0)
	{
		float text_width, text_height;
		wxString str = wxString::Format(_T("%d"), pos/10);
		if(!wxGetApp().get_text_size(str, &text_width, &text_height))return;
		glPushMatrix();
		glTranslated(dpos - half_width - text_width, -length + 2.05, 0.0);
		wxGetApp().render_text(str);
		glPopMatrix();
	}
	glEnable(GL_POLYGON_OFFSET_FILL);
}

HRuler::HRuler(): m_gl_list(0)
{
	m_width = 25;
	m_length = 306;
	m_empty_length = 3;
}

void HRuler::GetFourCorners(gp_Pnt *point)
{
	point[0] = gp_Pnt(-m_empty_length, -m_width, 0);
	point[1] = gp_Pnt(m_length - m_empty_length, -m_width, 0);
	point[2] = gp_Pnt(m_length - m_empty_length, 0, 0);
	point[3] = gp_Pnt(-m_empty_length, 0, 0);
}

void HRuler::CalculateMarks(std::list<RulerMark> &marks)
{
	int num_mm = (int)(m_length - 2 * m_empty_length + 0.0001);

	for(int i = 0; i<= num_mm; i++)
	{
		RulerMark mark;
		if(i % 10 == 0)
		{
			// big mark
			mark.length = 3.0; 
			mark.width = 0.1;
		}
		else if(i % 5 == 0)
		{
			// medium mark
			mark.length = 2.0; 
			mark.width = 0.1;
		}
		else
		{
			// small mark
			mark.length = 1.0; 
			mark.width = 0.1;
		}

		mark.pos = i;

		marks.push_back(mark);
	}
}

void HRuler::glCommands(bool select, bool marked, bool no_color)
{
	double m[16];
	extract_transposed(m_trsf, m);
	glPushMatrix();
	glMultMatrixd(m);

	if(m_gl_list)
	{
		glCallList(m_gl_list);
	}
	else{
		m_gl_list = glGenLists(1);
		glNewList(m_gl_list, GL_COMPILE_AND_EXECUTE);

		// draw a filled white rectangle
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 0.0);
		glColor4ub(255, 255, 255, 120); // white
		glEnable(GL_BLEND);
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glDepthMask(0);
		gp_Pnt point[4];
		GetFourCorners(point);
		glBegin(GL_TRIANGLES);
		glVertex3d(point[0].X(), point[0].Y(), point[0].Z());
		glVertex3d(point[1].X(), point[1].Y(), point[1].Z());
		glVertex3d(point[2].X(), point[2].Y(), point[2].Z());
		glVertex3d(point[0].X(), point[0].Y(), point[0].Z());
		glVertex3d(point[2].X(), point[2].Y(), point[2].Z());
		glVertex3d(point[3].X(), point[3].Y(), point[3].Z());
		glEnd();
		glDisable(GL_BLEND);
		glDepthMask(1);

		// draw a black rectangle border
		glColor4ub(0, 0, 0, 255); // black

		glBegin(GL_LINE_STRIP);
		glVertex3d(point[0].X(), point[0].Y(), point[0].Z());
		glVertex3d(point[1].X(), point[1].Y(), point[1].Z());
		glVertex3d(point[2].X(), point[2].Y(), point[2].Z());
		glVertex3d(point[3].X(), point[3].Y(), point[3].Z());
		glVertex3d(point[0].X(), point[0].Y(), point[0].Z());
		glEnd();

		// draw the marks ( with their numbers )
		std::list<RulerMark> marks;
		CalculateMarks(marks);
		for(std::list<RulerMark>::iterator It = marks.begin(); It != marks.end(); It++)
		{
			RulerMark& mark = *It;
			mark.glCommands();
		}
		glEnable(GL_POLYGON_OFFSET_FILL);

		glEndList();
	}

	glPopMatrix();
}

void HRuler::KillGLLists()
{
	if (m_gl_list)
	{
		glDeleteLists(m_gl_list, 1);
		m_gl_list = 0;
	}
}

void HRuler::GetBox(CBox &box)
{
	gp_Pnt point[4];
	GetFourCorners(point);

	for(int i = 0; i<4; i++)
	{
		point[i].Transform(m_trsf);
		box.Insert(point[i].X(), point[i].Y(), point[i].Z());
	}
}

HeeksObj *HRuler::MakeACopy(void)const{ return new HRuler(*this);}

bool HRuler::ModifyByMatrix(const double *mat)
{
	m_trsf = make_matrix(mat) * m_trsf;
	return false;
}

void HRuler::GetGripperPositions(std::list<double> *list, bool just_for_endof)
{
	gp_Pnt point[4];
	GetFourCorners(point);

	for(int i = 0; i<4; i++)point[i].Transform(m_trsf);

	list->push_back(GripperTypeRotateObject);
	list->push_back(point[0].X());
	list->push_back(point[0].Y());
	list->push_back(point[0].Z());
	list->push_back(GripperTypeRotateObject);
	list->push_back(point[2].X());
	list->push_back(point[2].Y());
	list->push_back(point[2].Z());
}

class ResetRulerTool:public Tool{
	// reset ruler
private:
	static wxBitmap* m_bitmap;

public:
	void Run(){
		wxGetApp().m_ruler->m_trsf = gp_Trsf();
		wxGetApp().m_frame->m_properties->RefreshByRemovingAndAddingAll();
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("ResetRuler");}
	wxBitmap* Bitmap(){if(m_bitmap == NULL){wxString exe_folder = wxGetApp().GetExeFolder();m_bitmap = new wxBitmap(ToolImage(_T("resetruler")));}return m_bitmap;}
	const wxChar* GetToolTip(){return _("Reset the ruler");}
};
wxBitmap* ResetRulerTool::m_bitmap = NULL;

static ResetRulerTool reset_ruler_tool;

void HRuler::GetProperties(std::list<Property *> *list)
{
}

void HRuler::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	t_list->push_back(&reset_ruler_tool);
}

bool HRuler::GetScaleAboutMatrix(double *m)
{
	extract(m_trsf, m);
	return true;
}
