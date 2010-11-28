// ConstrainedObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "Constraint.h"
#include "ConstrainedObject.h"
#include "../tinyxml/tinyxml.h"

std::string AbsoluteAngle[] = {
	"AbsoluteAngleHorizontal",
	"AbsoluteAngleVertical"
};

std::string ConstraintTypes[]={
	"CoincidantPointConstraint",
	"ParallelLineConstraint",
	"PerpendicularLineConstraint",
	"AbsoluteAngleConstraint",
	"LineLengthConstraint",
	"LineTangentConstraint",
	"RadiusConstraint",
	"EqualLengthConstraint",
	"ColinearConstraint",
	"EqualRadiusConstraint",
	"ConcentricConstraint",
	"PointOnLineConstraint",
	"PointOnLineMidpointConstraint",
	"PointOnArcMidpointConstraint",
	"PointOnArcConstraint",
	"PointOnCircleConstraint",
	"LineHorizontalLengthConstraint",
	"LineVerticalLengthConstraint",
	"FixedPointConstraint"
};


Constraint::Constraint(){

}

Constraint::Constraint(const Constraint* obj){
	m_type = obj->m_type;
	m_angle = obj->m_angle;
	m_length = obj->m_length;
	m_obj1 = obj->m_obj1;
	m_obj2 = obj->m_obj2;
	m_obj1->Add(this,NULL);
	if(m_obj2)
		m_obj2->Add(this,NULL);

}

Constraint::Constraint(EnumConstraintType type,EnumAbsoluteAngle angle, ConstrainedObject* obj)
{
    m_type = type;
	m_angle = angle;
	m_obj1 = obj;
	m_obj2 = NULL;
	m_length = 0;
	m_obj1->Add(this,NULL);
}

Constraint::Constraint(EnumConstraintType type,ConstrainedObject* obj)
{
    m_type = type;
	m_angle = (EnumAbsoluteAngle)0;
	m_obj1 = obj;
	m_obj2 = NULL;
	m_length = 0;
	m_obj1->Add(this,NULL);
}


Constraint::Constraint(EnumConstraintType type,EnumAbsoluteAngle angle, double length, ConstrainedObject* obj1, ConstrainedObject* obj2)
{
    m_type = type;
	m_angle = angle;
	m_obj1 = obj1;
	m_obj2 = obj2;
	m_length = length;
	m_obj1->Add(this,NULL);
	if(m_obj2)
		m_obj2->Add(this,NULL);
}

Constraint::Constraint(EnumConstraintType type,ConstrainedObject* obj1,ConstrainedObject* obj2)
{
    m_type = type;
	m_angle = (EnumAbsoluteAngle)0;
	m_obj1 = obj1;
	m_obj2 = obj2;
	m_length = 0;
	m_obj1->Add(this,NULL);
	m_obj2->Add(this,NULL);
}


Constraint::Constraint(EnumConstraintType type,double length,ConstrainedObject* obj1)
{
    m_type = type;
	m_angle = (EnumAbsoluteAngle)0;
	m_obj1 = obj1;
	m_obj2 = 0;
	m_length = length;
	obj1->Add(this,NULL);
}

const Constraint& Constraint::operator=(const Constraint &b){
	HeeksObj::operator=(b);
	m_type = b.m_type;
	m_angle = b.m_angle;
	m_obj1 = b.m_obj1;
	m_obj2 = b.m_obj2;
	m_length = b.m_length;
	return *this;
}

Constraint::~Constraint()
{

}

void Constraint::Disconnect(std::list<HeeksObj*> parents)
{
	HeeksObj* owner = GetFirstOwner();
	if(parents.back() == owner)
	{
		this->RemoveOwner(owner);
		return;
	}
	owner = GetNextOwner();
	if(parents.back() == owner)
		RemoveOwner(owner);
}

void Constraint::ReloadPointers()
{
	m_obj1 = (ConstrainedObject*)GetFirstOwner();
	m_obj2 = (ConstrainedObject*)GetNextOwner();
	if(!m_obj2)
	{
		int x=0;
		x++;
	}
}

bool Constraint::IsDifferent(HeeksObj* o)
{
	Constraint* other = (Constraint*)o;
	if(m_type != other->m_type || m_angle != other->m_angle || m_length != other->m_length)
		return true;

	int id1_1=0;
	int id1_2=0;
	int id2_1=0;
	int id2_2=0;

	if(m_obj1)
		id1_1 = m_obj1->m_id;
	if(other->m_obj1)
		id1_2 = other->m_obj1->m_id;
	if(m_obj2)
		id2_1 = m_obj2->m_id;
	if(other->m_obj2)
		id2_2 = other->m_obj2->m_id;

	if(id1_1 != id1_2 && id1_1 != id2_2)
		return true;

	if(id2_1 != id2_2 && id2_1 != id1_2)
		return true;

	return false;
}

void Constraint::render_text(const wxChar* str)
{
	wxGetApp().create_font();
	//glColor4ub(0, 0, 0, 255);
	wxGetApp().EnableBlend();
	glEnable(GL_TEXTURE_2D);
	glDepthMask(0);
	glDisable(GL_POLYGON_OFFSET_FILL);
	wxGetApp().m_gl_font.Begin();

	//Draws text with a glFont
	float scale = 0.08f;
	std::pair<int,int> size;
	wxGetApp().m_gl_font.GetStringSize(str,&size);
	wxGetApp().m_gl_font.DrawString(str, scale, -size.first/2.0f*scale, size.second/2.0f*scale);

	glDepthMask(1);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_TEXTURE_2D);
	wxGetApp().DisableBlend();
}

void Constraint::glCommands(HeeksColor color, gp_Ax1 axis)
{
	double mag = sqrt(axis.Direction().X() * axis.Direction().X() + axis.Direction().Y() * axis.Direction().Y());
	float rot = (float)atan2(axis.Direction().Y()/mag,axis.Direction().X()/mag);

	glPushMatrix();
	glTranslatef((float)axis.Location().X(),(float)axis.Location().Y(),(float)axis.Location().Z());
	glRotatef(rot*180/(float)Pi,0,0,1);
	glTranslatef(0,ANGLE_OFFSET_FROM_LINE,0);

	switch(m_type)
	{
		case AbsoluteAngleConstraint:
			switch(m_angle)
			{
				case AbsoluteAngleHorizontal:
					render_text(_("H"));
					break;
				case AbsoluteAngleVertical:
					glRotatef(90,0,0,1);
					render_text(_("V"));
					break;
			}
			break;
		case LineLengthConstraint:
			wxChar str[100];
            wxSprintf(str,_T("%f"),m_length/(double)wxGetApp().m_view_units);
			render_text(str);
			break;
		case ParallelLineConstraint:
			render_text(_("L"));
			break;
		case PerpendicularLineConstraint:
			render_text(_("P"));
			break;
		default:
			break;
	}

	glPopMatrix();
}

HeeksObj *Constraint::MakeACopy(void)const
{
	return new Constraint(*this);
}

static std::list<Constraint*> obj_to_save;
static std::set<Constraint*> obj_to_save_find;

void Constraint::BeginSave()
{
	obj_to_save.clear();
	obj_to_save_find.clear();
}

void Constraint::EndSave(TiXmlNode *root)
{
	std::list<Constraint*>::iterator it;
	for(it = obj_to_save.begin(); it != obj_to_save.end(); it++)
	{
		Constraint *c = *it;
		TiXmlElement * element;
		element = new TiXmlElement( "Constraint" );
		root->LinkEndChild( element );
		element->SetAttribute("type", ConstraintTypes[c->m_type].c_str());
		element->SetAttribute("angle", AbsoluteAngle[c->m_angle].c_str());
		element->SetDoubleAttribute("length", c->m_length);
		element->SetAttribute("obj1_id",c->m_obj1->m_id);
		element->SetAttribute("obj1_type",c->m_obj1->GetIDGroupType());
		if(c->m_obj2)
		{
			element->SetAttribute("obj2_id",c->m_obj2->m_id);
			element->SetAttribute("obj2_type",c->m_obj2->GetIDGroupType());
		}
		c->WriteBaseXML(element);
	}
}

void Constraint::WriteXML(TiXmlNode *root)
{
	if(obj_to_save_find.find(this)!=obj_to_save_find.end())
		return;

	obj_to_save.push_back(this);
	obj_to_save_find.insert(this);
}
//JT
#ifdef CHECK_FOR_INVALID_CONSTRAINT
bool checkForInvalidConstraint( const char* type, EnumConstraintType etype,const char* angle,EnumAbsoluteAngle eangle,double length,int obj1_id,int obj2_id,int obj1_type,int obj2_type)
{
    ConstrainedObject* obj1=0;
	ConstrainedObject* obj2=0;
    bool isItHosedUp = false;
    wxString errorMessage(type,wxConvUTF8);
    errorMessage +=  wxString::Format(wxT(" obj1_id = %d obj2_id= %d") ,obj1_id,obj2_id);
    if ((obj1_id ==0)&& (obj2_id == 0))//This should never happen
    {

        isItHosedUp =true;

    }
    else if ((obj1_id ==0)&& (obj2_id != 0))// I believe this should also never happen
    {    isItHosedUp = true;
        errorMessage += _("\n obj1_id should be defined before obj2_id");
    }
    else if ((obj1_id !=0)&& (obj2_id != 0))//This can happen all the time. Need to verify that these objects exist in memory
    {
        obj1 = (ConstrainedObject*)wxGetApp().GetIDObject(obj1_type,obj1_id);
        obj2 = (ConstrainedObject*)wxGetApp().GetIDObject(obj2_type,obj2_id);
        if (obj1 ==0)
        {
            errorMessage += _("\nobj1_id specified but does not exist in memory");
            isItHosedUp = true;
        }
        if (obj2 ==0)
        {
            errorMessage += _("\nobj2_id specified but does not exist in memory");
            isItHosedUp = true;
        }
        else
        {
            // todo need to go through to make sure that only the contraints that have two members are called
            // I've run into this bug.
        }
    }
    else if ((obj1_id !=0)&& (obj2_id == 0))//This can happen all the time. Need to verify that these objects exist in memory
    {
        obj1 = (ConstrainedObject*)wxGetApp().GetIDObject(obj1_type,obj1_id);
        if (obj1 ==0)
        {
            isItHosedUp = true;
            errorMessage += _("\nobj1_id specified but does not exist in memory");
        }
        else
        {


            switch (etype)
            {
                case ParallelLineConstraint:
                case PerpendicularLineConstraint:
                case LineTangentConstraint:
                case RadiusConstraint:
                case EqualLengthConstraint:
                case ColinearConstraint:
                case EqualRadiusConstraint:
                case ConcentricConstraint:
                case PointOnLineConstraint:
                case PointOnLineMidpointConstraint:
                case PointOnArcMidpointConstraint:
                case PointOnArcConstraint:
                case PointOnCircleConstraint:
                case CoincidantPointConstraint:// This is a common bug You need to variables.

                    errorMessage += _("\n Two objects required for this constraint only one was specified");
                    isItHosedUp = true;//I believe that these require multiple objs
                    break;
                case AbsoluteAngleConstraint:
                case LineLengthConstraint:
                case LineHorizontalLengthConstraint:
                case LineVerticalLengthConstraint:
                case FixedPointConstraint:

                    //These values are single point
                    break;
                default:
                    errorMessage += _("\n A constraint type needs to be added to the list");
            }
        }

         //if a CoincidantPointConstraint is defined with one point this is a boo boo.
         // todo Need to build checks here for single object contraints


    }
    else
    {
       wxMessageBox (_("This is a woops and you shouldn't be reading this message"));

    }


    if (isItHosedUp)
    {
#ifdef DISPLAY_CHECK_FOR_INVALID_CONSTRAINT_ERROR_MSGBOX
        wxMessageBox (errorMessage);

#endif
        wxPuts(errorMessage);

    }
#ifdef LET_BAD_CONSTRAINT_PASS

    return false;
#else

    return isItHosedUp;//false says all is well
#endif

}
#endif
#ifdef CONSTRAINT_TESTER
void Constraint::AuditHeeksObjTree4Constraints(HeeksObj * SketchPtr ,HeeksObj * mom,int level,bool ShowMsgInConsole,bool * ConstraintsAreOk)
{
    wxString message=wxT("");
    message.Pad(level*3,' ',true);
    message +=wxString::Format(wxT("%s ID=%d \n  ") ,GetTypeString(),m_id);
    message.Pad(level*3,' ',true);

    wxString wxstr_m_type(ConstraintTypes[m_type].c_str(), wxConvUTF8);
    message += wxstr_m_type;
    message += wxT(" consists of:\n");
    message.Pad(level*3+3,' ',true);
    if (!m_obj1 ==0)
    {
        message += wxString::Format(wxT("%s id=%d(%s) -"),m_obj1->GetTypeString(),m_obj1->m_id,((m_obj1==mom)?wxT("KNOWN"):wxT("UNKNOWN")));
    }
    else
    {
        message += wxString (wxT("!!!!! m_obj1 Should Always be specified!!!!! \n"));
        *ConstraintsAreOk=false;
    }

    if (m_obj2 ==0)
    {

        if (ReturnStdObjectCtForConstraint(m_type)==2)
            {    // We have a problem where
                message += wxString (wxT("!!!!! m_obj2 not specified in two object constraint!!!!! \n"));
                *ConstraintsAreOk=false;
            }
    }
    else
    {
        message += wxString::Format(wxT("  %s id=%d(%s)\n"),m_obj2->GetTypeString(),m_obj2->m_id,((m_obj2==mom)?wxT("KNOWN"):wxT("UNKNOWN")));
        message.Pad(level*3+3,' ',true);
    }
    if (ShowMsgInConsole)
    {
        wxPuts(message);
    }
    if (*ConstraintsAreOk==true)
    {
        message = wxT("Searching:");
        *ConstraintsAreOk =ValidateConstraint2Objects(SketchPtr,mom,this,level, ShowMsgInConsole);
        if (ShowMsgInConsole)
        {
            wxPuts(message);
        }
    }
}

bool Constraint::ValidateConstraint2Objects(HeeksObj * Sketch,HeeksObj * ConstrainedObject,HeeksObj * Constraint,int FromLevel,bool ShowMsgInConsole)
{

    bool TestsareOK =true; // assume all is well unless we run into a problem

    int StdNumberOfObjectsInConstraint = ReturnStdObjectCtForConstraint(m_type);
    wxString message = wxString::Format(wxT(" Objs:(%i)"),StdNumberOfObjectsInConstraint); //Need to determine if the constraint is one or two objects
    message +=_("");
    if (StdNumberOfObjectsInConstraint==1)
    {
        //this is a single object constraint which means
        if((m_obj1 ==ConstrainedObject)&&(m_obj2 ==NULL))
        {
            //at some point may want to check the object type against the contraint type to see if it makes sense
            //we wouldn't want a point called out with a horizontal constraint as an example
        }
        else
        {
            message += _("Object error in a single object constraint");
            TestsareOK =false;
        }
    }
    else if (StdNumberOfObjectsInConstraint==2)
    {
        if((m_obj1 ==ConstrainedObject)&&(m_obj2 !=NULL))
        {
            TestsareOK =Validate2ndObjectInConstraint(Sketch,m_obj2,this,FromLevel, ShowMsgInConsole);

        }

        else if ((m_obj1 !=NULL)&&(m_obj2 ==ConstrainedObject))
        {
            TestsareOK =Validate2ndObjectInConstraint(Sketch,m_obj1,this,FromLevel, ShowMsgInConsole);
        }
        else
        {
            message += _("Object error in a two object constraint");
            TestsareOK =false;
        }

    }
    else if (StdNumberOfObjectsInConstraint==0)
    {
        message += _("Undefined Constraint?");
        TestsareOK =false;
    }
    else
    {


    }
    if (!TestsareOK)
        if (ShowMsgInConsole)wxPuts(message);//just need to here about a problem if there is one

    return TestsareOK ;

}

//JT I don't like defining this list globally but I can't think of away around this at the moment




bool Constraint::Validate2ndObjectInConstraint(HeeksObj* aSketch,HeeksObj*  aConstrainedObject,HeeksObj* aConstraint,int FromLevel,bool ShowMsgInConsole)
{
    //JT
    //Ok.. this is where things get interesting.
    //A constraint can be the child of two objects.
    //One object of a two object constraint has already been validated if we reached this point
    //The second object takes a little more work.
    //What is known:
    //  sketch location
    //  2nd constrained object location.
    //  constraint
    //What needs to be validated?
    //  Is the second constrained object contained in the sketch?
    //      Does this object contain the constraint in its m_obj's list
    //Additional things to ponder:
    //If I understand this correctly,within a sketch a heeksobj should be a unique within objlist, but the constraint itself can
    //be either once or twice.
    //It's probably safe to assume the first statement,but I'm not sure if that's an assumption that can be made across all sketches.
    bool everythingOk = true;
    int *occurence = new int;
    *occurence =0;

    HeeksObjOccurrenceInSketch(aSketch,aConstrainedObject,occurence,FromLevel, ShowMsgInConsole);
    wxString occurencemsg=wxString::Format(wxT("Occurences:%d " ) ,*occurence);

    if ((*occurence)!=1)
    {
        everythingOk=false;
        //This looks promising if we go to here
        //now we need to verify that the ConstrainedObject has the constraint as a child
    }

    delete occurence;
    return everythingOk;

}

int Constraint::ReturnStdObjectCtForConstraint(EnumConstraintType etype)
{
//Sometimes it seems constraints are defined with the wrong object count. We need to know what we're supposed to have
//This is just defined within the object. Wonder if this would be handy to broaden it out?
    switch (etype)
    {
        case ParallelLineConstraint:
        case PerpendicularLineConstraint:
        case LineTangentConstraint:
        case RadiusConstraint:
        case EqualLengthConstraint:
        case ColinearConstraint:
        case EqualRadiusConstraint:
        case ConcentricConstraint:
        case PointOnLineConstraint:
        case PointOnLineMidpointConstraint:
        case PointOnArcMidpointConstraint:
        case PointOnArcConstraint:
        case PointOnCircleConstraint:
        case CoincidantPointConstraint:// This is a common bug You need to variables.
            return(2);
            break;
        case AbsoluteAngleConstraint:
        case LineLengthConstraint:
        case LineHorizontalLengthConstraint:
        case LineVerticalLengthConstraint:
        case FixedPointConstraint:
            return(1);
            break;
        default:
            return(0);
                    //todo need to figure out how to send back an errmessage
                    //errorMessage += _("\n Check to see  if a new constraint type has been added Err ref 201011200807");
    }
}

#endif



HeeksObj* Constraint::ReadFromXMLElement(TiXmlElement* pElem)
{
	const char* type=0;
	EnumConstraintType etype=(EnumConstraintType)0;
	const char* angle=0;
	EnumAbsoluteAngle eangle=(EnumAbsoluteAngle)0;
	double length=0;
	int obj1_id=0;
	int obj2_id=0;
	int obj1_type=0;
	int obj2_type=0;
	ConstrainedObject* obj1=0;
	ConstrainedObject* obj2=0;
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "type"){type = a->Value();}
		else if(name == "angle"){angle = a->Value();}
		else if(name == "length"){length = a->DoubleValue();}
		else if(name == "obj1_id"){obj1_id = a->IntValue();}
		else if(name == "obj1_type"){obj1_type = a->IntValue();}
		else if(name == "obj2_id"){obj2_id = a->IntValue();}
		else if(name == "obj2_type"){obj2_type = a->IntValue();}
	}

	//Ugh, we need to convert the strings back into types
	for(unsigned i=0; i < sizeof(ConstraintTypes); i++)
	{
		if(strcmp(ConstraintTypes[i].c_str(),type)==0)
		{
			etype = (EnumConstraintType)i;
			break;
		}
	}

	for(unsigned i=0; i < sizeof(AbsoluteAngle); i++)
	{
		if(strcmp(AbsoluteAngle[i].c_str(),angle)==0)
		{
			eangle = (EnumAbsoluteAngle)i;
			break;
		}
	}


	//JT
	//Ok.. There is a problem here.. Further up stream there a conditions where the xml has been written out for obj1_id, obj2_id != 0 for objects that don't exist
	//This is a huge pain, since the error is being introduced on the file save, and doesn't show up, until you load the file.
	// Sometimes you get a segmentation fault right at load... Or when you're really lucky it shows up when solvesketch kicks in and you have no clue whats going on.
    // At this point,until these critters get irridicated CheckforValidConstraint basically tries to see if the constraint makes sense
    // I hope to install this at both ends when constraints are being written out or read in my attempt to identify, isolate and irradicate these most annoying
    // critters

#ifdef CHECK_FOR_INVALID_CONSTRAINT
    bool blockConstraint =false;
    blockConstraint =  checkForInvalidConstraint(type,etype,angle,eangle,length,obj1_id,obj2_id,obj1_type,obj2_type);
    if (blockConstraint)
	{
      return NULL;
    }
#endif

	//Get real pointers to the objects
	obj1 = (ConstrainedObject*)wxGetApp().GetIDObject(obj1_type,obj1_id);
	obj2 = (ConstrainedObject*)wxGetApp().GetIDObject(obj2_type,obj2_id);

	Constraint *c = new Constraint(etype,eangle,length,obj1,obj2);

	unsigned int len;
	char *str = new char[512];
	char *cstr = str;

	printf("Searched for: 0x%X:0x%X, 0x%X:0x%X\n", obj1_type, obj1_id, obj2_type, obj2_id);
	if(obj1)
	{
		obj1->constraints.push_back(c);
		obj1->ToString(str,&len,512);
		cstr = str+len;
	}
	if(obj2)
	{
		obj2->constraints.push_back(c);
		obj2->ToString(cstr,&len,512);
		cstr = cstr+len;
	}
	printf("%s",str);

	//Don't let the xml reader try to insert us in the tree
	return NULL;
}
