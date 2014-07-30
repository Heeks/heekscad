// Part.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Part.h"
#include "Pad.h"
#include "Pocket.h"

CPart::CPart()
{

}

CPart::~CPart()
{

}

void CPart::glCommands(bool select, bool marked, bool no_color)
{
	DynamicSolid::glCommands(select,marked,no_color);

	Update();
}

void CPart::Update()
{
	HeeksObj *child = GetFirstChild();
	TopoDS_Shape shape;
	TopoDS_Shape new_shape;
	bool shape_found = false;
	while(child)
	{
		CPad* pad = dynamic_cast<CPad*>(child);
		if(pad)
		{
			BRepBuilderAPI_Transform trsf(pad->GetTransform());

			std::list<TopoDS_Shape>::iterator it;
			for(it = pad->m_shapes.begin(); it != pad->m_shapes.end(); it++)
			{
				if(!shape_found)
				{
					shape_found = true;
					new_shape = *it;
					trsf.Perform(new_shape);
					shape = trsf.Shape();
				}
				else
				{
					new_shape = *it;
					trsf.Perform(new_shape);
					shape = BRepAlgoAPI_Fuse(shape,trsf.Shape());
				}
			}
		}

		HPocket* pocket = dynamic_cast<HPocket*>(child);
		if(pocket)
		{
			BRepBuilderAPI_Transform trsf(pocket->GetTransform());

			std::list<TopoDS_Shape>::iterator it;
			for(it = pocket->m_shapes.begin(); it != pocket->m_shapes.end(); it++)
			{
				if(shape_found)
				{
					new_shape = *it;
					trsf.Perform(new_shape);
					shape = BRepAlgoAPI_Cut(shape,trsf.Shape());
				}
			}

		}
		child = GetNextChild();
	}

	std::list<TopoDS_Shape> shapes;
	shapes.push_back(shape);
	SetShapes(shapes);
}

