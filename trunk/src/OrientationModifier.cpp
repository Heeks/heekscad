

#include "stdafx.h"
#include "OrientationModifier.h"
#include "../interface/PropertyChoice.h"
#include "HeeksConfig.h"
#include <BRepAdaptor_Curve.hxx>

void COrientationModifierParams::set_initial_values()
{
	HeeksConfig config;

	config.Read(_T("OrientationModifier_m_spacing"), (int *) &m_spacing, int(eNormalSpacing));
}

void COrientationModifierParams::write_values_to_config()
{
	// We always want to store the parameters in mm and convert them back later on.

	HeeksConfig config;

	// These values are in mm.
	config.Write(_T("OrientationModifier_m_spacing"), m_spacing);
}

static void on_set_spacing(int zero_based_choice, HeeksObj* object)
{
	((COrientationModifier*)object)->m_params.m_spacing = COrientationModifierParams::eSpacing_t(zero_based_choice);
	((COrientationModifier*)object)->m_params.write_values_to_config();
}


void COrientationModifierParams::GetProperties(COrientationModifier * parent, std::list<Property *> *list)
{

	{
		int choice = int(m_spacing);
		std::list< wxString > choices;
		choices.push_back( wxString(_("Normally spaced")) );

		list->push_back(new PropertyChoice(_("Spacing"), choices, choice, parent, on_set_spacing));
	}
}

void COrientationModifierParams::WriteXMLAttributes(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "params" );
	root->LinkEndChild( element );

	element->SetAttribute("m_spacing", int(m_spacing));
}

void COrientationModifierParams::ReadParametersFromXMLElement(TiXmlElement* pElem)
{
	if (pElem->Attribute("m_spacing")) pElem->Attribute("m_spacing", (int *) &m_spacing);
}


COrientationModifier::COrientationModifier( const COrientationModifier & rhs ) : ObjList(rhs)
{
	m_params = rhs.m_params;
}

COrientationModifier & COrientationModifier::operator= ( const COrientationModifier & rhs )
{
	if (this != &rhs)
	{
	    m_params = rhs.m_params;
		ObjList::operator=(rhs);
	}

	return(*this);
}

const wxBitmap &COrientationModifier::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/text.png")));
	return *icon;
}

void COrientationModifier::glCommands(bool select, bool marked, bool no_color)
{
	ObjList::glCommands( select, marked, no_color );
}

HeeksObj *COrientationModifier::MakeACopy(void)const
{
	COrientationModifier *new_object = new COrientationModifier(*this);
	return(new_object);
}

void COrientationModifier::CopyFrom(const HeeksObj* object)
{
	if (object->GetType() == OrientationModifierType)
	{
		*this = *((COrientationModifier *) object);
	}
}


bool COrientationModifier::CanAddTo(HeeksObj* owner)
{
	return((owner != NULL) && (owner->GetType() == TextType));
}

bool COrientationModifier::CanAdd(HeeksObj* object)
{
    if (object == NULL) return(false);

	// We only want a single sketch.  Make sure we don't already have one.
	if (GetNumChildren() > 0)
	{
		wxString message;
		message << _("Only a single sketch is supported");
		wxMessageBox(message);

		return(false);
	}

	switch (object->GetType())
	{
		case SketchType:
			return(true);

		default:
			return(false);
	} // End switch
}

void COrientationModifier::GetProperties(std::list<Property *> *list)
{
	m_params.GetProperties( this, list );
	ObjList::GetProperties(list);
}


void COrientationModifier::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	ObjList::GetTools( t_list, p );
}



void COrientationModifier::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element = new TiXmlElement( "OrientationModifier" );
	root->LinkEndChild( element );

	m_params.WriteXMLAttributes(element);

	WriteBaseXML(element);
}

// static member function
HeeksObj* COrientationModifier::ReadFromXMLElement(TiXmlElement* element)
{
	COrientationModifier* new_object = new COrientationModifier();

	std::list<TiXmlElement *> elements_to_remove;

	// read point and circle ids
	for(TiXmlElement* pElem = TiXmlHandle(element).FirstChildElement().Element(); pElem; pElem = pElem->NextSiblingElement())
	{
		std::string name(pElem->Value());
		if(name == "params"){
			new_object->m_params.ReadParametersFromXMLElement(pElem);
			elements_to_remove.push_back(pElem);
		}
	}

	for (std::list<TiXmlElement*>::iterator itElem = elements_to_remove.begin(); itElem != elements_to_remove.end(); itElem++)
	{
		element->RemoveChild(*itElem);
	}

	new_object->ReadBaseXML(element);

	return new_object;
}


void COrientationModifier::ReloadPointers()
{
	ObjList::ReloadPointers();
}


/**
	This method returns a transformation matrix (scale, position and/or rotation) based
	on the distance (along X) for the text string being rendered.  The idea is to map
	this position to a location along the child sketch.  The transformation matrix
	would represent a translation and/or rotation so that the character can be
	rendered as through it is laying along this sketch.

	The existing_transformation matrix is that applied to the HText object and pushed
	onto the graphics stack before the HText is displayed.  It's passed in here so that
	this method may 'undo' some of its transformations so as to compare the 'distance'
	value (along X) with the distance around the sketch.
 */
gp_Trsf COrientationModifier::Get(gp_Trsf existing_transformation, const double _distance)
{
	double tolerance = wxGetApp().m_geom_tol;

	gp_Trsf reverse(existing_transformation);
	reverse.Invert();

	double distance(_distance);
	if (distance < 0) distance *= -1.0;

	// The distance in font coordinates is smaller than that used to place the characters.
	// Scale this distance up by the HText scale factor being applied to the font graphics.
	distance *= existing_transformation.ScaleFactor();

	if (GetNumChildren() == 0)
	{
		// No children so no modification of position.
		gp_Trsf unchanged;
		return(unchanged);
	}

	std::list<TopoDS_Shape> wires;
	if (! ::ConvertSketchToFaceOrWire( GetFirstChild(), wires, false))
    {
		gp_Trsf unchanged;
		return(unchanged);
    } // End if - then

	double distance_remaining = distance;
	for (std::list<TopoDS_Shape>::iterator itWire = wires.begin(); itWire != wires.end(); itWire++)
	{
		TopoDS_Wire wire(TopoDS::Wire(*itWire));
		std::vector<TopoDS_Edge> edges;

		for(BRepTools_WireExplorer expEdge(TopoDS::Wire(wire)); expEdge.More(); expEdge.Next())
		{
			edges.push_back( TopoDS_Edge(expEdge.Current()) );
		} // End for

		if (edges.size() > 0)
		{
			std::vector<TopoDS_Edge>::size_type i=0;
			while (distance_remaining > tolerance)
			{
				if (i == edges.size()) i=0;	// Loop around.

				BRepAdaptor_Curve curve(edges[i]);
				double edge_length = GCPnts_AbscissaPoint::Length(curve);

				if (edge_length < distance_remaining)
				{
					distance_remaining -= edge_length;
				}
				else
				{
					// The point we're after is along this edge somewhere.  Find the point and
					// the first derivative at that point.
					gp_Pnt point;
					gp_Vec vec;
					double proportion = distance_remaining / edge_length;
					Standard_Real U = ((curve.LastParameter() - curve.FirstParameter()) * proportion) + curve.FirstParameter();
					curve.D1(U, point, vec);
					// double angle = gp_Vec(1,0,0).AngleWithRef(vec, gp_Vec(gp_Pnt(0,0,0), gp_Pnt(1,0,0)));
					double angle = gp_Vec(1,0,0).Angle(vec);
					gp_Trsf transformation;
					transformation.SetRotation( gp_Ax1(gp_Pnt(distance,0.0,0.0), gp_Vec(0,0,-1)), angle );

					/*
					gp_Trsf back_to_origin;
					back_to_origin.SetTranslation(gp_Vec(gp_Pnt(0,0,0), gp_Pnt(-1.0 * distance, 0, 0)));
					transformation = transformation * back_to_origin;

					gp_Trsf around_sketch;

					around_sketch.SetTranslation(gp_Pnt(0,0,0), point);
					transformation = transformation * around_sketch;
					*/

					gp_Trsf around;
					around.SetTranslation(gp_Pnt(distance,0,0), point);
					transformation = transformation * around;

					// transformation = transformation * reverse;
					return(transformation);
				}

				i++;
			} // End while
		} // End if - then
	} // End for

	gp_Trsf unchanged;
	return(unchanged);
}




