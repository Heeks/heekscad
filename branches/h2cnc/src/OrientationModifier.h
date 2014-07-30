
#pragma once

#include "stdafx.h"
#include "../interface/ObjList.h"

class COrientationModifier;

class COrientationModifierParams
{
public:
	typedef enum
	{
	    eNormalSpacing = 0
	} eSpacing_t;

	eSpacing_t m_spacing;
	int m_number_of_rotations;  // Number of quarter circle rotations.
	bool m_sketch_rotates_text;

	typedef enum
	{
		eLeftJustified = 0, // Valid for both open and closed sketches
		eCentreJustified,   // Valid for open sketches only
		eRightJustified,    // Valid for both open and closed sketches
		eTopJustified,      // Valid only for closed sketches
		eBottomJustified    // Valid only for closed sketches
	} eJustification_t;

	eJustification_t	m_justification;

	void set_initial_values();
	void write_values_to_config();
	void GetProperties(COrientationModifier * parent, std::list<Property *> *list);
	void WriteXMLAttributes(TiXmlNode* pElem);
	void ReadParametersFromXMLElement(TiXmlElement* pElem);

	const wxString ConfigScope(void)const{return _T("OrientationModifier");}
};



/**
    This class may be used to generate a transformation matrix
    at some point along the child elements.  The first example of
    its use is to orient individual letters in a HText object
    along a line.

    This class does not re-orient anything itself.  If is simply a
    container used to hold child path (graphics objects such as
    sketches).  When asked to do so, it can return a transformation
    matrix (rotation and/or translation) for some point along the
    child curves.
 */
class COrientationModifier : public ObjList
{
public:
    COrientationModifierParams m_params;

    COrientationModifier() { m_params.set_initial_values(); }
	~COrientationModifier() { }
    COrientationModifier & operator= ( const COrientationModifier & rhs );
    COrientationModifier(const COrientationModifier & rhs );

    // HeeksObj's virtual functions
	int GetType()const{return OrientationModifierType;}
	const wxChar* GetTypeString(void)const{return _T("OrientationModifier");}
	void glCommands(bool select, bool marked, bool no_color);

    bool OneOfAKind(){return false;}
	const wxBitmap &GetIcon();
	void GetProperties(std::list<Property *> *list);
	HeeksObj *MakeACopy(void)const;
	void CopyFrom(const HeeksObj* object);
	void WriteXML(TiXmlNode *root);
	bool CanAddTo(HeeksObj* owner);
	bool CanAdd(HeeksObj* object);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	void ReloadPointers();

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
	gp_Pnt & Transform(gp_Trsf existing_transformation, const double _distance, gp_Pnt & point, const float width );
	bool SketchIsClosed();
	void InitializeFromSketch();

private:
    typedef std::list<std::pair<TopoDS_Edge, double> > Edges_t;

    // NOTE: These three variables are relatively transient.  They are only held here to reduce re-work
    // during a single rendering session.

    HeeksObj *m_last_child;
    Edges_t m_edges;
    double m_total_edge_length;

}; // End COrientationModifier class definition.

