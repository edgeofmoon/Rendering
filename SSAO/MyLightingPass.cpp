#include "MyLightingPass.h"
#include "Shader.h"

#include <iostream>
using namespace std;

#include <GL/glew.h>
#include <GL/freeglut.h>

MyLightingPass::MyLightingPass()
{
	ResetRenderingParameters();
}


MyLightingPass::~MyLightingPass()
{
}

void MyLightingPass::SetColorTexture(unsigned int tex){
	mColorTexture = tex;
}
void MyLightingPass::SetPositionTexture(unsigned int tex){
	mPositionTexture = tex;
}
void MyLightingPass::SetNormalTexture(unsigned int tex){
	mNormalTexture = tex;
}
void MyLightingPass::SetSsaoTexture(unsigned int tex){
	mSsaoTexture = tex;
}

void MyLightingPass::ResetRenderingParameters(){
	mLightItensity = 1;
	mAmbient = 0.2;
	mDiffuse = 0.5;
	mSpecular = 0.3;
	mShininess = 16;
	mUseSsao = 1;
}

void MyLightingPass::CompileShader(){

	glDeleteProgram(mShaderProgram);
	mShaderProgram = InitShader("Shaders\\lighting.vert", "Shaders\\lighting.frag", "fragColour");

	int loc = glGetAttribLocation(mShaderProgram, "position");
	if (loc < 0) {
		cerr << "Shader did not contain the 'position' attribute." << endl;
	}
	mQuad.SetPositionAttribute(loc);
}

void MyLightingPass::Build(){
	if (!glIsProgram(mShaderProgram)){
		CompileShader();
	}
	mQuad.Build();
}

void MyLightingPass::Render(){
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

	location = glGetUniformLocation(mShaderProgram, "gSsao");
	glUniform1i(location, 3);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, mSsaoTexture);

	location = glGetUniformLocation(mShaderProgram, "gAmbient");
	glUniform1f(location, mAmbient*mLightItensity/(mAmbient+mDiffuse+mSpecular));

	location = glGetUniformLocation(mShaderProgram, "gDiffuse");
	glUniform1f(location, mDiffuse*mLightItensity / (mAmbient + mDiffuse + mSpecular));

	location = glGetUniformLocation(mShaderProgram, "gSpecular");
	glUniform1f(location, mSpecular*mLightItensity / (mAmbient + mDiffuse + mSpecular));

	location = glGetUniformLocation(mShaderProgram, "gShininess");
	glUniform1f(location, mShininess);

	location = glGetUniformLocation(mShaderProgram, "gUseSsao");
	glUniform1i(location, mUseSsao);

	mQuad.Render();

	glUseProgram(0);
	glPopAttrib();
}

void MyLightingPass::Destory(){

}