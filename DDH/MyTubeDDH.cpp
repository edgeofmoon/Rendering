#include "MyTubeDDH.h"
#include "MyVec.h"
#include "Shader.h"
#include "MyGraphicsTool.h"
#include "MyMathHelper.h"
#include "MyBlackBodyColor.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <cassert>
using namespace std;

#include "GL\glew.h"
#include <GL/freeglut.h>

MyTubeDDH::MyTubeDDH()
	:MyTractVisBase()
{
	mDepthCueing = 1;
}


MyTubeDDH::~MyTubeDDH()
{
}
void MyTubeDDH::LoadShader(){

	glDeleteProgram(mShaderProgram);
	mShaderProgram = InitShader("..\\DDH\\shaders\\geom.vert", "..\\DDH\\shaders\\geom.frag", "fragColour");

	mPositionAttribute = glGetAttribLocation(mShaderProgram, "position");
	if (mPositionAttribute < 0) {
		cerr << "Shader did not contain the 'position' attribute." << endl;
	}
	mNormalAttribute = glGetAttribLocation(mShaderProgram, "normal");
	if (mNormalAttribute < 0) {
		cerr << "Shader did not contain the 'normal' attribute." << endl;
	}
	mColorAttribute = glGetAttribLocation(mShaderProgram, "color");
	if (mColorAttribute < 0) {
		cerr << "Shader did not contain the 'color' attribute." << endl;
	}
}


void MyTubeDDH::LoadGeometry(){
	if (glIsVertexArray(mVertexArray)){
		glDeleteVertexArrays(1, &mVertexArray);
	}
	glGenVertexArrays(1, &mVertexArray);
	glBindVertexArray(mVertexArray);
	// vertex
	if (glIsBuffer(mVertexBuffer)){
		glDeleteBuffers(1, &mVertexBuffer);
	}
	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(MyVec3f), &mVertices[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mPositionAttribute);
	glVertexAttribPointer(mPositionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// normal
	if (glIsBuffer(mNormalBuffer)){
		glDeleteBuffers(1, &mNormalBuffer);
	}
	glGenBuffers(1, &mNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(MyVec3f), &mNormals[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mNormalAttribute);
	glVertexAttribPointer(mNormalAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// index
	if (glIsBuffer(mIndexBuffer)){
		glDeleteBuffers(1, &mIndexBuffer);
	}
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(MyVec3i), &mIndices[0][0], GL_STATIC_DRAW);

	// unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// now free everything
	ClearGeometry();
}


void MyTubeDDH::Show(){
	//if (mShape == TRACK_SHAPE_TUBE){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(mVertexArray);
	glUseProgram(mShaderProgram);

	int mvmatLocation = glGetUniformLocation(mShaderProgram, "mvMat");
	float modelViewMat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMat);
	glUniformMatrix4fv(mvmatLocation, 1, GL_FALSE, modelViewMat);

	int normatLocation = glGetUniformLocation(mShaderProgram, "normalMat");
	float normalMat[16];
	bool bInv = MyMathHelper::InvertMatrix4x4ColMaj(modelViewMat, normalMat);
	assert(bInv);
	MyMathHelper::TransposeMatrix4x4ColMaj(normalMat, normalMat);
	glUniformMatrix4fv(normatLocation, 1, GL_FALSE, normalMat);

	int projmatLocation = glGetUniformLocation(mShaderProgram, "projMat");
	float projMat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
	glUniformMatrix4fv(projmatLocation, 1, GL_FALSE, projMat);

	int radiusLocation = glGetUniformLocation(mShaderProgram, "radius");
	glUniform1f(radiusLocation, mTrackRadius);

	glUniform4fv(glGetUniformLocation(mShaderProgram, "baseColor"), 1, &mRenderingParameters.BaseColor.r);
	glUniform1f(glGetUniformLocation(mShaderProgram, "lightIntensity"), mRenderingParameters.LightIntensity);
	glUniform1f(glGetUniformLocation(mShaderProgram, "ambient"), mRenderingParameters.Ambient);
	glUniform1f(glGetUniformLocation(mShaderProgram, "diffuse"), mRenderingParameters.Diffuse);
	glUniform1f(glGetUniformLocation(mShaderProgram, "specular"), mRenderingParameters.Specular);
	glUniform1f(glGetUniformLocation(mShaderProgram, "shininess"), mRenderingParameters.Shininess);

	radiusLocation = glGetUniformLocation(mShaderProgram, "depthCueing");
	glUniform1f(radiusLocation, mDepthCueing);

	if (mRenderingParameters.Shape == TRACK_SHAPE_TUBE){
		//this has no cap considered
		int nFaces = mRenderingParameters.Faces;
		for (int i = 0; i < mFiberToDraw.size(); i++){
			int fiberIdx = mFiberToDraw[i];
			//int offset = (mIdxOffset[fiberIdx] / (mFaces + 1) - fiberIdx)*mFaces * 6;
			//int numVertex = (this->GetNumVertex(fiberIdx) - 1)*(mFaces + 0) * 6;
			// add cap offset
			int offset = (mIdxOffset[fiberIdx] / (nFaces + 1) - fiberIdx * 2)*nFaces * 6;
			int numVertex = (mTracts->GetNumVertex(fiberIdx) - 1)*(nFaces + 0) * 6 + nFaces * 6;
			glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
		}
	}
	else{
		for (int i = 0; i < mFiberToDraw.size(); i++){
			int fiberIdx = mFiberToDraw[i];
			int offset = mIdxOffset[fiberIdx];
			int numVertex = mTracts->GetNumVertex(fiberIdx);
			glDrawElements(GL_LINE_STRIP, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
		}
	}

	glUseProgram(0);
	glBindVertexArray(0);
	glPopAttrib();
}