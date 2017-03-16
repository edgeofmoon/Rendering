#pragma once
#include "MyTracks.h"
class MyTrackRings :
	public MyTracks
{
public:
	MyTrackRings();
	~MyTrackRings();

	virtual void LoadShader();
	virtual void ComputeGeometry();
	float mRingRadius;
	virtual void Show();
	virtual void LoadGeometry();

protected:
	int mOrbitAttribute;
	unsigned int mOrbitBuffer;

	virtual void ComputeTubeGeometry();
	int mRingFaces;


	MyArray3f mOrbits;
};

