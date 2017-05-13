#pragma once

#include "MyVec.h"

class MyBoundingObject
{
public:
	MyBoundingObject(){};
	~MyBoundingObject(){};

	virtual bool IsIn(const MyVec3f& p) const = 0;
	virtual bool IsIntersected(const MyVec3f& st, const MyVec3f& ed) const = 0;
	virtual void Engulf(const MyVec3f& p) = 0;
	virtual void Translate(const MyVec3f& t) = 0;
	virtual void Expand(float e) = 0;
	virtual MyBoundingObject* MakeCopy() const = 0;
	virtual MyVec3f GetCenter() const = 0;
};

