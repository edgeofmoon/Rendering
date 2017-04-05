#pragma once

#include "MyTractVisBase.h"

class MyTrackDDH :
	public MyTractVisBase
{
public:
	MyTrackDDH();
	~MyTrackDDH();


	virtual void LoadShader();
	virtual void ComputeGeometry();
	virtual void Show();
	virtual void LoadGeometry();

	float mStripWidth;
	float mStripDepth;
	float mStrokeWidth;
	float mTaperLength;
	float mDepthCueing;

protected:
	MyArray3f mTextureCoords;
	unsigned int mTextureCoordBuffer;
	int mTextureCoordAttribute;

	MyArray3f mTangents;
	unsigned int mTangentBuffer;
	int mTangentAttribute;

};

