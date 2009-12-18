// Sketch.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Sketch.h"
#include "HLine.h"
#include "HArc.h"
#include "HSpline.h"
#include "HeeksFrame.h"
#include "ObjPropsCanvas.h"
#include "../interface/PropertyInt.h"
#include "../interface/PropertyChoice.h"
#include "../interface/PropertyCheck.h"
#include "../interface/Tool.h"
#include "MultiPoly.h"
#include "FaceTools.h"
#include "CoordinateSystem.h"

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
	m_solidify = false;
	m_coordinate_system = NULL;
	m_draw_with_transform = true;
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
	m_solidify = c.m_solidify;
	m_coordinate_system = c.m_coordinate_system;
	m_draw_with_transform = c.m_draw_with_transform;

	return *this;
}

void CSketch::ReloadPointers()
{
	HeeksObj* child = GetFirstChild();
	while(child)
	{
		CoordinateSystem *system = dynamic_cast<CoordinateSystem*>(child);
		if(system)
		{
			m_coordinate_system = system;
		}
		child = GetNextChild();
	}

	ObjList::ReloadPointers();
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
			wxGetApp().m_frame->m_properties->RefreshByRemovingAndAddingAll();
		}
	}
}

static void on_set_solidify(bool value, HeeksObj* object)
{
	((CSketch*)object)->m_solidify = value;
	wxGetApp().Repaint();
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

std::vector<TopoDS_Face> CSketch::GetFaces()
{
	std::list<CSketch*> sketches;
	sketches.push_back(this);
	return MultiPoly(sketches);
}

void CSketch::glCommands(bool select, bool marked, bool no_color)
{
	if(m_coordinate_system && m_draw_with_transform)
	{
		glPushMatrix();
		m_coordinate_system->ApplyMatrix();
	}

	ObjList::glCommands(select,marked,no_color);

	if(m_solidify)
	{
	    try
	    {
            //TODO: we should really only be doing this when geometry changes
            std::vector<TopoDS_Face> faces = GetFaces();

            double pixels_per_mm = wxGetApp().GetPixelScale();

            for(unsigned i=0; i < faces.size(); i++)
            {
                MeshFace(faces[i],pixels_per_mm);
                DrawFaceWithCommands(faces[i]);
            }
	    }catch(...)
	    {

	    }
	}

	if(m_coordinate_system && m_draw_with_transform)
		glPopMatrix();
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

	list->push_back ( new PropertyCheck( _("solidify"), m_solidify, this, on_set_solidify) );

	ObjList::GetProperties(list);
}

static CSketch* sketch_for_tools = NULL;

class SplitSketch:public Tool{
public:
	void Run(){
		if(sketch_for_tools == NULL)return;
		std::list<HeeksObj*> new_sketches;
		sketch_for_tools->ExtractSeparateSketches(new_sketches);
		for(std::list<HeeksObj*>::iterator It = new_sketches.begin(); It != new_sketches.end(); It++)
		{
			HeeksObj* new_object = *It;
			sketch_for_tools->Owner()->Add(new_object, NULL);
		}
		sketch_for_tools->Owner()->Remove(sketch_for_tools);
		wxGetApp().Repaint();
	}
	const wxChar* GetTitle(){return _T("Split Sketch");}
	wxString BitmapPath(){return _T("splitsketch");}
};

static SplitSketch split_sketch;

class ConvertSketchToFace: public Tool
{
public:
	void Run()
	{
		TopoDS_Face face;
		if(ConvertSketchToFaceOrWire(sketch_for_tools, face, true))
		{
			wxGetApp().Add(new CFace(face), NULL);
			wxGetApp().Repaint();
		}
	}
	const wxChar* GetTitle(){return _("Convert sketch to face");}
	wxString BitmapPath(){return _T("la2face");}
};

static ConvertSketchToFace convert_sketch_to_face;

class SketchArcsToLines: public Tool
{
public:
	void Run()
	{
		HeeksObj* new_object = SplitArcsIntoLittleLines(sketch_for_tools);
		wxGetApp().Remove(sketch_for_tools);
		sketch_for_tools = NULL;
		wxGetApp().Add(new_object, NULL);
	}

	const wxChar* GetTitle(){return _("Split arcs to little lines");}
	wxString BitmapPath(){return _T("splitarcs");}
};

static SketchArcsToLines sketch_arcs_to_lines;

void CSketch::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	sketch_for_tools = this;
	if(GetSketchOrder() == SketchOrderTypeBad || GetSketchOrder() == SketchOrderTypeMultipleCurves)
	{
		t_list->push_back(&split_sketch);
	}
	t_list->push_back(&convert_sketch_to_face);
	t_list->push_back(&sketch_arcs_to_lines);
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

	new_object->ReloadPointers();

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
	wxGetApp().Changed();
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

	Clear();

	for(std::list< std::list<HeeksObj*> >::iterator It = relinker.m_new_lists.begin(); It != relinker.m_new_lists.end(); It++)
	{
		for(std::list<HeeksObj*>::iterator It2 = It->begin(); It2 != It->end(); It2++)
		{
			Add(*It2, NULL);
		}
	}

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
	std::list<HeeksObj*> old_list = m_objects;

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
			case SplineType:
				((HSpline*)copy)->Reverse();
				break;
			default:
				break;
		}

		new_list.push_front(copy);
	}

	Clear();
	for(std::list<HeeksObj*>::iterator It = new_list.begin(); It != new_list.end(); It++)
	{
		HeeksObj* object = *It;
		Add(object, NULL);
	}
	//TODO: this is a hack. Must call remove before add, or else add has no effect. Why are we calling this here?
	wxGetApp().ObserversOnChange(NULL,&old_list,NULL);
	wxGetApp().ObserversOnChange(&new_list,NULL,NULL);
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

double CSketch::GetAngleAtJunction(HeeksObj* prev_object, HeeksObj* object)
{
	EndedObject *obj1 = (EndedObject*)prev_object;
	EndedObject *obj2 = (EndedObject*)object;
	gp_Vec prev_end_vector(obj1->B->m_p.XYZ() - obj1->A->m_p.XYZ());
	gp_Vec start_vector(obj2->B->m_p.XYZ() - obj2->A->m_p.XYZ());
	prev_end_vector.Normalize();
	start_vector.Normalize();

	double prev_segment_curvature = 0;//GetSegmentCurvature(prev_object);
	double segment_curvature = 0;//GetSegmentCurvature(object);

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
		if(((HArc*)object)->m_axis.Direction().Z() > 0)return 1;
		return -1;
	}

	return 0;
}

double CSketch::GetSegmentCurvature(HeeksObj* object)
{
	if(object->GetType() == ArcType)
	{
		int dir = (((HArc*)object)->m_axis.Direction().Z() > 0) ? 1:-1;
		return 1 / ((HArc*)object)->m_radius * dir;
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
/*			gp_Vec v0 = ((HArc*)object)->GetSegmentVector(0.0);
			gp_Vec v1 = ((HArc*)object)->GetSegmentVector(1.0);
			double start_angle = atan2(v0.Y(), v0.X());
			double end_angle = atan2(v1.Y(), v1.X());
			bool ccw = (((HArc*)object)->m_axis.Direction().Z() > 0);
			if(ccw)
			{
				if(start_angle < end_angle)start_angle += 6.2831853071795;
			}
			else
			{
				if(end_angle < start_angle)end_angle += 6.2831853071795;
			}
			turning_angle += end_angle - start_angle; */
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
			case SplineType:
				((HSpline*)new_object)->Reverse();
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


/**
	The Intersects() method is included in the heeks CAD interface as well as being
	a virtual method in the HeeksObj base class.  Since this Sketch object is, itself,
	simply a list of HeeksObj objects, we should be able to simply aggregate the
	intersection of the specified HeeksObj with all of 'our' HeeksObj objects.
 */
int CSketch::Intersects(const HeeksObj *object, std::list< double > *rl) const
{
	int number_of_intersections = 0;

	for (std::list<HeeksObj *>::const_iterator l_itObject = m_objects.begin(); l_itObject != m_objects.end(); l_itObject++)
	{
		number_of_intersections += (*l_itObject)->Intersects( object, rl );
	} // End for

	return(number_of_intersections);
} // End Intersects() method

