// matrix.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "matrix.h"

Matrix::Matrix(int n, int m, double init)
{
	m_n = n;
	m_m = m;
	m_data.resize(n*m);

	for(int i=0; i < n * m; i++)
		m_data[i] = init;
}

Matrix::~Matrix()
{
}

inline
double Matrix::GetElement(unsigned row, unsigned col)
{
	return m_data[m_m*row + col];
}

inline
double& Matrix::operator() (unsigned row, unsigned col)
{
   return m_data[m_m*row + col];
}
 
inline
double Matrix::operator() (unsigned row, unsigned col) const
{
   return m_data[m_m*row + col];
} 

void Matrix::Randomize()
{
	for(int i=0; i < m_m * m_n; i++)
	{
		m_data[i] = rand() / 100.0;
	}
}

Matrix Matrix::Multiplied(Matrix* mat)
{
	if(m_m != mat->m_n)
		return Matrix(0,0);

	Matrix nmat(m_n,mat->m_m);

	for(int i=0; i < m_n; i++)
	{
		for(int j=0; j < mat->m_m; j++)
		{
			nmat(i,j) = 0;
			for(int k=0; k < m_m; k++)
			{
				nmat(i,j) += GetElement(i,k) * mat->GetElement(k,j);
			}
		}
	}
	return nmat;
}

Matrix Matrix::Multiplied(double s)
{
	Matrix nmat(m_n,m_m);

	for(int i=0; i < m_n; i++)
	{
		for(int j=0; j < m_m; j++)
		{
			nmat(i,j) = s * GetElement(i,j);
		}
	}
	return nmat;
}

void Matrix::Multiply(Matrix *mat)
{
	Matrix m = Multiplied(mat);
	m_m = m.m_m;
	m_n = m.m_n;
	m_data = m.m_data;
}

Matrix Matrix::Transposed()
{
	Matrix nmat(m_m,m_n);
	for(int i=0; i < m_n; i++)
	{
		for(int j=0; j < m_m; j++)
		{
			nmat(j,i) = GetElement(i,j);
		}
	}
	return nmat;
}

void Matrix::Transpose()
{
	Matrix m = Transposed();
	m_m = m.m_m;
	m_n = m.m_n;
	m_data = m.m_data;
}


void Matrix::Multiply(double s)
{
	Matrix m = Multiplied(s);
	m_m = m.m_m;
	m_n = m.m_n;
	m_data = m.m_data;
}

void Matrix::Add(Matrix* mat)
{
	Matrix m = Added(mat);
	m_m = m.m_m;
	m_n = m.m_n;
	m_data = m.m_data;
}

void Matrix::Subtract(Matrix* mat)
{
	Matrix m = Subtracted(mat);
	m_m = m.m_m;
	m_n = m.m_n;
	m_data = m.m_data;
}

void Matrix::Subtract(double s)
{
	Matrix m = Subtracted(s);
	m_m = m.m_m;
	m_n = m.m_n;
	m_data = m.m_data;
}

void Matrix::Add(double s)
{
	Matrix m = Added(s);
	m_m = m.m_m;
	m_n = m.m_n;
	m_data = m.m_data;
}

void Matrix::Divide(double s)
{
	Matrix m = Divided(s);
	m_m = m.m_m;
	m_n = m.m_n;
	m_data = m.m_data;
}

Matrix Matrix::Added(Matrix *mat)
{
	if(m_m != mat->m_m || m_n != mat->m_n)
		return Matrix(0,0);

	Matrix nmat(m_n,m_m);
	for(int i=0; i < m_n; i++)
	{
		for(int j=0; j < m_m; j++)
		{
			nmat(i,j) = GetElement(i,j) + mat->GetElement(i,j);
		}
	}

	return nmat;
}

Matrix Matrix::Subtracted(Matrix *mat)
{
	if(m_m != mat->m_m || m_n != mat->m_n)
		return Matrix(0,0);

	Matrix nmat(m_n,m_m);
	for(int i=0; i < m_n; i++)
	{
		for(int j=0; j < m_m; j++)
		{
			nmat(i,j) = GetElement(i,j) - mat->GetElement(i,j);
		}
	}

	return nmat;
}

Matrix Matrix::Subtracted(double s)
{
	Matrix nmat(m_n,m_m);
	for(int i=0; i < m_n; i++)
	{
		for(int j=0; j < m_m; j++)
		{
			nmat(i,j) = GetElement(i,j) - s;
		}
	}

	return nmat;
}

Matrix Matrix::Added(double s)
{
	Matrix nmat(m_n,m_m);
	for(int i=0; i < m_n; i++)
	{
		for(int j=0; j < m_m; j++)
		{
			nmat(i,j) = GetElement(i,j) + s;
		}
	}

	return nmat;
}

Matrix Matrix::Divided(double s)
{
	Matrix nmat(m_n,m_m);
	for(int i=0; i < m_n; i++)
	{
		for(int j=0; j < m_m; j++)
		{
			nmat(i,j) = GetElement(i,j) / s;
		}
	}

	return nmat;
}

bool TestTranspose()
{
	Matrix m(2,2);
	m(0,0) = 0;
    m(0,1) = 1;
	m(1,0) = -1;
	m(1,1) = 0;

	m.Transpose();

	if(m(0,0) != 0 || m(0,1) != -1 || m(1,0) != 1 || m(1,1) != 0)
		return false;
	return true;
}

bool TestMultiply()
{
	Matrix m(2,2);
	m(0,0) = 0;
    m(0,1) = 1;
	m(1,0) = 2;
	m(1,1) = 3;

	Matrix m2(2,3);
	m2(0,0) = 4;
    m2(0,1) = 5;
	m2(0,2) = 6;

	m2(1,0) = 7;
	m2(1,1) = 8;
	m2(1,2) = 9;

	Matrix m3 = m.Multiplied(&m2);
	if(m3(0,0) != 7 || m3(0,1) != 8 || m3(0,2) != 9 || m3(1,0) != 29 || m3(1,1) != 34 || m3(1,2) != 39)
		return false;
	return true;
}

void TestMatrix()
{
	TestTranspose();
	TestMultiply();
}