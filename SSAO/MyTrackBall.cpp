
/*
 *  Simple trackball-like motion adapted (ripped off) from projtex.c
 *  (written by David Yu and David Blythe).  See the SIGGRAPH '96
 *  Advanced OpenGL course notes.
 */


/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
//#include <GL/glut.h>

#include "MyTrackBall.h"


MyTrackBall::MyTrackBall(void)
{
	mButtonX = 0;
	mButtonY = 0;
	mScale = 1.f;
	mAngle = 0.0;
	mRotating = false;
	/* put the identity in the trackball transform */
	mTranslateMatrix = MyMatrixf::IdentityMatrix();
	mRotateMatrix = MyMatrixf::IdentityMatrix();
	mScaleMatrix = MyMatrixf::IdentityMatrix();
	mScaleRange = MyVec2f(-FLT_MAX, FLT_MAX);
}


MyTrackBall::~MyTrackBall(void)
{
}

MyMatrixf MyTrackBall::Matrix() const
{
	return mTranslateMatrix*mRotateMatrix*mScaleMatrix;
}


void MyTrackBall::Reshape(int width, int height)
{
	mWidth = width;
	mHeight = height;
}

void MyTrackBall::StartMotion(int x, int y){
	mButtonX = (float)x;
	mButtonY = (float)y;
	pointToVector(x, y, mWidth, mHeight, mLastPos);
	mRotating = true;
}

void MyTrackBall::EndMotion(int x, int y){
	mAngle = 0.0;
	mRotating = false;
}

void MyTrackBall::RotateMotion(int x, int y)
{
	if (mRotating == false) return;

	MyVec3f current_position;
	pointToVector(x, y, mWidth, mHeight, current_position);

	/* calculate the angle to rotate by (directly proportional to the
	   length of the mouse movement */
	MyVec3f diff = current_position - mLastPos;
	mAngle = -3 * 90.0f*diff.norm();

	/* calculate the axis of rotation (cross product) */
	mAxis = mLastPos^current_position;

	/* reset for next time */
	mLastPos = current_position;

	mRotateMatrix = MyMatrixf::RotateMatrix(mAngle, mAxis[0], mAxis[1], mAxis[2])*mRotateMatrix;
}
void MyTrackBall::TranslateMotion(int x, int y){
	translate(x - mButtonX, y - mButtonY);
}

void MyTrackBall::TranslateMotionX(int x, int y){
	int dx = x - mButtonX;
	int dy = y - mButtonY;
	mButtonX = x;
	mButtonY = y;
	mTranslateMatrix = MyMatrixf::TranslateMatrix(dx / 2, -dy / 2, 0)*mTranslateMatrix;
}


void MyTrackBall::ScaleMotion(int x, int y){
	float dy = GetScaleFromMotion(x, y);
	this->ScaleAdd(dy / 20);
}

void MyTrackBall::SetScaleRange(float mis, float mas){
	mScaleRange[0] = mis;
	mScaleRange[1] = mas;
}

float MyTrackBall::GetScaleFromMotion(int x, int y) const{
	MyVec3f current_position;
	pointToVector(x, y, mWidth, mHeight, current_position);
	float dy = current_position[1] - mLastPos[1];
	return dy / 20 + 1.f;
}

void MyTrackBall::SetRotationMatrix(const MyMatrixf& mat){
	mRotateMatrix = mat;
}

void MyTrackBall::SetOrigin(const MyVec3f& origin){
	mOrigin = origin;
}

void MyTrackBall::ResetTranslate(){
	mTranslateMatrix = MyMatrixf::IdentityMatrix();
}
void MyTrackBall::ResetScale(){
	mScale = 1.f;
	mScaleMatrix = MyMatrixf::IdentityMatrix();
}
void MyTrackBall::ResetRotate(){
	SetRotationMatrix(MyMatrixf::IdentityMatrix());
}

void MyTrackBall::Translate(const MyVec3f& offset){
	mTranslateMatrix = MyMatrixf::TranslateMatrix(offset[0], offset[1], offset[2])*mTranslateMatrix;
}

void MyTrackBall::translate(float dx, float dy) {
	mButtonX += dx;
	mButtonY += dy;
	mTranslateMatrix = MyMatrixf::TranslateMatrix(dx, -dy, 0)*mTranslateMatrix;
}

void MyTrackBall::ScaleAdd(float dy)
{
	mScale += dy;
	if (mScale < mScaleRange[0]) mScale = mScaleRange[0];
	if (mScale > mScaleRange[1]) mScale = mScaleRange[1];
	mScaleMatrix = MyMatrixf::ScaleMatrix(mScale, mScale, mScale);
}

void MyTrackBall::ScaleMultiply(float dy){
	mScale *= dy;
	if (mScale < mScaleRange[0]) mScale = mScaleRange[0];
	if (mScale > mScaleRange[1]) mScale = mScaleRange[1];
	mScaleMatrix = MyMatrixf::ScaleMatrix(mScale, mScale, mScale);
}
/* functions */
void MyTrackBall::pointToVector(int x, int y, int width, int height, MyVec3f& v) const
{
	float d, a;

	/* project x, y onto a hemi-sphere centered within width, height. */
	v[0] = (2.0f * x - width) / width;
	v[1] = (height - 2.0f * y) / height;
	d = sqrt(v[0] * v[0] + v[1] * v[1]);
	v[2] = cos((3.14159265f / 2.0f) * ((d < 1.0f) ? d : 1.0f));
	a = 1.0f / v.norm();
	v *= a;
}