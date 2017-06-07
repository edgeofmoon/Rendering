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

protected:
	MyToggleButtonGroup* mGroup;
	void SetGroup(MyToggleButtonGroup* group);
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
	enum ButtonGroupAlignment{
		Alignment_Left = 1,
		Alignment_Middle = 2,
		Alignment_Right = 4,
		Alignment_Top = 8,
		Alignment_Center = 16,
		Alignment_Down = 32,
	};

	virtual int HandleMouseDown(float x, float y);
	virtual void ResetStatus();

	MyToggleButton* GetButton(int idx);
	void AddButton(MyToggleButton* button);
	void AddOption(const MyString& text, int index);
	void ClearOptions();
	int GetSelectedIndex() const;
	MyToggleButton* GetSelectedButton();
	bool SetToSelectedIndex(int idx);
	void ClearToggle();
	MyVec2f GetSize() const;

	inline void SetPosition(const MyVec2f& pos){ mPosition = pos; };
	inline void SetButtonSize(const MyVec2f& sz){ mButtonSize = sz; };
	inline void SetButtonInterval(const MyVec2f& it){ mButtonInterval = it; };
	inline void SetLayout(ButtonGroupLayout ly){ mLayout = ly; };
	inline void SetAlighment(ButtonGroupAlignment al){ mAlignment = al; };

	void UpdateLayout();

protected:
	int mSelectedIndex;
	MyVec2f mButtonSize;
	MyVec2f mPosition;
	MyVec2f mButtonInterval;
	ButtonGroupLayout mLayout;
	ButtonGroupAlignment mAlignment;
};
