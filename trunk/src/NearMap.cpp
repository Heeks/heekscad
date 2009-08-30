// NearMap.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "NearMap.h"

// This file implements a class very similar in nature to a hash map. 
// Using hash maps with doubles is plagues with problems because
// of numerical differences. This class circumvents the problem by returning
// a list of all objects that are within some tolerance of the requested item

// Binary searches can be tricky. The problem being that you need a constant time
// container, ie a vector, in order to get log N searches. However, by using such a
// container, you get very expensive inserts and adds, say O(n), so over a whole
// set of data you can get O(n^2). 

// We are somewhat lucky here because we know what multipoly is doing. It starts with a huge list
// of points and never adds to those points. It also never searches before all points are added. 
// This gives us an opportunity to use a single sort. Therefore we O(1) add, and log N search. 
// The remaining issue is that of removal. These would still be expensive when using a constant
// time container, but we don't actually have to remove them. Just mark them as invalid. 
// The memory is already allocated, and isn't going to be used for anything else until this
// function returns, so it is no big deal. On the other hand it will make the constants in the
// log N searches somewhat larger. 

OneDNearMap::OneDNearMap(double tol)
{
	init(tol);
}

OneDNearMap::OneDNearMap()
{

}

OneDNearMap::~OneDNearMap()
{
	for(unsigned i=0; i < m_data.size(); i++)
		delete m_data[i].second;
}

void OneDNearMap::init(double tol)
{
	m_tol = tol;
	n_vecs = 0;
}

void OneDNearMap::insert(double at, void * p_data)
{
	//Check if a vector exists for this *exact* double
	std::map<double, std::vector<void*>* >::iterator it = m_map.find(at);
	if(it != m_map.end())
	{
		//We got a vector
		std::vector<void*>* vec = (*it).second;
		vec->push_back(p_data);
		return;
	}

	//Allocate a new vector
	std::vector<void*>* new_vec = new std::vector<void*>();
	new_vec->push_back(p_data);
	m_map[at] = new_vec;
	m_data.push_back(std::pair<double,std::vector<void*>*>(at,new_vec));
	n_vecs++;
}

struct SAscending
{
     bool operator()(const std::pair<double,std::vector<void*>* > &pStart, const std::pair<double,std::vector<void*>* >& pEnd)
     {
		 return pStart.first < pEnd.first;
     }
};

struct SAscending2
{
     bool operator()(const std::pair<double,std::vector<void*>* > pStart, const std::pair<double,std::vector<void*>* > pEnd)
     {
		 return pStart.first < pEnd.first;
     }
};


void OneDNearMap::sort()
{
	std::sort(m_data.begin(),m_data.end(), SAscending());
}

void OneDNearMap::remove(double at, void* pData)
{
	remap(at,pData,0);
}

void OneDNearMap::remap(double at, void* pOld, void* pNew)
{
	//check for occurances of pOld around at and replace
	std::vector<void**> my_vec;
	find(at,my_vec);
	for(unsigned i=0; i < my_vec.size(); i++)
	{
		if(*my_vec[i] == pOld)
			*my_vec[i] = pNew;
	}
}

void OneDNearMap::find(double at, std::vector<void*>& pRet)
{
	std::vector<void**> my_vec;
	find(at,my_vec);
	for(unsigned i=0; i < my_vec.size(); i++)
		pRet.push_back(*my_vec[i]);
}

void OneDNearMap::find(double at, std::vector<void**>& pRet)
{
	//Get 2 interators using the std binary search code

	std::vector<void*> null_vec;
	std::vector< std::pair<double,std::vector<void*>* > >::iterator it1 = std::lower_bound(m_data.begin(),m_data.end(), std::pair<double,std::vector<void*>* >(at-m_tol,&null_vec),SAscending2());
	std::vector< std::pair<double,std::vector<void*>* > >::iterator it2 = std::upper_bound(m_data.begin(),m_data.end(), std::pair<double,std::vector<void*>* >(at+m_tol,&null_vec),SAscending2());

	//Increment the iterators as they are currently located on the outside of valid space
	//it1++;
	it2--;

	//Check the it1 is less than it2, otherwise we have a null solution
	if(it1 >= m_data.end() || it2 < m_data.begin())
		return;

	while(it1 < m_data.end() && (*it1).first <= (*it2).first)
	{
		for(size_t i=0; i < (*it1).second->size(); i++)
		{
			void* pFound = (*(*it1).second)[i];
			if(pFound)
				pRet.push_back(&(*(*it1).second)[i]);
		}
		it1++;
	}
}

int OneDNearMap::GetVecCount()
{
	return n_vecs;
}

double OneDNearMap::GetCoord(int vec)
{
	return m_data[vec].first;
}

bool OneDNearMap::IsValid(int vec)
{
	std::vector<void*>::iterator it;
	for(it = (*m_data[vec].second).begin(); it != (*m_data[vec].second).end(); it++)
		if(*it)
			return true;
	return false;
}

void Test1DNearMap()
{
	OneDNearMap map(.01);
	map.insert(1,(void*)1000);
	map.insert(2,(void*)2000);
	map.insert(3,(void*)3000);
	map.insert(1,(void*)1001);
	map.insert(2.001,(void*)2001);
	map.insert(1.999,(void*)2002);

	map.sort();

	std::vector<void*> myvec;
	map.find(2,myvec);

	map.remove(2,(void*)2000);
	myvec.clear();

	map.find(2,myvec);

}

//The following class uses the above to create a 2 dimensional version. Something like a quad-tree
//Basically we create a 1d Near Map of 1d Near maps. We need to do some funny stuff to get things in the
//correct branch. More or less we reimplement insert to get different allocation behavior

TwoDNearMap::TwoDNearMap(double tol) : OneDNearMap(tol)
{
}

TwoDNearMap::~TwoDNearMap()
{
}

void TwoDNearMap::insert(double atX, double atY, void * p_data)
{
	//Check if a vector exists for this *exact* double
	std::map<double, std::vector<void*>* >::iterator it = m_map.find(atX);
	if(it != m_map.end())
	{
		//We got a vector
		std::vector<void*>* vec = (*it).second;
		OneDNearMap* pMap = (OneDNearMap*)(*vec)[0];
		pMap->insert(atY,p_data);
		return;
	}

	//Allocate a new vector
	std::vector<void*>* new_vec = new std::vector<void*>();
	OneDNearMap *new_nearmap = new OneDNearMap(m_tol);
	new_nearmap->insert(atY,p_data);
	new_vec->push_back(new_nearmap);
	m_map[atX] = new_vec;
	m_data.push_back(std::pair<double,std::vector<void*>*>(atX,new_vec));
	n_vecs++;

}

void TwoDNearMap::sort()
{
	OneDNearMap::sort();
	for(size_t i=0; i < m_data.size(); i++)
	{
		((OneDNearMap*)(*m_data[i].second)[0])->sort();
	}
}

void TwoDNearMap::remove(double atX, double atY, void* pData)
{
	remap(atX,atY,pData,0);
}

void TwoDNearMap::remap(double atX, double atY, void* pOld, void* pNew)
{
	//Find all Y vectors near this X and pass the remap down
	std::vector<void*> my_vec;
	OneDNearMap::find(atX,my_vec);
	for(unsigned i=0; i < my_vec.size(); i++)
	{
		OneDNearMap* pMap = (OneDNearMap*)my_vec[i];
		pMap->remap(atY,pOld,pNew);
	}

}

void TwoDNearMap::find(double atX, double atY, std::vector<void*>& pRet)
{
	std::vector<void**> my_vec;
	find(atX,atY,my_vec);
	for(unsigned i=0; i < my_vec.size(); i++)
		pRet.push_back(*my_vec[i]);
}

void TwoDNearMap::find(double atX, double atY, std::vector<void**>& pRet)
{
	//Get 2 interators using the std binary search code

	std::vector<void*> null_vec;
	std::vector< std::pair<double,std::vector<void*>* > >::iterator it1 = std::lower_bound(m_data.begin(),m_data.end(), std::pair<double,std::vector<void*>* >(atX-m_tol,&null_vec),SAscending2());
	std::vector< std::pair<double,std::vector<void*>* > >::iterator it2 = std::upper_bound(m_data.begin(),m_data.end(), std::pair<double,std::vector<void*>* >(atX+m_tol,&null_vec),SAscending2());

	//Increment the iterators as they are currently located on the outside of valid space
	//it1++;
	it2--;

	//Check the it1 is less than it2, otherwise we have a null solution
	while(it1 < m_data.end() && (*it1).first <= (*it2).first)
	{
		OneDNearMap* pFound = (OneDNearMap*)(*(*it1).second)[0];
		pFound->find(atY,pRet);
		it1++;
	}
}

void Test2DNearMap()
{
	TwoDNearMap map(.01);
	map.insert(1,1,(void*)1000);
	map.insert(2,1,(void*)2000);
	map.insert(3,2,(void*)3000);
	map.insert(1,1,(void*)1001);
	map.insert(2.001,1,(void*)2001);
	map.insert(1.999,1,(void*)2002);

	map.sort();

	std::vector<void*> myvec;
	map.find(2,1,myvec);

	map.remove(2,1,(void*)2000);
	myvec.clear();

	map.find(2,1,myvec);

}

int TwoDNearMap::GetVecCount()
{
	return OneDNearMap::GetVecCount();
}

OneDNearMap* TwoDNearMap::GetElement(int vec)
{
	return (OneDNearMap*)(*m_data[vec].second)[0];
}

double TwoDNearMap::GetCoord(int vec)
{
	return OneDNearMap::GetCoord(vec);
}

