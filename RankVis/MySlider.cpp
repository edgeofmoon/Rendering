#include "MySlider.h"

using namespace MyUI;

MySlider::MySlider()
{
	mValueLow = MyVec2f::zero();
	mValueHigh = MyVec2f(1, 1);
}


MySlider::~MySlider()
{
}

bool MySlider::IsIn(float x, float y) const{
	const MyVec2f& A = mHandleTriangle[0];
	const MyVec2f& B = mHandleTriangle[1];
	const MyVec2f& C = mHandleTriangle[2];
	MyVec2f P(x, y);
	float alpha, beta, gamma;
	double det = (B[1] - C[1])*(A[0] - C[0]) + (C[0] - B[0])*(A[1] - C[1]);
	double factor_alpha = (B[1] - C[1])*(P[0] - C[0]) + (C[0] - B[0])*(P[1] - C[1]);
	double factor_beta = (C[1] - A[1])*(P[0] - C[0]) + (A[0] - C[0])*(P[1] - C[1]);
	alpha = factor_alpha / det;
	beta = factor_beta / det;
	gamma = 1.0 - alpha - beta;
	return P == A || P == B || P == C || (within(alpha) && within(beta) && within(gamma));

	//return x >= mBottomLeft[0] && x <= mBottomLeft[0] + mSize[0]
	//	&& y >= mBottomLeft[1] && y <= mBottomLeft[1] + mSize[1];
}

void MySlider::Show(){
}

int MySlider::HandleMouseDown(float x, float y){
	if (!HasStatusBit(ENABLE_BIT)) return 0;
	if (IsIn(x, y)){
		mValue = GetValueFromPos(MyVec2f(x, y));
		SetStatusBit(DOWN_BIT);
		return 1;
	}
	return 0;
}

int MySlider::HandleMouseUp(float x, float y){
	ClearStatusBit(DOWN_BIT);
	return 0;
}

int MySlider::HandleMouseMove(float x, float y){
	int h = MyUIObject::HandleMouseMove(x, y);
	if (HasStatusBit(DOWN_BIT)){
		mValue = GetValueFromPos(MyVec2f(x, y));
		return 1;
	}
	return h;
}


MyVec2f MySlider::GetValueFromPos(MyVec2f pos) const{
	MyVec2f p = pos - mBottomLeft;
	float x = p[0] / mSize[0];
	float y = p[1] / mSize[1];
	if (x < 0) x = 0;
	if (x > 1) x = 1;
	if (y < 0) y = 0;
	if (y > 1) y = 1;
	x = x * (mValueHigh[0] - mValueLow[0]) + mValueLow[0];
	y = y * (mValueHigh[1] - mValueLow[1]) + mValueLow[1];
	return MyVec2f(x, y);
}