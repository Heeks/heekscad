// matrixfactorization.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "matrix.h"

Matrix Negatived(Matrix *m)
{
	Matrix nmat(m->GetN(),m->GetM());
	for(int i=0; i < m->GetN(); i++)
		for(int j=0; j < m->GetM(); j++)
		{
			double v = m->GetElement(i,j);
			nmat(i,j) = (fabs(v) - v)/2;
		}
	return nmat;
}

Matrix Positived(Matrix *m)
{
	Matrix nmat(m->GetN(),m->GetM());
	for(int i=0; i < m->GetN(); i++)
		for(int j=0; j < m->GetM(); j++)
		{
			double v = m->GetElement(i,j);
			nmat(i,j) = (fabs(v) + v)/2;
		}
	return nmat;
}

int n_iters = 1000;
int n_features = 2;
std::pair<Matrix,Matrix> ConvexNMF(Matrix X)
{
	Matrix Xt = X.Transposed();
	Matrix XtX = Xt.Multiplied(&X);
    Matrix XtXN = Negatived(&XtX);
	Matrix XtXP = Positived(&XtX);

	Matrix W(X.GetM(),n_features);
	W.Randomize();
	Matrix G(X.GetM(),n_features);
	G.Randomize();

	for(int i=0; i < n_iters; i++)
	{
		//Calculate matrices for update of G
		Matrix NG = G;

		Matrix Wt = W.Transposed();
		Matrix GWt = G.Multiplied(&Wt);

		Matrix XtXPW = XtXP.Multiplied(&W); //umm?
		Matrix XtXNW = XtXN.Multiplied(&W);

		Matrix GWtXtX = GWt.Multiplied(&XtX);
		Matrix GWtXtXN = Negatived(&GWtXtX);
		Matrix GWtXtXP = Positived(&GWtXtX);

		Matrix GWtXtXNW = GWtXtXN.Multiplied(&W);
		Matrix GWtXtXPW = GWtXtXP.Multiplied(&W);

		//Update G
		for(int j =0; j < G.GetN(); j++)
			for(int k=0; k < G.GetM(); k++)
			{
				NG(j,k) = G(j,k) * sqrt((XtXPW(j,k) + GWtXtXNW(j,k))/(XtXNW(j,k) + GWtXtXPW(j,k)));
			}

		//Calculate additional matrices for update of W
		Matrix Gt = G.Transposed();
		Matrix GtG = Gt.Multiplied(&G);
		Matrix WGtG = W.Multiplied(&GtG);

		Matrix XtXPG = XtXP.Multiplied(&G);
		Matrix XtXNG = XtXN.Multiplied(&G);

		Matrix XtXPWGtG = XtXP.Multiplied(&WGtG);
		Matrix XtXNWGtG = XtXN.Multiplied(&WGtG);

		//Update W
		for(int j =0; j < W.GetN(); j++)
			for(int k=0; k < W.GetM(); k++)
			{
				W(j,k) = W(j,k) * sqrt((XtXPG(j,k) + XtXNWGtG(j,k))/(XtXNG(j,k) + XtXPWGtG(j,k)));
			}

		G = NG;
	}

	Matrix F = X.Multiplied(&W);
	return std::pair<Matrix,Matrix>(G,F);
}

void TestMatrixFac()
{
	Matrix m(5,7);
	m(0,0) = 1.3;
	m(0,1) = 1.8;
	m(0,2) = 4.8;
	m(0,3) = 7.1;
	m(0,4) = 5.0;
	m(0,5) = 5.2;
	m(0,6) = 8.0;

	m(1,0) = 1.5;
	m(1,1) = 6.9;
	m(1,2) = 3.9;
	m(1,3) = -5.5;
	m(1,4) = -8.5;
	m(1,5) = -3.9;
	m(1,6) = -5.5;

	m(2,0) = 6.5;
	m(2,1) = 1.6;
	m(2,2) = 8.2;
	m(2,3) = -7.2;
	m(2,4) = -8.7;
	m(2,5) = -7.9;
	m(2,6) = -5.2;

	m(3,0) = 3.8;
	m(3,1) = 8.3;
	m(3,2) = 4.7;
	m(3,3) = 6.4;
	m(3,4) = 7.5;
	m(3,5) = 3.2;
	m(3,6) = 7.4;

	m(4,0) = -7.3;
	m(4,1) = -1.8;
	m(4,2) = -2.1;
	m(4,3) = 2.7;
	m(4,4) = 6.8;
	m(4,5) = 4.8;
	m(4,6) = 6.2;

	ConvexNMF(m);
}