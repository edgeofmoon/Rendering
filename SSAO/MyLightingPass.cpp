#include "MyLightingPass.h"
#include "Shader.h"

#include <iostream>
using namespace std;

#include <GL/glew.h>
#include <GL/freeglut.h>

MyLightingPass::MyLightingPass()
{
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

	mQuad.Render();

	glUseProgram(0);
	glPopAttrib();
}

void MyLightingPass::Destory(){

}