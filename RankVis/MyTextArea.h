#pragma once
#include "MyUIObject.h"
#include "MyString.h"
#include "MyColor4.h"

class MyTextArea :
	public MyUIObject
{
public:
	MyTextArea();
	~MyTextArea();

	virtual void Show();
	virtual bool IsIn(float x, float y) const;

	inline const MyString& GetText() const { return mText; };
	inline const MyString& GetBlankText() const { return mBlankText; };
	inline int GetMaxSize() const { return mMaxSize; };

	inline void SetPosition(const MyVec2f& bl){ mBottomLeft = bl; };
	inline void SetSize(const MyVec2f& sz){ mSize = sz; };
	inline void SetConstant(bool c){ mbConstant = c; };
	inline void SetTextColor(const MyColor4f& color){ mTextColor = color; };
	void SetText(const MyString& str);
	void SetBlankText(const MyString &str);
	void ClearText();
	inline void SetMaxSize(int ms){ mMaxSize = ms; };
	//virtual int HandleMouseDown(float x, float y);
	//virtual int HandleMouseUp(float x, float y);
	//virtual int HandleMouseMove(float x, float y);
	virtual int HandleKey(unsigned char key, float x, float y);
	virtual int HandleSpecialKey(unsigned char key, float x, float y);

protected:
	bool mbConstant;
	int mMaxSize;
	MyString mBlankText;
	MyString mText;
	int mCursor;
	MyVec2f mBottomLeft;
	MyVec2f mSize;
	MyColor4f mTextColor;

	virtual bool IsKeyValid(unsigned char key) { return true; };
	virtual bool IsSpecialKeyValid(unsigned char key){ return true; };
};

