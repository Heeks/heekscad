// OCCSolder.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "OCCSolver.h"

OCCSolver::OCCSolver()
{

}

OCCSolver::~OCCSolver()
{

}

int OCCSolver::solve(double  **x,int xLength, constraint * cons, int consLength, int isFine)
{
	pert = 10e-11;

	Load(cons,consLength,x,xLength);
	math_Vector myvec = math_Vector(1,GetVectorSize());//TODO: inclusive?
	vector = &myvec;
	for(int i=1; i <= GetVectorSize(); i++)
		(*vector)(i) = GetInitialValue(i-1);

	math_BFGS bfgs(*this,*vector,10e-8);
	Unload();
	return 0;
}

Standard_Boolean OCCSolver::Value(const math_Vector& X,Standard_Real& F)
{
	(*vector).Set(1,GetVectorSize(),X);
	F = GetError();
	return true;
}

Standard_Boolean OCCSolver::Gradient(const math_Vector& X,math_Vector& G)
{
	(*vector).Set(1,GetVectorSize(),X);
	for(int i=1; i <= GetVectorSize(); i++)
	{
		G(i) = GetGradient(i-1,pert);
	}
	return true;
}

Standard_Boolean OCCSolver::Values(const math_Vector& X,Standard_Real& F,math_Vector& G)
{
	Value(X,F);
	Gradient(X,G);
	return true;
}
