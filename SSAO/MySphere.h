#pragma once

#include "MyBoundingObject.h"

class MySphere
	: public MyBoundingObject
{
public:
	MySphere();
	MySphere(const MyVec3f& c, float r);
	~MySphere();

	virtual bool IsIn(const MyVec3f& p) const;
	virtual void Engulf(const MyVec3f& p);
	virtual bool IsIntersected(const MyVec3f& st, const MyVec3f& ed) const;
	virtual void Translate(const MyVec3f& t);
	virtual void Expand(float e);
	virtual MyBoundingObject* MakeCopy() const;
	virtual MyVec3f GetCenter() const { return mCenter; };

	void SetCenter(const MyVec3f& c){ mCenter = c; };

	void SetRadius(float r){ mRadius = r; };
	float GetRadius() const { return mRadius; };

	float GetCenterDistance(const MyVec3f& p) const;
	float GetSurfaceDistance(const MyVec3f& p) const;

protected:
	MyVec3f mCenter;
	float mRadius;
};

