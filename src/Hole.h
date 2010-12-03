// Hole.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "../interface/ObjList.h"

class CHolePositions: public ObjList {
public:
	CHolePositions() { }
	CHolePositions & operator= ( const CHolePositions & rhs );
	CHolePositions( const CHolePositions & rhs );

	bool operator==( const CHolePositions & rhs ) const { return(ObjList::operator==(rhs)); }
	bool operator!=( const CHolePositions & rhs ) const { return(! (*this == rhs)); }

	bool IsDifferent(HeeksObj *other) { return(*this != (*(CHolePositions *)other)); }

	// HeeksObj's virtual functions
	bool OneOfAKind(){return true;} // only one in each hole
	int GetType()const{return HolePositionsType;}
	const wxChar* GetTypeString(void)const{return _("Positions");}
	HeeksObj *MakeACopy(void)const{ return new CHolePositions(*this);}
	const wxBitmap &GetIcon();
	bool CanAddTo(HeeksObj* owner){return owner->GetType() == HoleType;}
	bool CanAdd(HeeksObj* object);
	bool CanBeRemoved(){return false;}
	void WriteXML(TiXmlNode *root);
	bool AutoExpand(){return true;}
	bool UsesID() { return(false); }

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
};

class CHole: public ObjList
{
private:
	double m_depth;
	double m_diameter;
	CSketch* m_profile_sketch;	// this should be a sketch with positive X representing radius and negative Y representing depth.
	CHolePositions* m_positions;

	void renderHole(bool marked, bool no_color);

public:
	CHole(double depth, double diameter);

	// HeeksObj's virtual functions
	int GetType()const{return HoleType;}
	const wxChar* GetTypeString(void)const{return _T("Hole");}
	void glCommands(bool select, bool marked, bool no_color);
	const wxBitmap &GetIcon();
	void GetProperties(std::list<Property *> *list);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	HeeksObj *MakeACopy(void)const;
	void CopyFrom(const HeeksObj* object);
	void WriteXML(TiXmlNode *root);
	bool Add(HeeksObj* object, HeeksObj* prev_object);
	void Remove(HeeksObj* object);
	bool CanAdd(HeeksObj* object);
	bool CanAddTo(HeeksObj* owner);

	// Data access methods.
	CHolePositions* Positions(){return m_positions;}
	CSketch* ProfileSketch(){return m_profile_sketch;}
};