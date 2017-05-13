#include "MyToggleButton.h"

using namespace MyUI;

MyToggleButton::MyToggleButton()
	:MyButton(){
}


MyToggleButton::~MyToggleButton()
{
}

int MyToggleButton::HandleMouseDown(float x, float y){
	if (!HasStatusBit(ENABLE_BIT)) return 0;
	if (IsIn(x, y)){
		if (!HasStatusBit(DOWN_BIT)){
			if (mGroup) mGroup->ClearToggle();
			SetStatusBit(DOWN_BIT | PUSHED_BIT);
		}
		else{
			ClearStatusBit(DOWN_BIT);
			SetStatusBit(POPED_BIT);
		}
		SetStatusBit(FOCUS_BIT);
		return 1;
	}
	return 0;
}

int MyToggleButton::HandleMouseUp(float x, float y){
	if (!HasStatusBit(ENABLE_BIT)) return 0;
	ClearStatusBit(FOCUS_BIT);
	return 0;
}

void MyToggleButton::SetGroup(MyToggleButtonGroup* group){
	mGroup = group;
	if (mGroup){
		mGroup->AddButton(this);
	}
};

MyToggleButtonGroup::~MyToggleButtonGroup(){
	ClearOptions();
}

int MyToggleButtonGroup::HandleMouseDown(float x, float y){
	if (!IsEnabled()) return 0;
	int rst = 0;
	for (int i = 0; i < size(); i++){
		if (rst = at(i)->HandleMouseDown(x, y)) {
			mSelectedIndex = at(i)->GetIndex();
			return rst;
		}
	}
	return rst;
}

void MyToggleButtonGroup::ResetStatus() {
	mStatus = MyUI::ENABLE_BIT;
	for (int i = 0; i < size(); i++){
		at(i)->ResetStatus();
	}
};

MyToggleButton* MyToggleButtonGroup::GetButton(int idx){
	for (int i = 0; i < size(); i++){
		MyToggleButton* button = dynamic_cast<MyToggleButton*>(at(i));
		if (button->GetIndex() == idx){
			return button;
		}
	}
	return NULL;
}

void MyToggleButtonGroup::AddButton(MyToggleButton* button){
	if (!this->HasOne(button)){
		this->PushBack(button);
		mExternalButtons.PushBack(button);
	}
}

void MyToggleButtonGroup::AddOption(const MyString& text, int index){
	int posIdx = this->size();
	MyToggleButton* button = new MyToggleButton;
	button->SetSize(mButtonSize);
	MyVec2f offset = posIdx * (mButtonSize+mButtonInterval);
	if (mLayout == Layout_Horizontal){
		offset[1] = 0;
	}
	else offset[0] = 0;
	button->SetPosition(mBottomLeft + offset);
	button->SetText(text);
	button->SetIndex(index);
	this->PushBack(button);
	button->SetGroup(this);
}

void MyToggleButtonGroup::ClearOptions(){
	for (int i = 0; i < size(); i++){
		if (!mExternalButtons.HasOne(at(i))){
			delete at(i);
		}
	}
	this->clear();
}

int MyToggleButtonGroup::GetSelectedIndex() const{
	return mSelectedIndex;
}

bool MyToggleButtonGroup::SetToSelectedIndex(int idx){
	ClearToggle();
	for (int i = 0; i < size(); i++){
		MyToggleButton* button = dynamic_cast<MyToggleButton*>(at(i));
		if (button->GetIndex()==idx){
			button->SetStatusBit(DOWN_BIT);
			mSelectedIndex = idx;
			return true;
		}
	}
	return false;
}

void MyToggleButtonGroup::ClearToggle(){
	for (int i = 0; i < size(); i++){
		MyToggleButton* button = dynamic_cast<MyToggleButton*>(at(i));
		if (button){
			button->ClearStatusBit(DOWN_BIT);
		}
	}
}

MyVec2f MyToggleButtonGroup::GetSize() const{
	MyVec2f bsize(0, 0);
	if (size() == 0) return bsize;
	bsize += size()*mButtonSize;
	bsize += (size() - 1)*mButtonInterval;
	return bsize;
}

void MyToggleButtonGroup::UpdateLayout(){
	for (int i = 0; i < size(); i++){
		int posIdx = i;
		MyToggleButton* button = dynamic_cast<MyToggleButton*>(at(i));
		if (button){
			button->SetSize(mButtonSize);
			MyVec2f offset = posIdx * (mButtonSize + mButtonInterval);
			if (mLayout == Layout_Horizontal){
				offset[1] = 0;
			}
			else offset[0] = 0;
			button->SetPosition(mBottomLeft + offset);
		}
	}
}