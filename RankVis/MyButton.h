#pragma once

#include "MyVec.h"
#include "MyString.h"
#include "MyUIObject.h"

class MyButton
	: public MyUIObject
{
public:
	MyButton();
	~MyButton();

	virtual void Show();
	virtual bool IsIn(float x, float y) const;
	//virtual int HandleMouseDown(float x, float y);
	//virtual int HandleMouseUp(float x, float y);
	//virtual int HandleMouseMove(float x, float y);
	//virtual int HandleKey(unsigned char key, float x, float y);

	inline MyVec2f GetCenter() const { return mBottomLeft + (mSize * 0.5f); };
	inline MyVec2f GetPosition() const { return mBottomLeft; };
	inline MyVec2f GetSize() const { return mSize; };
	inline MyString GetText() const { return mText; };

	inline void SetPosition(const MyVec2f& bl){ mBottomLeft = bl; };
	inline void SetSize(const MyVec2f& sz){ mSize = sz; };
	inline void SetText(const MyString& str){ mText = str; };

protected:
	MyString mText;
	MyVec2f mBottomLeft;
	MyVec2f mSize;
};