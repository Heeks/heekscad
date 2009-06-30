// matrix.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "matrix.h"

Matrix::Matrix(int n, int m)
{
	m_n = n;
	m_m = m;
	m_temp = false;
	m_data = new double[n*m];
}

Matrix::~Matrix()
{
	if(!m_temp)
		delete m_data;
}

inline
double Matrix::GetElement(unsigned row, unsigned col)
{
	return m_data[m_n*row + col];
}

inline
double& Matrix::operator() (unsigned row, unsigned col)
{
   return m_data[m_n*row + col];
}
 
inline
double Matrix::operator() (unsigned row, unsigned col) const
{
   return m_data[m_n*row + col];
} 

Matrix Matrix::Multiplied(Matrix* mat)
{
	if(m_m != mat->m_n)
		return Matrix(0,0);

	Matrix nmat(m_n,mat->m_m);

	for(int i=0; i < m_n; i++)
	{
		for(int j=0; j < m_m; j++)
		{
			nmat(i,j) = 0;
			for(int k=0; k < mat->m_m; k++)
			{
				nmat(i,j) += GetElement(i,j) * mat->GetElement(j,k);
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
	m.m_temp = true;
	m_m = m.m_m;
	m_n = m.m_n;
	delete m_data;
	m_data = m.m_data;
}

void Matrix::Multiply(double s)
{
	Matrix m = Multiplied(s);
	m.m_temp = true;
	m_m = m.m_m;
	m_n = m.m_n;
	delete m_data;
	m_data = m.m_data;
}

void Matrix::Add(Matrix* mat)
{
	Matrix m = Added(mat);
	m.m_temp = true;
	m_m = m.m_m;
	m_n = m.m_n;
	delete m_data;
	m_data = m.m_data;
}

void Matrix::Subtract(Matrix* mat)
{
	Matrix m = Subtracted(mat);
	m.m_temp = true;
	m_m = m.m_m;
	m_n = m.m_n;
	delete m_data;
	m_data = m.m_data;
}

void Matrix::Subtract(double s)
{
	Matrix m = Subtracted(s);
	m.m_temp = true;
	m_m = m.m_m;
	m_n = m.m_n;
	delete m_data;
	m_data = m.m_data;
}

void Matrix::Add(double s)
{
	Matrix m = Added(s);
	m.m_temp = true;
	m_m = m.m_m;
	m_n = m.m_n;
	delete m_data;
	m_data = m.m_data;
}

void Matrix::Divide(double s)
{
	Matrix m = Divided(s);
	m.m_temp = true;
	m_m = m.m_m;
	m_n = m.m_n;
	delete m_data;
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