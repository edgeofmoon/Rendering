#include "MyUIObject.h"

using namespace MyUI;

MyUIObject::MyUIObject(){
	ResetStatus();
	mDepth = 0;
}

MyUIObject::~MyUIObject(){
}

int MyUIObject::HandleMouseDown(float x, float y){
	if (!HasStatusBit(ENABLE_BIT)) return 0;
	if (IsIn(x, y)){
		if (!HasStatusBit(DOWN_BIT)){
			SetStatusBit(DOWN_BIT | PUSHED_BIT);
		}
		SetStatusBit(FOCUS_BIT);
		return 1;
	}
	return 0;
}

int MyUIObject::HandleMouseUp(float x, float y){
	if (!HasStatusBit(ENABLE_BIT)) return 0;
	ClearStatusBit(FOCUS_BIT);
	if (HasStatusBit(DOWN_BIT)){
		ClearStatusBit(DOWN_BIT);
		if (IsIn(x, y)) SetStatusBit(POPED_BIT);
		return 1;
	}
	return 0;
}

int MyUIObject::HandleMouseMove(float x, float y){
	if (!HasStatusBit(ENABLE_BIT)) return 0;
	if (IsIn(x, y)){
		if (!HasStatusBit(INSIDE_BIT)){
			SetStatusBit(ENTERED_BIT | INSIDE_BIT);
			return 1;
		}
	}
	else{
		if (HasStatusBit(INSIDE_BIT)){
			SetStatusBit(EXITED_BIT);
			ClearStatusBit(INSIDE_BIT);
			return 1;
		}
	}
	if (HasStatusBit(FOCUS_BIT)) return 1;
	return 0;
}

int MyUIObject::HandleKey(unsigned char key, float x, float y){
	return 0;
}

int MyUIObject::HandleSpecialKey(unsigned char key, float x, float y){
	return 0;
}

void MyUIObjectGroup::Show(){
	if (this->IsHidden()) return;
	for (int i = 0; i < size(); i++){
		at(i)->Show();
	}
}

bool MyUIObjectGroup::IsIn(float x, float y) const{
	for (int i = 0; i < size(); i++){
		if (at(i)->IsIn(x, y)) return true;
	}
	return false;
}
int MyUIObjectGroup::HandleMouseDown(float x, float y){
	if (!this->IsEnabled()) return 0;
	int rst = 0;
	for (int i = 0; i < size(); i++){
		if (rst = at(i)->HandleMouseDown(x, y)) return rst;
	}
	return rst;
}

int MyUIObjectGroup::HandleMouseUp(float x, float y){
	if (!this->IsEnabled()) return 0;
	int rst = 0;
	for (int i = 0; i < size(); i++){
		if (rst = at(i)->HandleMouseUp(x, y)) return rst;
	}
	return rst;
}

int MyUIObjectGroup::HandleMouseMove(float x, float y){
	if (!this->IsEnabled()) return 0;
	int rst = 0;
	for (int i = 0; i < size(); i++){
		if (rst = at(i)->HandleMouseMove(x, y)) return rst;
	}
	return rst;
}

int MyUIObjectGroup::HandleKey(unsigned char key, float x, float y){
	if (!this->IsEnabled()) return 0;
	int rst = 0;
	for (int i = 0; i < size(); i++){
		if (rst = at(i)->HandleKey(key, x, y)) return rst;
	}
	return rst;
}

int MyUIObjectGroup::HandleSpecialKey(unsigned char key, float x, float y){
	if (!this->IsEnabled()) return 0;
	int rst = 0;
	for (int i = 0; i < size(); i++){
		if (rst = at(i)->HandleSpecialKey(key, x, y)) return rst;
	}
	return rst;
}
