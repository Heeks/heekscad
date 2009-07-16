// MultiPoly.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Sketch.h"
#include "EndedObject.h"
#include "BentleyOttmann.h"
//This algorithm takes an array of complex sketches (CSketch* constaining multiple closed paths)
//And creates a new set of paths that are no longer self intersecting
//these lists are returns an array of trees of objects
//each level of the trees corresponds to whether or not the objects should be added or removed
//
//all sketches must only contain closed shapes

std::vector<MyLine> shapes;

void MultiPoly(std::list<CSketch*> sketches)
{
	shapes.clear();
	//first pass: build lists of closed shapes
	std::list<CSketch*>::iterator it;
	for(it = sketches.begin(); it!= sketches.end(); ++it)
	{
         CSketch* sketch = *it;
		 //Copy this sketches objects into a new list
		 std::list<EndedObject*> sketchobjs;
		 HeeksObj* obj = sketch->GetFirstChild();
		 while(obj)
		 {
			//TODO: for now we only handle EndedObject
		    EndedObject* eobj = dynamic_cast<EndedObject*>(obj);
			if(eobj)
			{
				MyLine ntobj(eobj->A->m_p,eobj->B->m_p);
				shapes.push_back(ntobj);
			}
			obj = sketch->GetNextChild();
		 }
	}
	//Get a list of all the intersection points
	std::map<MyLine*, std::vector<Intersection> > intersections = Intersections(shapes);

	std::map<double, std::map<double, std::vector<BoundedCurve> > >bcurves;

	//Create a new list of bounded segment objects. Whose endpoints are locatable via hash
	//with the exception that the hash requires a search of the 4 adjacent elements(if they exist)
	std::map<MyLine*, std::vector<Intersection> >::iterator it2;
	for(it2 = intersections.begin(); it2 != intersections.end(); ++it2)
	{
		MyLine *tline = (*it2).first;
		std::vector<Intersection> inter = (*it2).second;
		double startu=tline->GetU(inter[0].X,inter[0].Y);
		for(int i=1; i < inter.size(); i++)
		{
			double newu=tline->GetU(inter[i].X,inter[i].Y);
			bcurves[MyRound(inter[i-1].X)][MyRound(inter[i-1].Y)].push_back(BoundedCurve(tline,startu,newu));
			bcurves[MyRound(inter[i].X)][MyRound(inter[i].Y)].push_back(BoundedCurve(tline,startu,newu));
			startu = newu;
		}
	}

	//Create a new tree of boundedcurves, that is much smaller. follow all chains and attempt to remove
	//segments that are connected to only 2 other curves. This will yield a non-orientable graph
	//so our definition of polygons better be very graph theoretical
}

