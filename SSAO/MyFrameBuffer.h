#pragma once

#include <vector>

class MyFrameBuffer{
public:
	MyFrameBuffer(){};
	MyFrameBuffer(int width, int height);
	~MyFrameBuffer();

	void Destory();
	void SetSize(int width, int height);
	void Clear();
	void Build();
	void AddExtraDrawTexture(int internamFormat, int format, int type, int filter, int wrap);
	unsigned int GetExtraDrawTexture(int i){ return mExtraDrawTextures[i]; };
	unsigned int GetFrameBuffer() const { return mFrameBuffer; };
	unsigned int GetColorTexture() const { return mColorTexture; };
	unsigned int GetDepthTexture() const { return mDepthTexture; };

protected:
	unsigned int mWidth, mHeight;
	unsigned int mFrameBuffer;
	unsigned int mColorTexture;
	unsigned int mDepthTexture;

	std::vector<int> mExtraDrawTextureFormats;
	std::vector<unsigned int> mExtraDrawTextures;
};