#pragma once

#include "MyTractVisBase.h"


class MyTractVisLabelled :
	public MyTractVisBase
{
public:
	MyTractVisLabelled();
	~MyTractVisLabelled();

	virtual void ComputeGeometry();
};

