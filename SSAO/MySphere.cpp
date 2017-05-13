#include "MySphere.h"
#include "MyMathHelper.h"
#include <algorithm>
using namespace std;

MySphere::MySphere(){
}

MySphere::MySphere(const MyVec3f& c, float r)
	:mCenter(c), mRadius(r){
}

MySphere::~MySphere()
{
}

bool MySphere::IsIn(const MyVec3f& p) const{
	return GetCenterDistance(p) <= mRadius;
}

void MySphere::Engulf(const MyVec3f& p){
	float r = GetCenterDistance(p);
	mRadius = max(r, mRadius);
}

float MySphere::GetCenterDistance(const MyVec3f& p) const{
	MyVec3f diff = mCenter - p;
	float dist = diff.norm();
	return dist;
}

bool MySphere::IsIntersected(const MyVec3f& st, const MyVec3f& ed) const{
	float dist = MyMathHelper::PointToLineSegmentDistance(mCenter, st, ed);
	return dist <= mRadius;
}

void MySphere::Translate(const MyVec3f& t){
	mCenter += t;
}

void MySphere::Expand(float e){
	mRadius = max(0.f, mRadius + e);
}

MyBoundingObject* MySphere::MakeCopy() const{
	return new MySphere(mCenter, mRadius);
}

float MySphere::GetSurfaceDistance(const MyVec3f& p) const{
	return GetCenterDistance(p) - mRadius;
}