#pragma once

#include "MyQuadGeometry.h"

class MyBlurPass
{
public:
	MyBlurPass();
	~MyBlurPass();

	void CompileShader();
	void Build();
	void Render();
	void Destory();

	void SetInputTexture(unsigned int tex);

	// rendering parameter
	int mBlurRadius;
	void ResetRenderingParameters();

protected:
	unsigned int mInputTexture;

	// shaders
	int mShaderProgram;

	MyQuadGeometry mQuad;
};

