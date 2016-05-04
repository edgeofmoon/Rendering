#pragma once

#define SSAO_KERNEL_SIZE 64
#define NOISE_TILE_SIZE 4

#include "MyQuadGeometry.h"

class MySsaoPass
{
public:
	MySsaoPass();
	~MySsaoPass();

	void SetColorTexture(unsigned int tex);
	void SetPositionTexture(unsigned int tex);
	void SetNormalTexture(unsigned int tex);

	void CompileShader();
	void Build();
	void Render();
	void Destory();

	// rendering parameter
	float mSampleRadius;
	void ResetRenderingParameters();

protected:
	unsigned int mColorTexture;
	unsigned int mPositionTexture;
	unsigned int mNormalTexture;

	float mSsaoKernel[SSAO_KERNEL_SIZE*3];
	void BuildSsaoKernel();

	unsigned int mNoiseTeture;
	void BuildNoiseTexture();

	// shaders
	int mShaderProgram;

	// quad geometry
	MyQuadGeometry mQuad;
	
private:
	float lerp(float a, float b, float f) const;
};

