// MultiPoly.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.


#pragma once

#include "BentleyOttmann.h"
#include "NearMap.h"

std::vector<TopoDS_Face> MultiPoly(std::list<CSketch*> sketches);

class BoundedCurve
{
public:
	FastCurve* line;
	double startu,endu;
	BoundedCurve(FastCurve *line, double startu, double endu){this->line = line; this->startu = startu; this->endu = endu;}

	void Reverse()
	{
		double temp = endu;
		endu = startu;
		startu = temp;
		line->Reverse();
	}

	double GetAX()
	{
		return GetX(startu);
	}

	double GetAY()
	{
		return GetY(startu);
	}

	double GetX(double u)
	{
		return line->GetXatU(u);
	}

	double GetY(double u)
	{
		return line->GetYatU(u);
	}
	
	double GetBX()
	{
		return GetX(endu);
	}

	double GetBY()
	{
		return GetY(endu);
	}

	gp_Pnt GetMidPoint()
	{
		double mu = startu + (endu - startu)/2;
		return gp_Pnt(GetX(mu),GetY(mu),0);
	}

	int RayIntersects(gp_Pnt pnt)
	{
		std::vector<double> vec = line->RayIntersects(pnt);
		int count=0;
		//TODO: add tolerance
		for(int i=0; i < vec.size(); i++)
			if((vec[i] > startu && vec[i] < endu)||(vec[i] < startu && vec[i] > endu))
				count++;
		return count;
	}

	gp_Pnt Begin()
	{
		return gp_Pnt(GetAX(),GetAY(),0);
	}

	gp_Pnt End()
	{
		return gp_Pnt(GetBX(),GetBY(),0);
	}

};

enum WhichEnd
{
	FirstEnd,
	LastEnd,
	NoEnd
};

enum WhichPoint
{
	PointA,
	PointB
};

class CompoundSegment
{
public:
	BoundedCurve *firstline;
	WhichPoint firstpoint;
	BoundedCurve *lastline;
	WhichPoint lastpoint;
	std::list<BoundedCurve*> lines;
	std::vector<WhichPoint> points;
	double m_tol;
	CompoundSegment(){}
	CompoundSegment(FastCurve *line, double tol, double startu, double endu)
	{
		firstline = new BoundedCurve(line,startu,endu);
		lastline=firstline;
		lines.push_back(firstline);
		firstpoint = PointA;
		lastpoint = PointB;
		m_tol = tol;
	}
	~CompoundSegment(){}
	void Reverse() 
	{
		BoundedCurve* tline=firstline; 
		firstline=lastline;
		lastline=tline;
		WhichPoint tpoint = firstpoint;
		firstpoint = lastpoint;
		lastpoint = tpoint;
		lines.reverse();

#ifndef OLDLINES
		firstpoint = (firstpoint == PointA)?PointB:PointA;
		lastpoint = (lastpoint == PointA)?PointB:PointA;
		std::list<BoundedCurve*>::iterator it;
		for(it = lines.begin(); it!=lines.end(); it++)
		{
			(*it)->Reverse();
		}
#endif	
	}

	WhichEnd GetWhichEnd(double atx, double aty)
	{
		if(firstpoint == PointA)
		{
			if(MyIsEqual(firstline->GetAX(),atx) && MyIsEqual(firstline->GetAY(),aty))
				return FirstEnd;
		}
		else
		{
			if(MyIsEqual(firstline->GetBX(),atx) && MyIsEqual(firstline->GetBY(),aty))
				return FirstEnd;  
		}

		if(lastpoint == PointA)
		{
		
			if(MyIsEqual(lastline->GetAX(),atx) && MyIsEqual(lastline->GetAY(),aty))
				return LastEnd;
		}
		else
		{
			if(MyIsEqual(lastline->GetBX(),atx) && MyIsEqual(lastline->GetBY(),aty))
				return LastEnd;
		}
		return FirstEnd; //added to remove warning. I assume this should never happen should we be returning an error value
	}

	gp_Pnt Begin()
	{
		if(firstpoint == PointA)
			return firstline->Begin();
		else
			return firstline->End();
	}

	gp_Pnt End()
	{
		if(lastpoint == PointA)
			return lastline->Begin();
		else
			return lastline->End();
	}

	double GetArea(BoundedCurve* c1, WhichPoint dir1, BoundedCurve* c2, WhichPoint dir2)
	{
		gp_Pnt gPnt1 = c1->Begin();
		if(dir1 == PointB)
			gPnt1 = c1->End();

		gp_Pnt gPnt2 = c2->Begin();
		if(dir2 == PointB)
			gPnt2 = c2->End();

		return gPnt1.X()*gPnt2.Y()-gPnt2.X()*gPnt1.Y();
	}

	void GetEdges(std::list<TopoDS_Edge>& edges)
	{
		std::list<BoundedCurve*>::iterator it;
		lines.reverse();
		int i=0;
		for(it = lines.begin(); it!= lines.end(); ++it)
		{
			BoundedCurve* curve = (*it);
			FastArc* arc = dynamic_cast<FastArc*>(curve->line);
			if(arc)
			{
				gp_Circ cir = arc->GetCircle();
				if(points[i] == PointB)
				{
					cir.SetAxis(cir.Axis().Reversed());
#ifdef DEBUGEDGES
					DrawDebugLine(curve->End(), curve->Begin(),i);
#endif
					edges.push_back(BRepBuilderAPI_MakeEdge(cir, curve->End(), curve->Begin()));
				}
				else
				{
					edges.push_back(BRepBuilderAPI_MakeEdge(cir, curve->Begin(), curve->End()));
#ifdef DEBUGEDGES
					DrawDebugLine(curve->Begin(), curve->End(),i);
#endif 
				}
			}
			else
				if(points[i] == PointB)
				{
#ifdef DEBUGEDGES
					DrawDebugLine(curve->End(), curve->Begin(),i);
#endif
					edges.push_back(BRepBuilderAPI_MakeEdge(curve->End(), curve->Begin()));
				}
				else 
				{
//#ifdef DEBUGEDGES
					DrawDebugLine(curve->Begin(), curve->End(),i);
//#endif 
					edges.push_back(BRepBuilderAPI_MakeEdge(curve->Begin(), curve->End()));
				}
			i++;
		}
	}

	void DrawDebugLine(gp_Pnt A, gp_Pnt B, int i)
	{
		gp_Pnt mpnt = A.XYZ() + (B.XYZ()-A.XYZ())/2;

		double theta = atan2(B.Y() - A.Y(),B.X() - A.X());

		gp_Pnt lpnt1(mpnt.X() + cos(theta+Pi/4),mpnt.Y() + sin(theta+Pi/4),0);
		gp_Pnt lpnt2(mpnt.X() + cos(theta-Pi/4),mpnt.Y() + sin(theta-Pi/4),0);
		gp_Pnt tpnt1(mpnt.X() + cos(theta-Pi/2)*2,mpnt.Y() + sin(theta-Pi/2)*2,0);
		gp_Pnt tpnt2(mpnt.X() + cos(theta+Pi/2)*2,mpnt.Y() + sin(theta+Pi/2)*2,0);


		glBegin(GL_LINES);
		glVertex3d(mpnt.X(),mpnt.Y(),mpnt.Z());
		glVertex3d(lpnt1.X(),lpnt1.Y(),lpnt1.Z());
		glVertex3d(mpnt.X(),mpnt.Y(),mpnt.Z());
		glVertex3d(lpnt2.X(),lpnt2.Y(),lpnt2.Z());
		glEnd();

		wxChar str[100];
		wxSprintf(str,_("%d"),i);

		glPushMatrix();
		glTranslatef(tpnt1.X(),tpnt1.Y(),tpnt1.Z());
		render_text(str);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(tpnt2.X(),tpnt2.Y(),tpnt2.Z());
		render_text(str);
		glPopMatrix();
	}

	void render_text(const wxChar* str)
	{
		wxGetApp().create_font();
		//glColor4ub(0, 0, 0, 255);
		glEnable(GL_BLEND);
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
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
		glDisable(GL_BLEND);
	}

	double GetArea()
	{ 
		double total = 0;
		std::list<BoundedCurve*>::iterator it=lines.begin();
		std::list<BoundedCurve*>::iterator it2=lines.begin();
		int idx = 0;
		for(it2++; it2 != lines.end(); ++it2)
		{
			total += GetArea(*it++,points[idx],*it2,points[idx+1]);
			idx++;
		}
		total+=GetArea(*it,points[idx],*lines.begin(),points[0]);
		return total*.5;
	}
	int GetRayIntersectionCount(gp_Pnt pnt)
	{
		return GetRayIntersectionCount(pnt,NULL);
	}

	int GetRayIntersectionCount(gp_Pnt pnt, BoundedCurve* without)
	{
		int intersections=0;
		std::list<BoundedCurve*>::iterator it;
		for(it = lines.begin(); it!= lines.end(); ++it)
		{
			BoundedCurve* curve = (*it);
			if(curve == without)
				continue;
			intersections+=curve->RayIntersects(pnt);
		}
		return intersections;
	}

	bool GetCW()
	{
		std::list<BoundedCurve*>::iterator it;
		for(it = lines.begin(); it!=lines.end(); it++)
		{
			gp_Pnt mpnt = (*it)->GetMidPoint();
			int count = GetRayIntersectionCount(mpnt,*it);
			if(count%2)
			{
				gp_Pnt begin = (*it)->Begin();
				gp_Pnt end = (*it)->End();
				if(begin.Y() > end.Y())
					return false;
				if(begin.Y() == end.Y() && begin.X() > end.X())
					return false;
				return true;
			}	
		}
		return true;
	}


	void Order()
	{
		//TODO: is it possible that the segments are not in a logical order?
		gp_Pnt lastpoint = Begin();
		std::list<BoundedCurve*>::iterator it;
		points.clear();
		for(it = lines.begin(); it!= lines.end(); ++it)
		{
			if((*it)->Begin().Distance(lastpoint) <= m_tol)
			{
				points.push_back(PointA);
				lastpoint = (*it)->End();
			}
			else
			{
				if((*it)->End().Distance(lastpoint) <= m_tol)
				{
					points.push_back(PointB);
					lastpoint = (*it)->Begin();
				}
				else
				{
					//Kaboom
					int x=0;
					x++;
				}
			}
		}
	}

	void Add(CompoundSegment* seg, double atx, double aty)
	{
		std::list<BoundedCurve*>::iterator it;

		WhichEnd myend = GetWhichEnd(atx,aty);
		WhichEnd oend = seg->GetWhichEnd(atx,aty);

		if(myend == FirstEnd)
		{
			if(oend == FirstEnd)
			{
				firstline = seg->lastline;
				firstpoint = seg->lastpoint;
			}
			else
			{
				firstline = seg->firstline;
				firstpoint = seg->firstpoint;
				seg->lines.reverse();
			}

			for(it = seg->lines.begin(); it!= seg->lines.end(); ++it)
			{
				lines.push_front(*it);
			}

		}
		else
		{
			if(oend == FirstEnd)
			{
				lastline = seg->lastline;
				lastpoint = seg->lastpoint;
			}
			else
			{
				lastline = seg->firstline;
				lastpoint = seg->firstpoint;

				seg->lines.reverse();
			}
			for(it = seg->lines.begin(); it!= seg->lines.end(); ++it)
			{
				lines.push_back(*it);
			}
		}
	}
};

std::vector<CompoundSegment*> find_level(bool odd, 
				std::vector<std::pair<CompoundSegment*,std::vector<CompoundSegment*> > > &pRet,
				std::vector<CompoundSegment*>& closed_shapes, 
				std::vector<std::vector<CompoundSegment*> >& inside_of, 
				std::vector<CompoundSegment*> parents);

std::vector<TopoDS_Face> TopoDSFaceAdaptor(
	std::vector<std::pair<CompoundSegment*,std::vector<CompoundSegment*> > > &data);

void ConcatSegments(double x_coord, double y_coord, CompoundSegment* seg1, CompoundSegment* seg2, TwoDNearMap &bcurves);
void AnalyzeNearMap(TwoDNearMap &bcurves);