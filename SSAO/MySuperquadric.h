#pragma once

#include "MyVec.h"
#include "MyArray.h"
#include "MyTensor.h"

class MySuperquadric
{
public:
	MySuperquadric();
	~MySuperquadric();

	// input parameters
	// static
	inline static void SetGamma(float g){ Gamma = g; };
	inline static float GetGamma() { return Gamma; };
	inline static int GetVertexCount() {
		return (MeshDetailX + 1)*(MeshDetailY + 1); };
	inline static int GetTriangleCount(){
		return MeshDetailX*(MeshDetailY-1) * 2;
	};

	inline static void SetMeshDetail(int dx, int dy){ 
		MeshDetailX = dx; MeshDetailY = dy; };

	inline void SetCenter(const MyVec3f& c){ mCenter = c; };
	inline const MyVec3f& GetCenter() const { return mCenter; };
	inline void SetTensor(const MyTensor3f* tensor){ mTensor = tensor; };
	inline const MyTensor3f* GetTensor() const { return mTensor; };
	inline void SetScale(float s) { mScale = s; };
	inline float GetScale() const { return mScale; };

	void Build();
	void BuildOn(MyArray3f& vertices, MyArray3f& normals, 
		MyArray2f& texCoords, MyArray3i& triangles, 
		int vIdxOffset = 0, int tIdxOffset = 0) const;

protected:
	// static members
	static float Gamma;
	static int MeshDetailX, MeshDetailY;

	// input
	float mScale;
	MyVec3f mCenter;
	const MyTensor3f* mTensor;

	// output
	MyArray3f mVertices;
	MyArray3f mNormals;
	MyArray2f mTexCoords;
	MyArray3i mIndices;

	MyVec3f TransformCanonicalPoints(const MyVec3f& p) const;

	bool ComputeTriangleDirection(const MyVec3f& p0, 
		const MyVec3f& p1, const MyVec3f& p2) const;

private:
	static MyVec3f GetOneVertexOnCanonicalSurfaceAroundZAxis(
		float alpha, float beta, float theta, float phi);
	static MyVec3f GetOneVertexOnCanonicalSurfaceAroundXAxis(
		float alpha, float beta, float theta, float phi);
	inline static float Clamp(float f, float lo = 0.f, float hi = 1.f);
};

