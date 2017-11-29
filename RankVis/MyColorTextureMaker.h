#pragma once

#include "MyArray.h"
#include "MyString.h"

class MyColorTextureMaker
{
public:
	MyColorTextureMaker();
	~MyColorTextureMaker();

	static MyArrayui MakeColorTextures();
	static MyString GetColorName(int idx);
};

