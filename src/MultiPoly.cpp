// MultiPoly.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Sketch.h"
#include "EndedObject.h"
#include "BentleyOttmann.h"
#include "SimpleIntersector.h"
#include "MultiPoly.h"
#include "HArc.h"

//This algorithm takes an array of complex sketches (CSketch* constaining multiple closed paths)
//And creates a new set of paths that are no longer self intersecting
//these lists are returns an array of trees of objects
//each level of the trees corresponds to whether or not the objects should be added or removed
//
//all sketches must only contain closed shapes

std::vector<FastCurve*> shapes;
extern double tol;
std::vector<TopoDS_Face> MultiPoly(std::list<CSketch*> sketches)
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
				HArc* arc = dynamic_cast<HArc*>(obj);
				if(arc)
				{
					shapes.push_back(new FastArc(arc->A->m_p,arc->B->m_p,arc->C->m_p,arc->m_axis.Direction().Z() > 0, arc->GetCircle()));
				}
				else
					shapes.push_back(new FastLine(eobj->A->m_p,eobj->B->m_p));
			}
			obj = sketch->GetNextChild();
		 }
	}
	//Get a list of all the intersection points
	Intersector *m_int = new SimpleIntersector();
	std::map<FastCurve*, std::vector<Intersection> > intersections = m_int->Intersect(shapes);

	//Make our flashy double hashmap
	TwoDNearMap bcurves(tol);

	//Create a new list of bounded segment objects. Whose endpoints are locatable via hash
	//with the exception that the hash returns values within tolerance of the specified point
	std::map<FastCurve*, std::vector<Intersection> >::iterator it2;
	for(it2 = intersections.begin(); it2 != intersections.end(); ++it2)
	{
		FastCurve *tline = (*it2).first;
		std::vector<Intersection> inter = (*it2).second;
		for(unsigned i=0; i < inter.size()-1; i++)
		{
			double startu=tline->GetU(inter[i].X,inter[i].Y);
			double newu=tline->GetU(inter[i+1].X,inter[i+1].Y);
			CompoundSegment* segment;
			if(startu < newu)
				segment = new CompoundSegment(tline,tol,startu,newu);
			else
				segment = new CompoundSegment(tline,tol,newu,startu);
			bcurves.insert(inter[i].X,inter[i].Y,segment);
			bcurves.insert(inter[i+1].X,inter[i+1].Y,segment);
			startu = newu;
		}
	}

	//This gets the hashtable working
	bcurves.sort();

	AnalyzeNearMap(bcurves);

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

			ConcatSegments(x_coord,y_coord,seg1,seg2,bcurves);
		}
	}

	//Now we have a graph of CompoundSegment*. These should be fast to traverse.
	//Non self intersecting shapes are already in closed_shapes
	//We could speed this up by regenerating near_map to get rid of removed references

	bool done=false;
	while(!done)
	{
		bool found=false;
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

				//for most cases, returnvec should have 4 elements. more complicated cases have an even number > 4
				//check for elements that are the same, which mean this polygon could terminate here, so terminate it
				//and merge the other CompoundSegments if there are only two remaining
				int nfound=0;
				for(size_t k=0; k < returnvec.size(); k++)
				{
					for(size_t l=k+1; l < returnvec.size(); l++)
					{
						if(returnvec[k] == returnvec[l])
						{
							//this means we have found a closed shape. Remove it from the bcurves and add it to a list of closed shapes
							//remove from the map
							bcurves.remove(x_coord,y_coord,returnvec[k]);
							bcurves.remove(x_coord,y_coord,returnvec[l]);
							closed_shapes.push_back((CompoundSegment*)returnvec[k]);
							nfound+=2;
							found=true;
						}
					}
				}

				//now we know that no 2 elements in returnvec are equal.
				//make sure there are the right number of elements for our next op
				if(returnvec.size() - nfound != 2)
					continue;

				//Quick and dirty way to get the pointers
				returnvec.clear();
				bcurves.find(x_coord,y_coord,returnvec);

				//Merge the segments and get them out of this coordinate
				ConcatSegments(x_coord,y_coord,(CompoundSegment*)returnvec[0],(CompoundSegment*)returnvec[1],bcurves);
				found=true;
			}
		}
		if(!found)
			done = true;
	}

	//Now that we have all closed shapes, we need to define the relationships. Since we know that they are not intersecting
	//3 kinds of things can happen. A shape is either inside, enclosing, adjacent, or unrelated to another.

	std::vector<std::vector<CompoundSegment*> > inside_of;
	inside_of.resize(closed_shapes.size());

	for(unsigned i=0; i < closed_shapes.size(); i++)
	{
		for(unsigned j=0; j < closed_shapes.size(); j++)
		{
			//We can determine if a shape is inside or outside by finding the winding number of just 1 point with the
			//entire other polygon

			if(i==j)
				continue;

			int rays = closed_shapes[i]->GetRayIntersectionCount(closed_shapes[j]->Begin());
			if(rays%2)
			{
				//Polygon J is inside of polygon I
				inside_of[j].push_back(closed_shapes[i]);
				int x=0;
				x++;
			}
		}
	}

	//This is used to reverse the ordering of a closed shape. Getting it to be CW or CCW
	//OCC is picky about the ordering when the polygon has holes

	for(unsigned i=0; i < closed_shapes.size(); i++)
	{
		closed_shapes[i]->Order();
#ifdef FORCEPOLYGONORDERING
		bool cw = closed_shapes[i]->GetCW();
		if(!cw)
		{
			closed_shapes[i]->Reverse();
			closed_shapes[i]->Order();
		}
#endif
	}


	//Sort these lists for easy comparison
	for(unsigned i=0; i < inside_of.size(); i++)
	{
		std::sort(inside_of[i].begin(),inside_of[i].end());
	}

	//Now we want to descend into this thing. First find all shapes that are inside of nothing else
	std::vector<std::pair<CompoundSegment*,std::vector<CompoundSegment*> > > gold;
	find_level(true,gold,closed_shapes,inside_of,std::vector<CompoundSegment*>());

	return TopoDSFaceAdaptor(gold);
}

//This is a recursive function that will analyze the graph for islands and such
//It could be sped up by removing elements from closed_shapes and inside_of as the get consumed

std::vector<CompoundSegment*> find_level(bool odd,
				std::vector<std::pair<CompoundSegment*,std::vector<CompoundSegment*> > > &pRet,
				std::vector<CompoundSegment*>& closed_shapes,
				std::vector<std::vector<CompoundSegment*> >& inside_of,
				std::vector<CompoundSegment*> parents)
{
	std::vector<CompoundSegment*> retValue;

	for(unsigned i=0; i < closed_shapes.size(); i++)
	{
		if(inside_of[i].size() != parents.size())
			continue;

		bool no_match = false;
		for(unsigned j=0; j < inside_of[i].size(); j++)
		{
			if(inside_of[i][j] != parents[j])
			{
				no_match = true;
				break;
			}
		}
		if(no_match)
			continue;

		//this closed shape is good to go

		std::vector<CompoundSegment*> nparents = parents;
		nparents.push_back(closed_shapes[i]);
		std::sort(nparents.begin(), nparents.end());

		std::vector<CompoundSegment*> deletions = find_level(!odd,pRet,closed_shapes,inside_of,nparents);

		if(odd)
		{
			pRet.push_back(std::pair<CompoundSegment*,std::vector<CompoundSegment*> >(closed_shapes[i],deletions));
		}
		else
			retValue.push_back(closed_shapes[i]);
	}
	return retValue;
}

void ConcatSegments(double x_coord, double y_coord, CompoundSegment* seg1, CompoundSegment* seg2, TwoDNearMap &bcurves)
{
	seg1->Add(seg2,x_coord,y_coord);

	//Must find the pointer at the end of seg2 and change it
	gp_Pnt begin = seg2->Begin();
	gp_Pnt end = seg2->End();
	if(MyIsEqual(begin.X(),x_coord) && MyIsEqual(begin.Y(),y_coord))
		bcurves.remap(end.X(),end.Y(),seg2,seg1);
	else
	{
		if(MyIsEqual(end.X(),x_coord) && MyIsEqual(end.Y(),y_coord))
			bcurves.remap(begin.X(),begin.Y(),seg2,seg1);
		else
		{
			//kaboom
			int x=0;
			x++;
		}
	}
	//remove from the map
	bcurves.remove(x_coord,y_coord,seg1);
	bcurves.remove(x_coord,y_coord,seg2);

	//delete seg2;

#ifdef TESTORDERING
	seg1->Order();
#endif
}

TopoDS_Wire TopoDSWireAdaptor(CompoundSegment* poly, bool inside)
{
	std::list<TopoDS_Edge> edges;
	//if(inside)
		//poly->();
	poly->GetEdges(edges);

	BRepLib_MakeWire wire_maker;
	std::list<TopoDS_Edge>::iterator It;
	for(It = edges.begin(); It != edges.end(); It++)
	{
		TopoDS_Edge &edge = *It;
	//	BRepLib_BuildCurve3d( edge );
		wire_maker.Add(edge);
	}
	TopoDS_Wire wire = wire_maker.Wire();

//	if(inside)
//		wire = TopoDS::Wire(wire.Oriented(TopAbs_REVERSED));
//	else
		wire = TopoDS::Wire(wire.Oriented(TopAbs_FORWARD));
	return wire;
}

std::vector<std::pair<TopoDS_Wire,std::vector<TopoDS_Wire> > > TopoDSWireAdaptor(
	std::vector<std::pair<CompoundSegment*,std::vector<CompoundSegment*> > > &data)
{
	std::vector<std::pair<TopoDS_Wire,std::vector<TopoDS_Wire> > > ret;
	for(unsigned i=0; i < data.size(); i++)
	{
		TopoDS_Wire outside = TopoDSWireAdaptor(data[i].first,false);
		std::vector<TopoDS_Wire> insides;
		for(unsigned j=0; j < data[i].second.size(); j++)
			insides.push_back(TopoDSWireAdaptor(data[i].second[j],true));
		ret.push_back(std::pair<TopoDS_Wire,std::vector<TopoDS_Wire> >(outside,insides));
	}
	return ret;
}

std::vector<TopoDS_Face> TopoDSFaceAdaptor(
	std::vector<std::pair<CompoundSegment*,std::vector<CompoundSegment*> > > &data)
{
	std::vector<TopoDS_Face> faces;
	std::vector<std::pair<TopoDS_Wire,std::vector<TopoDS_Wire> > > wires;
	wires = TopoDSWireAdaptor(data);

	for(unsigned i=0; i < wires.size(); i++)
	{
		BRepBuilderAPI_MakeFace makeFace(wires[i].first);
		for(unsigned j=0; j < wires[i].second.size(); j++)
			makeFace.Add(wires[i].second[j]);

		faces.push_back(makeFace.Face());
	}

	return faces;
}

void AnalyzeNearMap(TwoDNearMap &bcurves)
{
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

			int x=0;
			x++;
		}
	}
}

