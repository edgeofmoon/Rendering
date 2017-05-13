#pragma once
#include "MyTractVisBase.h"

class MyTrackCurv :
	public MyTractVisBase
{
public:
	MyTrackCurv();
	~MyTrackCurv();

	virtual void LoadShader();
	virtual void ComputeGeometry();
	virtual void Show();
	virtual void LoadGeometry();

protected:
	MyArray3f mTextureCoords;
	unsigned int mTextureCoordBuffer;
	int mTextureCoordAttribute;

	MyArrayf mCurvatures;
	unsigned int mCurvatureBuffer;
	int mCurvatureAttribute;

	MyArray3f mTangents;
	unsigned int mTangentBuffer;
	int mTangentAttribute;
};

