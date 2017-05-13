#pragma once

#include "MySphere.h"
#include "MyColor4.h"

class MySphereGeometry
{
public:
	MySphereGeometry();
	~MySphereGeometry();

	virtual int GetVertexCount() {
		return (mMeshDetailX + 1)*(mMeshDetailY + 1);
	};
	virtual int GetTriangleCount(){
		return mMeshDetailX*(mMeshDetailY - 1) * 2;
	};

	virtual void GenerateGeometry();
	virtual void DrawGeometry();

	void SetColor(const MyColor4f& c){ mColor = c; };
	MyColor4f GetColor() const { return mColor; };
	void SetShaderProgram(unsigned int sp){ mShaderProgram = sp; };
	void SetSphere(const MySphere* sp){ mSphere = sp; };
	const MySphere* GetSphere() const { return mSphere; };

protected:
	// input data
	const MySphere* mSphere;
	MyColor4f mColor;
	unsigned int mShaderProgram;
	int mMeshDetailX, mMeshDetailY;

	// geometry on GPU
	unsigned int mVertexArray;
	unsigned int mVertexBuffer;
	unsigned int mNormalBuffer;
	unsigned int mTexCoordBuffer;
	unsigned int mColorBuffer;
	unsigned int mValueBuffer;
	unsigned int mIndexBuffer;
};

