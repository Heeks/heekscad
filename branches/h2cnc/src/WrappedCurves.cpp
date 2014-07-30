// WrappedCurves.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Intersector.h"
#include "WrappedCurves.h"
#include "BentleyOttmann.h"

#define DEBUGEDGES

BoundedCurve::BoundedCurve(FastCurve *line, double startu, double endu, double tol)
{
	this->line = line;
	this->startu = startu;
	this->endu = endu;
	this->m_tol = tol;
}

void BoundedCurve::Reverse()
{
	double temp = endu;
	endu = startu;
	startu = temp;
	line->Reverse();
}

double BoundedCurve::GetAX()
{
	return GetX(startu);
}

double BoundedCurve::GetAY()
{
	return GetY(startu);
}

double BoundedCurve::GetX(double u)
{
	return line->GetXatU(u);
}

double BoundedCurve::GetY(double u)
{
	return line->GetYatU(u);
}

double BoundedCurve::GetBX()
{
	return GetX(endu);
}

double BoundedCurve::GetBY()
{
	return GetY(endu);
}

gp_Pnt BoundedCurve::GetMidPoint()
{
	double mu = startu + (endu - startu)/2;
	return gp_Pnt(GetX(mu),GetY(mu),0);
}

std::vector<RayIntersection> BoundedCurve::RayIntersects(gp_Pnt pnt)
{
	std::vector<RayIntersection> vec = line->RayIntersects(pnt);
	std::vector<RayIntersection> points;
	int count=0;
		for(unsigned int i=0; i < vec.size(); i++)
		if((vec[i].u >= startu && vec[i].u <= endu)||(vec[i].u <= startu && vec[i].u >= endu))
		{
			if(vec[i].u > startu - TOLERANCE && vec[i].u < startu + TOLERANCE)
			{
				//Bounded on startu
				vec[i].bounded = true;
				if(End().Y() < pnt.Y())
					vec[i].lower = true;
			}
				if(vec[i].u > endu - TOLERANCE && vec[i].u < endu + TOLERANCE)
			{
				//Bounded on startu
				vec[i].bounded = true;
				if(Begin().Y() < pnt.Y())
					vec[i].lower = true;
			}
			points.push_back(vec[i]);
				count++;
		}
		return points;
}

gp_Pnt BoundedCurve::Begin()
{
	return gp_Pnt(GetAX(),GetAY(),0);
}

gp_Pnt BoundedCurve::End()
{
	return gp_Pnt(GetBX(),GetBY(),0);
}


CompoundSegment::CompoundSegment()
{
}

CompoundSegment::CompoundSegment(FastCurve *line, double tol, double startu, double endu)
{
	firstline = new BoundedCurve(line,startu,endu,tol);
	lastline=firstline;
	lines.push_back(firstline);
	firstpoint = PointA;
	lastpoint = PointB;
	m_tol = tol;
}

CompoundSegment::~CompoundSegment()
{
}

void CompoundSegment::Reverse()
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

WhichEnd CompoundSegment::GetWhichEnd(double atx, double aty)
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

gp_Pnt CompoundSegment::Begin()
{
	if(firstpoint == PointA)
		return firstline->Begin();
	else
		return firstline->End();
}

gp_Pnt CompoundSegment::End()
{
	if(lastpoint == PointA)
		return lastline->Begin();
	else
		return lastline->End();

}

double CompoundSegment::GetArea(BoundedCurve* c1, WhichPoint dir1, BoundedCurve* c2, WhichPoint dir2)
{
	gp_Pnt gPnt1 = c1->Begin();
	if(dir1 == PointB)
		gPnt1 = c1->End();
		gp_Pnt gPnt2 = c2->Begin();
		if(dir2 == PointB)
		gPnt2 = c2->End();
		return gPnt1.X()*gPnt2.Y()-gPnt2.X()*gPnt1.Y();
}

void CompoundSegment::GetEdges(std::list<TopoDS_Edge>& edges)
{
	std::list<BoundedCurve*>::iterator it;
	lines.reverse();
	int i=0;
	gp_Pnt startpoint;
	gp_Pnt lastpoint;
    bool isend=false;;
	for(it = lines.begin(); it!= lines.end(); ++it)
	{
	    //Figure out if this is the last element of the curve
        std::list<BoundedCurve*>::iterator it2 = it;
        it2++;
        if(it2 == lines.end())
            isend = true;

        BoundedCurve* curve = (*it);
        gp_Pnt begin = curve->Begin();
        gp_Pnt end = curve->End();
        if(points[i] == PointB)
        {
            begin = curve->End();
            end = curve->Begin();
        }

        if(it == lines.begin())
        {
            startpoint = begin;
        }
        else
        {
            begin = lastpoint;
        }

        lastpoint = end;
        if(isend)
            end = startpoint;

		FastArc* arc = dynamic_cast<FastArc*>(curve->line);
		if(arc)
		{
			gp_Circ cir = arc->GetCircle();
			if(points[i] == PointB)
			{
				cir.SetAxis(cir.Axis().Reversed());
			}

			cir.SetRadius(cir.Location().Distance(begin)/2 + cir.Location().Distance(end)/2);
			edges.push_back(BRepBuilderAPI_MakeEdge(cir, curve->Begin(), curve->End()));
#ifdef DEBUGEDGES
			DrawDebugLine(curve->Begin(), curve->End(),i);
#endif
		}
		else
		{
#ifdef DEBUGEDGES
			DrawDebugLine(curve->Begin(), curve->End(),i);
#endif
			edges.push_back(BRepBuilderAPI_MakeEdge(curve->Begin(), curve->End()));
		}
		i++;
	}
}

void CompoundSegment::DrawDebugLine(gp_Pnt A, gp_Pnt B, int i)
{
	gp_Pnt mpnt = A.XYZ() + (B.XYZ()-A.XYZ())/2;
	double theta = atan2(B.Y() - A.Y(),B.X() - A.X());

	gp_Pnt lpnt1(mpnt.X() + cos(theta+M_PI/4),mpnt.Y() + sin(theta+M_PI/4),0);
	gp_Pnt lpnt2(mpnt.X() + cos(theta-M_PI/4),mpnt.Y() + sin(theta-M_PI/4),0);
	gp_Pnt tpnt1(mpnt.X() + cos(theta-M_PI/2)*2,mpnt.Y() + sin(theta-M_PI/2)*2,0);
	gp_Pnt tpnt2(mpnt.X() + cos(theta+M_PI/2)*2,mpnt.Y() + sin(theta+M_PI/2)*2,0);

	glBegin(GL_LINES);
	glVertex3d(mpnt.X(),mpnt.Y(),mpnt.Z());
	glVertex3d(lpnt1.X(),lpnt1.Y(),lpnt1.Z());
	glVertex3d(mpnt.X(),mpnt.Y(),mpnt.Z());
	glVertex3d(lpnt2.X(),lpnt2.Y(),lpnt2.Z());
	glEnd();
	wxChar str[100];
	wxSprintf(str,_T("%d"),i);
	glPushMatrix();
	glTranslatef((float)tpnt1.X(),(float)tpnt1.Y(),(float)tpnt1.Z());
	render_text(str);
	glPopMatrix();

	glPushMatrix();
	glTranslatef((float)tpnt2.X(),(float)tpnt2.Y(),(float)tpnt2.Z());
	render_text(str);
	glPopMatrix();
}

void CompoundSegment::render_text(const wxChar* str)
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

double CompoundSegment::GetArea()
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

int CompoundSegment::GetRayIntersectionCount(gp_Pnt pnt)
{
	return GetRayIntersectionCount(pnt,NULL);
}

int CompoundSegment::GetRayIntersectionCount(gp_Pnt pnt, BoundedCurve* without)
{
	int intersections=0;
	std::list<BoundedCurve*>::iterator it;
	std::vector<RayIntersection> points;
	for(it = lines.begin(); it!= lines.end(); ++it)
	{
		BoundedCurve* curve = (*it);
		if(curve == without)
			continue;
		std::vector<RayIntersection> npoints = curve->RayIntersects(pnt);
		for(size_t i=0; i < npoints.size(); i++)
		points.push_back(npoints[i]);
		intersections+=npoints.size();
	}

	//Remove duplicates
	for(size_t i=0; i < points.size(); i++)
	{
		for(size_t j=i+1; j < points.size(); j++)
			if(points[i].pnt.Distance(points[j].pnt) < m_tol)
			{
				intersections--;
				//If these points just intersect the ray without crossing it, then remove the point all togeather
				if(points[i].bounded && points[j].bounded && points[i].lower == points[j].lower)
					intersections--;
			}
	}

	return intersections;
}

bool CompoundSegment::GetCW()
{
	std::list<BoundedCurve*>::iterator it;
	for(it = lines.begin(); it!=lines.end(); it++)
	{
		//Find a non horizontal starting point
		gp_Pnt begin = (*it)->Begin();
		gp_Pnt end = (*it)->End();
		if(begin.Y() < end.Y() + m_tol/4 && begin.Y() > end.Y() - m_tol/4)
			continue;
		gp_Pnt mpnt = (*it)->GetMidPoint();
		int count = GetRayIntersectionCount(mpnt,*it);
		if(count%2)
		{
			if(begin.Y() > end.Y())
				return true;
			if(begin.Y() == end.Y() && begin.X() > end.X())
				return true;
			return false;
		}
	}
	return false;
}


void CompoundSegment::Order()
{
	//TODO: is it possible that the segments are not in a logical order?
	gp_Pnt m_lastpoint = Begin();
	firstpoint = PointA;
	std::list<BoundedCurve*>::iterator it;
	points.clear();
	for(it = lines.begin(); it!= lines.end(); ++it)
	{
		if((*it)->Begin().Distance(m_lastpoint) <= m_tol)
		{
			points.push_back(PointA);
			m_lastpoint = (*it)->End();
			lastpoint = PointB;
		}
		else
		{
			if((*it)->End().Distance(m_lastpoint) <= m_tol)
			{
				points.push_back(PointB);
				m_lastpoint = (*it)->Begin();
				lastpoint = PointA;
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

void CompoundSegment::Add(CompoundSegment* seg, double atx, double aty)
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
