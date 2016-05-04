#pragma once

#include "MyQuadGeometry.h"

class MyLightingPass
{
public:
	MyLightingPass();
	~MyLightingPass();

	void CompileShader();
	void Build();
	void Render();
	void Destory();

	void SetColorTexture(unsigned int tex);
	void SetPositionTexture(unsigned int tex);
	void SetNormalTexture(unsigned int tex);
	void SetSsaoTexture(unsigned int tex);

	// rendering parameters
	float mLightItensity;
	float mAmbient, mDiffuse, mSpecular, mShininess;
	int mUseSsao;
	void ResetRenderingParameters();

protected:
	unsigned int mColorTexture;
	unsigned int mPositionTexture;
	unsigned int mNormalTexture;
	unsigned int mSsaoTexture;

	// shaders
	int mShaderProgram;

	MyQuadGeometry mQuad;
};

