// PropertyChange.h

class PropertyChangeString: public Undoable
{
public:
	wxString m_value;
	wxString m_old;
	HeeksObj* m_object;
	void(*m_callbackfunc)(const wxChar*, HeeksObj*);

	PropertyChangeString(const wxString& value, PropertyString* property);

	void Run(bool redo);
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change String");}
};

class PropertyDouble;

class PropertyChangeDouble: public Undoable
{
public:
	double m_value;
	double m_old;
	HeeksObj* m_object;
	void(*m_callbackfunc)(double, HeeksObj*);

	PropertyChangeDouble(const double& value, PropertyDouble* property);

	void Run(bool redo);
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change Double");}
};

class PropertyLength;

class PropertyChangeLength: public Undoable
{
public:
	double m_value;
	double m_old;
	HeeksObj* m_object;
	void(*m_callbackfunc)(double, HeeksObj*);

	PropertyChangeLength(const double& value, PropertyLength* property);

	void Run(bool redo);
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change Length");}
};

class PropertyInt;

class PropertyChangeInt: public Undoable
{
public:
	int m_value;
	int m_old;
	HeeksObj* m_object;
	void(*m_callbackfunc)(int, HeeksObj*);

	PropertyChangeInt(const int& value, PropertyInt* property);

	void Run(bool redo);
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change Int");}
};

class PropertyColor;

class PropertyChangeColor: public Undoable
{
public:
	HeeksColor m_value;
	HeeksColor m_old;
	HeeksObj* m_object;
	void(*m_callbackfunc)(HeeksColor, HeeksObj*);

	PropertyChangeColor(const HeeksColor& value, PropertyColor* property);

	void Run(bool redo);
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change Color");}
};

class PropertyVertex;

class PropertyChangeVertex: public Undoable
{
public:
	double m_value[3];
	double m_old[3];
	HeeksObj* m_object;
	void(*m_callbackfunc)(const double*, HeeksObj*);

	PropertyChangeVertex(const double* value, PropertyVertex* property);

	void Run(bool redo);
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change Vertex");}
};

class PropertyTrsf;

class PropertyChangeTrsf: public Undoable
{
public:
	gp_Trsf m_value;
	gp_Trsf m_old;
	HeeksObj* m_object;
	void(*m_callbackfunc)(const gp_Trsf&, HeeksObj*);

	PropertyChangeTrsf(const gp_Trsf &value, PropertyTrsf* property);

	void Run(bool redo);
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change Trsf");}
};

class PropertyChoice;

class PropertyChangeChoice: public Undoable
{
public:
	int m_value;
	int m_old;
	HeeksObj* m_object;
	void(*m_callbackfunc)(int, HeeksObj*, bool);

	PropertyChangeChoice(const int& value, PropertyChoice* property);

	void Run(bool redo);
	void RollBack();
	const wxChar* GetTitle(){return _("Property Change Choice");}
};
