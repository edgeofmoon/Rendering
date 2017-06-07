#pragma once

#include "MyTractVisBase.h"

class MyTractVisBaseLegend
	: public MyTractVisBase
{
public:
	MyTractVisBaseLegend();
	~MyTractVisBaseLegend();

	MyVec3f GetBase() const { return mBase; };
	MyVec3f GetDir() const { return mDir; };
	int GetNumSamples() const{ return mNumberSamples; };
	virtual void ComputeGeometry();
	virtual void Show();

protected:
	int mNumberSamples;
	MyVec3f mDir;
	MyVec3f mBase;
};

