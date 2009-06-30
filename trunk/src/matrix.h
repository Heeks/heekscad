#pragma once


class Matrix
{
	double* m_data;
	int m_n;
	int m_m;
	bool m_temp;
public:
	Matrix(int n, int m);
	~Matrix();
	double& operator() (unsigned row, unsigned col); 
	double  operator() (unsigned row, unsigned col) const;
	//double Matrix::operator() (unsigned row, unsigned col);
	double GetElement(unsigned row, unsigned col);
	Matrix Multiplied(Matrix* mat);
	void Multiply(Matrix* mat);
	Matrix Multiplied(double s);
	void Multiply(double s);
	Matrix Added(double s);
	Matrix Added(Matrix *mat);
	void Add(double s);
	void Add(Matrix *mat);
	Matrix Subtracted(double s);
	Matrix Subtracted(Matrix *mat);
	void Subtract(Matrix* mat);
	void Subtract(double s);
	void Divide(double s);
	Matrix Divided(double s);
};
