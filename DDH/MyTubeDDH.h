#pragma once
#include "MyTractVisBase.h"
class MyTubeDDH :
	public MyTractVisBase
{
public:
	MyTubeDDH();
	~MyTubeDDH();

	float mDepthCueing;
	virtual void LoadShader();
	//virtual void ComputeGeometry();
	virtual void Show();
	virtual void LoadGeometry();
};

