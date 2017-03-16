#pragma once
#include "MyArray.h"

class MyBlackBodyColor
{
public:
	MyBlackBodyColor();
	~MyBlackBodyColor();

	static unsigned int GetTexture();
	static unsigned int mTexture;
	static float mData[512][3];
};

