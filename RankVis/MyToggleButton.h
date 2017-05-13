#pragma once
#include "MyButton.h"
#include "MyArray.h"

class MyToggleButtonGroup;

class MyToggleButton :
	public MyButton
{
public:
	MyToggleButton();
	~MyToggleButton();

	virtual int HandleMouseDown(float x, float y);
	virtual int HandleMouseUp(float x, float y);

	friend MyToggleButtonGroup;
	void SetGroup(MyToggleButtonGroup* group);

protected:
	MyToggleButtonGroup* mGroup;
};

class MyToggleButtonGroup
	: public MyUIObjectGroup{
public:

	MyToggleButtonGroup(){};
	~MyToggleButtonGroup();

	enum ButtonGroupLayout{
		Layout_Horizontal = 1,
		Layout_Vertical = 2,
	};

	virtual int HandleMouseDown(float x, float y);
	virtual void ResetStatus();

	MyToggleButton* GetButton(int idx);
	void AddButton(MyToggleButton* button);
	void AddOption(const MyString& text, int index);
	void ClearOptions();
	int GetSelectedIndex() const;
	bool SetToSelectedIndex(int idx);
	void ClearToggle();
	MyVec2f GetSize() const;

	inline void SetPosition(const MyVec2f& bl){ mBottomLeft = bl; };
	inline void SetButtonSize(const MyVec2f& sz){ mButtonSize = sz; };
	inline void SetButtonInterval(const MyVec2f& it){ mButtonInterval = it; };
	inline void SetLayout(ButtonGroupLayout ly){ mLayout = ly; };

	void UpdateLayout();

protected:
	int mSelectedIndex;
	MyVec2f mButtonSize;
	MyVec2f mBottomLeft;
	MyVec2f mButtonInterval;
	ButtonGroupLayout mLayout;

	MyArray<MyUIObject*> mExternalButtons;
};
