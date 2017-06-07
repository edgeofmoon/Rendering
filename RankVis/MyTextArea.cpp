#include "MyTextArea.h"
#include "MyPrimitiveDrawer.h"

#include <sstream>
#include <algorithm>
#include <ctime>

#include "GL/glew.h"
#include <GL/freeglut.h>

using namespace std;
using namespace MyUI;
MyTextArea::MyTextArea()
	:MyUIObject(){
	mCursor = 0;
	mMaxSize = 255;
	mTextColor = MyColor4f::black();
}


MyTextArea::~MyTextArea()
{
}

void MyTextArea::Show(){
	if (HasStatusBit(HIDE_BIT)) return;
	MyArray3f vertices = { mBottomLeft.toDim<3>(-mDepth),
		(mBottomLeft + MyVec2f(mSize[0], 0)).toDim<3>(-mDepth),
		(mBottomLeft + mSize).toDim<3>(-mDepth),
		(mBottomLeft + MyVec2f(0, mSize[1])).toDim<3>(-mDepth), };
	MyPrimitiveDrawer::PushAllAttributes();

	if (mText.empty() && !mBlankText.empty()){
		MyPrimitiveDrawer::Color(MyColor4f(0.5, 0.5, 0.5, 1));
		MyVec3f bl = mBottomLeft.toDim<3>(-mDepth);
		MyPrimitiveDrawer::DrawBitMapTextLarge(bl, mBlankText, 1);
	}
	else{
		std::stringstream ss;
		ss.str(mText);
		int nLines = 0;
		for (char c : mText) nLines += (c == '\n');
		if (nLines == 0 && !mText.empty()) nLines = 1;
		std::string curLine;
		int idx = 0;
		int lineIdx = 0;
		float height = MyPrimitiveDrawer::GetBitmapLargeHeight();
		while (std::getline(ss, curLine, '\n')) {
			int curLoc = -1;
			if (mCursor >= idx && mCursor <= idx + curLine.size()){
				curLoc = mCursor - idx;
				// text
				// at least draw the cursor
				MyString text0 = curLine.substr(0, curLoc);
				MyString text1 = curLine.substr(curLoc);
				float width = MyPrimitiveDrawer::GetBitMapLargeTextBox(curLine + '|').GetWidth();
				float scale = mSize[0] / width * 3;
				float width0 = MyPrimitiveDrawer::GetBitMapLargeTextBox(text0).GetWidth();
				float width1 = width0 + (mbConstant ? 0 : MyPrimitiveDrawer::GetBitMapLargeTextBox("|").GetWidth());
				MyVec3f bl = (mBottomLeft + MyVec2f(0, height*(nLines-1-lineIdx)) - MyVec2f(width / 2, 0)).toDim<3>(-mDepth);
				MyPrimitiveDrawer::Color(mTextColor);
				MyPrimitiveDrawer::DrawBitMapTextLarge(bl, text0);
				MyPrimitiveDrawer::DrawBitMapTextLarge(bl + MyVec3f(width1, 0, 0), text1);
				if (!mbConstant){
					MyPrimitiveDrawer::Color(MyColor4f::green());
					MyPrimitiveDrawer::DrawBitMapTextLarge(bl + MyVec3f(width0, 0, 0), "|");
				}
			}
			else{
				MyPrimitiveDrawer::Color(mTextColor);
				float width = MyPrimitiveDrawer::GetBitMapLargeTextBox(curLine).GetWidth();
				MyVec3f bl = (mBottomLeft + MyVec2f(0, height*(nLines - 1 - lineIdx)) - MyVec2f(width / 2, 0)).toDim<3>(-mDepth);
				MyPrimitiveDrawer::DrawBitMapTextLarge(bl, curLine);
			}
			idx += curLine.size();
			lineIdx++;
		}
	}

	// inside
	//MyPrimitiveDrawer::FillPolygon();
	//MyPrimitiveDrawer::Color(MyColor4f(1, 1, 1, 1));
	//MyPrimitiveDrawer::DrawQuadsAt(vertices);
	// boarder
	//MyPrimitiveDrawer::WirePolygon();
	//MyPrimitiveDrawer::Color(MyColor4f(0, 0, 0, 1));
	//MyPrimitiveDrawer::DrawQuadsAt(vertices);

	MyPrimitiveDrawer::PopAttributes();
}

bool MyTextArea::IsIn(float x, float y) const{
	return false;
}

void MyTextArea::SetText(const MyString& str){
	mText = str.substr(0, mMaxSize);
	mCursor = str.size();
};

void MyTextArea::SetBlankText(const MyString& str){
	mBlankText = str;// .substr(0, mMaxSize);
	mCursor = 0;
};

void MyTextArea::ClearText(){
	mText = "";
	mCursor = 0;
}

int MyTextArea::HandleKey(unsigned char key, float x, float y){
	if (!HasStatusBit(ENABLE_BIT) || mbConstant) return 0;
	if (!IsKeyValid(key)) return 0;
	if (key == '\b') {
		// backspace
		if (mCursor > 0){
			mText.erase(mCursor - 1, 1);
			mCursor--;
		}
	}
	else if (key == 127){
		// delete key
		if (mCursor < mText.size()){
			mText.erase(mCursor, 1);
		}
	}
	else if (mText.size() < mMaxSize){
		mText.insert(mCursor, 1, key);
		mCursor++;
	}
	return 1;
}

int MyTextArea::HandleSpecialKey(unsigned char key, float x, float y){
	if (!HasStatusBit(ENABLE_BIT) || mbConstant) return 0;
	if (!IsSpecialKeyValid(key)) return 0;
	if (key == GLUT_KEY_LEFT){
		mCursor = max(0, mCursor - 1);
	}
	else if (key == GLUT_KEY_RIGHT){
		mCursor = min(int(mText.size()), mCursor + 1);
	}
	else if (key == GLUT_KEY_HOME){
		mCursor = 0;
	}
	else if (key == GLUT_KEY_END){
		mCursor = mText.size();
	}
	else return 0;
	return 1;
}