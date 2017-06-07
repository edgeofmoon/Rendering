#pragma once

#include <stdlib.h>
#include <time.h>

#include <GL/glew.h>

#include "Shader.h"
#include "MyTractVisBase.h"

class MyLineAO :
	public MyTractVisBase{

public:
	MyLineAO();
	~MyLineAO();

	virtual void Resize(int w, int h);
	virtual void LoadShader();
	virtual void ComputeGeometry();
	virtual void Show();
	virtual void LoadGeometry();

	static int SamplesPerFragment;

	float mLineAOTotalStrength;
	int mBlurRadius;

	virtual inline unsigned int GetShaderProgram() const { return mLineShader; };
protected:
	void ComputeTubeGeometry();
	void ComputeLineGeometry();
	void ClearGeometry();

private:
	MyArray3f mTangents;
	unsigned int mTangentBuffer;
	int mTangentAttribute;

	GLuint quadVAO, quadVBO, quadIBO, quadTex;

	// G-Buffer
	GLuint gBuffer;
	GLuint gColor, gNDMap, gZoomMap;
	GLuint depthRBO;
	GLuint noise;

	// B-Buffer
	GLuint aoBuffer;
	GLuint aoColor;

	unsigned int mLineShader;
	unsigned int mTextureShader;
	unsigned int mAOShader;

	GLuint m_width, m_height;

	void initQuad();
	void initGBuffer();
	void initAOBuffer();
	void genNoiseTexture();

	// rendering passes
	void lineShadingPass() const;
	void lineAOPass() const;
	void lightningPass() const;
};	
