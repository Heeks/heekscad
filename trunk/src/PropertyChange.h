// PropertyChange.h

class PropertyChangeString: public Tool
{
public:
	wxString m_value;
	wxString m_old;
	HeeksObj* m_object;
	void(*m_callbackfunc)(const wxChar*, HeeksObj*);

	PropertyChangeString(const wxString& value, PropertyString* property);

	void Run();
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change String");}
	bool Undoable(){return true;}
};

class PropertyDouble;

class PropertyChangeDouble: public Tool
{
public:
	double m_value;
	double m_old;
	HeeksObj* m_object;
	void(*m_callbackfunc)(double, HeeksObj*);

	PropertyChangeDouble(const double& value, PropertyDouble* property);

	void Run();
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change Double");}
	bool Undoable(){return true;}
};

class PropertyLength;

class PropertyChangeLength: public Tool
{
public:
	double m_value;
	double m_old;
	HeeksObj* m_object;
	void(*m_callbackfunc)(double, HeeksObj*);

	PropertyChangeLength(const double& value, PropertyLength* property);

	void Run();
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change Length");}
	bool Undoable(){return true;}
};

class PropertyInt;

class PropertyChangeInt: public Tool
{
public:
	int m_value;
	int m_old;
	HeeksObj* m_object;
	void(*m_callbackfunc)(int, HeeksObj*);

	PropertyChangeInt(const int& value, PropertyInt* property);

	void Run();
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change Int");}
	bool Undoable(){return true;}
};

class PropertyColor;

class PropertyChangeColor: public Tool
{
public:
	HeeksColor m_value;
	HeeksColor m_old;
	HeeksObj* m_object;
	void(*m_callbackfunc)(HeeksColor, HeeksObj*);

	PropertyChangeColor(const HeeksColor& value, PropertyColor* property);

	void Run();
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change Color");}
	bool Undoable(){return true;}
};

class PropertyVertex;

class PropertyChangeVertex: public Tool
{
public:
	double m_value[3];
	double m_old[3];
	HeeksObj* m_object;
	void(*m_callbackfunc)(const double*, HeeksObj*);

	PropertyChangeVertex(const double* value, PropertyVertex* property);

	void Run();
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change Vertex");}
	bool Undoable(){return true;}
};

class PropertyTrsf;

class PropertyChangeTrsf: public Tool
{
public:
	gp_Trsf m_value;
	gp_Trsf m_old;
	HeeksObj* m_object;
	void(*m_callbackfunc)(const gp_Trsf&, HeeksObj*);

	PropertyChangeTrsf(const gp_Trsf &value, PropertyTrsf* property);

	void Run();
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change Trsf");}
	bool Undoable(){return true;}
};

class PropertyChoice;

class PropertyChangeChoice: public Tool
{
public:
	int m_value;
	int m_old;
	HeeksObj* m_object;
	void(*m_callbackfunc)(int, HeeksObj*);

	PropertyChangeChoice(const int& value, PropertyChoice* property);

	void Run();
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change Choice");}
	bool Undoable(){return true;}
};
