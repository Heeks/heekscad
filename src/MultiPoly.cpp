// MultiPoly.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Sketch.h"
#include "EndedObject.h"
#include "MultiPoly.h"

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
	std::map<MyLine*, std::vector<Intersection> > intersections = Intersections(shapes);

	std::map<double, std::map<double, std::vector<CompoundSegment*> > >bcurves;

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
			CompoundSegment* segment = new CompoundSegment(tline,startu,newu);
			bcurves[MyRound(inter[i-1].X)][MyRound(inter[i-1].Y)].push_back(segment);
			bcurves[MyRound(inter[i].X)][MyRound(inter[i].Y)].push_back(segment);
			startu = newu;
		}
	}

	//Fix up that 4 adjacent elements problem
	std::map<double, std::map<double, std::vector<CompoundSegment*> > >::iterator it3;
	std::map<double, std::vector<CompoundSegment*> > *last_x=NULL;
	double last_x_coord;
	for(it3 = bcurves.begin(); it3 != bcurves.end();it3++)
	{
		std::map<double, std::vector<CompoundSegment*> > *this_x=&(*it3).second;
		std::map<double, std::vector<CompoundSegment*> >::iterator it4;
		std::vector<CompoundSegment*> *last_y = NULL;
		double this_x_coord = (*it3).first;
		double last_y_coord;
		for(it4 = (*it3).second.begin(); it4 != (*it3).second.end();)
		{
			std::map<double, std::vector<CompoundSegment*> >::iterator it5 = it4++;
			std::vector<CompoundSegment*> *this_y = &(*it4).second;
			double this_y_coord = (*it3).first;
			bool erasedy=false;
			if(last_x && this_x - last_x < 1.6 * tol)
			{
				for(int i=0; i < bcurves[this_x_coord][this_y_coord].size(); i++)
					bcurves[last_x_coord][this_y_coord].push_back(bcurves[this_x_coord][this_y_coord][i]);
				bcurves[this_x_coord].erase(it5);
				erasedy=true;
			}
			else if(last_y && this_y - last_y < 1.5 * tol)
			{
				for(int i=0; i < bcurves[this_x_coord][this_y_coord].size(); i++)
					bcurves[this_x_coord][last_y_coord].push_back(bcurves[this_x_coord][this_y_coord][i]);
				bcurves[this_x_coord].erase(it5);
				erasedy=true;
			}
			last_y = 0;
			if(!erasedy)
			{
				last_y = &(*it4).second;
				last_y_coord = (*it4).first;
			}
		}
		last_x = &(*it3).second;
		last_x_coord = (*it3).first;
	}
	//Create a new tree of boundedcurves, that is much smaller. follow all chains and attempt to remove
	//segments that are connected to only 2 other curves. This will yield a non-orientable graph
	//so our definition of polygons better be very graph theoretical

	for(it3 = bcurves.begin(); it3 != bcurves.end();)
	{
		std::map<double, std::vector<CompoundSegment*> >::iterator it4;
		for(it4 = (*it3).second.begin(); it4 != (*it3).second.end();)
		{
			//TODO: should check the 4 adjacent nodes
			if((*it4).second.size() != 2)
			{
				++it4;
				continue;
			}

			//Concatenate the 2 groups and remove *it4 from the map
			CompoundSegment* seg1 = (*it4).second[0];
			CompoundSegment* seg2 = (*it4).second[1];

			seg1->Add(seg2,(*it3).first,(*it4).first);

			//Must find the pointer at the end of seg2 and change it
			gp_Pnt begin = seg2->Begin();
			if(MyIsEqual(begin.X(),(*it3).first) && MyIsEqual(begin.Y(),(*it4).first))
			{
				gp_Pnt end = seg2->End();
				std::replace(bcurves[MyRound(end.X())][MyRound(end.Y())].begin(),bcurves[MyRound(end.X())][MyRound(end.Y())].end(),seg2,seg1);
			}
			else
			{
				std::replace(bcurves[MyRound(begin.X())][MyRound(begin.Y())].begin(),bcurves[MyRound(begin.X())][MyRound(begin.Y())].end(),seg2,seg1);

			}
			//remove from the map
			std::map<double, std::vector<CompoundSegment*> >::iterator it5=it4++;
			bcurves[(*it3).first].erase(it5);
		}
		if(bcurves[(*it3).first].size() == 0)
		{
			std::map<double, std::map<double, std::vector<CompoundSegment*> > >::iterator it5 = it3++;
			bcurves.erase(it5);
		}
		else
			++it3;
	}

	//Now we have a graph of CompoundSegment*. These should be fast to traverse. 
	//TODO: something happens to non self intersecting shapes. They either dissapear, or become a single CompoundSegment
	//we should probably know which one. 

}

