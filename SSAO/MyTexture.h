#pragma once

#include "MyImage.h"
#include "MyArray.h"

class MyTexture
{
public:
	MyTexture();
	~MyTexture();

	static unsigned int MakeGLTexture(const MyImage* image);
	static unsigned int MakeGLTexture(const float* image, int width, int height);
	static unsigned int MakeGLTexture(const MyArray<MyColor4f>& colors, int width, int height);

	static void SetInterpolateMethod(int m){ InterpolateMethod = m; };
	static void SetClampMethod(int m){ ClampMethod = m; };

protected:
	static int InterpolateMethod;
	static int ClampMethod;
};

