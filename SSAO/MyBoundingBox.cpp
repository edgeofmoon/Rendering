#include "MyBoundingBox.h"
#include "MyPrimitiveDrawer.h"
#include <algorithm>
#include <cmath>
using namespace std;

MyBoundingBox::MyBoundingBox(void)
	:mLowPos(0.f, 0.f, 0.f), mHighPos(0.f, 0.f, 0.f)
{
}


MyBoundingBox::MyBoundingBox(const MyVec3f& low,const MyVec3f& high){
	mLowPos = low;
	mHighPos = high;
}
MyBoundingBox::~MyBoundingBox(void)
{
}

MyBoundingBox::MyBoundingBox(const MyVec3f& center, float size){
	MyVec3f offset(size / 2, size / 2, size / 2);
	mLowPos = center - offset;
	mHighPos = center + offset;
}

MyBoundingBox::MyBoundingBox(const MyVec3f& center, float width, float height, float depth){
	MyVec3f offset(width/2, height/2, depth/2);
	mLowPos = center - offset;
	mHighPos = center + offset;
}

bool MyBoundingBox::IsIn(const MyVec3f& pos) const{
	return pos >= mLowPos && pos <= mHighPos;
}


void MyBoundingBox::Engulf(const MyVec3f& pos){
	if(pos[0]<mLowPos[0]) mLowPos[0] = pos[0];
	if(pos[1]<mLowPos[1]) mLowPos[1] = pos[1];
	if(pos[2]<mLowPos[2]) mLowPos[2] = pos[2];
	if(pos[0]>mHighPos[0]) mHighPos[0] = pos[0];
	if(pos[1]>mHighPos[1]) mHighPos[1] = pos[1];
	if(pos[2]>mHighPos[2]) mHighPos[2] = pos[2];
}

bool MyBoundingBox::IsIntersected(const MyVec3f& st, const MyVec3f& ed) const{
	// Get line midpoint and extent
	MyVec3f lst = st - GetCenter();
	MyVec3f led = ed - GetCenter();
	MyVec3f size(GetRange(0), GetRange(1), GetRange(2));
	MyVec3f LMid = (lst + led) * 0.5f;
	MyVec3f L = (lst - LMid);
	MyVec3f LExt(fabs(L[0]), fabs(L[1]), fabs(L[2]));

	// Use Separating Axis Test
	// Separation vector from box center to line center is LMid, since the line is in box space
	if (fabs(LMid[0]) > size[0] + LExt[0]) return false;
	if (fabs(LMid[1]) > size[1] + LExt[1]) return false;
	if (fabs(LMid[2]) > size[2] + LExt[2]) return false;
	// Crossproducts of line and each axis
	if (fabs(LMid[1] * L[2] - LMid[2] * L[1])  >  (size[1] * LExt[2] + size[2] * LExt[1])) return false;
	if (fabs(LMid[0] * L[2] - LMid[2] * L[0])  >  (size[0] * LExt[2] + size[2] * LExt[0])) return false;
	if (fabs(LMid[0] * L[1] - LMid[1] * L[0])  >  (size[0] * LExt[1] + size[1] * LExt[0])) return false;
	// No separating axis, the line intersects
	return true;
}

void MyBoundingBox::Translate(const MyVec3f& offset){
	mLowPos += offset;
	mHighPos += offset;
}

void MyBoundingBox::Expand(float e){
	expand(e, 0);
	expand(e, 1);
	expand(e, 2);
}

MyBoundingObject* MyBoundingBox::MakeCopy() const{
	return new MyBoundingBox(mLowPos, mHighPos);
}

void MyBoundingBox::Engulf(const MyBoundingBox& box){
	this->Engulf(box.GetLowPos());
	this->Engulf(box.GetHighPos());
}

bool MyBoundingBox::RayHit(const MyVec3f& source, const MyVec3f& dir) const{
	// not implemented
	return true;
}
void MyBoundingBox::Reset(){
	mLowPos = MyVec3f(FLT_MAX, FLT_MAX, FLT_MAX);
	mHighPos = MyVec3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}

void MyBoundingBox::Show(){
	MyVec3f vecs[8]={
		MyVec3f(mLowPos[0],	mLowPos[1],	mLowPos[2]),
		MyVec3f(mHighPos[0],	mLowPos[1],	mLowPos[2]),
		MyVec3f(mHighPos[0],	mHighPos[1],	mLowPos[2]),
		MyVec3f(mLowPos[0],	mHighPos[1],	mLowPos[2]),
		MyVec3f(mLowPos[0],	mLowPos[1],	mHighPos[2]),
		MyVec3f(mHighPos[0],	mLowPos[1],	mHighPos[2]),
		MyVec3f(mHighPos[0],	mHighPos[1],	mHighPos[2]),
		MyVec3f(mLowPos[0],	mHighPos[1],	mHighPos[2]),
	};
	std::vector<MyVec3f> quads;
	// front
	quads.push_back(vecs[0]);
	quads.push_back(vecs[1]);
	quads.push_back(vecs[2]);
	quads.push_back(vecs[3]);
	// top
	quads.push_back(vecs[3]);
	quads.push_back(vecs[2]);
	quads.push_back(vecs[6]);
	quads.push_back(vecs[7]);
	// back
	quads.push_back(vecs[7]);
	quads.push_back(vecs[6]);
	quads.push_back(vecs[5]);
	quads.push_back(vecs[4]);
	// bottom
	quads.push_back(vecs[4]);
	quads.push_back(vecs[5]);
	quads.push_back(vecs[1]);
	quads.push_back(vecs[0]);
	// left
	quads.push_back(vecs[0]);
	quads.push_back(vecs[3]);
	quads.push_back(vecs[7]);
	quads.push_back(vecs[4]);
	// right
	quads.push_back(vecs[1]);
	quads.push_back(vecs[5]);
	quads.push_back(vecs[6]);
	quads.push_back(vecs[2]);

	MyPrimitiveDrawer::DrawQuadsAt(quads);
}

const MyVec3f& MyBoundingBox::operator[](int i) const{
	return *((MyVec3f*)&mLowPos+i);
}

float MyBoundingBox::GetRange(int dim) const{
	return mHighPos[dim]-mLowPos[dim];
}

float MyBoundingBox:: GetLeft() const{
	return mLowPos[0];
}

float MyBoundingBox:: GetRight() const{
	return mHighPos[0];
}

float MyBoundingBox:: GetTop() const{
	return mHighPos[1];
}

float MyBoundingBox:: GetBottom() const{
	return mLowPos[1];
}

float MyBoundingBox:: GetNear() const{
	return mLowPos[2];
}

float MyBoundingBox:: GetFar() const{
	return mHighPos[2];
}

float MyBoundingBox::GetWidth() const{
	return mHighPos[0]-mLowPos[0];
}
float MyBoundingBox::GetHeight() const{
	return mHighPos[1]-mLowPos[1];
}
float MyBoundingBox::GetDepth() const{
	return mHighPos[2]-mLowPos[2];
}
float MyBoundingBox::GetVolume() const{
	return (this->GetWidth())*(this->GetHeight())*(this->GetDepth());
}
float MyBoundingBox::GetFrontFaceArea() const{
	return (this->GetWidth())*(this->GetHeight());
}

MyVec3f MyBoundingBox::GetCornerPos(int i) const{
	MyVec3f rst;
	rst[0] = (*this)[(i/1)%2][0];
	rst[1] = (*this)[(i/2)%2][1];
	rst[2] = (*this)[(i/4)%2][2];
	return rst;
}

MyVec3f MyBoundingBox::GetLowPos() const{
	return mLowPos;
}
MyVec3f MyBoundingBox::GetHighPos() const{
	return mHighPos;
}

MyVec3f MyBoundingBox::GetCenter() const{
	return (mLowPos + mHighPos) / 2;
}

MyVec3f MyBoundingBox::GetRandomPos() const{
	MyVec3f tmp;
	tmp[0] = mLowPos[0]+(float)(rand()%1024)/1024*(mHighPos[0]-mLowPos[0]);
	tmp[1] = mLowPos[1]+(float)(rand()%1024)/1024*(mHighPos[1]-mLowPos[1]);
	tmp[2] = mLowPos[2]+(float)(rand()%1024)/1024*(mHighPos[2]-mLowPos[2]);
	return tmp;
}

MyArray3f* MyBoundingBox::MakeRandomPositions(int n) const{
	MyArray3f* arr = new MyArray3f;
	for(int i = 0;i<n;i++){
		arr->push_back(this->GetRandomPos());
	}
	return arr;
}


void MyBoundingBox::Scale(float sc){
//	MyVec3f center = this->GetCenter();
//	mLowPos = (mLowPos-center)*sc+center;
//	mHighPos = (mHighPos-center)*sc+center;
	mLowPos *= sc;
	mHighPos *= sc;
}

void MyBoundingBox::expand(float amount, int dim){
	// incase of shrink
	if ((mHighPos[dim] - mLowPos[dim]) / 2 < -amount){
		float mid = (mHighPos[dim] / 2 + mLowPos[dim]) / 2;
		mHighPos[dim] = mid;
		mLowPos[dim] = mid;
	}
	else{
		expandHigh(amount, dim);
		expandLow(amount, dim);
	}
}

void MyBoundingBox::expandHigh(float amount, int dim){
	mHighPos[dim]+=amount;
}
void MyBoundingBox::expandLow(float amount, int dim){
	mLowPos[dim]-=amount;
}


void MyBoundingBox::SquashDimension(int dim){
	float average = (mLowPos[dim] + mHighPos[dim]) / 2;
	mLowPos[dim] = average;
	mHighPos[dim] = average;
}

MyVec3f MyBoundingBox::BoundPoint(const MyVec3f& pos) const{
	MyVec3f newPos;
	newPos[0] = std::min(mHighPos[0],std::max(mLowPos[0],pos[0]));
	newPos[1] = std::min(mHighPos[1],std::max(mLowPos[1],pos[1]));
	newPos[2] = std::min(mHighPos[2],std::max(mLowPos[2],pos[2]));
	return newPos;
}
	
void MyBoundingBox::MapPoints(const std::vector<MyVec3f>& oldPoints,
		std::vector<MyVec3f>& newPoints, const MyBoundingBox& oldBox) const{
	MyVec3f oldCenter = oldBox.GetCenter();
	MyVec3f newCenter = this->GetCenter();
	MyVec3f scaleSize;
	for(int i = 0;i<3;i++){
		if(oldBox.GetRange(i) == 0){
			scaleSize[i] = 0.f;
		}
		else{
			scaleSize[i] = this->GetRange(i)/oldBox.GetRange(i);
		}
	}
	newPoints.reserve(newPoints.size()+oldPoints.size());
	for(unsigned int i = 0;i<oldPoints.size();i++){
		MyVec3f oldPoint = oldPoints[i];
		MyVec3f toOrigin = oldPoint-oldCenter;
		toOrigin.scale(scaleSize);
		MyVec3f newPoint = toOrigin+newCenter;
		newPoints.push_back(newPoint);
	}
}

MyVec4i MyBoundingBox::GetFaceIndexSet(int i){
	const int faces[6][4] = {
		{0,1,2,3},
		{2,3,6,7},
		{6,7,4,5},
		{4,5,0,1},
		{4,0,6,2},
		{1,5,3,7},
	};
	return MyVec4i(faces[i][0],faces[i][1],faces[i][2],faces[i][3]);
}