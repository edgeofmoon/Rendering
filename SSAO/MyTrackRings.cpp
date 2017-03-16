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

#include "GL\glew.h"
#include <GL/freeglut.h>

MyTrackRings::MyTrackRings()
{
	mRingFaces = 6;
	mRingRadius = 0.05;
}


MyTrackRings::~MyTrackRings()
{
}

/*
void MyTrackRings::ComputeTubeGeometry(){
	MyTracks::ComputeTubeGeometry();
	int currentIdx = 0;
	for (int it = 0; it < mTracks.size(); it++){
		int npoints = mTracks[it].mSize;
		for (int i = 0; i<npoints; i++){
			MyVec3f p = this->GetCoord(it, i);
			float size = 0.4;
			//float size = 0;
			MyVec3f d;
			if (i == npoints - 1){
				d = p - this->GetCoord(it, i - 1);
			}
			else if (i == 0){
				d = this->GetCoord(it, i + 1) - p;
			}
			else{
				d = this->GetCoord(it, i + 1) - this->GetCoord(it, i - 1);
			}
			for (int is = 0; is<mFaces; is++){
				MyColor4f& color = mColors[currentIdx + i*(mFaces + 1) + is];
				MyVec3f  tagent = d.normalized();
				color.r = tagent[0];
				color.g = tagent[1];
				color.b = tagent[2];
				color.a = i;
			}
			mColors[currentIdx + i*(mFaces + 1) + mFaces] = mColors[currentIdx + i*(mFaces + 1)];
		}
		currentIdx += npoints*(mFaces + 1);
		// add front cap
		{
			mColors[currentIdx] = mColors[currentIdx - npoints*(mFaces + 1)];
			mColors[currentIdx].a = -1;
			for (int is = 0; is < mFaces; is++){
				mColors[currentIdx + is + 1] = mColors[currentIdx - npoints*(mFaces + 1)];
				mColors[currentIdx + is + 1].a = -1;
			}
		}
		// add back cap
		currentIdx += mFaces + 1;
		{
			mColors[currentIdx] = mColors[currentIdx - (mFaces + 2)];
			mColors[currentIdx].a = -1;
			for (int is = 0; is < mFaces; is++){
				mColors[currentIdx + is + 1] = mColors[currentIdx - (mFaces + 2)];
				mColors[currentIdx + is + 1].a = -1;
			}
		}
		currentIdx += mFaces + 1;
	}
}
*/


void MyTrackRings::ComputeTubeGeometry(){
	int currentIdx = 0;
	mIdxOffset.clear();
	int totalPoints = 0;
	for (int it = 0; it < mTracks.size(); it++){
		totalPoints += mTracks[it].mSize;
	}
	totalPoints *= mFaces * mRingFaces;

	cout << "Allocating Storage for Geometry...\r";

	mVertices.resize(totalPoints);
	mNormals.resize(totalPoints);
	//mTexCoords.resize(totalPoints);
	//mRadius.resize(totalPoints);
	mColors.resize(totalPoints);
	mOrbits.resize(totalPoints);

	for (int it = 0; it < mTracks.size(); it++){
		if ((int)((it + 1) * 100 / (float)mTracks.size())
			- (int)(it * 100 / (float)mTracks.size()) >= 1){
			cout << "Computing: " << it*100.f / mTracks.size() << "%.          \r";
		}
		int npoints = mTracks[it].mSize;

		const float myPI = 3.1415926f;
		float dangle = 2 * myPI / mFaces;
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
		MyVec3f genDir = this->GetCoord(it, 0) - this->GetCoord(it, npoints - 1);
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
			MyVec3f p = this->GetCoord(it, i);
			float size = 0.4;
			//float size = 0;
			MyVec3f d;
			if (i == npoints - 1){
				d = p - this->GetCoord(it, i - 1);
			}
			else if (i == 0){
				d = this->GetCoord(it, i + 1) - p;
			}
			else{
				d = this->GetCoord(it, i + 1) - this->GetCoord(it, i - 1);
			}

			MyVec3f perpend1 = (pole^d).normalized();
			MyVec3f perpend2 = (perpend1^d).normalized();

			//if ((perpend1^perpend2)*d < 0) dangle = -dangle;
			for (int is = 0; is<mFaces; is++){
				float angle = dangle*is;
				MyVec3f pt = sin(angle)*perpend1 + cos(angle)*perpend2;
				MyVec3f anchor = pt * mTrackRadius + p;
				MyVec3f  tangent = d.normalized();
				for (int ir = 0; ir < mRingFaces; ir++){
					float ringAngle = 2 * myPI / mRingFaces * ir;
					MyVec3f ptr = sin(ringAngle)*pt + cos(ringAngle)*tangent;
					MyVec3f ringVertex = anchor + ptr*mRingRadius;
					int vertexIdx = currentIdx + i*mFaces*mRingFaces + is*mRingFaces + ir;
					mVertices[vertexIdx] = ringVertex;
					mNormals[vertexIdx] = ptr;
					if (mHeader.n_scalars == 3){
						mColors[vertexIdx] = MyColor4f(
							mTracks[it].mPointScalars[i][0], mTracks[it].mPointScalars[i][1],
							mTracks[it].mPointScalars[i][2], 1);
					}
					else if (mHeader.n_scalars == 13){
						mColors[vertexIdx] = MyColor4f(
							1, 1, 1-mTracks[it].mPointScalars[i][0], 1);
					}
					else mColors[vertexIdx] = MyColor4f(1, 1, 1, 1);
					mNormals[vertexIdx] = ptr;
					mOrbits[vertexIdx] = pt;
					// rewrite to encode info
					float fa = 1 - mColors[vertexIdx].b;
					mColors[vertexIdx].r = tangent[0] * fa;
					mColors[vertexIdx].g = tangent[1] * fa;
					mColors[vertexIdx].b = tangent[2] * fa;
					if (i == npoints - 1 || i == 0){
						mColors[vertexIdx].a = -1;
					}
				}
			}
		}

		mIdxOffset << currentIdx;
		currentIdx += npoints*mFaces*mRingFaces;
	}
	// index
	mIndices.clear();
	for (int it = 0; it<this->GetNumTracks(); it++){
		int offset = mIdxOffset[it];
		for (int i = 0; i<this->GetNumVertex(it); i++){
			int segStrIdx = offset + i*mFaces*mRingFaces;
			for (int j = 0; j < mFaces; j++){
				for (int k = 0; k < mRingFaces; k++){
					mIndices << MyVec3i(segStrIdx + j*mRingFaces + k,
						segStrIdx + ((j + 1) % mFaces)*mRingFaces + (k + 1) % mRingFaces,
						segStrIdx + ((j + 1) % mFaces)*mRingFaces + k
						);
					mIndices << MyVec3i(segStrIdx + j*mRingFaces + k,
						segStrIdx + j*mRingFaces + (k + 1) % mRingFaces,
						segStrIdx + ((j + 1) % mFaces)*mRingFaces + (k + 1) % mRingFaces
						);
				}
			}
		}
	}
	cout << "Computing completed.\n";
}

void MyTrackRings::LoadGeometry(){
	if (mShape == TRACK_SHAPE_LINE){
		//	return;
	}
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
	// color
	if (glIsBuffer(mColorBuffer)){
		glDeleteBuffers(1, &mColorBuffer);
	}
	glGenBuffers(1, &mColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, mColors.size() * sizeof(MyColor4f), &mColors[0].r, GL_STATIC_DRAW);
	glEnableVertexAttribArray(mColorAttribute);
	glVertexAttribPointer(mColorAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
	// orbit
	if (glIsBuffer(mOrbitBuffer)){
		glDeleteBuffers(1, &mOrbitBuffer);
	}
	glGenBuffers(1, &mOrbitBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mOrbitBuffer);
	glBufferData(GL_ARRAY_BUFFER, mOrbits.size() * sizeof(MyVec3f), &mOrbits[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mOrbitAttribute);
	glVertexAttribPointer(mOrbitAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	/*
	// texCoord
	if (glIsBuffer(mTexCoordBuffer)){
	glDeleteBuffers(1, &mTexCoordBuffer);
	}
	glGenBuffers(1, &mTexCoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mTexCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, mTexCoords.size() * sizeof(MyVec2f), &mTexCoords[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mTexCoordAttribute);
	glVertexAttribPointer(mTexCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	// radius
	if (glIsBuffer(mRadiusBuffer)){
	glDeleteBuffers(1, &mRadiusBuffer);
	}
	glGenBuffers(1, &mRadiusBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mRadiusBuffer);
	glBufferData(GL_ARRAY_BUFFER, mRadius.size() * sizeof(float), &mRadius[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mRadiusAttribute);
	glVertexAttribPointer(mRadiusAttribute, 1, GL_FLOAT, GL_FALSE, 0, 0);
	*/
	// texture
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, texture);
	// index
	if (glIsBuffer(mIndexBuffer)){
		glDeleteBuffers(1, &mIndexBuffer);
	}
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	if (mShape == TRACK_SHAPE_TUBE && mIndices.size()>0){
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(MyVec3i), &mIndices[0][0], GL_STATIC_DRAW);
	}
	else if (mShape == TRACK_SHAPE_LINE && mLineIndices.size()>0){
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mLineIndices.size() * sizeof(int), &mLineIndices[0], GL_STATIC_DRAW);
	}
	else{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
	}
	// unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// now free everything
	ClearGeometry();
}

void MyTrackRings::LoadShader(){
	if (mShape == TRACK_SHAPE_LINE){
		//	return;
	}

	glDeleteProgram(mShaderProgram);
	mShaderProgram = InitShader("Shaders\\ring.vert", "Shaders\\ring.frag", "fragColour", "position");
	//mShaderProgram = InitShader("Shaders\\tracks.vert", "Shaders\\tracks.frag", "fragColour");

	mNormalAttribute = glGetAttribLocation(mShaderProgram, "normal");
	if (mNormalAttribute < 0) {
		cerr << "Shader did not contain the 'normal' attribute." << endl;
	}
	mPositionAttribute = glGetAttribLocation(mShaderProgram, "position");
	if (mPositionAttribute < 0) {
		cerr << "Shader did not contain the 'position' attribute." << endl;
	}
	mColorAttribute = glGetAttribLocation(mShaderProgram, "color");
	if (mColorAttribute < 0) {
		cerr << "Shader did not contain the 'color' attribute." << endl;
	}
	mOrbitAttribute = glGetAttribLocation(mShaderProgram, "orbit");
	if (mOrbitAttribute < 0) {
		cerr << "Shader did not contain the 'orbit' attribute." << endl;
	}
}


void MyTrackRings::ComputeGeometry(){
	ClearGeometry();
	this->ComputeTubeGeometry();
}


void MyTrackRings::Show(){
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

	int colorLocation = glGetUniformLocation(mShaderProgram, "color");
	glUniform3f(colorLocation, 1, 1, 1);

	int radiusLocation = glGetUniformLocation(mShaderProgram, "radius");
	if (mShape == TRACK_SHAPE_LINE){
		glUniform1f(radiusLocation, 0);
	}
	else{
		glUniform1f(radiusLocation, mTrackRadius);
	}

	int offset0Location = glGetUniformLocation(mShaderProgram, "offset0");
	glUniform3fv(offset0Location, 1, &mBoxOffset0[0]);

	int offset1Location = glGetUniformLocation(mShaderProgram, "offset1");
	glUniform3fv(offset1Location, 1, &mBoxOffset1[0]);

	int textureLocation = glGetUniformLocation(mShaderProgram, "colorTex");
	glUniform1i(textureLocation, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	unsigned int texture = MyBlackBodyColor::GetTexture();
	glBindTexture(GL_TEXTURE_1D, texture);

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
			int offset = mIdxOffset[fiberIdx] * 6;
			int numVertex = this->GetNumVertex(fiberIdx) * mFaces * mRingFaces * 6;
			glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
		}
	}

	glUseProgram(0);
	glBindVertexArray(0);
	glPopAttrib();
	//}
	//else{
	//	int offset = 0;
	//	for (int i = 0; i < mFiberToDraw.size(); i++){
	//		int fiberIdx = mFiberToDraw[i];
	//		int numVertex = this->GetNumVertex(fiberIdx);
	//		glBegin(GL_LINE_STRIP);
	//		for (int j = 0; j < numVertex; j++){
	//			MyGraphicsTool::Normal(mNormals[mLineIndices[offset + j]]);
	//			MyGraphicsTool::Vertex(mVertices[mLineIndices[offset + j]]);
	//		}
	//		glEnd();
	//		offset += numVertex;
	//	}
	//}
}