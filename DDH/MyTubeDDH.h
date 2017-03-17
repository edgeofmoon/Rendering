#pragma once
#include "MyTracks.h"
class MyTubeDDH :
	public MyTracks
{
public:
	MyTubeDDH();
	~MyTubeDDH();

	float mDepthCueing;
	virtual void Show();
};

