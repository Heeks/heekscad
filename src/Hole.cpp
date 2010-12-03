// Hole.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Hole.h"
#include "HPoint.h"

bool CHolePositions::CanAdd(HeeksObj* object)
{
	return ((object != NULL) && (object->GetType() == HolePositionsType));
}

CHolePositions & CHolePositions::operator= ( const CHolePositions & rhs )
{
	if (this != &rhs)
	{
		ObjList::operator=( rhs );
	}

	return(*this);
}

CHolePositions::CHolePositions( const CHolePositions & rhs ) : ObjList(rhs)
{
}

const wxBitmap &CHolePositions::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/holeposns.png")));
	return *icon;
}

void CHolePositions::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "HolePositions" );
	root->LinkEndChild( element );
	WriteBaseXML(element);
}

//static
HeeksObj* CHolePositions::ReadFromXMLElement(TiXmlElement* pElem)
{
	CHolePositions* new_object = new CHolePositions;
	new_object->ReadBaseXML(pElem);
	return new_object;
}

bool CHole::Add(HeeksObj* object, HeeksObj* prev_object)
{
	switch(object->GetType())
	{
	case SketchType:
		m_profile_sketch = (CSketch*)object;
		break;
	case HolePositionsType:
		m_positions = (CHolePositions*)object;
		break;
	}

	return ObjList::Add(object, prev_object);
}

void CHole::Remove(HeeksObj* object)
{
	// set the m_tags pointer to NULL, when Tags is removed from here
	if(object == m_profile_sketch)m_profile_sketch = NULL;
	if(object == m_positions)m_positions = NULL;

	ObjList::Remove(object);
}

static void renderCircle()
{
	// renders 20 lines in a circle radius 1.0 in XY plane, with centre of 0,0,0
	glBegin(GL_LINE_STRIP);
	glVertex2d(1.0, 0.0);
	glVertex2d(0.951056516295, 0.309016994375);
	glVertex2d(0.809016994375, 0.587785252292);
	glVertex2d(0.587785252292, 0.809016994375);
	glVertex2d(0.309016994375, 0.951056516295);
	glVertex2d(0.0, 1.0);
	glVertex2d(-0.309016994375, 0.951056516295);
	glVertex2d(-0.587785252292, 0.809016994375);
	glVertex2d(-0.809016994375, 0.587785252292);
	glVertex2d(-0.951056516295, 0.309016994375);
	glVertex2d(-1.0, 0.0);
	glVertex2d(-0.951056516295, -0.309016994375);
	glVertex2d(-0.809016994375, -0.587785252292);
	glVertex2d(-0.587785252292, -0.809016994375);
	glVertex2d(-0.309016994375, -0.951056516295);
	glVertex2d(0.0, -1.0);
	glVertex2d(0.309016994375, -0.951056516295);
	glVertex2d(0.587785252292, -0.809016994375);
	glVertex2d(0.809016994375, -0.587785252292);
	glVertex2d(0.951056516295, -0.309016994375);
	glVertex2d(1.0, 0.0);
	glEnd();
}

void CHole::renderHole(bool marked, bool no_color)
{
	// renders one hole untransformed
	if(m_profile_sketch == NULL)return;

	for(HeeksObj* object = m_profile_sketch->GetFirstChild(); object; object = m_profile_sketch->GetNextChild())
	{
		double pos[3];
		if(object->GetStartPoint(pos))
		{
			glPushMatrix();
			glTranslated(pos[0], pos[1], pos[2]);
			double dScale = this->m_diameter/2;
			glScaled(dScale, dScale, dScale);
			renderCircle();
			glPopMatrix();
		}
	}
}

void CHole::glCommands(bool select, bool marked, bool no_color)
{
	// render the points where they are
	m_positions->glCommands(select, marked, no_color);

	// render the hole at each of the positions
	for(HeeksObj* object = m_positions->GetFirstChild(); object; object = m_positions->GetNextChild())
	{
		if(object->GetType() == PointType)
		{
			glPushMatrix();
			glTranslated(((HPoint*)object)->m_p.X(), ((HPoint*)object)->m_p.Y(), ((HPoint*)object)->m_p.Z());
			renderHole(marked, no_color);
		}
	}
}

const wxBitmap &CHole::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/hole.png")));
	return *icon;
}
