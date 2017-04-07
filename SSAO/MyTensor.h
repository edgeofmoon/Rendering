#pragma once

#include "MyVec.h"

template <typename T>
class MyTensor3
{
public:
	MyTensor3(){};
	~MyTensor3(){};

	inline void SetEigenValues(const T* vals){
		mEigenValues[0] = vals[0];
		mEigenValues[1] = vals[1];
		mEigenValues[2] = vals[2];
	};

	inline void SetEigenVectors(const MyVec<T, 3>* vecs){
		mEigenVectors[0] = vecs[0];
		mEigenVectors[1] = vecs[1];
		mEigenVectors[2] = vecs[2];
	};

	inline void SetEigenVectors(const T* v){
		mEigenVectors[0] = MyVec<T, 3>(v[0], v[1], v[2]);
		mEigenVectors[1] = MyVec<T, 3>(v[3], v[4], v[5]);
		mEigenVectors[2] = MyVec<T, 3>(v[6], v[7], v[8]);
	};

	void NormalizeEigenVectors() {
		mEigenVectors[0].normalize();
		mEigenVectors[1].normalize();
		mEigenVectors[2].normalize();
	}

	inline const MyVec<T, 3>& GetEigenVector(int idx) const { return mEigenVectors[idx]; };
	inline const MyVec<T, 3>* GetEigenVectors() const { return mEigenVectors; };

	inline T GetEigenValue(int idx) const { return mEigenValues[idx]; };
	inline const T* GetEigenValues() const { return mEigenValues; };

	inline T GetEigenValueSum() const { 
		return mEigenValues[0] + mEigenValues[1] + mEigenValues[2]; };

	inline T GetLinearAnisotropy() const {
		return (mEigenValues[0] - mEigenValues[1]) / GetEigenValueSum();
	}

	inline T GetPlanarAnisotropy() const {
		return T(2) * (mEigenValues[1] - mEigenValues[2]) / GetEigenValueSum();
	}

	inline T GetSphericalAnisotropy() const{
		return T(3) * mEigenValues[2] / GetEigenValueSum();
	}

	inline T GetFA() const{
		T d1 = mEigenValues[0] - mEigenValues[1];
		T d2 = mEigenValues[1] - mEigenValues[2];
		T d3 = mEigenValues[2] - mEigenValues[0];
		T sqardRt = (d1*d1 + d2*d2 + d3*d3) / T(2) 
			/ (mEigenValues[0] * mEigenValues[0] 
			+ mEigenValues[1] * mEigenValues[1] + mEigenValues[2] * mEigenValues[2]);
		return sqrt(sqardRt);
	}

	inline bool CheckEigenValueOrder() const {
		return mEigenValues[0] >= mEigenValues[1]
			&& mEigenValues[1] >= mEigenValues[2]
			&& mEigenValues[2] >= 0;
	}

protected:
	float mEigenValues[3];
	MyVec<T, 3> mEigenVectors[3];
};

typedef MyTensor3<float> MyTensor3f;
