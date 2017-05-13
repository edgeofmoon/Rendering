#pragma once
#include "MySphereGeometry.h"
class MySphereDDHGeometry :
	public MySphereGeometry
{
public:
	MySphereDDHGeometry();
	~MySphereDDHGeometry();

	virtual int GetVertexCount() {
		return (mMeshDetailX + 1)*(mMeshDetailY + 1)
			+ mMeshDetailX + 1;
	};
	virtual int GetTriangleCount(){
		return mMeshDetailX*(mMeshDetailY - 1) * 2
			+ mMeshDetailX;
	};

	virtual void GenerateGeometry();
	virtual void DrawGeometry();

	void CompileShaders();
	void SetHaloColor(const MyColor4f& c){ mHaloColor = c; };

protected:

	unsigned int mBasicShader;
	unsigned int mDDHShader;
	unsigned int mDDHVertexArray;
	unsigned int mDDHVertexBuffer;
	unsigned int mDDHAngleBuffer;
	unsigned int mDDHIndexBuffer;

	MyColor4f mHaloColor;

};

