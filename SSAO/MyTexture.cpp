#include "MyTexture.h"

#include <GL/freeglut.h>


int MyTexture::InterpolateMethod = GL_LINEAR;
int MyTexture::ClampMethod = GL_CLAMP;

MyTexture::MyTexture()
{
}


MyTexture::~MyTexture()
{
}

unsigned int MyTexture::MakeGLTexture(const MyImage* image){
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, InterpolateMethod);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, InterpolateMethod);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ClampMethod);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ClampMethod);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->GetWidth(),
		image->GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, image->GetPixelBufferRGB());
	return texture;
}

unsigned int MyTexture::MakeGLTexture(const float* image, int width, int height){
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, InterpolateMethod);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, InterpolateMethod);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ClampMethod);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ClampMethod);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width,
		height, 0, GL_RGB, GL_FLOAT, image);
	return texture;
}

unsigned int MyTexture::MakeGLTexture(
	const MyArray<MyColor4f>& colors, int width, int height){
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, InterpolateMethod);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, InterpolateMethod);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ClampMethod);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ClampMethod);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width,
		height, 0, GL_RGBA, GL_FLOAT, &colors[0].r);
	return texture;
}