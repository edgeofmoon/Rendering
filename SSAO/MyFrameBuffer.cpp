#include "MyFrameBuffer.h"

#include <gl/glew.h>
#include "GL/freeglut.h"

#define EXTRATEXTUREATTRIBUTES 5
MyFrameBuffer::MyFrameBuffer(int width, int height){
	SetSize(width, height);
}

void MyFrameBuffer::SetSize(int width, int height){
	mWidth = width;
	mHeight = height;
}

MyFrameBuffer::~MyFrameBuffer(){
	Destory();
}

void MyFrameBuffer::Destory(){
	if (glIsTexture(mColorTexture)) {
		glDeleteTextures(1, &(mColorTexture));
	}
	if (glIsTexture(mDepthTexture)) {
		glDeleteTextures(1, &mDepthTexture);
	}
	for (int i = 0; i < mExtraDrawTextureFormats.size() / EXTRATEXTUREATTRIBUTES; i++){
		if (glIsTexture(mExtraDrawTextures[i])) {
			glDeleteTextures(1, &mExtraDrawTextures[i]);
		}
	}
	if (glIsFramebuffer(mFrameBuffer)) {
		glDeleteFramebuffers(1, &mFrameBuffer);
	}
}

void MyFrameBuffer::Clear(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void MyFrameBuffer::Build(){
	std::vector<GLenum> drawBuffers;
	// COLOR
	if (glIsTexture(mColorTexture)) {
		glDeleteTextures(1, &(mColorTexture));
	}
	glGenTextures(1, &mColorTexture);
	glBindTexture(GL_TEXTURE_2D, mColorTexture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	drawBuffers.push_back(GL_COLOR_ATTACHMENT0);

	// DEPTH
	if (glIsTexture(mDepthTexture)) {
		glDeleteTextures(1, &mDepthTexture);
	}
	glGenTextures(1, &mDepthTexture);
	glBindTexture(GL_TEXTURE_2D, mDepthTexture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, mWidth, mHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// EXTRA
	mExtraDrawTextures.resize(mExtraDrawTextureFormats.size() / EXTRATEXTUREATTRIBUTES);
	for (int i = 0; i < mExtraDrawTextureFormats.size() / EXTRATEXTUREATTRIBUTES; i++){
		if (glIsTexture(mExtraDrawTextures[i])) {
			glDeleteTextures(1, &mExtraDrawTextures[i]);
		}
		glGenTextures(1, &mExtraDrawTextures[i]);
		glBindTexture(GL_TEXTURE_2D, mExtraDrawTextures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mExtraDrawTextureFormats[i * EXTRATEXTUREATTRIBUTES + 3]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mExtraDrawTextureFormats[i * EXTRATEXTUREATTRIBUTES + 3]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mExtraDrawTextureFormats[i * EXTRATEXTUREATTRIBUTES + 4]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mExtraDrawTextureFormats[i * EXTRATEXTUREATTRIBUTES + 4]);
		glTexImage2D(GL_TEXTURE_2D, 0, mExtraDrawTextureFormats[i * EXTRATEXTUREATTRIBUTES],
			mWidth, mHeight, 0, mExtraDrawTextureFormats[i * EXTRATEXTUREATTRIBUTES + 1],
			mExtraDrawTextureFormats[i * EXTRATEXTUREATTRIBUTES + 2], 0);
		drawBuffers.push_back(GL_COLOR_ATTACHMENT0+i+1);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	// FRAMEBUFFER ASSEMBLE
	if (glIsFramebuffer(mFrameBuffer)) {
		glDeleteFramebuffers(1, &mFrameBuffer);
	}
	glGenFramebuffers(1, &mFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mDepthTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mColorTexture, 0);

	for (int i = 1; i < drawBuffers.size(); i++){
		glFramebufferTexture(GL_FRAMEBUFFER, drawBuffers[i], mExtraDrawTextures[i-1], 0);
	}
	glDrawBuffers(drawBuffers.size(), &drawBuffers[0]);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MyFrameBuffer::AddExtraDrawTexture(int internamFormat, int format, int type, int filter, int wrap){
	mExtraDrawTextureFormats.push_back(internamFormat);
	mExtraDrawTextureFormats.push_back(format);
	mExtraDrawTextureFormats.push_back(type);
	mExtraDrawTextureFormats.push_back(filter);
	mExtraDrawTextureFormats.push_back(wrap);
}