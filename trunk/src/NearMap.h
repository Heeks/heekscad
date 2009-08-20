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
	std::vector<std::vector<void*> > m_vectors;

public:
	OneDNearMap(double tol, int n_vectors);
	OneDNearMap();
	~OneDNearMap();

	void init(double tol, int n_vectors);
	void insert(double, void*);
	virtual void sort();
	void remove(double, void*);
	void remap(double, void*, void*);
	void find(double, std::vector<void*>&);

};

class TwoDNearMap : private OneDNearMap
{
	std::vector<OneDNearMap> m_nearmaps;
	
public:
	TwoDNearMap(double tol, int x_vectors, int y_vectors);
	~TwoDNearMap();

	void insert(double, double, void*);
	void sort();
	void remove(double, double, void*);
	void remap(double, double, void*, void*);
	void find(double, double, std::vector<void*>&);

};

