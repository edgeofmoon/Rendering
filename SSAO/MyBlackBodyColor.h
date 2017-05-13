#pragma once
#include "MyArray.h"

class MyBlackBodyColor
{
public:
	MyBlackBodyColor();
	~MyBlackBodyColor();

	static unsigned int GetTexture();
	static unsigned int mTexture;
	static float mData_512_3[512][3];
	static float mData_1024_3[1024][3];
};

