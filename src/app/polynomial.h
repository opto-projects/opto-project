

#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <stdlib.h>
#include <math.h>

class Polynomial
{
public:
	Polynomial(float* coeffs, unsigned int order, unsigned int size);
	Polynomial();
	~Polynomial();

	void setOrder(unsigned int order);
	unsigned int getOrder() { return this->order; }
	void setCoeffs(float* coeffs, unsigned int order);
	void setCoeff(float coeff, unsigned int coeffNr);
	float getCoeff(unsigned int coeffNr);
	void setSize(unsigned int size);
	unsigned int getSize() { return this->size; }
	float getValueAt(float x);
	float* getData();
	static void clamp(float* inputData, unsigned int inputLength, float min, float max);
	

private:
	void updateData();

	float* data;
	float* coeffs;
	unsigned int size;
	unsigned int order;
	bool polynomialChanged;
};
#endif // POLYNOMIAL_H
