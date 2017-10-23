#include "MyTrackCurv.h"
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

MyTrackCurv::MyTrackCurv()
{
}


MyTrackCurv::~MyTrackCurv()
{
}

void MyTrackCurv::LoadGeometry(){
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
	// curvature
	if (glIsBuffer(mCurvatureBuffer)){
		glDeleteBuffers(1, &mCurvatureBuffer);
	}
	glGenBuffers(1, &mCurvatureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mCurvatureBuffer);
	glBufferData(GL_ARRAY_BUFFER, mCurvatures.size() * sizeof(float), &mCurvatures[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mCurvatureAttribute);
	glVertexAttribPointer(mCurvatureAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
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

void MyTrackCurv::LoadShader(){

	glDeleteProgram(mShaderProgram);
	mShaderProgram = InitShader("shaders\\curvature.vert", "shaders\\curvature.frag", "fragColour");

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
	mCurvatureAttribute = glGetAttribLocation(mShaderProgram, "curvature");
	if (mCurvatureAttribute < 0) {
		cerr << "Shader did not contain the 'curvature' attribute." << endl;
	}
}


void MyTrackCurv::ComputeGeometry(){
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

			if (j == 0 || j == mTracts->At(i).Size()-1){
				mCurvatures << 0 << 0;
			}
			else{
				MyVec3f vpre = mTracts->GetCoord(i, j - 1) - mTracts->GetCoord(i, j);
				MyVec3f vpst = mTracts->GetCoord(i, j + 1) - mTracts->GetCoord(i, j);
				vpre.normalize(); vpst.normalize();
// formula page 4 ftp://ftp.disi.unige.it/person/MagilloP/PDF/reportDISI-TR-11-12.pdf
				float cosGamma = vpre*vpst;
				if (cosGamma < -1) cosGamma = -1;
				else if (cosGamma > 1) cosGamma = 1;
				float gamma = acos(cosGamma) * 180 / MY_PI;
				float beta = 90 + gamma / 2;
				float curvature = (1 - gamma / 180);
				if (!(curvature <= 1)) 
					curvature = 1;
				else if (!(curvature >= 0)) 
					curvature = 0;
				mCurvatures << curvature << curvature;
			}

			if (j < mTracts->At(i).Size() - 1){
				mIndices << MyVec3i(currentVertexIdx + j * 2 + 0,
					currentVertexIdx + j * 2 + 2, currentVertexIdx + j * 2 + 3);
				mIndices << MyVec3i(currentVertexIdx + j * 2 + 0,
					currentVertexIdx + j * 2 + 3, currentVertexIdx + j * 2 + 1);
			}
		}

		currentVertexIdx += mTracts->At(i).Size() * 2;

		mIdxOffset << currentIndexIdx;
		currentIndexIdx = mIndices.size() * 3;
	}
}


void MyTrackCurv::Show(){
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

	int radiusLocation = glGetUniformLocation(mShaderProgram, "radius");
	glUniform1f(radiusLocation, mTrackRadius);

	for (int i = 0; i < mFiberToDraw.size(); i++){
		int fiberIdx = mFiberToDraw[i];
		int offset = mIdxOffset[fiberIdx];
		int numVertex = (mTracts->GetNumVertex(fiberIdx) - 1) * 6;
		glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
	}

	/*
	// draw all
	int numVertex = 0;
	for (int i = 0; i < mTracts->GetNumTracks(); i++){
	numVertex += (this->GetNumVertex(i) - 1) * 6;
	}
	glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_INT, 0);
	*/

	glUseProgram(0);
	glBindVertexArray(0);
	glPopAttrib();
}