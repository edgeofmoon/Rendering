#pragma once
#include "MyUIObject.h"
class MySlider :
	public MyUIObject
{
public:
	MySlider();
	~MySlider();

	virtual void Show();
	virtual bool IsIn(float x, float y) const;
	virtual int HandleMouseDown(float x, float y);
	virtual int HandleMouseUp(float x, float y);
	virtual int HandleMouseMove(float x, float y);
	//virtual int HandleKey(unsigned char key, float x, float y);

	inline MyVec2f GetCenter() const { return mBottomLeft + (mSize * 0.5f); };
	inline MyVec2f GetPosition() const { return mBottomLeft; };
	inline MyVec2f GetSize() const { return mSize; };
	inline MyString GetText() const { return mText; };

	inline void SetPosition(const MyVec2f& bl){ mBottomLeft = bl; };
	inline void SetSize(const MyVec2f& sz){ mSize = sz; };
	inline void SetText(const MyString& str){ mText = str; };

	inline void SetRange(const MyVec2f& vlow, const MyVec2f& vhigh){ 
		mValueLow = vlow; mValueHigh = vhigh; };
	inline void SetRange(float low, float high){
		mValueLow = MyVec2f(low, low); mValueHigh = MyVec2f(high, high); };
	inline MyVec2f GetValue() const { return mValue; };
	inline void SetValue(const MyVec2f& v){ mValue = v; };
	inline void SetValue(float v){ mValue = MyVec2f(v, v); };

	inline void SetTriangleHandle(const MyVec2f& p0, const MyVec2f& p1,
		const MyVec2f& p2){
		mHandleTriangle[0] = p0;
		mHandleTriangle[1] = p1;
		mHandleTriangle[2] = p2;
	}

	inline bool IsDown() const { return HasStatusBit(MyUI::DOWN_BIT); };
	inline bool IsHover() const { return HasStatusBit(MyUI::INSIDE_BIT); };

protected:
	MyString mText;
	MyVec2f mBottomLeft;
	MyVec2f mSize;

	MyVec2f mValueLow;
	MyVec2f mValueHigh;
	MyVec2f mValue;

	MyVec2f GetValueFromPos(MyVec2f pos) const;

	MyVec2f mHandleTriangle[3];
	bool within(float x) const{
		return 0 <= x && x <= 1;
	}
};

