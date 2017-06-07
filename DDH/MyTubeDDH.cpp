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
	mDepthCueing = 0.5;
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


void MyTubeDDH::ComputeGeometry(){
	int currentIdx = 0;
	mIdxOffset.clear();
	int totalPoints = 0;
	for (int it = 0; it < mTracts->GetNumTracks(); it++){
		totalPoints += mTracts->At(it).Size();
	}
	totalPoints *= (mRenderingParameters.Faces + 1);

	// for caps
	totalPoints += mTracts->GetNumTracks()*(1 + mRenderingParameters.Faces) * 2;

	//cout << "Allocating Storage for Geometry...\r";

	mVertices.resize(totalPoints);
	mNormals.resize(totalPoints);
	mTexCoords.resize(totalPoints);
	//mRadius.resize(totalPoints);
	mColors.resize(totalPoints);
	mValues.resize(totalPoints);

	float R = mTrackRadius;

	for (int it = 0; it < mTracts->GetNumTracks(); it++){
		PrintProgress(it, mTracts->GetNumTracks(), 1);
		int npoints = mTracts->At(it).Size();
		if (npoints < 2) continue;
		const float myPI = 3.1415926f;
		float dangle = 2 * myPI / mRenderingParameters.Faces;
		MyVec3f pole(0.6, 0.8, 0);

		MyArray3f candicates;
		candicates << MyVec3f(0, 0, 1)
			<< MyVec3f(0, 1, 1)
			<< MyVec3f(0, 1, 0)
			<< MyVec3f(1, 1, 0)
			<< MyVec3f(1, 0, 0)
			<< MyVec3f(1, 0, 1);
		float max = -1;
		int maxIdx;
		MyVec3f genDir = mTracts->GetCoord(it, 0) - mTracts->GetCoord(it, npoints - 1);
		genDir.normalize();
		for (int i = 0; i<candicates.size(); i++){
			float cp = (candicates[i].normalized() ^ genDir).norm();
			if (cp>max){
				max = cp;
				maxIdx = i;
			}
		}
		pole = candicates[maxIdx].normalized();

		for (int i = 0; i<npoints; i++){
			MyVec3f p = mTracts->GetCoord(it, i);
			MyVec3f d;
			if (i == npoints - 1) d = p - mTracts->GetCoord(it, i - 1);
			else if (i == 0) d = mTracts->GetCoord(it, i + 1) - p;
			else d = mTracts->GetCoord(it, i + 1) - mTracts->GetCoord(it, i - 1);

			MyVec3f perpend1 = (pole^d).normalized();
			MyVec3f perpend2 = (perpend1^d).normalized();
			for (int is = 0; is<mRenderingParameters.Faces; is++){
				float angle = dangle*is;
				MyVec3f pt = sin(angle)*perpend1 + cos(angle)*perpend2;
				mVertices[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = pt * R + p;
				mNormals[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = pt;
			}
			mVertices[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = mVertices[currentIdx + i*(mRenderingParameters.Faces + 1)];
			mNormals[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = mNormals[currentIdx + i*(mRenderingParameters.Faces + 1)];
		}

		mIdxOffset << currentIdx;
		currentIdx += npoints*(mRenderingParameters.Faces + 1);

		float capTexCoordx;
		if (mRenderingParameters.CapType == CAP_TYPE_ROUND){
			capTexCoordx = 0;
		}
		else capTexCoordx = -0.001;
		// add front cap
		{
			MyVec3f p = mTracts->GetCoord(it, 0);
			MyVec3f d = mTracts->GetCoord(it, 1) - p;
			d.normalize();
			MyVec3f perpend1 = (pole^d).normalized();
			MyVec3f perpend2 = (perpend1^d).normalized();
			mVertices[currentIdx] = p;
			mNormals[currentIdx] = -d;
			for (int is = 0; is < mRenderingParameters.Faces; is++){
				float angle = dangle*is;
				MyVec3f pt = sin(angle)*perpend1 + cos(angle)*perpend2;
				mVertices[currentIdx + is + 1] = pt*R + p;
				if (mRenderingParameters.CapType == CAP_TYPE_ROUND){
					mNormals[currentIdx + is + 1] = pt;
				}
				else {
					mNormals[currentIdx + is + 1] = -d;
				}
			}
		}

		// add back cap
		currentIdx += mRenderingParameters.Faces + 1;
		{
			MyVec3f p = mTracts->GetCoord(it, npoints - 1);
			MyVec3f d = mTracts->GetCoord(it, npoints - 2) - p;
			d.normalize();
			MyVec3f perpend1 = (pole^d).normalized();
			MyVec3f perpend2 = (perpend1^d).normalized();
			mVertices[currentIdx] = p;
			mNormals[currentIdx] = -d;
			for (int is = 0; is < mRenderingParameters.Faces; is++){
				float angle = dangle*is;
				MyVec3f pt = sin(angle)*perpend1 + cos(angle)*perpend2;
				mVertices[currentIdx + is + 1] = pt*R + p;
				if (mRenderingParameters.CapType == CAP_TYPE_ROUND){
					mNormals[currentIdx + is + 1] = pt;
				}
				else {
					mNormals[currentIdx + is + 1] = -d;
				}
			}
		}
		currentIdx += mRenderingParameters.Faces + 1;
	}
	// index
	mIndices.clear();
	for (int it = 0; it<mTracts->GetNumTracks(); it++){
		int offset = mIdxOffset[it];
		for (int i = 1; i<mTracts->GetNumVertex(it); i++){
			for (int j = 0; j < mRenderingParameters.Faces; j++){
				/*
				// wrong direction!
				mIndices << MyVec3i((i - 1)*(mRenderingParameters.Faces + 1) + j + offset,
				(i)*(mRenderingParameters.Faces + 1) + j + offset,
				(i)*(mRenderingParameters.Faces + 1) + (j + 1) + offset);
				mIndices << MyVec3i((i - 1)*(mRenderingParameters.Faces + 1) + j + offset,
				(i)*(mRenderingParameters.Faces + 1) + (j + 1) + offset,
				(i - 1)*(mRenderingParameters.Faces + 1) + (j + 1) + offset);
				*/
				// fixed direction
				mIndices << MyVec3i((i - 1)*(mRenderingParameters.Faces + 1) + j + offset,
					(i)*(mRenderingParameters.Faces + 1) + (j + 1) + offset,
					(i)*(mRenderingParameters.Faces + 1) + j + offset);
				mIndices << MyVec3i((i - 1)*(mRenderingParameters.Faces + 1) + j + offset,
					(i - 1)*(mRenderingParameters.Faces + 1) + (j + 1) + offset,
					(i)*(mRenderingParameters.Faces + 1) + (j + 1) + offset);
			}
		}
		// add caps
		offset += mTracts->GetNumVertex(it)*(mRenderingParameters.Faces + 1);
		for (int j = 0; j < mRenderingParameters.Faces; j++){
			mIndices << MyVec3i(offset, offset + (j + 1) % (mRenderingParameters.Faces) + 1, offset + j + 1);
		}
		offset += 1 + mRenderingParameters.Faces;
		for (int j = 0; j < mRenderingParameters.Faces; j++){
			mIndices << MyVec3i(offset, offset + (j + 1) % (mRenderingParameters.Faces) + 1, offset + j + 1);
		}
	}
	cout << "Computing completed.\r";
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
	//glUniform1f(radiusLocation, mTrackRadius);
	glUniform1f(radiusLocation, 0);

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