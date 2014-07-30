// Polygon.cpp
// Copyright (c) 2009, Perttu "celero55" Ahola
// This program is released under the BSD license. See the file COPYING for details.

//#ifndef UNITTEST_NO_HEEKS
	#include <stdafx.h>
//#endif // I was getting error, building in Windows ( VS 2008 ): 1>.\Polygon.cpp(7) : fatal error C1020: unexpected #endif
// Changed "stdafx.h" to <stdafx.h> and created an empty file to the unittest directory, to make unittest/Polygontest to build.

#include "Polygon.h"

//UnionPolygons_old debug
#ifndef UPODEBUG
	#define UPODEBUG 0
#endif
//general debug
#ifndef DEBUG
	#define DEBUG 1
#endif
//SweepLine debug
#ifndef SLDEBUG
	#define SLDEBUG 0
#endif

const char *PDstr(PolygonDirection d)
{
	switch(d)
	{
	case PolyCW: return "PolyCW";
	case PolyCCW: return "PolyCCW";
	case PolyUndefinedW: return "PolyUndefinedW";
	}
	return "[Erroneous PolyDirection]";
}

double VecAngle(gp_Vec v1, gp_Vec v2)
{
	double v1a = atan2(v1.Y(), v1.X());
	double v2a = atan2(v2.Y(), v2.X());
	double angle = v2a - v1a;
	if(angle <= -M_PI) angle += M_PI*2;
	else if(angle > M_PI) angle -= M_PI*2;
	return angle;
}


PolygonDirection CPolygon::Direction()
{
	if(m_dir != PolyUndefinedW) return m_dir;
	if(points.size() == 0){
		m_dir = PolyUndefinedW;
		return m_dir;
	}
	double angle_sum = 0.;
	std::list<gp_Pnt>::const_iterator ilp = points.begin();
	gp_Pnt p1 = *ilp;
	ilp++;
	gp_Vec v_last(p1, *ilp);
	p1 = *ilp;
	ilp++;
	for(; ilp != points.end(); ilp++)
	{
		if(p1.IsEqual(*ilp, 0.000000001)) continue;
		gp_Vec v_current(p1, *ilp);
		//angle_sum += v_last.Angle(v_current);
		angle_sum += VecAngle(v_last, v_current);
		v_last = v_current;
		p1 = *ilp;
	}
	ilp = points.begin();
	gp_Vec v_current(p1, *ilp);
	//angle_sum += v_last.Angle(v_current);
	angle_sum += VecAngle(v_last, v_current);
	v_last = v_current;
	p1 = *ilp;
	ilp++;
	v_current = gp_Vec(p1, *ilp);
	//angle_sum += v_last.Angle(v_current);
	angle_sum += VecAngle(v_last, v_current);
	/*if(DEBUG)std::cout<<"polygon starting with ("<<points.begin()->X()<<","<<points.begin()->Y()
			<<"): angle_sum = "<<angle_sum<<std::endl;*/
	//we can just look if it's positive or negative
	if(angle_sum < 0.0) m_dir = PolyCW;
	else m_dir = PolyCCW;
	return m_dir;
}

class LineTreeNode;
std::string NodeInfo(LineTreeNode *n);

class LineTreeNode
{
public:
	LineTreeNode(LineTreeNode *p_parent)
			: m_ptr(NULL), m_left(NULL), m_right(NULL), m_parent(p_parent)
	{
		//std::cout<<"created new LineTreeNode with parent="<<p_parent<<std::endl;
	}
	/*~LineTreeNode()
	{
		//This is a very bad idea when deleting single nodes which have children
		Clear();
	}*/
	LineTreeNode * Insert(LineSegment *p_ptr, double x)
	{
		//std::cout<<"LineTreeNode::Insert(): current node: m_ptr="<<m_ptr<<std::endl;
		if(m_ptr == NULL)
		{
			m_ptr = p_ptr;
			return this;
		}
		if(m_ptr == p_ptr)
		{
			return this;
		}
		if(p_ptr->IsUpperOrEqualAfterX(*m_ptr, x))
		//if(p_ptr->IsUpperOrEqual(*m_ptr))
		//if(m_ptr->y_value_at(x) >= p_ptr->y_value_at(x))
		//if(IsUpperOrEqual(m_ptr->left(), m_ptr->right(), p_ptr->left()))
		{
			if(m_right == NULL) m_right = new LineTreeNode(this);
			return m_right->Insert(p_ptr, x);
		}
		else{
			if(m_left == NULL) m_left = new LineTreeNode(this);
			return m_left->Insert(p_ptr, x);
		}
	}
	LineTreeNode * Search(LineSegment *p_ptr, double x)
	{
		if(m_ptr == NULL)
		{
			//std::cout<<"LineTreeNode::Search("<<p_ptr->str()<<"): returning NULL"<<std::endl;
			return NULL;
		}
		if(m_ptr == p_ptr)
		{
			//std::cout<<"LineTreeNode::Search("<<p_ptr->str()<<"): returning this"<<std::endl;
			return this;
		}
		if(p_ptr->IsUpperOrEqualAfterX(*m_ptr, x))
		//if(p_ptr->IsUpperOrEqual(*m_ptr))
		//if(m_ptr->y_value_at(lastx) >= p_ptr->y_value_at(lastx))
		//if(IsUpperOrEqual(m_ptr->left(), m_ptr->right(), p_ptr->left()))
		{
			if(m_right == NULL){
				/*std::cout<<"LineTreeNode::Search("<<p_ptr->str()
						<<"): returning NULL (m_right == NULL)"<<std::endl;*/
				return NULL;
			}
			/*std::cout<<"LineTreeNode::Search("<<p_ptr->str()
					<<"): returning m_right->Search("<<p_ptr->str()<<")"<<std::endl;*/
			return m_right->Search(p_ptr, x);
		}
		else{
			if(m_left == NULL){
				/*std::cout<<"LineTreeNode::Search("<<p_ptr->str()
						<<"): returning NULL (m_left == NULL)"<<std::endl;*/
				return NULL;
			}
			/*std::cout<<"LineTreeNode::Search("<<p_ptr->str()
					<<"): returning m_left->Search("<<p_ptr->str()<<")"<<std::endl;*/
			return m_left->Search(p_ptr, x);
		}
	}
	void SearchRightPoint(const gp_Pnt &p, std::list<LineTreeNode*> &result)
	{
		if(m_ptr == NULL)
		{
			return;
		}
		if(m_ptr->right().IsEqual(p, POLYGON_TOLERANCE))
		{
			result.push_back(this);
		}
		if(m_left != NULL){
			m_left->SearchRightPoint(p, result);
		}
		if(m_right != NULL){
			m_right->SearchRightPoint(p, result);
		}
	}
	void SearchInteriorPoint(const gp_Pnt &p, std::list<LineTreeNode*> &result)
	{
		/*if(SLDEBUG)std::cout<<" SearchInteriorPoint(p=("<<p.X()<<","<<p.Y()
				<<"), result): m_ptr="<<(m_ptr?m_ptr->str():"NULL")<<std::endl;*/
		if(m_ptr == NULL)
		{
			return;
		}
		if(m_ptr->HasPointInterior(p))
		{
			result.push_back(this);
		}
		if(m_left != NULL){
			m_left->SearchInteriorPoint(p, result);
		}
		if(m_right != NULL){
			m_right->SearchInteriorPoint(p, result);
		}
	}
	LineTreeNode *Lower()
	{
		//std::cout<<"LineTreeNode::Lower()"<<std::endl;
		LineTreeNode *n;
		//The nearest lower one is one to the left and then as far to the
		//right as is possible.
		if(m_left)
		{
			n = m_left;
			while(n->m_right) n = n->m_right;
			return n;
		}
		else if(m_parent == NULL) return NULL;
		//The next lower one is, when going towards the root, the first one,
		//which has the child node as its right child.
		n = this;
		for(;;)
		{
			if(n->m_parent == NULL) return NULL;
			if(n == n->m_parent->m_right) return n->m_parent;
			n = n->m_parent;
		}
	}
	LineTreeNode *Higher()
	{
		//std::cout<<"LineTreeNode::Higher()"<<std::endl;
		LineTreeNode *n;
		//The nearest higher one is one to the right and then as far to the
		//left as is possible.
		if(m_right)
		{
			n = m_right;
			while(n->m_left) n = n->m_left;
			return n;
		}
		else if(m_parent == NULL) return NULL;
		//The next higher one is, when going towards the root, the first one,
		//which has the child node as its left child.
		n = this;
		for(;;)
		{
			if(n->m_parent == NULL) return NULL;
			if(n == n->m_parent->m_left) return n->m_parent;
			n = n->m_parent;
		}
	}
	LineTreeNode * Lowest()
	{
		if(m_left == NULL){
			if(m_ptr == NULL) return NULL;
			return this;
		}
		return m_left->Lowest();
	}
	LineTreeNode * Highest()
	{
		if(m_right == NULL){
			if(m_ptr == NULL) return NULL;
			return this;
		}
		return m_right->Highest();
	}
	void Clear()
	{
		if(m_left != NULL)
		{
			m_left->Clear();
			delete m_left;
		}
		if(m_right != NULL)
		{
			m_right->Clear();
			delete m_right;
		}
	}
	void Print()
	{
		Print(0);
		/*if(m_ptr != NULL) std::cout<<m_ptr->str()<<std::endl;
		if(m_left != NULL)
		{
			//std::cout<<"Print going left:"<<std::endl;
			m_left->Print();
		}
		if(m_right != NULL)
		{
			//std::cout<<"Print going right:"<<std::endl;
			m_right->Print();
		}*/
	}
	void Print(int level)
	{
		if(level==0) std::cout<<"------------------------"<<std::endl;
		if(m_right != NULL)
		{
			if(m_ptr == NULL) std::cout<<"\tshouldn't be: "
					"m_ptr == NULL && m_right != NULL"<<std::endl;
			//std::cout<<"Print going right:"<<std::endl;
			if(m_right->m_parent != this) std::cout<<"\tshouldn't be:"
					" m_right->m_parent == "<<NodeInfo(m_right->m_parent)
					<<" != this"<<std::endl;
			m_right->Print(level+1);
		}
		if(m_ptr != NULL)
		{
			//for(int i=0; i<level; i++) std::cout<<" ";
			if(level==0)
			{
				if(m_parent != NULL) std::cout<<"\tshoudn't be:"
						" m_parent == "<<NodeInfo(m_parent)
						<<" != NULL"<<std::endl;
			}
			if(m_ptr == NULL) std::cout<<"\tshouldn't be: "
					"m_ptr == NULL"<<std::endl;
			std::cout<<level;
			for(int i=0; i<level; i++) std::cout<<"  ";
			if(m_parent == NULL) std::cout<<" P";
			else if(m_parent->m_left == this) std::cout<<"'L";
			else if(m_parent->m_right == this) std::cout<<".R";
			else std::cout<<"EE";
			std::cout<<" ";
			for(int i=0; i<(7-level); i++) std::cout<<"- ";
			//std::cout<<m_ptr->str()<<std::endl;
		}
		else
		{
			if(level==0) std::cout<<"0   ";
			std::cout<<"NULL"<<std::endl;
		}
		if(m_left != NULL)
		{
			if(m_ptr == NULL) std::cout<<"\tshouldn't be: "
					"m_ptr == NULL && m_left != NULL"<<std::endl;
			//std::cout<<"Print going left:"<<std::endl;
			if(m_left->m_parent != this) std::cout<<"\tshoudn't be:"
					" m_left->m_parent == "<<NodeInfo(m_left->m_parent)
					<<" != this"<<std::endl;
			m_left->Print(level+1);
		}
		if(level==0) std::cout<<"------------------------"<<std::endl;
	}
	LineSegment * m_ptr;
		LineTreeNode * m_left;
	LineTreeNode * m_right;
	LineTreeNode * m_parent;
};

std::string NodeInfo(LineTreeNode *n)
{
	std::stringstream ss;
	if(n==NULL) ss<<"[NULL]";
	else{
		ss<<n->m_ptr->str();
	}
	return ss.str();
}

void RemoveLineTreeNode(LineTreeNode * & tree, LineTreeNode * n)
{
	//std::cout<<"RemoveLineTreeNode(tree="<<int(tree)<<",n="<<int(n)<<")"<<std::endl;
	//std::cout<<"RemoveLineTreeNode(tree="<<NodeInfo(tree)<<",n="<<NodeInfo(n)<<")"<<std::endl;
	if(n->m_left == NULL && n->m_right == NULL)
	{
		//it has no children
		if(n->m_parent)
		{
			//set parent to know it no longer exists
			if(n->m_parent->m_right == n) n->m_parent->m_right = NULL;
			if(n->m_parent->m_left == n) n->m_parent->m_left = NULL;
			//delete it
			delete n;
		}
		else
		{
			//it is the root node, so it doesn't have a parent.
			//just change it's value to nothing
			n->m_ptr = NULL;
		}
	}
	else if(n->m_left != NULL && n->m_right == NULL)
	{
		if(n->m_parent)
		{
			//set its parent's pointer to point to the left child of it
			if(n->m_parent->m_left == n) n->m_parent->m_left = n->m_left;
			if(n->m_parent->m_right == n) n->m_parent->m_right = n->m_left;
		}
		//set the child to know who its parent is now
		n->m_left->m_parent = n->m_parent;
		if(n == tree)
		{
			tree = n->m_left;
		}
		delete n;
	}
	else if(n->m_right != NULL && n->m_left == NULL)
	{
		if(n->m_parent)
		{
			//set its parent's pointer to point to the right child of it
			if(n->m_parent->m_right == n) n->m_parent->m_right = n->m_right;
			if(n->m_parent->m_left == n) n->m_parent->m_left = n->m_right;
		}
		//set the child to know who its parent is now
		n->m_right->m_parent = n->m_parent;
		if(n == tree)
		{
			tree = n->m_right;
		}
		delete n;
	}
	else
	{
		//it has both children
		//find the lowest value of the right subtree (=nearest higher)
		LineTreeNode * m = n->m_right;
		while(m->m_left != NULL) m = m->m_left;
		//set the pointers right
		if(n->m_parent)
		{
			if(n->m_parent->m_left == n) n->m_parent->m_left = m;
			if(n->m_parent->m_right == n) n->m_parent->m_right = m;
		}
		if(m->m_parent)
		{
			if(m->m_parent != n){
				//if m is the left child of it's parent
				if(m->m_parent->m_left == m)
				{
					//if m doesn't have a right child, m can be completely
					//removed (it doesn't have a left one either)
					if(m->m_right == NULL)
					{
						m->m_parent->m_left = NULL;
					}
					//because m doesn't have a left child, the right child can
					//be easily moved to the old place of m
					if(m->m_right) m->m_right->m_parent = m->m_parent;
					m->m_parent->m_left = m->m_right;
				}
				//if m is the right child of it's parent
				if(m->m_parent->m_right == m)
				{
					//if m doesn't have a right child, m can be completely
					//removed (it doesn't have a left one either)
					if(m->m_right == NULL)
					{
						m->m_parent->m_right = NULL;
					}
					//because m doesn't have a left child, the right child can
					//be easily moved to the old place of m
					if(m->m_right) m->m_right->m_parent = m->m_parent;
					m->m_parent->m_right = m->m_right;
				}
			}
		}
		else std::cout<<"RemoveLineTreeNode(): shouldn't happen: m->m_parent==NULL"<<std::endl;
		/*//TODO: what if m->m_parent has a right child?
		//- then m->m_right should be added at the first NULL point at path
		//  m->m_parent->m_right->m_left->m_left->m_left...
		if(m->m_right) m->m_right->m_parent = m->m_parent;*/
		if(n == tree) tree = m;
		n->m_left->m_parent = m;
		m->m_left = n->m_left;
		if(n->m_right != m) m->m_right = n->m_right;
		m->m_parent = n->m_parent;
		if(n->m_left && n->m_left != m) n->m_left->m_parent = m;
		if(n->m_right && n->m_right != m) n->m_right->m_parent = m;
		//delete the current one
		delete n;
	}
	/*if(SLDEBUG){
		std::cout<<"tree:"<<std::endl;
		tree->Print();
	}*/
}

enum SweepEventType
{
	EndpointEvent,
	IntersectionEvent
};

class SweepEvent
{
public:
	SweepEvent(Endpoint p_ep) : t(EndpointEvent), ep(p_ep) {}
	SweepEvent(const LineSegment *p_s1, const LineSegment *p_s2, gp_Pnt p_ip)
			: t(IntersectionEvent), ep(NULL,Left), s1(p_s1), s2(p_s2), ip(p_ip) {}
	void GetPoint(gp_Pnt &p) const
	{
		switch(t){
		case EndpointEvent:
			p = ep.value();
			break;
		case IntersectionEvent:
			p = ip;
			break;
		}
	}
	double x() const
	{
		gp_Pnt p;
		GetPoint(p);
		return p.X();
	}
	double y() const
	{
		gp_Pnt p;
		GetPoint(p);
		return p.Y();
	}
	bool operator < (const SweepEvent &e) const
	{
		if(x() < e.x()) return true;
		if(dabs(x() - e.x()) < POLYGON_TOLERANCE)
			if(y() > e.y()) return true;
		return false;
	}
	bool operator == (const SweepEvent &e) const
	{
		if(t != e.t) return false;
		switch(t){
		case EndpointEvent:
			return ep == e.ep;
			break;
		case IntersectionEvent:
			return (s1 == e.s1 && s2 == e.s2
					&& ip.X() == e.ip.X() && ip.Y() == e.ip.Y());
			break;
		}
	}
	std::string str() const
	{
		std::stringstream ss;
		switch(t){
		case EndpointEvent:
			ss<<"EndpointEvent ("<<ep.value().X()<<","<<ep.value().Y()<<")";
			break;
		case IntersectionEvent:
			ss<<"IntersectionEvent ("<<ip.X()<<","<<ip.Y()<<")";
			break;
		}
		return ss.str();
	}
	SweepEventType t;
	Endpoint ep;
	const LineSegment *s1;
	const LineSegment *s2;
	gp_Pnt ip;
};

/*void InsertToStatus(std::list<LineSegment*> &status, LineSegment *s, gp_Pnt &p)
{
	//status has all the lines going underneath the sweep line,
	//the highest y first
	
	if(status.size()==0)
	{
		if(SLDEBUG)std::cout<<"InsertToStatus(status, s="<<s->str()<<", p=("<<p.X()<<","
				<<p.Y()<<")): status was empty"<<std::endl;
		status.push_back(s);
	}
	else
	{
		if(SLDEBUG)std::cout<<"InsertToStatus(status, s="<<s->str()<<", p=("<<p.X()<<","
				<<p.Y()<<"))"<<std::endl;
		
		std::list<LineSegment*>::iterator j = status.begin();

		while(j != status.end())
		{
			if(SLDEBUG)std::cout<<s->str()<<"->IsUpperOrEqualAfterX("<<(*j)->str()
					<<", "<<p.X()<<") == ";
			if(s->IsUpperOrEqualAfterX(*(*j), p.X()))
			{
				if(SLDEBUG)std::cout<<"true"<<std::endl;
				break;
			}
			if(SLDEBUG)std::cout<<"false"<<std::endl;
			j++;
		}
		if(j == status.end()) if(SLDEBUG)std::cout<<"j == status.end()"<<std::endl;

		status.insert(j, s);
	}
	
	if(SLDEBUG)std::cout<<"\tstatus after inserting:"<<std::endl;
	for(std::list<LineSegment*>::iterator i = status.begin();
			i != status.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}
}*/

void FindNewEvent(std::vector<LineSegment> &lines_vector,
		std::set<SweepEvent> &event_queue,
		LineSegment * s1, LineSegment * s2, gp_Pnt &p)
{
	IntersectionInfo info = s1->intersects(*s2);
	if(info.t == NoIntersection) return;
	if(!(info.p.X() > p.X() || (dabs(info.p.X() - p.X()) < POLYGON_TOLERANCE
			&& info.p.Y() < p.Y())))
		return;
	SweepEvent event(s1, s2, info.p);
	std::set<SweepEvent>::iterator
			j = event_queue.find(event);
	if(j != event_queue.end()) return;
	event_queue.insert(event);
	if(SLDEBUG)std::cout<<"FindNewEvent(): inserted intersection event"<<std::endl
			<<"\t\tof "<<s1->str()<<" and "<<s2->str()<<std::endl
			<<"\t\tat ("<<info.p.X()<<","<<info.p.Y()<<")"<<std::endl;
}

void HandleEvent(std::vector<LineSegment> &lines_vector,
		std::set<SweepEvent> &event_queue,
		LineTreeNode * & status, const SweepEvent &ep, double oldx,
		unsigned int &num_intersection_points)
{
	gp_Pnt p;
	ep.GetPoint(p);
	if(SLDEBUG)std::cout<<"HandleEvent(): "<<ep.str()<<std::endl;
	//if(SLDEBUG)std::cout<<"\tstatus="<<int(status)<<std::endl;
	std::list<LineSegment*> left, right, contain;
	
	if(SLDEBUG)std::cout<<"\tstatus:"<<std::endl;
	if(SLDEBUG)status->Print();
	/*for(std::list<LineSegment*>::iterator i = status.begin();
			i != status.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}*/
	
	//find the lines whose left points are at this point
	for(std::vector<LineSegment>::iterator i = lines_vector.begin();
			i != lines_vector.end(); i++)
	{
		if(i->left().IsEqual(p, POLYGON_TOLERANCE)){
			/*std::list<LineSegment*>::iterator
				j = find(status.begin(), status.end(), &(*i));
			if(j != status.end()){*/
			LineTreeNode *node = status->Search(&(*i), oldx);
			if(node != NULL){
				if(SLDEBUG)std::cout<<"EE\t\tline.left==p but line is already in status"
						<<std::endl;
				continue;
			}
			left.push_back(&(*i));
		}
		/*if(i->right().IsEqual(p, POLYGON_TOLERANCE))
			right.push_back(&(*i));*/
	}

	//Find the lines whose left points are at this point.
	//They are all found in status.
	std::list<LineTreeNode*> right_nodes;
	status->SearchRightPoint(p, right_nodes);
	for(std::list<LineTreeNode*>::iterator i = right_nodes.begin();
			i != right_nodes.end(); i++)
	{
		right.push_back((*i)->m_ptr);
	}
	
	//search all the segments in status that contain p and add them to contain
	std::list<LineTreeNode*> contain_nodes;
	status->SearchInteriorPoint(p, contain_nodes);
	for(std::list<LineTreeNode*>::iterator i = contain_nodes.begin();
			i != contain_nodes.end(); i++)
	{
		contain.push_back((*i)->m_ptr);
	}

	/*std::list<LineSegment*> contain;
	for(std::list<LineSegment*>::iterator i = status.begin();
			i != status.end(); i++)
	{
		std::list<LineSegment*>::iterator j;
		
		if((*i)->HasPointInterior(p))
		{
			contain.push_back(*i);
		}
	}*/

	if(ep.t == IntersectionEvent && contain.size() < 2)
	{
		if(SLDEBUG)std::cout<<"EE\t\tthis is an intersection event "
				"and contain.size() < 2"<<std::endl;
	}
	
	if(SLDEBUG)std::cout<<"\tleft:"<<std::endl;
	for(std::list<LineSegment*>::iterator i = left.begin();
			i != left.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}

	if(SLDEBUG)std::cout<<"\tright:"<<std::endl;
	for(std::list<LineSegment*>::iterator i = right.begin();
			i != right.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}

	if(SLDEBUG)std::cout<<"\tcontain:"<<std::endl;
	for(std::list<LineSegment*>::iterator i = contain.begin();
			i != contain.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}

	//if (left U right U contain) has more than one segments
	if(left.size() + right.size() + contain.size() > 1)
	{
		//here be intersections!
		num_intersection_points++;
		if(SLDEBUG)std::cout<<"\tthere are intersections at this point."<<std::endl;
		std::list<LineSegment*> all_at_point;
		all_at_point.insert(all_at_point.end(), left.begin(), left.end());
		all_at_point.insert(all_at_point.end(), right.begin(), right.end());
		all_at_point.insert(all_at_point.end(), contain.begin(), contain.end());
		for(std::list<LineSegment*>::iterator i = all_at_point.begin();
				i != all_at_point.end(); i++)
		{
			for(std::list<LineSegment*>::iterator j = all_at_point.begin();
					j != all_at_point.end(); j++)
			{
				if(*j == *i) continue;
				IntersectionInfo info;
				info.i = (*j)->m_index;
				info.t = SomeIntersection;
				info.p = p;
				(*i)->m_intersections_set.insert(info);
			}
		}
	}

	if(SLDEBUG)std::cout<<"\tremoving right and contain from status"<<std::endl;

	//remove (right U contain) from status
	//right
	for(std::list<LineSegment*>::iterator i = right.begin();
			i != right.end(); i++)
	{
		/*
		std::list<LineSegment*>::iterator
				j = find(status.begin(), status.end(), *i);
		if(j != status.end()) status.erase(j);
		else if(SLDEBUG)std::cout<<"\t\tdid not find "<<(*i)->str()<<std::endl;
		
		j = find(status.begin(), status.end(), *i);
		if(j != status.end()) if(SLDEBUG)
				if(SLDEBUG)std::cout<<"EE\t\tthere are multiple copies of"
				" the same line in status"<<std::endl;
		*/
		LineTreeNode *n = status->Search(*i, oldx);
		if(n != NULL) RemoveLineTreeNode(status, n);
		else if(SLDEBUG)std::cout<<"\t\tdid not find "<<(*i)->str()<<std::endl;
		n = status->Search(*i, oldx);
		if(n != NULL) if(SLDEBUG)std::cout<<"EE\t\tthere are multiple copies of"
				" the same line in status"<<std::endl;
	}
	//contain
	for(std::list<LineSegment*>::iterator i = contain.begin();
			i != contain.end(); i++)
	{
		/*
		std::list<LineSegment*>::iterator
				j = find(status.begin(), status.end(), *i);
		if(j != status.end()) status.erase(j);
		else if(SLDEBUG)std::cout<<"\t\tdid not find "<<(*i)->str()<<std::endl;

		j = find(status.begin(), status.end(), *i);
		if(j != status.end()) if(SLDEBUG)
				if(SLDEBUG)std::cout<<"EE\t\tthere are multiple copies of"
				" the same line in status"<<std::endl;
		*/
		LineTreeNode *n = status->Search(*i, oldx);
		if(n != NULL) RemoveLineTreeNode(status, n);
		else if(SLDEBUG)std::cout<<"\t\tdid not find "<<(*i)->str()<<std::endl;
		n = status->Search(*i, oldx);
		if(n != NULL) if(SLDEBUG)std::cout<<"EE\t\tthere are multiple copies of"
				" the same line in status"<<std::endl;
	}

	//if(SLDEBUG)std::cout<<"\tstatus="<<int(status)<<std::endl;
	if(SLDEBUG)std::cout<<"\tstatus:"<<std::endl;
	if(SLDEBUG)status->Print();
	/*for(std::list<LineSegment*>::iterator i = status.begin();
			i != status.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}*/

	std::set<LineSegment*> left_and_contain;

	for(std::list<LineSegment*>::iterator i = left.begin();
			i != left.end(); i++)
	{
		left_and_contain.insert(*i);
		//left_and_contain.push_back(*i);
	}
	for(std::list<LineSegment*>::iterator i = contain.begin();
			i != contain.end(); i++)
	{
		left_and_contain.insert(*i);
		//left_and_contain.push_back(*i);
	}

	if(SLDEBUG)std::cout<<"\tadding left and contain to status"<<std::endl;

	//add (left U contain) to status
	for(std::set<LineSegment*>::iterator i = left_and_contain.begin();
			i != left_and_contain.end(); i++)
	{
		//InsertToStatus(status, *i, p);
		status->Insert(*i, p.X());
	}
	
	if(SLDEBUG)std::cout<<"\tstatus:"<<std::endl;
	if(SLDEBUG)status->Print();

	//checking intersections of new vertical lines
	for(std::list<LineSegment*>::iterator i = left.begin();
			i != left.end(); i++)
	{
		if(!(*i)->is_vertical()) continue;
		if(SLDEBUG)std::cout<<"\tchecking vertical intersections of "<<(*i)->str()
				<<std::endl;
		double u = (*i)->upper().Y();
		double l = (*i)->lower().Y();

		LineTreeNode *j = status->Highest();

		/*for(std::list<LineSegment*>::iterator j = status.begin();
				j != status.end(); j++)*/
		for(; j != NULL; j = j->Lower())
		{
			//if(SLDEBUG)std::cout<<"j->m_ptr="<<j->m_ptr->str()<<std::endl;

			if(j->m_ptr == *i) continue;

			IntersectionInfo info;
			info.t = SomeIntersection;

			if((j->m_ptr)->is_vertical()){
				double u2 = (j->m_ptr)->upper().Y();
				double l2 = (j->m_ptr)->lower().Y();
				if(l2 > u) continue;
				if(u2 < l) continue;
				if(dabs(u2-u) < POLYGON_TOLERANCE) continue;
				if(dabs(l2-u) < POLYGON_TOLERANCE) continue;
				if(dabs(u2-l) < POLYGON_TOLERANCE) continue;
				if(dabs(l2-l) < POLYGON_TOLERANCE) continue;
				if(SLDEBUG)std::cout<<"\t\tfound at with vertical line "<<(j->m_ptr)->str()<<std::endl;
				info.p = gp_Pnt(p.X(), u>u2?u:u2, 0);
			}
			else{
				double y = (j->m_ptr)->y_value_at(p.X());
				if(y > u) continue;
				if(y < l) break;
				if(dabs(y-u) < POLYGON_TOLERANCE) continue;
				if(dabs(y-l) < POLYGON_TOLERANCE) break;
				if(SLDEBUG)std::cout<<"\t\tfound at y="<<y<<" with "<<(j->m_ptr)->str()<<std::endl;
				info.p = gp_Pnt(p.X(), y, 0);
			}
			
			info.i = (j->m_ptr)->m_index;
			(*i)->m_intersections_set.insert(info);
			info.i = (*i)->m_index;
			(j->m_ptr)->m_intersections_set.insert(info);
		}
	}
	
	/*if(SLDEBUG)std::cout<<"\tstatus:"<<std::endl;
	for(std::list<LineSegment*>::iterator i = status.begin();
			i != status.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}*/

	if(SLDEBUG)std::cout<<"\tfinding events and adding them"<<std::endl;

	//if (left U contain) is empty
	if(left_and_contain.size() == 0)
	{
		if(SLDEBUG)std::cout<<"\tleft_and_contain is empty"<<std::endl;
		LineSegment *su = NULL, *sl = NULL;
		/*std::list<LineSegment*>::iterator i = status.begin();
		for(; i != status.end(); i++)
		{
			if(!((*i)->y_value_at(p.X()) < p.Y()))
			{
				if(i != status.begin())
				{
					i--;
					sl = (*i);
					i++;
				}
				break;
			}
		}
		for(; i != status.end(); i++)
		{
			if((*i)->y_value_at(p.X()) > p.Y())
			{
				su = (*i);
				break;
			}
		}*/
		LineTreeNode *highest = status->Highest();
		LineTreeNode *i = highest;
		for(; i != NULL; i = i->Lower())
		{
			if(!(i->m_ptr->y_value_at(p.X()) < p.Y()))
			{
				if(i != highest)
				{
					i = i->Higher();
					sl = i->m_ptr;
					i = i->Lower();
				}
				break;
			}
		}
		for(; i != NULL; i = i->Lower())
		{
			if(i->m_ptr->y_value_at(p.X()) > p.Y())
			{
				su = i->m_ptr;
				break;
			}
		}
		if(sl == NULL){ if(SLDEBUG)std::cout<<"\t\tno lower neighbour"<<std::endl; }
		else if(su == NULL){ if(SLDEBUG)std::cout<<"\t\tno upper neighbour"<<std::endl; }
		else
		{
			FindNewEvent(lines_vector, event_queue, su, sl, p);
		}
	}
	//(left U contain) isn't empty
	else
	{
		if(SLDEBUG)std::cout<<"\tleft_and_contain is not empty"<<std::endl;
		LineSegment *su2 = NULL; //the uppermost segment of left_and_contain in status
		LineSegment *su = NULL; //the upper neighbor of su2 in status
		/*for(std::list<LineSegment*>::reverse_iterator i = status.rbegin();
				i != status.rend(); i++)
		{
			std::set<LineSegment*>::iterator
				j = left_and_contain.find(*i);
			if(j != left_and_contain.end())
			{
				su2 = *i;
				if(i != status.rbegin()){
					i--;
					su = *i;
				}
				break;
			}
		}*/
		LineTreeNode *i;
		LineTreeNode *lowest = status->Lowest();
		i = lowest;
		for(; i != NULL; i = i->Higher())
		{
			std::set<LineSegment*>::iterator
				j = left_and_contain.find(i->m_ptr);
			if(j != left_and_contain.end())
			{
				su2 = i->m_ptr;
				if(i != lowest){
					i = i->Lower();
					su = i->m_ptr;
				}
				break;
			}
		}
		if(su2 == NULL) if(SLDEBUG)std::cout<<"EE\t\tsu2 == NULL"<<std::endl;
		if(su != NULL)
		{
			FindNewEvent(lines_vector, event_queue, su, su2, p);
		}
		else if(SLDEBUG)std::cout<<"\t\tsu == NULL"<<std::endl;

		LineSegment *sl2 = NULL; //the lowermost segment of left_and_contain in status
		LineSegment *sl = NULL; //the lower neighbor of sl2 in status
		/*for(std::list<LineSegment*>::iterator i = status.begin();
				i != status.end(); i++)
		{
			std::set<LineSegment*>::iterator
				j = left_and_contain.find(*i);
			if(j != left_and_contain.end())
			{
				sl2 = *i;
				if(i != status.begin()){
					i--;
					sl = *i;
				}
				break;
			}
		}*/
		LineTreeNode *highest = status->Highest();
		/*if(SLDEBUG)std::cout<<"looping status from highest to lowest, finding the first"
				" which is also in left_and_contain"<<std::endl;*/
		if(highest==NULL) if(SLDEBUG)std::cout<<"highest == NULL"<<std::endl;
		i = highest;
		for(; i != NULL; i = i->Lower())
		{
			//if(SLDEBUG)std::cout<<"i->m_ptr="<<i->m_ptr->str()<<std::endl;
			std::set<LineSegment*>::iterator
				j = left_and_contain.find(i->m_ptr);
			if(j != left_and_contain.end())
			{
				//if(SLDEBUG)std::cout<<"is in left_and_contain"<<std::endl;
				sl2 = i->m_ptr;
				if(i != highest){
					i = i->Higher();
					sl = i->m_ptr;
				}
				break;
			}
			else{
				//if(SLDEBUG)std::cout<<"is not in left_and_contain"<<std::endl;
			}
		}
		if(sl2 == NULL) if(SLDEBUG)std::cout<<"EE\t\tsl2 == NULL"<<std::endl;
		if(sl != NULL)
		{
			FindNewEvent(lines_vector, event_queue, sl, sl2, p);
		}
		else if(SLDEBUG)std::cout<<"\t\tsl == NULL"<<std::endl;
	}
	/*if(SLDEBUG)std::cout<<"\tstatus:"<<std::endl;
	for(std::list<LineSegment*>::iterator i = status.begin();
			i != status.end(); i++)
	{
		if(SLDEBUG)std::cout<<"\t\t"<<(*i)->str()<<std::endl;
	}*/
	//if(SLDEBUG)std::cout<<"\tstatus="<<int(status)<<std::endl;
}

unsigned int SweepLine(std::vector<LineSegment> &lines_vector)
{
	unsigned int num_intersection_points = 0;

	std::set<SweepEvent> event_queue;

	//status has all the lines going underneath the sweep line,
	//the highest y first
	LineTreeNode *status = new LineTreeNode(NULL);
	//if(SLDEBUG)std::cout<<"status="<<int(status)<<std::endl;
	//std::list<LineSegment*> status;

	if(SLDEBUG)std::cout<<"SweepLine(): lines_vector:"<<std::endl;
	for(unsigned int i=0; i<lines_vector.size(); i++)
	{
		lines_vector[i].m_index = i;
		if(SLDEBUG)std::cout<<"\t"<<lines_vector[i].str()<<std::endl;
		event_queue.insert(SweepEvent(Endpoint(&lines_vector[i], Left)));
		event_queue.insert(SweepEvent(Endpoint(&lines_vector[i], Right)));
	}
	//As status is initially empty, it doesn't really matter what is the value
	//of this, but set it as low as possible just to be sure.
	double oldx = -1.7e+308;
	while(!event_queue.empty())
	{
		if(SLDEBUG)std::cout<<"event_queue:"<<std::endl;
		for(std::set<SweepEvent>::iterator i = event_queue.begin();
				i != event_queue.end(); i++)
		{
			if(SLDEBUG)std::cout<<"\t"<<i->str()<<std::endl;
		}

		HandleEvent(lines_vector, event_queue, status, *event_queue.begin(),
				oldx, num_intersection_points);
		oldx = event_queue.begin()->x();
		event_queue.erase(event_queue.begin());

		/*if(SLDEBUG)std::cout<<"SweepLine(): status:"<<std::endl;
		for(std::list<LineSegment*>::iterator i = status.begin();
				i != status.end(); i++)
		{
			if(SLDEBUG)std::cout<<"\t"<<(*i)->str()<<std::endl;
		}*/
	}
	
	delete status;

	if(SLDEBUG)std::cout<<"SweepLine(): lines_vector:"<<std::endl;
	for(unsigned int i=0; i<lines_vector.size(); i++)
	{
		if(SLDEBUG)std::cout<<"\t"<<lines_vector[i].str();
		if(!lines_vector[i].m_intersections_set.empty())
			{ if(SLDEBUG)std::cout<<" intersects with:"<<std::endl; }
		else
			{ if(SLDEBUG)std::cout<<" doesn't intersect."<<std::endl; }
		for(std::set<IntersectionInfo>::const_iterator
				it = lines_vector[i].m_intersections_set.begin();
				it != lines_vector[i].m_intersections_set.end(); it++)
		{
			if(SLDEBUG)std::cout<<"\t\t"<<it->str()<<"  \t"<<lines_vector[it->i].str()<<std::endl;
		}
	}

	return num_intersection_points;
}

//Not used

class IntersectionPoint
{
public:
	bool operator == (gp_Pnt &i)
	{
		return (i.IsEqual(p, POLYGON_TOLERANCE) != Standard_False);
	}
	bool operator == (IntersectionPoint &i)
	{
		return (i.p.IsEqual(p, POLYGON_TOLERANCE) != Standard_False);
	}
	//from which line segment one came to this intersection
	LineSegment *from;
	//to what direction one moved along the segment
	Direction dir;
	//the intersection point
	gp_Pnt p;
};

//Not used

bool PointsCCW(std::list<IntersectionPoint> &points)
{
	if(points.size() == 0){
		return true;
	}
	double angle_sum = 0.;
	std::list<IntersectionPoint>::const_iterator ip = points.begin();
	gp_Pnt p1 = ip->p;
	ip++;
	gp_Vec v_last(p1, ip->p);
	p1 = ip->p;
	ip++;
	for(; ip != points.end(); ip++)
	{
		if(p1.IsEqual(ip->p, POLYGON_TOLERANCE)) continue;
		gp_Vec v_current(p1, ip->p);
		angle_sum += VecAngle(v_last, v_current);
		v_last = v_current;
		p1 = ip->p;
	}
	ip = points.begin();
	if(!p1.IsEqual(ip->p, POLYGON_TOLERANCE)){
		gp_Vec v_current(p1, ip->p);
		angle_sum += VecAngle(v_last, v_current);
		v_last = v_current;
		p1 = ip->p;
		ip++;
		if(!p1.IsEqual(ip->p, POLYGON_TOLERANCE)){
			v_current = gp_Vec(p1, ip->p);
			angle_sum += VecAngle(v_last, v_current);
		}
	}
	//we can just look if it's positive or negative
	if(angle_sum < 0.0) return false;
	return true;
}

bool UnionPolygons(std::vector<LineSegment> &lines_vector,
		std::list<CPolygon> &result_list)
{
	std::cout<<"UnionPolygons(): "<<lines_vector.size()<<" lines"<<std::endl;

	int num = SweepLine(lines_vector);
	
	std::cout<<"lines_vector:"<<std::endl;
	for(unsigned int i=0; i<lines_vector.size(); i++)
	{
		std::cout<<"\t"<<lines_vector[i].str();
		if(!lines_vector[i].m_intersections_set.empty())
			{ std::cout<<" intersects with:"<<std::endl; }
		else
			{ std::cout<<" doesn't intersect."<<std::endl; }
		for(std::set<IntersectionInfo>::const_iterator
				it = lines_vector[i].m_intersections_set.begin();
				it != lines_vector[i].m_intersections_set.end(); it++)
		{
			std::cout<<"\t\t"<<it->str()<<"  \t"<<lines_vector[it->i].str()<<std::endl;
		}
	}

	std::cout<<num<<" intersection points"<<std::endl;

	return true;
}

bool UnionPolygons(std::list<CPolygon> &polygons_list, std::list<CPolygon> &result_list)
{
	std::vector<LineSegment> lines_vector;
	for(std::list<CPolygon>::const_iterator i = polygons_list.begin();
			i != polygons_list.end(); i++)
	{
		for(std::list<gp_Pnt>::const_iterator j = i->begin();
				j != i->end(); j++)
		{
			lines_vector.push_back(i->GetLine(j));
		}
	}

	return UnionPolygons(lines_vector, result_list);
}


/*
This is an old version of the UnionPolygons algorithm. It worked
with many test cases but failed with real-life data, for some unknown reason.
*/

class Intersection
{
public:
	std::list<CPolygon>::const_iterator ipoly_a;
	std::list<CPolygon>::const_iterator ipoly_b;
	//first points of the lines intersecting, from the two polygons
	std::list<gp_Pnt>::const_iterator ipoint_a;
	std::list<gp_Pnt>::const_iterator ipoint_b;
	gp_Pnt p;
	double d;

	Intersection(const Intersection & i)
	{
		ipoly_a = i.ipoly_a;
		ipoly_b = i.ipoly_b;
		ipoint_a = i.ipoint_a;
		ipoint_b = i.ipoint_b;
		p = i.p;
		d = i.d;
	}
	Intersection(std::list<CPolygon>::const_iterator p_ipoly_a,
			std::list<CPolygon>::const_iterator p_ipoly_b,
			std::list<gp_Pnt>::const_iterator p_ipoint_a,
			std::list<gp_Pnt>::const_iterator p_ipoint_b,
			gp_Pnt p_p,
			double p_d)
	{
		ipoly_a = p_ipoly_a;
		ipoly_b = p_ipoly_b;
		ipoint_a = p_ipoint_a;
		ipoint_b = p_ipoint_b;
		p = p_p;
		d = p_d;
	}
	Intersection() {}
};

//TODO: make this use the new SweepLine

bool UnionPolygons_old(std::list<CPolygon> & polygons_list,
		std::list<CPolygon> & result_list)
{
	std::list< std::list<CPolygon>::const_iterator > used_polygons;
	int polygon_total_count = polygons_list.size();
	int polygon_use_count = 0;
	
	for(std::list<CPolygon>::iterator ipoly = polygons_list.begin();
			ipoly != polygons_list.end(); ipoly++)
	{
		if(ipoly->Direction() != PolyCW){
			if(UPODEBUG)std::cout<<"UnionPolygons: Works only with clockwise polygons."<<std::endl;
			//alternatively, you could just turn them here...
			return false;
		}
	}
	
	for(std::list<CPolygon>::const_iterator ipoly = polygons_list.begin();
			ipoly != polygons_list.end(); ipoly++)
	{
		//if(UPODEBUG)std::cout<<"loop level 1"<<std::endl;
		if(std::find(used_polygons.begin(), used_polygons.end(), ipoly) != used_polygons.end())
		{
			continue;
		}
		if(UPODEBUG)std::cout<<"----------------------"<<std::endl;
		if(UPODEBUG)std::cout<<"starting with ipoly = ";
		ipoly->Print();

		std::list< std::list<CPolygon>::const_iterator > union_used_polygons;
		std::list<gp_Pnt> union_points;
		std::list<gp_Pnt> intersection_points;
		std::list<gp_Pnt>::const_iterator ipoint;
		gp_Pnt a1, a2, b1, b2;
		bool recently_intersected = false;
		Intersection last_intersection; //valid only if recently_intersected
		bool first_point_of_union = true;
		
		if(UPODEBUG)std::cout<<"adding to union_used_polygons"<<std::endl;
		union_used_polygons.push_back(ipoly);

		//get an iterator to the polygon's points
		ipoint = ipoly->begin();

		while(true){
			if(UPODEBUG)std::cout<<std::endl;
			/*if(UPODEBUG)std::cout<<"current ipoly = ";
			ipoly->Print();*/
			
			//get the current point
			a1 = *ipoint;

			std::list<gp_Pnt>::const_iterator ia1 = ipoint;

			if(UPODEBUG)std::cout<<"a1=("<<a1.X()<<","<<a1.Y()<<")";
			if(recently_intersected)
			{
				if(UPODEBUG)std::cout<<" (is before the last intersection)"<<std::endl;
			}
			else
			{
				if(UPODEBUG)std::cout<<std::endl;
			}
			
			if(!recently_intersected){
				//check if we have made a loop
				if(union_points.size() >= 2){
					std::list<gp_Pnt>::iterator findresult;
					for(findresult = union_points.begin(); findresult != union_points.end(); findresult++)
					{
						if(findresult->IsEqual(a1, 0.0000000001)) break;
					}
					std::list<gp_Pnt>::iterator itemp = union_points.end();
					itemp--;
					if(findresult != union_points.end() && findresult != itemp)
					{
						if(UPODEBUG)std::cout<<"a1 found on union_points -> got a union"<<std::endl;
						//we have a union
						used_polygons.insert(used_polygons.end(), union_used_polygons.begin(),
								union_used_polygons.end());
						polygon_use_count += union_used_polygons.size();
						std::stringstream ss_name;
						ss_name<<"from "<<(*union_used_polygons.begin())->name;
						union_used_polygons.clear();
						if(UPODEBUG)std::cout<<"collected points: ";
						for(std::list<gp_Pnt>::iterator ipoint = union_points.begin();
								ipoint != union_points.end(); ipoint++)
						{
							if(UPODEBUG)std::cout<<"("<<ipoint->X()<<","<<ipoint->Y()<<"), ";
						}
						if(UPODEBUG)std::cout<<std::endl;
						std::list<gp_Pnt> points_stripped;
						points_stripped.insert(points_stripped.end(), findresult, union_points.end());
						CPolygon polygon(points_stripped);
						//Polygon polygon(union_points);
						polygon.name = ss_name.str();
						if(UPODEBUG)std::cout<<"adding polygon: "<<polygon.str()<<std::endl;
						result_list.push_back(polygon);
						if(polygon_use_count == polygon_total_count){
							//done it all!
							return true;
						}
						//get a next one
						break;
					}
				}
				if(!first_point_of_union)
				{
					//add the first point of line to union_points to keep track of where we're going
					if(UPODEBUG)std::cout<<"adding a1 to union_points"<<std::endl;
					union_points.push_back(a1);
				}
				else
				{
					if(UPODEBUG)std::cout<<"because a1 is the first point of this union, "
							"not adding it to union_points"<<std::endl;
				}
			}
			first_point_of_union = false;
			//get the second point
			ipoint++;
			//if a1 was the last one, jump to the first one
			if(ipoint == ipoly->end())
			{
				ipoint = ipoly->begin();
			}
			a2 = *ipoint;
			if(UPODEBUG)std::cout<<"collecting intersections with line: "<<"("<<a1.X()<<", "<<a1.Y()
					<<") -> ("<<a2.X()<<", "<<a2.Y()<<") ..."<<std::endl;

			//collect intersections

			std::list<Intersection*> is_list;

			//loop through every polygon
			for(std::list<CPolygon>::const_iterator b_ipoly = polygons_list.begin();
					b_ipoly != polygons_list.end(); b_ipoly++)
			{
				//skip the already used ones
				if(std::find(used_polygons.begin(), used_polygons.end(), b_ipoly) !=used_polygons.end())
				{
					//if(UPODEBUG)std::cout<<"already used  b_ipoly = ";
					//b_ipoly->Print();
					continue;
				}
				if(b_ipoly == ipoly) continue;
				if(UPODEBUG)std::cout<<"\tchecking points of "<<b_ipoly->str()
						<<"=b_ipoly"<<std::endl;

				//now we have an another polygon
				//we should check if any of its lines intersects with our current one
				
				//get the points of the polygon
				std::list<gp_Pnt>::const_iterator b_ipoint;
				b_ipoint = b_ipoly->begin();
				
				//and loop through them
				bool endwent = false;
				while(!endwent)
				{
					//if(UPODEBUG)std::cout<<"loop level 3"<<std::endl;

					//the first point for our line
					b1 = *b_ipoint;

					std::list<gp_Pnt>::const_iterator ib1 = b_ipoint;

					//and the second point. if we just got the last one, get the first one for this.
					b_ipoint++;
					if(b_ipoint == b_ipoly->end())
					{
						b_ipoint = b_ipoly->begin();
						endwent = true;
					}
					b2 = *b_ipoint;
					
					/*if(UPODEBUG)std::cout<<"checking line ("<<a1.X()<<", "<<a1.Y()<<")->("<<a2.X()<<", "<<a2.Y()
							<<") with line ("
							<<b1.X()<<", "<<b1.Y()<<")->("<<b2.X()<<", "<<b2.Y()<<")"<<std::endl;*/
					//check if it intersects with our current line
					LineSegment s1;
					s1.a = a1;
					s1.b = a2;
					LineSegment s2;
					s2.a = b1;
					s2.b = b2;
					IntersectionInfo info = s1.intersects(s2);
					gp_Pnt p = info.p;
					//if(LineIntersect(a1,a2,b1,b2,p)==true)
					if(info.t != NoIntersection)
					{
						double distance_from_a1 = p.Distance(a1);
						//double distance_from_last = -1.0;
						if(recently_intersected)
						{
							//distance_from_last = p.SquareDistance(last_intersection.p);
							double a1x = a1.X();
							double a1y = a1.Y();
							double px = p.X();
							double py = p.Y();
							double lx = last_intersection.p.X();
							double ly = last_intersection.p.Y();
							//check that last intersection was on the current line a1->a2
							gp_Lin lin(a1, gp_Vec(a1, a2));
							if(lin.SquareDistance(last_intersection.p) < 0.00000001){
								//if(UPODEBUG)std::cout<<"last intersection at ("<<lx<<","<<ly<<")"<<std::endl;
								//skip if intersection point is between the last intersection and a1
								if(!((a1x-px)*(px-lx)<0.0) && !((a1y-py)*(py-ly)<0.0))
								{
									if(UPODEBUG)std::cout<<"\t("<<p.X()<<", "<<p.Y()
											<<") is between last_intersection("
											<<lx<<","<<ly<<") and a1 -> skipping"
											<<std::endl;
									continue;
								}
							}
						}
						
						if(UPODEBUG)std::cout<<"\tintersection at ("<<p.X()<<", "<<p.Y()<<"), d="
								<<distance_from_a1<<std::endl;
						
						std::list<Intersection*>::iterator is_it = is_list.begin();
						if(is_it != is_list.end())
						{
							if((*is_it)->d < distance_from_a1){
								for(; is_it != is_list.end(); is_it++)
								{
									if((*is_it)->d < distance_from_a1)
									{
										is_it++;
										break;
									}
								}
							}
							is_list.insert(is_it, new Intersection(ipoly, b_ipoly, ia1, ib1, p, distance_from_a1));
						}
					} //if(LineIntersect())
				} //while(!endwent)
			} //for polygons_list
			
			if(is_list.empty()){ if(UPODEBUG)std::cout<<"did not find intersections"<<std::endl;}
			else if(UPODEBUG)std::cout<<"found some, looping through them..."<<std::endl;

			bool found_intersection = false;
			//std::list<Intersection>::const_iterator is_it
			for(std::list<Intersection*>::const_iterator is_it = is_list.begin();
					is_it != is_list.end(); is_it++)
			{
				if(UPODEBUG)std::cout<<"\tintersection at ("<<(*is_it)->p.X()<<", "
						<<(*is_it)->p.Y()<<"), d="<<(*is_it)->d<<std::endl;

				std::list<gp_Pnt>::const_iterator ipoint_b = (*is_it)->ipoint_b;
				gp_Pnt p1 = *ipoint_b;
				ipoint_b++;
				if(ipoint_b == ((*is_it)->ipoly_b)->end()) ipoint_b = ((*is_it)->ipoly_b)->begin();
				gp_Pnt p2 = *ipoint_b;
				if(UPODEBUG)std::cout<<"\t is line ("<<p1.X()<<", "<<p1.Y()<<")->("<<p2.X()<<", "<<p2.Y()<<")"<<std::endl;

				gp_Vec v1(a1, a2);
				gp_Vec v2(p1, p2);
				//double angle = v1.Angle(v2);
				double v1a = atan2(v1.Y(), v1.X());
				double v2a = atan2(v2.Y(), v2.X());
				double angle = v2a - v1a;
				if(angle <= -M_PI) angle += M_PI*2;
				else if(angle > M_PI) angle -= M_PI*2;
				if(UPODEBUG)std::cout<<"\t angle="<<angle<<std::endl;
				
				//if the end of the intersecting line is equal to the
				//intersection point, the line is not useful now
				if(p2.IsEqual((*is_it)->p, 0.0000000001)){
					if(UPODEBUG)std::cout<<"\t (p2.IsEqual((*is_it)->p, 0.0000000001))"<<std::endl;
					if(UPODEBUG)std::cout<<"\t -> intersection not useful"<<std::endl;
					continue;
				}
				
				if(angle < 0.0000000001){
					if(UPODEBUG)std::cout<<"\t line not useful: turns to right or doesn't turn"<<std::endl;
					continue;
				}
				//if the start of the intersecting line is equal to the
				//intersection point
				if(p1.IsEqual((*is_it)->p, 0.0000000001)){
					if(UPODEBUG)std::cout<<"\t (p1.IsEqual((*is_it)->p, 0.0000000001))"<<std::endl;
					if(a2.IsEqual((*is_it)->p, 0.0000000001)){
						std::list<gp_Pnt>::const_iterator ipoint_a = ipoint;
						ipoint_a++;
						if(ipoint_a == ipoly->end()) ipoint_a = ipoly->begin();
						gp_Pnt a3 = *ipoint_a;
						gp_Vec v3(a2, a3);
						double v3a = atan2(v3.Y(), v3.X());
						double angle = v3a - v2a;
						if(angle <= -M_PI) angle += M_PI*2;
						else if(angle > M_PI) angle -= M_PI*2;
						if(UPODEBUG)std::cout<<"\t angle="<<angle<<std::endl;
						if(angle < 0.0000000001){
							if(UPODEBUG)std::cout<<"\t line not useful: turns to right or doesn't turn"<<std::endl;
							continue;
						}
					}
					else{
					}
				}
				
				if(found_intersection) continue; //for printing all of them

				//check if we already have this intersection point

				std::list<gp_Pnt>::iterator findresult;
				for(findresult = intersection_points.begin(); findresult != intersection_points.end(); findresult++)
				{
					if(findresult->IsEqual((*is_it)->p, 0.0000000001)) break;
				}
				if(findresult != intersection_points.end())
				{
					if(UPODEBUG)std::cout<<"\tintersection point already on intersection_points"
							<<std::endl;
					//check if we have made a loop
					if(union_points.size() >= 2){
						std::list<gp_Pnt>::iterator findresult;
						for(findresult = union_points.begin(); findresult != union_points.end(); findresult++)
						{
							if(findresult->IsEqual((*is_it)->p, 0.0000000001)) break;
						}
						std::list<gp_Pnt>::iterator itemp = union_points.end();
						itemp--;
						if(findresult != union_points.end() && findresult != itemp)
						{
							//we have a union
							if(UPODEBUG)std::cout<<"\t:: p found on union_points -> got a union"<<std::endl;
							if(UPODEBUG)std::cout<<"\t-> adding it to union_points"<<std::endl;
							union_points.push_back((*is_it)->p);
							used_polygons.insert(used_polygons.end(), union_used_polygons.begin(),
									union_used_polygons.end());
							polygon_use_count += union_used_polygons.size();
							std::stringstream ss_name;
							ss_name<<"from "<<(*union_used_polygons.begin())->name;
union_used_polygons.clear();
							if(UPODEBUG)std::cout<<"collected points: ";
							for(std::list<gp_Pnt>::iterator ipoint = union_points.begin();
									ipoint != union_points.end(); ipoint++)
							{
								if(UPODEBUG)std::cout<<"("<<ipoint->X()<<","<<ipoint->Y()<<"), ";
							}
							if(UPODEBUG)std::cout<<std::endl;
							std::list<gp_Pnt> points_stripped;
							points_stripped.insert(points_stripped.end(), findresult, union_points.end());
							if(UPODEBUG)std::cout<<"adding polygon = ";
							CPolygon polygon(points_stripped);
							//Polygon polygon(union_points);
							polygon.name = ss_name.str();
							if(UPODEBUG)std::cout<<"adding polygon: "<<polygon.str()<<std::endl;
							result_list.push_back(polygon);
							if(polygon_use_count == polygon_total_count){
								//done it all!
								return true;
							}
							//get a next one
							goto unionpoly1;
						}
						else
						{
							if(UPODEBUG)std::cout<<"\t:: p not found on union_points -> "
									"didn't get a union"<<std::endl;
							continue;
						}
					}
					else if(UPODEBUG)std::cout<<"\t!(union_points.size() >= 2)"<<std::endl;
				}

				if(UPODEBUG)std::cout<<"\t\t!! Selected intersection"<<std::endl;
				
				if(UPODEBUG)std::cout<<"\t\t-> adding it to union_points and intersection_points"
						<<std::endl;
				union_points.push_back((*is_it)->p);
				intersection_points.push_back((*is_it)->p);
				
				/*if(UPODEBUG)std::cout<<"\tchanging ipoly = ";
				(*is_it)->ipoly_b->Print();*/
				ipoly = (*is_it)->ipoly_b;
				ipoint = (*is_it)->ipoint_b;
				recently_intersected = true;
				last_intersection = *(*is_it);
				
				//add to union_used_polygons if it isn't there
				if(std::find(union_used_polygons.begin(),
						union_used_polygons.end(), ipoly) == union_used_polygons.end())
				{
					if(UPODEBUG)std::cout<<"\t\t-> adding polygon to union_used_polygons"<<std::endl;
					union_used_polygons.push_back(ipoly);
				}
				found_intersection = true;
				//commented for printing all intersections
				//break;
			}

			for(std::list<Intersection*>::const_iterator is_it = is_list.begin();
					is_it != is_list.end(); is_it++)
			{
				delete (*is_it);
			}
			
			if(found_intersection)
			{
				if(UPODEBUG)std::cout<<std::endl<<"changed to "<<ipoly->str()<<"=ipoly"<<std::endl;
				continue;
			}
			if(UPODEBUG)std::cout<<"recently_intersected = false;"<<std::endl;
			recently_intersected = false;
		} //while(true)
unionpoly1:
		continue;
	} //for polygons_list

	return true;
}



