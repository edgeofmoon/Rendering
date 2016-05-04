#include <cstdlib>
#include <string>
#include <iostream>
#include <vector>
using namespace std;

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "MySsaoPass.h"
#include "MyVec.h"
#include "Shader.h"
MySsaoPass::MySsaoPass()
{
	ResetRenderingParameters();
}


MySsaoPass::~MySsaoPass()
{
}

void MySsaoPass::SetColorTexture(unsigned int tex){
	mColorTexture = tex;
}
void MySsaoPass::SetPositionTexture(unsigned int tex){
	mPositionTexture = tex;
}
void MySsaoPass::SetNormalTexture(unsigned int tex){
	mNormalTexture = tex;
}

void MySsaoPass::ResetRenderingParameters(){
	mOcclusionPower = 1.f;
	mSampleRadius = 10.f;
}

void MySsaoPass::CompileShader(){

	glDeleteProgram(mShaderProgram);
	mShaderProgram = InitShader("Shaders\\ssao.vert", "Shaders\\ssao.frag", "fragColour");

	int loc = glGetAttribLocation(mShaderProgram, "position");
	if (loc < 0) {
		cerr << "Shader did not contain the 'position' attribute." << endl;
	}
	mQuad.SetPositionAttribute(loc);
}

void MySsaoPass::Build(){
	BuildSsaoKernel();
	BuildNoiseTexture();
	if (!glIsProgram(mShaderProgram)){
		CompileShader();
	}
	mQuad.Build();
}

void MySsaoPass::Render(){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glUseProgram(mShaderProgram);

	int location = glGetUniformLocation(mShaderProgram, "gColor");
	glUniform1i(location, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mColorTexture);

	location = glGetUniformLocation(mShaderProgram, "gPositionDepth");
	glUniform1i(location, 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, mPositionTexture);

	location = glGetUniformLocation(mShaderProgram, "gNormal");
	glUniform1i(location, 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, mNormalTexture);

	location = glGetUniformLocation(mShaderProgram, "texNoise");
	glUniform1i(location, 3);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, mNoiseTeture);

	location = glGetUniformLocation(mShaderProgram, "samples");
	glUniform3fv(location, 64, mSsaoKernel);

	location = glGetUniformLocation(mShaderProgram, "projection");
	float projMat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
	glUniformMatrix4fv(location, 1, GL_FALSE, projMat);

	location = glGetUniformLocation(mShaderProgram, "radius");
	glUniform1f(location, mSampleRadius);

	location = glGetUniformLocation(mShaderProgram, "occlusionPower");
	glUniform1f(location, mOcclusionPower);

	mQuad.Render();

	glUseProgram(0);
	glPopAttrib();
}

void MySsaoPass::Destory(){
	if (glIsTexture(mNoiseTeture)){
		glDeleteTextures(1, &mNoiseTeture);
	}
}
void MySsaoPass::BuildSsaoKernel(){
	for (int i = 0; i < SSAO_KERNEL_SIZE; i++){
		MyVec3f ranVec(rand() / (float)RAND_MAX * 2.0 - 1.0,
			rand() / (float)RAND_MAX * 2.0 - 1.0,
			rand() / (float)RAND_MAX);
		ranVec.normalize();
		ranVec *= (rand() / (float)RAND_MAX);
		float scale = (float)i / (SSAO_KERNEL_SIZE - 1);
		scale = lerp(0.1, 1.0, scale*scale);
		ranVec *= scale;
		memcpy(&mSsaoKernel[i * 3], &ranVec[0], 3 * sizeof(float));
	}
}

void MySsaoPass::BuildNoiseTexture(){
	float noiseKernel[NOISE_TILE_SIZE*NOISE_TILE_SIZE*3];
	for (int i = 0; i < NOISE_TILE_SIZE*NOISE_TILE_SIZE; i++){
		noiseKernel[i * 3 + 0] = rand() / (float)RAND_MAX * 2.0 - 1.0;
		noiseKernel[i * 3 + 1] = rand() / (float)RAND_MAX * 2.0 - 1.0;
		noiseKernel[i * 3 + 2] = 0;
	}

	if (!glIsTexture(mNoiseTeture)){
		glGenTextures(1, &mNoiseTeture);
	}
	glBindTexture(GL_TEXTURE_2D, mNoiseTeture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, noiseKernel);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
}

float MySsaoPass::lerp(float a, float b, float f) const{
	return a + f*(b - a);
}