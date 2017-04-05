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

void MyTubeDDH::Show(){
	//if (mShape == TRACK_SHAPE_TUBE){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_DEPTH_TEST);
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

	radiusLocation = glGetUniformLocation(mShaderProgram, "depthCueing");
	glUniform1f(radiusLocation, mDepthCueing);

#ifdef RIC
	glEnable(GL_TEXTURE_3D);
	int filterVolLocation = glGetUniformLocation(mShaderProgram, "filterVol");
	glUniform1i(filterVolLocation, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_3D, mFilterVolumeTexture);
#endif
	if (mShape == TRACK_SHAPE_TUBE){
		//this has no cap considered
		for (int i = 0; i < mFiberToDraw.size(); i++){
			int fiberIdx = mFiberToDraw[i];
			//int offset = (mIdxOffset[fiberIdx] / (mFaces + 1) - fiberIdx)*mFaces * 6;
			//int numVertex = (this->GetNumVertex(fiberIdx) - 1)*(mFaces + 0) * 6;
			// add cap offset
			int offset = (mIdxOffset[fiberIdx] / (mFaces + 1) - fiberIdx * 2)*mFaces * 6;
			int numVertex = (mTracts->GetNumVertex(fiberIdx) - 1)*(mFaces + 0) * 6 + mFaces * 6;
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