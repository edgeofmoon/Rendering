#include "MyTrackDDH.h"
#include "MyTrackRings.h"
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

MyTrackDDH::MyTrackDDH()
{
	mStripWidth = 2;
	mStripDepth = 0.01;
	mStrokeWidth = 0.25;
	mTaperLength = 1;
	mDepthCueing = 0;
}


MyTrackDDH::~MyTrackDDH()
{
}


void MyTrackDDH::LoadGeometry(){
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
	// tagent
	if (glIsBuffer(mTangentBuffer)){
		glDeleteBuffers(1, &mTangentBuffer);
	}
	glGenBuffers(1, &mTangentBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mTangentBuffer);
	glBufferData(GL_ARRAY_BUFFER, mTangents.size() * sizeof(MyVec3f), &mTangents[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mTangentAttribute);
	glVertexAttribPointer(mTangentAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// texCoord
	if (glIsBuffer(mTextureCoordBuffer)){
		glDeleteBuffers(1, &mTextureCoordBuffer);
	}
	glGenBuffers(1, &mTextureCoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mTextureCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, mTextureCoords.size() * sizeof(MyVec3f), &mTextureCoords[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mTextureCoordAttribute);
	glVertexAttribPointer(mTextureCoordAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);

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

void MyTrackDDH::LoadShader(){

	glDeleteProgram(mShaderProgram);
	mShaderProgram = InitShader("shaders\\ddh.vert", "shaders\\ddh.frag", "fragColour");

	mPositionAttribute = glGetAttribLocation(mShaderProgram, "position");
	if (mPositionAttribute < 0) {
		cerr << "Shader did not contain the 'position' attribute." << endl;
	}
	mTextureCoordAttribute = glGetAttribLocation(mShaderProgram, "texcoord");
	if (mTextureCoordAttribute < 0) {
		cerr << "Shader did not contain the 'texcoord' attribute." << endl;
	}
	mTangentAttribute = glGetAttribLocation(mShaderProgram, "tangent");
	if (mTangentAttribute < 0) {
		cerr << "Shader did not contain the 'tagent' attribute." << endl;
	}
}


void MyTrackDDH::ComputeGeometry(){
	ClearGeometry();

	int totalPoints = 0;
	for (int it = 0; it < mTracts->GetNumTracks(); it++){
		totalPoints += mTracts->At(it).Size();
	}
	int currentVertexIdx = 0;
	int currentIndexIdx = 0;
	mVertices.reserve(totalPoints * 2);
	mTextureCoords.reserve(totalPoints * 2);
	for (int i = 0; i < mTracts->GetNumTracks(); i++){
		float u = 0;
		float len = mTracts->ComputeTrackLength(i);
		for (int j = 0; j < mTracts->At(i).Size(); j++){
			// push twice to make a pair
			mVertices << mTracts->GetCoord(i, j) << mTracts->GetCoord(i, j);

			u += (mTracts->GetCoord(i, j) - mTracts->GetCoord(i, j>0 ? j - 1 : 0)).norm();
			mTextureCoords << MyVec3f(u, 0, len) << MyVec3f(u, 1, len);

			MyVec3f tangent;

			if (j == mTracts->At(i).Size() - 1)
				tangent = mTracts->GetCoord(i, j) - mTracts->GetCoord(i, j - 1);
			else 
				tangent = mTracts->GetCoord(i, j + 1) - mTracts->GetCoord(i, j);
			tangent.normalize();
			mTangents << tangent << tangent;

			if (j < mTracts->At(i).Size() - 1){
				mIndices << MyVec3i(currentVertexIdx + j * 2 + 0,
					currentVertexIdx + j * 2 + 2, currentVertexIdx + j * 2 + 3);
				mIndices << MyVec3i(currentVertexIdx + j * 2 + 0,
					currentVertexIdx + j * 2 + 3, currentVertexIdx + j * 2 + 1);
			}
		}

		currentVertexIdx += mTracts->At(i).Size() * 2;

		mIdxOffset << currentIndexIdx;
		currentIndexIdx = mIndices.size()*3;
	}
}


void MyTrackDDH::Show(){
	// load color texture

	//if (mShape == TRACK_SHAPE_TUBE){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(mVertexArray);
	glUseProgram(mShaderProgram);

	int loc = glGetUniformLocation(mShaderProgram, "mvMat");
	float modelViewMat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMat);
	glUniformMatrix4fv(loc, 1, GL_FALSE, modelViewMat);

	loc = glGetUniformLocation(mShaderProgram, "mvMatInv");
	float mvMatInv[16];
	bool bInv = MyMathHelper::InvertMatrix4x4ColMaj(modelViewMat, mvMatInv);
	assert(bInv);
	glUniformMatrix4fv(loc, 1, GL_FALSE, mvMatInv);

	loc = glGetUniformLocation(mShaderProgram, "projMat");
	float projMat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
	glUniformMatrix4fv(loc, 1, GL_FALSE, projMat);

	loc = glGetUniformLocation(mShaderProgram, "stripWidth");
	glUniform1f(loc, mStripWidth);
	loc = glGetUniformLocation(mShaderProgram, "stripDepth");
	glUniform1f(loc, mStripDepth);
	loc = glGetUniformLocation(mShaderProgram, "strokeWidth");
	glUniform1f(loc, mStrokeWidth);
	loc = glGetUniformLocation(mShaderProgram, "taperLength");
	glUniform1f(loc, mTaperLength);
	loc = glGetUniformLocation(mShaderProgram, "depthCueing");
	glUniform1f(loc, mDepthCueing);

	for (int i = 0; i < mFiberToDraw.size(); i++){
		int fiberIdx = mFiberToDraw[i];
		int offset = mIdxOffset[fiberIdx];
		int numVertex = (mTracts->GetNumVertex(fiberIdx)-1)*6;
		glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
	}

	/*
	// draw all
	int numVertex = 0;
	for (int i = 0; i < mTracks.size(); i++){
		numVertex += (this->GetNumVertex(i) - 1) * 6;
	}
	glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_INT, 0);
	*/

	glUseProgram(0);
	glBindVertexArray(0);
	glPopAttrib();
}