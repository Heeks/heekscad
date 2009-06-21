// ConstraintTools.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "../interface/Tool.h"

void GetConstraintMenuTools(std::list<Tool*>* t_list);
void ApplyCoincidentConstraints(HeeksObj* extobj, std::list<HeeksObj*> list);

