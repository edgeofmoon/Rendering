#pragma once

#include "MyMatrix.h"
#include "MyArray.h"

class MyMathHelper
{
public:
	MyMathHelper(void);
	~MyMathHelper(void);

	// memory leak alert
	// outMat and eigens are new
	static void SingularValueDecomposition(const MyMatrixf* inMat, MyMatrixf* outMat, float * eigens);

	static void MultiDimensionalProjection(const MyMatrixf* distMat, MyMatrixf* projMat, int nDim = 2);

	static int BinomialCoefficient(int n, int i);

	static int Factorial(int i);

	static bool InvertMatrix4x4ColMaj(const float m[16], float invOut[16]);

	static void TransposeMatrix4x4ColMaj(const float m[16], float transOut[16]);

	static MyVec3f MatMulVec(const MyMatrixf& mat, const MyVec3f& vec);

	static MyMatrixf InverseMatrix4x4ColMaj(const MyMatrixf& mat);

protected:
	static MyArrayi Factorials;
};

