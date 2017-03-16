#pragma once

#include "MyArray.h"
#include "MyBoundingBox.h"
#include "MyColor4.h"
#include "MyMatrix.h"
#include <string>

class MyMesh
{
public:
	MyMesh();
	~MyMesh();

	int Read(const MyString& fileName);
	int Write(const MyString& fileName);
	int ReadAsc(const MyString& fileName);
	int WriteAsc(const MyString& fileName);
	int WriteOFF(const MyString& fileName);
	int WriteNOFF(const MyString& fileName);
	int WriteSMFD(const MyString& fileName);
	int WritePLY(const MyString& fileName);
	int WriteBIN(const MyString& fileName);

	void CompileShader();
	void Build();
	void Render();
	void Destory();

	void ApplyTransform(const MyMatrixf& mat);
	int GenPerVertexNormal();

	int Merge(const MyMesh& mesh);
	int MergeVertices(float prec = 0.01);
	int RemoveVertex(const MyArrayi& vertices);

	MyVec3f GetVertex(int idx) const;
	MyVec3f GetNormal(int idx) const;
	MyVec3i GetTriangle(int idx) const;
	MyBoundingBox GetBoundingBox() const;

	int GetNumVertex() const;
	int GetNumNormal() const;
	int GetNumTriangle() const;

	const float* GetVertexData() const;
	const float* GetNormalData() const;
	const int* GetTriangleData() const;

	MyVec3f ComputeTriangleNormal(const MyVec3i& triangle) const;
	float ComputeTriangleArea(const MyVec3i& triangle) const;

	int ClearNonRegularFaces();

	unsigned int mTexture;

protected:
	MyArray3f mVertices;
	MyArray3f mNormals;
	MyArray3i mTriangles;
	MyArray<MyColor4f> mColors;
	MyBoundingBox mBox;

	static MyVec2i makeEdge(int i, int j);

	static bool compareEdge(MyVec2i edge0, MyVec2i edge1);

	unsigned int mShaderProgram;
	unsigned int mVertexArray;
	unsigned int mVertexBuffer;
	unsigned int mNormalBuffer;
	unsigned int mColorBuffer;
	unsigned int mIndexBuffer;

	int mNormalAttribute;
	int mPositionAttribute;
	int mColorAttribute;

	static std::string vertex2string(const MyVec3f& vec, float precision = 0.01);
};

