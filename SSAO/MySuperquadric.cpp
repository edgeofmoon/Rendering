#include "MySuperquadric.h"

#include <cmath>
#include <algorithm>
#include <cassert>
using namespace std;

float MySuperquadric::Gamma = 3.f;
int MySuperquadric::MeshDetailX = 10;
int MySuperquadric::MeshDetailY = 10;

#define isignf(x) (x<0?-1:1)
#define PI 3.141592653589793238462643

MySuperquadric::MySuperquadric():
	mCenter(0,0,0){
	mTensor = NULL;
	mScale = 1;
}

MySuperquadric::~MySuperquadric()
{
}

void MySuperquadric::Build(){
	mVertices.clear();
	mNormals.clear();
	mTexCoords.clear();
	mIndices.clear();
	BuildOn(mVertices, mNormals, mTexCoords, mIndices);
}

void MySuperquadric::BuildOn(MyArray3f& vertices, MyArray3f& normals,
	MyArray2f& texCoords, MyArray3i& triangles, int idxOffset, int tIdxOffset) const{

	int vSize = idxOffset + GetVertexCount();
	int tSize = tIdxOffset + GetTriangleCount();
	if (vertices.size() < vSize) vertices.resize(vSize);
	if (normals.size() < vSize) normals.resize(vSize);
	if (texCoords.size() < vSize) texCoords.resize(vSize);
	if (triangles.size() < tSize) triangles.resize(tSize);

	float cl = mTensor->GetLinearAnisotropy();
	float cp = mTensor->GetPlanarAnisotropy();
	float alpha = powf(Clamp(1 - cl), Gamma);
	float beta = powf(Clamp(1 - cp), Gamma);
	MyVec3f(*surfacePoint)(float, float, float, float);
	if (cl > cp) {
		std::swap(alpha, beta);
		surfacePoint = &GetOneVertexOnCanonicalSurfaceAroundXAxis;
	}
	else surfacePoint = &GetOneVertexOnCanonicalSurfaceAroundZAxis;

	for (int h = 0, idx = idxOffset; h < MeshDetailX; h++){
		float angleX = h * PI * 2.f / MeshDetailX;
		for (int v = 0; v <= MeshDetailY; v++, idx++){
			float angleY = v * PI / MeshDetailY;
			MyVec3f p = surfacePoint(alpha, beta, angleX, angleY);
			vertices[idx] = TransformCanonicalPoints(p);
			texCoords[idx] = MyVec2f((float)h / MeshDetailX, (float)v / MeshDetailY);
		}
	}
	memcpy(&vertices[MeshDetailX*(MeshDetailY + 1) + idxOffset][0], 
		&vertices[idxOffset][0], (MeshDetailY + 1)*sizeof(MyVec3f));
	for (int v = 0, idx = MeshDetailX*(MeshDetailY + 1) + idxOffset; v <= MeshDetailY; v++, idx++){
		//vertices[idx] = vertices[idxOffset + v];
		texCoords[idx] = MyVec2f(1.f, (float)v / MeshDetailY);
	}

	// indices
	// determin triangle direction
	bool triangleDirection = ComputeTriangleDirection(vertices[idxOffset], 
		vertices[idxOffset+MeshDetailY + 2], vertices[idxOffset+1]);
	if (triangleDirection){
		for (int h = 0, tidx = tIdxOffset; h < MeshDetailX; h++){
			for (int v = 0; v < MeshDetailY; v++){
				int idx = h*(MeshDetailY + 1) + v + idxOffset;
				if (v != 0) triangles[tidx++] = MyVec3i(idx, idx + MeshDetailY + 1, idx + MeshDetailY + 2);
				if (v != MeshDetailY - 1) triangles[tidx++] = MyVec3i(idx, idx + MeshDetailY + 2, idx + 1);
			}
		}
	}
	else{
		for (int h = 0, tidx = tIdxOffset; h < MeshDetailX; h++){
			for (int v = 0; v < MeshDetailY; v++){
				int idx = h*(MeshDetailY + 1) + v + idxOffset;
				if (v != 0) triangles[tidx++] = MyVec3i(idx, idx + MeshDetailY + 2, idx + MeshDetailY + 1);
				if (v != MeshDetailY - 1) triangles[tidx++] = MyVec3i(idx, idx + 1, idx + MeshDetailY + 2);
			}
		}
	}

	// normal
	memset(&normals[idxOffset][0], 0, GetVertexCount()*sizeof(MyVec3f));
	for (int i = 0; i < GetTriangleCount(); i++){
		const MyVec3i& tri = triangles[i + tIdxOffset];
		const MyVec3f& p0 = vertices[tri[0]];
		const MyVec3f& p1 = vertices[tri[1]];
		const MyVec3f& p2 = vertices[tri[2]];
		MyVec3f normal = (p1 - p0) ^ (p2 - p0);
		normals[tri[0]] += normal;
		normals[tri[1]] += normal;
		normals[tri[2]] += normal;
	}
	// process the first and last column
	for (int i = 0; i <= MeshDetailY; i++){
		MyVec3f normal = normals[i + idxOffset];
		normals[i + idxOffset] += normals[i + idxOffset + MeshDetailX*(MeshDetailY + 1)];
		normals[i + idxOffset + MeshDetailX*(MeshDetailY + 1)] = normals[i + idxOffset];
	}
	for (int i = 0; i < GetVertexCount(); i++){
		normals[i+idxOffset].normalize();
	}
	// pole points may need more process
}

MyVec3f MySuperquadric::TransformCanonicalPoints(const MyVec3f& p) const{
	const MyVec3f* eigenVectors = mTensor->GetEigenVectors();
	MyVec3f sp = p;
	sp.scale(MyVec3f(mTensor->GetEigenValues()).normalized());
	MyVec3f rp;
	rp[0] = sp[0] * eigenVectors[0][0] + sp[1] * eigenVectors[1][0] + sp[2] * eigenVectors[2][0];
	rp[1] = sp[0] * eigenVectors[0][1] + sp[1] * eigenVectors[1][1] + sp[2] * eigenVectors[2][1];
	rp[2] = sp[0] * eigenVectors[0][2] + sp[1] * eigenVectors[1][2] + sp[2] * eigenVectors[2][2];

	// reverse Z because coordinates z is 
	// somehow reversed in the data file
	rp[2] *= -1;

	rp *= mScale;
	rp += mCenter;

	return rp;
}

bool MySuperquadric::ComputeTriangleDirection(const MyVec3f& p0, 
	const MyVec3f& p1, const MyVec3f& p2) const{
	MyVec3f normal = (p1 - p0) ^ (p2 - p0);
	MyVec3f outwards = p0 - mCenter;
	return normal*outwards > 0;
}

MyVec3f MySuperquadric::GetOneVertexOnCanonicalSurfaceAroundZAxis(
	float alpha, float beta, float theta, float phi){
	assert(alpha >= beta);
	float ct1 = cos(phi);
	float ct2 = cos(theta);
	float st1 = sin(phi);
	float st2 = sin(theta);
	MyVec3f p;
	p[0] = isignf(ct2)*isignf(st1)*powf(fabs(ct2), alpha)*powf(fabs(st1), beta);
	p[1] = isignf(st2)*isignf(st1)*powf(fabs(st2), alpha)*powf(fabs(st1), beta);
	p[2] = isignf(ct1)*powf(fabs(ct1), beta);
	return p;
}

MyVec3f MySuperquadric::GetOneVertexOnCanonicalSurfaceAroundXAxis(
	float alpha, float beta, float theta, float phi){
	assert(alpha >= beta);
	float ct1 = cos(phi);
	float ct2 = cos(theta);
	float st1 = sin(phi);
	float st2 = sin(theta);
	MyVec3f p;
	p[0] = isignf(ct1)*powf(fabs(ct1), beta);
	p[1] = -isignf(st2)*isignf(st1)*powf(fabs(st2), alpha)*powf(fabs(st1), beta);
	p[2] = isignf(ct2)*isignf(st1)*powf(fabs(ct2), alpha)*powf(fabs(st1), beta);
	return p;
}

float MySuperquadric::Clamp(float f, float lo, float hi){
	return f > hi ? hi : std::max(lo, f);
}