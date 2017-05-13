#pragma once
#include "MyUI.h"
#include "MyArray.h"

class MyUIObject
{
public:
	MyUIObject();
	~MyUIObject();

	virtual void Show() = 0;
	virtual bool IsIn(float x, float y) const = 0;

	virtual int HandleMouseDown(float x, float y);
	virtual int HandleMouseUp(float x, float y);
	virtual int HandleMouseMove(float x, float y);
	virtual int HandleKey(unsigned char key, float x, float y);
	virtual int HandleSpecialKey(unsigned char key, float x, float y);

	inline int GetEventBit() const { return mStatus & MyUI::ALL_EVENT_BITS; };
	inline void ClearEventBit(){ mStatus &= ~MyUI::ALL_EVENT_BITS; };

	virtual void ResetStatus() { mStatus = MyUI::ENABLE_BIT; };
	inline void SetDepth(float d){ mDepth = d; };
	inline void SetIndex(int idx){ mIndex = idx; };
	inline void Enable() { mStatus |= MyUI::ENABLE_BIT; };
	inline void Disable() { mStatus &= ~MyUI::ENABLE_BIT; };
	inline void SetHidden(bool f){ f ? (mStatus |= MyUI::HIDE_BIT) : (mStatus &= ~MyUI::HIDE_BIT); };
	inline void SetDown(bool f){ f ? (mStatus |= MyUI::DOWN_BIT) : (mStatus &= ~MyUI::DOWN_BIT); };

	inline float GetDepth() const { return mDepth; };
	inline int GetIndex() const { return mIndex; };
	inline bool IsEnabled() const { return mStatus & MyUI::ENABLE_BIT; };
	inline bool IsHidden() const { return mStatus & MyUI::HIDE_BIT; };

protected:
	int mIndex;
	float mDepth;
	int mStatus;

	inline void SetStatusBit(int st){ mStatus |= st; };
	inline void ClearStatusBit(int st){ mStatus &= ~st; };;
	inline bool HasStatusBit(int st) const { return mStatus & st; };
};

class MyUIObjectGroup
	: public MyUIObject, protected MyArray<MyUIObject*>{
public:

	virtual void Show();
	virtual bool IsIn(float x, float y) const;

	virtual int HandleMouseDown(float x, float y);
	virtual int HandleMouseUp(float x, float y);
	virtual int HandleMouseMove(float x, float y);
	virtual int HandleKey(unsigned char key, float x, float y);
	virtual int HandleSpecialKey(unsigned char key, float x, float y);
};