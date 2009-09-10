// OCCSolver.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../sketchsolve/src/solve.h"

class OCCSolver: public SolveImpl, public math_MultipleVarFunctionWithGradient
{
	double pert;
	math_Vector *vector;
public:
	OCCSolver();
	~OCCSolver();
	
	int solve(double  **x,int xLength, constraint * cons, int consLength, int isFine);
	double GetElement(size_t i){return (*vector)(i+1);}
	void SetElement(size_t i, double v) {(*vector)(i+1) = v;}
	Standard_Integer NbVariables() const {return GetVectorSize();}
	Standard_Boolean Value(const math_Vector& X,Standard_Real& F);
	Standard_Boolean Gradient(const math_Vector& X,math_Vector& G);
	Standard_Boolean Values(const math_Vector& X,Standard_Real& F,math_Vector& G);
};
