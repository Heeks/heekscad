// Sketch.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Sketch.h"
#include "HLine.h"
#include "HArc.h"
#include "HeeksFrame.h"
#include "ObjPropsCanvas.h"
#include "../interface/PropertyInt.h"
#include "../interface/PropertyChoice.h"
#include "../interface/Tool.h"
#include "../tinyxml/tinyxml.h"

std::string CSketch::m_sketch_order_str[MaxSketchOrderTypes] = {
	std::string("unknown"),
	std::string("empty"),
	std::string("open"),
	std::string("reverse"),
	std::string("bad"),
	std::string("re-order"),
	std::string("clockwise"),
	std::string("counter-clockwise"),
	std::string("multiple")
};

CSketch::CSketch():m_order(SketchOrderTypeUnknown)
{
	m_title = _("Sketch");
}

CSketch::CSketch(const CSketch& c)
{
	operator=(c);
}

CSketch::~CSketch()
{
}

const CSketch& CSketch::operator=(const CSketch& c)
{
	// just copy all the lines and arcs, not the id
	ObjList::operator =(c);

	color = c.color;
	m_order = c.m_order;
	m_title = c.m_title;

	return *this;
}

static std::map<int, int> order_map_for_properties; // maps drop-down index to SketchOrderType

static void on_set_order_type(int value, HeeksObj* object)
{
	std::map<int, int>::iterator FindIt = order_map_for_properties.find(value);
	if(FindIt != order_map_for_properties.end())
	{
		int order = FindIt->second;
		if(((CSketch*)object)->ReOrderSketch((SketchOrderType)order))
		{
			wxGetApp().m_frame->m_properties->RefreshByRemovingAndAddingAll(false);
		}
	}
}

static bool SketchOrderAvailable(SketchOrderType old_order, SketchOrderType new_order)
{
	// can we change from the older order type to the new order type?
	if(old_order == new_order)return true;

	switch(old_order)
	{
	case SketchOrderTypeOpen:
		switch(new_order)
		{
		case SketchOrderTypeReverse:
			return true;
		default:
			break;
		}
		break;

	case SketchOrderTypeBad:
		switch(new_order)
		{
		case SketchOrderTypeReOrder:
			return true;
		default:
			break;
		}
		break;

	case SketchOrderTypeCloseCW:
		switch(new_order)
		{
		case SketchOrderTypeCloseCCW:
			return true;
		default:
			break;
		}
		break;

	case SketchOrderTypeCloseCCW:
		switch(new_order)
		{
		case SketchOrderTypeCloseCW:
			return true;
		default:
			break;
		}
		break;

	default:
		break;
	}

	return false;
}

void CSketch::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyInt(_("Number of elements"), ObjList::GetNumChildren(), this));

	int initial_index = 0;
	std::list< wxString > choices;
	SketchOrderType sketch_order = GetSketchOrder();
	order_map_for_properties.clear();
	int j = 0;
	for(int i = 0; i< MaxSketchOrderTypes; i++)
	{
		if((SketchOrderType)i == sketch_order)initial_index = j;

		if(SketchOrderAvailable(sketch_order, (SketchOrderType)i))
		{
			order_map_for_properties.insert(std::pair<int, int>(j, i));
			choices.push_back(Ctt(m_sketch_order_str[i].c_str()));
			j++;
		}
	}
		
	list->push_back ( new PropertyChoice ( _("order"), choices, initial_index, this, on_set_order_type ) );

	ObjList::GetProperties(list);
}

class SplitSketch:public Tool{
public:
	CSketch* m_sketch;
	SplitSketch():m_sketch(NULL){}
	void Run(){
		if(m_sketch == NULL)return;
		std::list<HeeksObj*> new_sketches;
		m_sketch->ExtractSeparateSketches(new_sketches);
		wxGetApp().StartHistory();
		for(std::list<HeeksObj*>::iterator It = new_sketches.begin(); It != new_sketches.end(); It++)
		{
			HeeksObj* new_object = *It;
			wxGetApp().AddUndoably(new_object, NULL, NULL);
		}
		wxGetApp().DeleteUndoably(m_sketch);
		wxGetApp().EndHistory();
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Split Sketch");}
	wxString BitmapPath(){return _T("splitsketch");}
};

static SplitSketch split_sketch;

void CSketch::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	if(GetSketchOrder() == SketchOrderTypeBad || GetSketchOrder() == SketchOrderTypeMultipleCurves)
	{
		split_sketch.m_sketch = this;
		t_list->push_back(&split_sketch);
	}
}

HeeksObj *CSketch::MakeACopy(void)const
{
	return (ObjList*)(new CSketch(*this));
}

void CSketch::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element = new TiXmlElement( "Sketch" );  
	root->LinkEndChild( element );
	element->SetAttribute("title", Ttc(m_title.c_str()));
	WriteBaseXML(element);
}

// static member function
HeeksObj* CSketch::ReadFromXMLElement(TiXmlElement* pElem)
{
	CSketch* new_object = new CSketch;
	const char* title = pElem->Attribute("title");
	if(title)new_object->m_title = wxString(Ctt(title));
	new_object->ReadBaseXML(pElem);

	return (ObjList*)new_object;
}

void CSketch::SetColor(const HeeksColor &col)
{
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
		object->SetColor(col);
	}
}

const HeeksColor* CSketch::GetColor()const
{
	if(m_objects.size() == 0)return NULL;
	return m_objects.front()->GetColor();
}

void CSketch::OnEditString(const wxChar* str){
	m_title.assign(str);
	wxGetApp().WasModified(this);
}

SketchOrderType CSketch::GetSketchOrder()
{
	if(m_order == SketchOrderTypeUnknown)CalculateSketchOrder();
	return m_order;
}

void CSketch::CalculateSketchOrder()
{
	if(m_objects.size() == 0)
	{
		m_order = SketchOrderTypeEmpty;
		return;
	}

	HeeksObj* prev_object = NULL;
	HeeksObj* first_object = NULL;

	bool well_ordered = true;

	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;

		if(object->GetType() == CircleType)
		{
			m_order = SketchOrderHasCircles;
			return;
		}

		if(prev_object)
		{
			double prev_e[3], s[3];
			if(!prev_object->GetEndPoint(prev_e)){well_ordered = false; break;}
			if(!object->GetStartPoint(s)){well_ordered = false; break;}
			if(!(make_point(prev_e).IsEqual(make_point(s), wxGetApp().m_geom_tol))){well_ordered = false; break;}
		}

		if(first_object == NULL)first_object = object;
		prev_object = object;
	}

	if(well_ordered)
	{
		if(prev_object && first_object)
		{
			double e[3], s[3];
			if(prev_object->GetEndPoint(e))
			{
				if(first_object->GetStartPoint(s))
				{
					if(make_point(e).IsEqual(make_point(s), wxGetApp().m_geom_tol))
					{
						// closed
						if(GetClosedSketchTurningNumber() > 0)m_order = SketchOrderTypeCloseCCW;
						else m_order = SketchOrderTypeCloseCW;
						return;
					}
				}
			}
		}

		m_order = SketchOrderTypeOpen;
		return;
	}

	m_order = SketchOrderTypeBad; // although it might still be multiple, but will have to wait until ReOrderSketch is done.
}

bool CSketch::ReOrderSketch(SketchOrderType new_order)
{
	SketchOrderType old_order = GetSketchOrder();
	bool done = false;

	switch(old_order)
	{
	case SketchOrderTypeOpen:
		switch(new_order)
		{
		case SketchOrderTypeReverse:
			ReverseSketch();
			done = true;
			break;
		default:
			break;
		}
		break;

	case SketchOrderTypeBad:
		switch(new_order)
		{
		case SketchOrderTypeBad:
			break;
		default:
			ReLinkSketch();
			done = true;
			break;
		}
		break;

	case SketchOrderTypeCloseCW:
		switch(new_order)
		{
		case SketchOrderTypeCloseCCW:
			ReverseSketch();
			done = true;
			break;
		default:
			break;
		}
		break;

	case SketchOrderTypeCloseCCW:
		switch(new_order)
		{
		case SketchOrderTypeCloseCW:
			ReverseSketch();
			done = true;
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}

	return done;
}

void CSketch::ReLinkSketch()
{
	CSketchRelinker relinker(m_objects);

	relinker.Do();

	wxGetApp().StartHistory();
	wxGetApp().DeleteUndoably(m_objects);
	for(std::list< std::list<HeeksObj*> >::iterator It = relinker.m_new_lists.begin(); It != relinker.m_new_lists.end(); It++)
	{
		wxGetApp().AddUndoably(*It, this);
	}
	wxGetApp().EndHistory();

	if(relinker.m_new_lists.size() > 1)
	{
		m_order = SketchOrderTypeMultipleCurves;
	}
	else
	{
		CalculateSketchOrder();
	}
}

void CSketch::ReverseSketch()
{
	if(m_objects.size() == 0)return;

	std::list<HeeksObj*> new_list;

	for(std::list<HeeksObj*>::iterator It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
		HeeksObj* copy = object->MakeACopy();

		// reverse object
		switch(object->GetType()){
			case LineType:
				((HLine*)copy)->Reverse();
				break;
			case ArcType:
				((HArc*)copy)->Reverse();
				break;
			default:
				break;
		}

		new_list.push_front(copy);
	}

	wxGetApp().StartHistory();
	wxGetApp().DeleteUndoably(m_objects);
	wxGetApp().AddUndoably(new_list, this);
	wxGetApp().EndHistory();
}

void CSketch::ExtractSeparateSketches(std::list<HeeksObj*> &new_separate_sketches)
{
	CSketch* re_ordered_sketch = NULL;
	CSketch* sketch = this;

	if(GetSketchOrder() == SketchOrderHasCircles)
	{
		std::list<HeeksObj*>::iterator It;
		for(It=m_objects.begin(); It!=m_objects.end() ;It++)
		{
			HeeksObj* object = *It;
			CSketch* new_object = new CSketch();
			new_object->color = color;
			new_object->Add(object->MakeACopy(), NULL);
			new_separate_sketches.push_back(new_object);
		}
	}

	if(GetSketchOrder() == SketchOrderTypeBad)
	{
		re_ordered_sketch = (CSketch*)(this->MakeACopy());
		re_ordered_sketch->ReOrderSketch(SketchOrderTypeReOrder);
		sketch = re_ordered_sketch;
	}
	if(sketch->GetSketchOrder() == SketchOrderTypeMultipleCurves)
	{

		CSketchRelinker relinker(sketch->m_objects);

		relinker.Do();

		for(std::list< std::list<HeeksObj*> >::iterator It = relinker.m_new_lists.begin(); It != relinker.m_new_lists.end(); It++)
		{
			std::list<HeeksObj*>& list = *It;
			CSketch* new_object = new CSketch();
			new_object->color = color;
			for(std::list<HeeksObj*>::iterator It2 = list.begin(); It2 != list.end(); It2++)
			{
				HeeksObj* object = *It2;
				new_object->Add(object->MakeACopy(), NULL);
			}
			new_separate_sketches.push_back(new_object);
		}
	}

	if(re_ordered_sketch)delete re_ordered_sketch;
}

static gp_Vec GetSegmentVector(HeeksObj* object, double fraction)
{
	switch(object->GetType())
	{
	case LineType:
		return ((HLine*)object)->GetSegmentVector(fraction);
	case ArcType:
		return ((HArc*)object)->GetSegmentVector(fraction);
	default:
		return gp_Vec(0, 0, 0);
	}
}

double CSketch::GetAngleAtJunction(HeeksObj* prev_object, HeeksObj* object)
{
	gp_Vec prev_end_vector = GetSegmentVector(prev_object, 1.0);
	gp_Vec start_vector = GetSegmentVector(object, 0.0);

	double prev_segment_curvature = GetSegmentCurvature(prev_object);
	double segment_curvature = GetSegmentCurvature(object);

	return GetAngleBetweenVectors(prev_end_vector, start_vector, prev_segment_curvature, segment_curvature);
}

double CSketch::GetAngleBetweenVectors(const gp_Vec& v0, const gp_Vec& v1, double prev_segment_curvature, double segment_curvature)
{
	double start_angle = atan2(v0.Y(), v0.X());
	double end_angle = atan2(v1.Y(), v1.X());
	if(end_angle < start_angle)end_angle += 6.2831853071795;

	double diff = end_angle - start_angle;
	if(fabs(diff - 3.1415926535897932) < 0.000000000001){
		// back on itself
		double curvature_diff = segment_curvature - prev_segment_curvature;
		if(fabs(curvature_diff) < 0.00000000001)
		{
			// not the best case where they come in and go out at exactly the same direction and curve
		}
		else
		{
			if(curvature_diff< 0)diff = -3.1415926535897932;
		}
	}
	else if(diff > 3.1415926535897932){
		diff -= 6.2831853071795; // anti-clockwise direction more than 180 degrees is a clockwise direction
	}

	return diff;
}

int CSketch::GetSegmentType(HeeksObj* object)
{
	if(object->GetType() == ArcType)
	{
		if(((HArc*)object)->m_circle.Axis().Direction().Z() > 0)return 1;
		return -1;
	}

	return 0;
}

double CSketch::GetSegmentCurvature(HeeksObj* object)
{
	if(object->GetType() == ArcType)
	{
		int dir = (((HArc*)object)->m_circle.Axis().Direction().Z() > 0) ? 1:-1;
		return 1 / ((HArc*)object)->m_circle.Radius() * dir;
	}

	return 0.0;
}

int CSketch::GetClosedSketchTurningNumber()
{
	double turning_angle = 0.0;

	HeeksObj* prev_object = NULL;
	HeeksObj* first_object = NULL;

	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;

		// internal angle
		if(object->GetType() == ArcType)
		{
			gp_Vec v0 = ((HArc*)object)->GetSegmentVector(0.0);
			gp_Vec v1 = ((HArc*)object)->GetSegmentVector(1.0);
			double start_angle = atan2(v0.Y(), v0.X());
			double end_angle = atan2(v1.Y(), v1.X());
			bool ccw = (((HArc*)object)->m_circle.Axis().Direction().Z() > 0);
			if(ccw)
			{
				if(start_angle < end_angle)start_angle += 6.2831853071795;
			}
			else
			{
				if(end_angle < start_angle)end_angle += 6.2831853071795;
			}
			turning_angle += end_angle - start_angle;
		}

		if(prev_object)
		{
			turning_angle += GetAngleAtJunction(prev_object, object);
		}

		if(first_object == NULL)first_object = object;
		prev_object = object;
	}

	if(first_object && prev_object)
	{
		turning_angle += GetAngleAtJunction(prev_object, first_object);
	}

	double turning_number = turning_angle / 6.2831853071795;

	// round up or down to nearest int
	if(turning_number > 0.00001)turning_number += 0.5;
	else if(turning_number < -0.00001)turning_number -= 0.5;

	int i_turning_number = (int)turning_number;

	return i_turning_number;
}

bool CSketch::Add(HeeksObj* object, HeeksObj* prev_object)
{
	m_order = SketchOrderTypeUnknown;
	return ObjList::Add(object, prev_object);
}

void CSketch::Remove(HeeksObj* object)
{
	m_order = SketchOrderTypeUnknown;
	ObjList::Remove(object);
}

bool CSketchRelinker::TryAdd(HeeksObj* object)
{
	// if the object is not already added
	if(m_added_from_old_set.find(object) == m_added_from_old_set.end())
	{
		double old_point[3];
		double new_point[3];
		m_new_front->GetEndPoint(old_point);

		// try the object, the right way round
		object->GetStartPoint(new_point);
		if(make_point(old_point).IsEqual(make_point(new_point), wxGetApp().m_geom_tol))
		{
			HeeksObj* new_object = object->MakeACopy();
			m_new_lists.back().push_back(new_object);
			m_new_front = new_object;
			m_added_from_old_set.insert(object);
			return true;
		}

		// try the object, the wrong way round
		object->GetEndPoint(new_point);
		if(make_point(old_point).IsEqual(make_point(new_point), wxGetApp().m_geom_tol))
		{
			HeeksObj* new_object = object->MakeACopy();

			// reverse object
			switch(new_object->GetType()){
			case LineType:
				((HLine*)new_object)->Reverse();
				break;
			case ArcType:
				((HArc*)new_object)->Reverse();
				break;
			default:
				break;
			}

			m_new_lists.back().push_back(new_object);
			m_new_front = new_object;
			m_added_from_old_set.insert(object);
			return true;
		}
	}

	return false;
}

bool CSketchRelinker::AddNext()
{
	// returns true, if another object was added to m_new_lists

	if(m_new_front)
	{
		bool added_at_front = false;

		// look through all of the old list, starting at m_old_front
		std::list<HeeksObj*>::const_iterator It = m_old_front;
		do{
			It++;
			if(It == m_old_list.end())It = m_old_list.begin();
			HeeksObj* object = *It;

			added_at_front = TryAdd(object);

		}while(It != m_old_front && !added_at_front);

		if(added_at_front)return true;

		// nothing fits the current new list

		m_new_front = NULL;

		if(m_old_list.size() > m_added_from_old_set.size())
		{
			// there are still some to add, find a unused object
			for(std::list<HeeksObj*>::const_iterator It = m_old_list.begin(); It != m_old_list.end(); It++)
			{
				HeeksObj* object = *It;
				if(m_added_from_old_set.find(object) == m_added_from_old_set.end())
				{
					HeeksObj* new_object = object->MakeACopy();
					std::list<HeeksObj*> empty_list;
					m_new_lists.push_back(empty_list);
					m_new_lists.back().push_back(new_object);
					m_added_from_old_set.insert(object);
					m_old_front = It;
					m_new_front = new_object;
					return true;
				}
			}
		}
	}

	return false;
}

bool CSketchRelinker::Do()
{
	if(m_old_list.size() > 0)
	{
		HeeksObj* new_object = m_old_list.front()->MakeACopy();
		std::list<HeeksObj*> empty_list;
		m_new_lists.push_back(empty_list);
		m_new_lists.back().push_back(new_object);
		m_added_from_old_set.insert(m_old_list.front());
		m_old_front = m_old_list.begin();
		m_new_front = new_object;

		while(AddNext()){}
	}

	return true;
}
