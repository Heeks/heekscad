// PropertyChange.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "PropertyChange.h"
#include "../interface/PropertyString.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "../interface/PropertyInt.h"
#include "../interface/PropertyColor.h"
#include "../interface/PropertyVertex.h"
#include "PropertyTrsf.h"
#include "../interface/PropertyChoice.h"

PropertyChangeString::PropertyChangeString(const wxString& value, PropertyString* property)
{
	m_value = value;
	m_old = ((PropertyString*)property)->m_initial_value;
	m_callbackfunc = ((PropertyString*)property)->m_callbackfunc;
	m_object = ((PropertyString*)property)->m_object;
}

void PropertyChangeString::Run(bool redo)
{
	(*(m_callbackfunc))(m_value, m_object);
	wxGetApp().WasModified(m_object);
}

void PropertyChangeString::RollBack()
{
	(*(m_callbackfunc))(m_old, m_object);
	wxGetApp().WasModified(m_object);
}

PropertyChangeDouble::PropertyChangeDouble(const double& value, PropertyDouble* property)
{
	m_value = value;
	m_old = ((PropertyDouble*)property)->m_initial_value;
	m_callbackfunc = ((PropertyDouble*)property)->m_callbackfunc;
	m_object = ((PropertyDouble*)property)->m_object;
}

void PropertyChangeDouble::Run(bool redo)
{
	(*(m_callbackfunc))(m_value, m_object);
	wxGetApp().WasModified(m_object);
}

void PropertyChangeDouble::RollBack()
{
	(*(m_callbackfunc))(m_old, m_object);
	wxGetApp().WasModified(m_object);
}

PropertyChangeLength::PropertyChangeLength(const double& value, PropertyLength* property)
{
	m_value = value;
	m_old = ((PropertyLength*)property)->m_initial_value * wxGetApp().m_view_units;
	m_callbackfunc = ((PropertyLength*)property)->m_callbackfunc;
	m_object = ((PropertyLength*)property)->m_object;
}

void PropertyChangeLength::Run(bool redo)
{
	(*(m_callbackfunc))(m_value, m_object);
	wxGetApp().WasModified(m_object);
}

void PropertyChangeLength::RollBack()
{
	(*(m_callbackfunc))(m_old, m_object);
	wxGetApp().WasModified(m_object);
}

PropertyChangeInt::PropertyChangeInt(const int& value, PropertyInt* property)
{
	m_value = value;
	m_old = ((PropertyInt*)property)->m_initial_value;
	m_callbackfunc = ((PropertyInt*)property)->m_callbackfunc;
	m_object = ((PropertyInt*)property)->m_object;
}

void PropertyChangeInt::Run(bool redo)
{
	(*(m_callbackfunc))(m_value, m_object);
	wxGetApp().WasModified(m_object);
}

void PropertyChangeInt::RollBack()
{
	(*(m_callbackfunc))(m_old, m_object);
	wxGetApp().WasModified(m_object);
}

PropertyChangeColor::PropertyChangeColor(const HeeksColor& value, PropertyColor* property)
{
	m_value = value;
	m_old = ((PropertyColor*)property)->m_initial_value;
	m_callbackfunc = ((PropertyColor*)property)->m_callbackfunc;
	m_object = ((PropertyColor*)property)->m_object;
}

void PropertyChangeColor::Run(bool redo)
{
	(*(m_callbackfunc))(m_value, m_object);
	wxGetApp().WasModified(m_object);
}

void PropertyChangeColor::RollBack()
{
	(*(m_callbackfunc))(m_old, m_object);
	wxGetApp().WasModified(m_object);
}

PropertyChangeVertex::PropertyChangeVertex(const double* value, PropertyVertex* property)
{
	memcpy(m_value, value, 3 * sizeof(double));
	memcpy(m_old, ((PropertyVertex*)property)->m_x, 3 * sizeof(double));
	m_callbackfunc = ((PropertyVertex*)property)->m_callbackfunc;
	m_object = ((PropertyVertex*)property)->m_object;
}

void PropertyChangeVertex::Run(bool redo)
{
	(*(m_callbackfunc))(m_value, m_object);
	wxGetApp().WasModified(m_object);
}

void PropertyChangeVertex::RollBack()
{
	(*(m_callbackfunc))(m_old, m_object);
	wxGetApp().WasModified(m_object);
}

PropertyChangeTrsf::PropertyChangeTrsf(const gp_Trsf &value, PropertyTrsf* property)
{
	m_value = value;
	m_old = ((PropertyTrsf*)property)->m_trsf;
	m_callbackfunc = ((PropertyTrsf*)property)->m_callbackfunc;
	m_object = ((PropertyTrsf*)property)->m_object;
}

void PropertyChangeTrsf::Run(bool redo)
{
	(*(m_callbackfunc))(m_value, m_object);
	wxGetApp().WasModified(m_object);
}

void PropertyChangeTrsf::RollBack()
{
	(*(m_callbackfunc))(m_old, m_object);
	wxGetApp().WasModified(m_object);
}

PropertyChangeChoice::PropertyChangeChoice(const int& value, PropertyChoice* property)
{
	m_value = value;
	m_old = ((PropertyChoice*)property)->m_initial_index;
	m_callbackfunc = ((PropertyChoice*)property)->m_callbackfunc;
	m_object = ((PropertyChoice*)property)->m_object;
}

void PropertyChangeChoice::Run(bool redo)
{
	(*(m_callbackfunc))(m_value, m_object, redo);
	wxGetApp().WasModified(m_object);
}

void PropertyChangeChoice::RollBack()
{
	(*(m_callbackfunc))(m_old, m_object, true);
	wxGetApp().WasModified(m_object);
}
