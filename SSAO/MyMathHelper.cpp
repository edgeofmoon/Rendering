#include "MyMathHelper.h"

#include "svd.c"

#include <vector>
#include <algorithm>

// more than 40 is hardly expressable
MyArrayi MyMathHelper::Factorials = MyArrayi(40, -1);

MyMathHelper::MyMathHelper(void)
{
}


MyMathHelper::~MyMathHelper(void)
{
}

void MyMathHelper::SingularValueDecomposition(const MyMatrixf* inMat, MyMatrixf* outMat, float * eigens){
	int m = inMat->GetNumRows();
	int n = inMat->GetNumCols();
	float** a = new float*[m];
	float** v = new float*[m];
	for (int i = 0;i<m;i++){
		a[i] = new float[n];
		v[i] = new float[n];
		for (int j = 0;j<n;j++){
			a[i][j] = inMat->At(i,j);
		}
	}
	eigens = new float[n];
	
	dsvd(a,m,n,eigens,v);

	outMat = new MyMatrixf((const float **)a,m,n);

	delete[] a;
	delete[] v;
}

float *w;
bool largerThan(int i,int j){
	return w[i]>w[j];
};

void MyMathHelper::MultiDimensionalProjection(const MyMatrixf* distMat, MyMatrixf* projMat, int nDim){
	MyMatrixf* transMat;
	float *eigens;
	MyMathHelper::SingularValueDecomposition(distMat,transMat,eigens);

	
	std::vector<int> evs;
	for (int i = 0;i<transMat->GetNumCols();i++){
		evs.push_back(i);
	}

	w = eigens;

	std::sort(evs.begin(),evs.end(),largerThan);
	evs.resize(nDim);

	projMat = new MyMatrixf(transMat->GetRows(evs));
}


int MyMathHelper::BinomialCoefficient(int n, int i){
	if(i == 0 || i == n) return 1;
	return Factorial(n)/(Factorial(i)*Factorial(n-i));
}

int MyMathHelper::Factorial(int i){
	if(i == 0) return 1;
	if(Factorials[i] > 0){
		return Factorials[i];
	}
	else{
		int rst = i*Factorial(i-1);
		Factorials[i] = rst;
		return rst;
	}
}

bool MyMathHelper::InvertMatrix4x4ColMaj(const float m[16], float invOut[16]){
	double inv[16], det;
	int i;

	inv[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
		return false;

	det = 1.0 / det;

	for (i = 0; i < 16; i++)
		invOut[i] = inv[i] * det;

	return true;
}

void MyMathHelper::TransposeMatrix4x4ColMaj(const float m[16], float transOut[16]){
	float tmp[16];
	memcpy(tmp, m, 16 * sizeof(float));
	transOut[0] = tmp[0];
	transOut[1] = tmp[4];
	transOut[2] = tmp[8];
	transOut[3] = tmp[12];

	transOut[4] = tmp[1];
	transOut[5] = tmp[5];
	transOut[6] = tmp[9];
	transOut[7] = tmp[13];

	transOut[8] = tmp[2];
	transOut[9] = tmp[6];
	transOut[10] = tmp[10];
	transOut[11] = tmp[14];

	transOut[12] = tmp[3];
	transOut[13] = tmp[7];
	transOut[14] = tmp[11];
	transOut[15] = tmp[15];
}

MyVec3f MyMathHelper::MatMulVec(const MyMatrixf& mat, const MyVec3f& vec){
	float d[16];
	memcpy(d, mat.GetData(), 16 * sizeof(float));
	MyVec4f rst(0,0,0,0);
	MyVec4f _vec4(vec[0], vec[1], vec[2], 1);
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			rst[i] += mat.At(i, j)*_vec4[j];
		}
	}
	return MyVec3f(rst[0] / rst[3], rst[1] / rst[3], rst[2] / rst[3]);
}

MyMatrixf MyMathHelper::InverseMatrix4x4ColMaj(const MyMatrixf& mat){
	float inverse[16];
	MyMatrixf rmjMat = mat.Transpose();
	InvertMatrix4x4ColMaj(rmjMat.GetData(), inverse);
	return MyMatrixf(inverse, 4, 4).Transpose();
}