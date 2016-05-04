#include "MyBlurPass.h"
#include "Shader.h"

#include <iostream>
using namespace std;

#include <GL/glew.h>
#include <GL/freeglut.h>

MyBlurPass::MyBlurPass()
{
	ResetRenderingParameters();
}


MyBlurPass::~MyBlurPass()
{
}

void MyBlurPass::CompileShader(){
	if (glIsProgram(mShaderProgram)){
		glDeleteProgram(mShaderProgram);
	}
	mShaderProgram = InitShader("Shaders\\blur.vert", "Shaders\\blur.frag", "fragColour");

	int loc = glGetAttribLocation(mShaderProgram, "position");
	if (loc < 0) {
		cerr << "Shader did not contain the 'position' attribute." << endl;
	}
	mQuad.SetPositionAttribute(loc);
}
void MyBlurPass::Build(){
	if (!glIsProgram(mShaderProgram)){
		CompileShader();
	}
	mQuad.Build();
}

void MyBlurPass::Render(){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glUseProgram(mShaderProgram);

	int location = glGetUniformLocation(mShaderProgram, "gColor");
	glUniform1i(location, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mInputTexture);

	location = glGetUniformLocation(mShaderProgram, "blurRadius");
	glUniform1i(location, mBlurRadius);

	mQuad.Render();
	glUseProgram(0);
	glPopAttrib();
}
void MyBlurPass::Destory(){

}

void MyBlurPass::ResetRenderingParameters(){
	mBlurRadius = 2;
}

void MyBlurPass::SetInputTexture(unsigned int tex){
	mInputTexture = tex;
}
