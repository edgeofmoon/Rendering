#pragma once

#include "MyTracks.h"

class MyTrackDDH :
	public MyTracks
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

