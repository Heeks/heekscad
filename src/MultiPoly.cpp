// MultiPoly.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Sketch.h"
#include "EndedObject.h"
#include "BentleyOttmann.h"
#include "SimpleIntersector.h"
#include "MultiPoly.h"
#include "NearMap.h"

//This algorithm takes an array of complex sketches (CSketch* constaining multiple closed paths)
//And creates a new set of paths that are no longer self intersecting
//these lists are returns an array of trees of objects
//each level of the trees corresponds to whether or not the objects should be added or removed
//
//all sketches must only contain closed shapes

std::vector<MyLine> shapes;
extern double tol;
void MultiPoly(std::list<CSketch*> sketches)
{
	tol = wxGetApp().m_geom_tol;
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
	Intersector *m_int = new SimpleIntersector();
	std::map<MyLine*, std::vector<Intersection> > intersections = m_int->Intersect(shapes);

	//TODO: either calculate reasonable values for x_vectors and y_vectors, or fix NearMap to not require it
	TwoDNearMap bcurves(tol);

	//Create a new list of bounded segment objects. Whose endpoints are locatable via hash
	//with the exception that the hash requires a search of the 4 adjacent elements(if they exist)
	std::map<MyLine*, std::vector<Intersection> >::iterator it2;
	for(it2 = intersections.begin(); it2 != intersections.end(); ++it2)
	{
		MyLine *tline = (*it2).first;
		std::vector<Intersection> inter = (*it2).second;
		double startu=tline->GetU(inter[0].X,inter[0].Y);
		for(unsigned i=1; i < inter.size(); i++)
		{
			double newu=tline->GetU(inter[i].X,inter[i].Y);
			CompoundSegment* segment = new CompoundSegment(tline,tol,startu,newu);
			bcurves.insert(inter[i-1].X,inter[i-1].Y,segment);
			bcurves.insert(inter[i].X,inter[i].Y,segment);
			startu = newu;
		}
	}

	bcurves.sort();

	std::vector<CompoundSegment*> closed_shapes;

	//Create a new tree of boundedcurves, that is much smaller. follow all chains and attempt to remove
	//segments that are connected to only 2 other curves. This will yield a non-orientable graph
	//so our definition of polygons better be very graph theoretical
	std::vector<void*> returnvec; 

	for(int i=0; i < bcurves.GetVecCount(); i++)
	{
		OneDNearMap* ptMap = bcurves.GetElement(i);
		double x_coord = bcurves.GetCoord(i);
		for(int j=0; j < ptMap->GetVecCount(); j++)
		{
			double y_coord = ptMap->GetCoord(j);
			if(!ptMap->IsValid(j))
				continue;

			returnvec.clear();
			bcurves.find(x_coord,y_coord,returnvec);

			if(returnvec.size() == 1)
			{
				//TODO: this means the current segment is part of an unclosed shape. However it is not clear that
				//this shape is fully concatenated or does not intersect a closed shape. these should probably be removed
				//prior to this loop
				continue;
			}

			if(returnvec.size() != 2)
				continue;

			//Concatenate the 2 groups and remove *it4 from the map
			CompoundSegment* seg1 = (CompoundSegment*)returnvec[0];
			CompoundSegment* seg2 = (CompoundSegment*)returnvec[1];

			if(seg1 == seg2)
			{
				//this means we have found a closed shape. Remove it from the bcurves and add it to a list of closed shapes
				//remove from the map
				bcurves.remove(x_coord,y_coord,seg1);
				bcurves.remove(x_coord,y_coord,seg2);
				closed_shapes.push_back(seg1);
				continue;
			}

			seg1->Add(seg2,x_coord,y_coord);

			//Must find the pointer at the end of seg2 and change it
			gp_Pnt begin = seg2->Begin();
			if(MyIsEqual(begin.X(),x_coord) && MyIsEqual(begin.Y(),y_coord))
			{
				gp_Pnt end = seg2->End();
				bcurves.remap(end.X(),end.Y(),seg2,seg1);
			}
			else
			{
				bcurves.remap(begin.X(),begin.Y(),seg2,seg1);
			}
			//remove from the map
			bcurves.remove(x_coord,y_coord,seg1);
			bcurves.remove(x_coord,y_coord,seg2);
		}
	}

	//Now we have a graph of CompoundSegment*. These should be fast to traverse. 
	//Non self intersecting shapes are already in closed_shapes
	//We could speed this up by regenerating near_map to get rid of removed references

	for(int i=0; i < bcurves.GetVecCount(); i++)
	{
		OneDNearMap* ptMap = bcurves.GetElement(i);
		double x_coord = bcurves.GetCoord(i);
		for(int j=0; j < ptMap->GetVecCount(); j++)
		{
			double y_coord = ptMap->GetCoord(j);
			if(!ptMap->IsValid(j))
				continue;

			returnvec.clear();
			bcurves.find(x_coord,y_coord,returnvec);

			CompoundSegment *segment = (CompoundSegment*)returnvec[0];

			//We've got something to look at now. What to do, What to do.
		}
	}

	//Now that we have all closed shapes, we need to define the relationships. Since we know that they are not intersecting
	//3 kinds of things can happen. A shape is either inside, enclosing, adjacent, or unrelated to another.

	//Get the closed shapes well ordered
	for(int i=0; i < closed_shapes.size(); i++)
	{
		closed_shapes[i]->Order();
		double area = closed_shapes[i]->GetArea();

		int x=0;
		x++;
	}

	for(int i=0; i < closed_shapes.size(); i++)
	{
		for(int j=i+1; j < closed_shapes.size(); j++)
		{
			//We can determine if a shape is inside or outside by finding the winding number of just 1 point with the
			//entire other polygon

			closed_shapes[i]->GetWindingNumber(closed_shapes[j]->Begin());
		}
	}
}

