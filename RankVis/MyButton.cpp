#include "MyButton.h"
#include "MyPrimitiveDrawer.h"

using namespace MyUI;

MyButton::MyButton()
	:MyUIObject(){
}


MyButton::~MyButton(){
}

bool MyButton::IsIn(float x, float y) const{
	return x >= mBottomLeft[0] && x <= mBottomLeft[0] + mSize[0]
		&& y >= mBottomLeft[1] && y <= mBottomLeft[1] + mSize[1];
}

void MyButton::Show(){
	if (HasStatusBit(HIDE_BIT)) return;
	MyArray3f vertices = { mBottomLeft.toDim<3>(-mDepth),
		(mBottomLeft + MyVec2f(mSize[0], 0)).toDim<3>(-mDepth),
		(mBottomLeft + mSize).toDim<3>(-mDepth),
		(mBottomLeft + MyVec2f(0, mSize[1])).toDim<3>(-mDepth),};
	MyPrimitiveDrawer::PushAllAttributes();


	// text
	if (!HasStatusBit(ENABLE_BIT)) MyPrimitiveDrawer::Color(MyColor4f(0.5, 0.5, 0.5, 1));
	else if (!HasStatusBit(DOWN_BIT)) MyPrimitiveDrawer::Color(MyColor4f(0, 0, 0, 1));
	else MyPrimitiveDrawer::Color(MyColor4f(1, 1, 1, 1));
	//float width = MyPrimitiveDrawer::GetStrokeTextWidth(mText);
	//float scale = mSize[0] / width * 3;
	//MyPrimitiveDrawer::DrawStrokeText(mBottomLeft.toDim<3>(-mDepth), mText, MyVec3f(scale, scale, 1));
	MyPrimitiveDrawer::DrawBitMapTextLarge(GetCenter().toDim<3>(-mDepth), mText, 1);

	// inside
	if (HasStatusBit(DOWN_BIT)){
		MyPrimitiveDrawer::FillPolygon();
		MyPrimitiveDrawer::Color(MyColor4f(0, 0, 0, 1));
		MyPrimitiveDrawer::DrawQuadsAt(vertices);
	}
	// boarder
	MyPrimitiveDrawer::WirePolygon();
	if (!HasStatusBit(ENABLE_BIT)) MyPrimitiveDrawer::Color(MyColor4f(0.5, 0.5, 0.5, 1));
	else if (!HasStatusBit(INSIDE_BIT)) MyPrimitiveDrawer::Color(MyColor4f(0, 0, 0, 1)); 
	else MyPrimitiveDrawer::Color(MyColor4f(1, 1, 0, 1));
	MyPrimitiveDrawer::DrawQuadsAt(vertices);
	MyPrimitiveDrawer::PopAttributes();
}
