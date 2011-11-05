// NearMap.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class OneDNearMap
{
protected:
	double m_tol;
	int n_vecs;

	std::vector< std::pair<double,std::vector<void*>* > > m_data;
	std::map<double, std::vector<void*>* > m_map;

public:
	OneDNearMap(double tol);
	OneDNearMap();
	~OneDNearMap();

	void init(double tol);
	void insert(double, void*);
	virtual void sort();
	void remove(double, void*);
	void remap(double, void*, void*);
	void find(double, std::vector<void*>&);
	void find(double, std::vector<void**>&);
	virtual int GetVecCount();
	virtual double GetCoord(int vec);
	bool IsValid(int vec);
};

class TwoDNearMap : private OneDNearMap
{
public:
	TwoDNearMap(double tol);
	~TwoDNearMap();

	void insert(double, double, void*);
	void sort();
	void remove(double, double, void*);
	void remap(double, double, void*, void*);
	void find(double, double, std::vector<void*>&);
	void find(double, double, std::vector<void**>&);
	int GetVecCount();
	OneDNearMap* GetElement(int vec);
	double GetCoord(int vec);
};

