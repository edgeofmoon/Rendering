#pragma once

#include "MyBoundingObject.h"
#include <vector>

#include "MyArray.h"

class MyBoundingBox
	: public MyBoundingObject
{
public:
	MyBoundingBox(void);
	MyBoundingBox(const MyVec3f& low, const MyVec3f& high);
	MyBoundingBox(const MyVec3f& center, float size);
	MyBoundingBox(const MyVec3f& center, float width, float height, float depth = 0.f);
	~MyBoundingBox(void);

	virtual bool IsIn(const MyVec3f& pos) const;
	virtual void Engulf(const MyVec3f& pos);
	virtual bool IsIntersected(const MyVec3f& st, const MyVec3f& ed) const;
	virtual void Translate(const MyVec3f& t);
	virtual void Expand(float e);
	virtual void SetNull();
	virtual MyBoundingObject* MakeCopy() const;
	virtual MyVec3f GetCenter() const;

	bool RayHit(const MyVec3f& source, const MyVec3f& dir) const;
	const MyVec3f& operator[](int i) const;
	float GetRange(int dim) const;
	float GetLeft() const;
	float GetRight() const;
	float GetTop() const;
	float GetBottom() const;
	float GetNear() const;
	float GetFar() const;
	float GetWidth() const;
	float GetHeight() const;
	float GetDepth() const;
	float GetVolume() const;
	float GetFrontFaceArea() const;
/*
                6---7
        2---3   |	|
        |   |   |	|
        |   |   4---5
        0---1
*/
	MyVec3f GetCornerPos(int i) const;
	MyVec3f GetLowPos() const;
	MyVec3f GetHighPos() const;
	MyVec3f GetRandomPos() const;

	MyArray3f* MakeRandomPositions(int n) const;

	void Show();
	void Engulf(const MyBoundingBox& box);
	void Reset();
	void Scale(float sc);
	void expand(float amount, int dim);
	void expandHigh(float amount, int dim);
	void expandLow(float amount, int dim);
	void SquashDimension(int dim = 2);

	MyVec3f BoundPoint(const MyVec3f& pos) const;
	void MapPoints(const std::vector<MyVec3f>& oldPoints,
		std::vector<MyVec3f>& newPoints, const MyBoundingBox& oldBox) const;

	static MyVec4i GetFaceIndexSet(int i);
protected:
	MyVec3f mLowPos, mHighPos;
};

