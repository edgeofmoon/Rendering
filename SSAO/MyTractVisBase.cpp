#include "MyTractVisBase.h"

#include "MyVec.h"
#include "Shader.h"
#include "MyGraphicsTool.h"
#include "MyMathHelper.h"
#include "MyBlackBodyColor.h"
#include "MySuperquadric.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <cassert>
#include <omp.h>
using namespace std;

#include "GL\glew.h"
#include <GL/freeglut.h>

MyTractVisBase::RenderingParameters MyTractVisBase::DefaultRenderingParameters = {
	MyTractVisBase::TRACK_SHAPE_TUBE,
	20,
	0,
	0,
	MyColor4f(1, 1, 1, 0),
	0,
	MyColor4f(0.5, 0.5, 0.5, 1),
	1,
	0,
	1,
	0,
	32,
	1,
	0,
	0,
	0
};

MyTractVisBase::MyTractVisBase(){
	ResetRenderingParameters();
}

MyTractVisBase::~MyTractVisBase()
{
	if (glIsProgram(mShaderProgram)) glDeleteProgram(mShaderProgram);
	if (glIsVertexArray(mVertexArray)) glDeleteVertexArrays(1, &mVertexArray);
	if (glIsBuffer(mVertexBuffer)) glDeleteBuffers(1, &mVertexBuffer);
	if (glIsBuffer(mNormalBuffer)) glDeleteBuffers(1, &mNormalBuffer);
	if (glIsBuffer(mTexCoordBuffer)) glDeleteBuffers(1, &mTexCoordBuffer);
	if (glIsBuffer(mColorBuffer)) glDeleteBuffers(1, &mColorBuffer);
	if (glIsBuffer(mValueBuffer)) glDeleteBuffers(1, &mValueBuffer);
	if (glIsBuffer(mIndexBuffer)) glDeleteBuffers(1, &mIndexBuffer);
}


void MyTractVisBase::SetTracts(const MyTracks* tracts){
	mTracts = tracts;
	mFiberToDraw = MyArrayi::GetSequence(0, tracts->GetNumTracks() - 1);
}

void MyTractVisBase::ResetRenderingParameters(){
	//assuming superquadric has radius 0.5
	//average (1-cl)^3=0.46
	//average (1-cp)^3=0.59
	//which will have average max area (gamma=3):
	//integration 0 to pi/2: 4*(0.5*0.5)*(cosx)^0.460*(sinx)^0.593 = 0.8247
	//4 is for each quarter on x/y plan, 0.5 is base glyph radius
	//average 1/2 will be projected
	//for tube, projected will always be 2*R*1
	//1 is length is sample distance
	//let 2*R=0.8247/2, we get R=0.206
	mTrackRadius = 0.206;
	mRenderingParameters = DefaultRenderingParameters;
	mSphereGeometry = NULL;
}

void MyTractVisBase::ComputeTubeGeometry(){
	int currentIdx = 0;
	mIdxOffset.clear();
	int totalPoints = 0;
	for (int it = 0; it < mTracts->GetNumTracks(); it++){
		totalPoints += mTracts->At(it).Size();
	}
	totalPoints *= (mRenderingParameters.Faces + 1);

	// for caps
	totalPoints += mTracts->GetNumTracks()*(1 + mRenderingParameters.Faces) * 2;

	cout << "Allocating Storage for Geometry...\r";

	mVertices.resize(totalPoints);
	mNormals.resize(totalPoints);
	mTexCoords.resize(totalPoints);
	//mRadius.resize(totalPoints);
	mColors.resize(totalPoints);
	mValues.resize(totalPoints);

	float R = 0;

	for (int it = 0; it < mTracts->GetNumTracks(); it++){
		PrintProgress(it, mTracts->GetNumTracks(), 1);
		int npoints = mTracts->At(it).Size();

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
			//if ((perpend1^perpend2)*d < 0) dangle = -dangle;
			float fa = mTracts->GetTensor(it, i).GetFA();
			if (fa > 1) fa = 1;
			else if (fa < 0) fa = 0;
			for (int is = 0; is<mRenderingParameters.Faces; is++){
				float angle = dangle*is;
				MyVec3f pt = sin(angle)*perpend1 + cos(angle)*perpend2;
				mVertices[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = pt * R + p;
				mNormals[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = pt;
				mColors[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = mTracts->GetTrackColor(it);
				mTexCoords[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = MyVec2f(i, is / (float)mRenderingParameters.Faces);
				mValues[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = fa;
				/*
				if (mTracts->GetHeader().n_scalars == 3){
					mColors[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = MyColor4f(
						mTracts->At(it).mPointScalars[i][0], mTracts->At(it).mPointScalars[i][1],
						mTracts->At(it).mPointScalars[i][2], 1);
				}
				else if (mTracts->GetHeader().n_scalars == 13){
					mColors[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = MyColor4f(
						1, 1, 1 - mTracts->At(it).mPointScalars[i][0], 1);
				}
				else mColors[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = MyColor4f(1, 1, 1, 1);
				*/
			}
			mVertices[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = mVertices[currentIdx + i*(mRenderingParameters.Faces + 1)];
			mNormals[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = mNormals[currentIdx + i*(mRenderingParameters.Faces + 1)];
			//mTexCoords[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = MyVec2f(i, 1);
			//mRadius[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = mRadius[currentIdx + i*(mRenderingParameters.Faces + 1)];
			mColors[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = mColors[currentIdx + i*(mRenderingParameters.Faces + 1)];
			mTexCoords[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = MyVec2f(i, 1);
			mValues[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = fa;
		}

		mIdxOffset << currentIdx;
		currentIdx += npoints*(mRenderingParameters.Faces + 1);

		// add front cap
		{
			float fa = mTracts->GetTensor(it, 0).GetFA();
			if (fa > 1) fa = 1;
			else if (fa < 0) fa = 0;
			MyVec3f p = mTracts->GetCoord(it, 0);
			MyVec3f d = mTracts->GetCoord(it, 1) - p;
			d.normalize();
			MyVec3f perpend1 = (pole^d).normalized();
			MyVec3f perpend2 = (perpend1^d).normalized();
			mVertices[currentIdx] = p;
			mNormals[currentIdx] = -d;
			mColors[currentIdx] = mColors[currentIdx - npoints*(mRenderingParameters.Faces + 1)];
			mTexCoords[currentIdx] = MyVec2f(0, 0.5);
			mValues[currentIdx] = fa;
			for (int is = 0; is < mRenderingParameters.Faces; is++){
				float angle = dangle*is;
				MyVec3f pt = sin(angle)*perpend1 + cos(angle)*perpend2;
				MyVec3f pe = pt*R + p;
				mVertices[currentIdx + is + 1] = pe;
				mNormals[currentIdx + is + 1] = pt;
				//mNormals[currentIdx + is + 1] = -d;
				mColors[currentIdx + is + 1] = mColors[currentIdx - npoints*(mRenderingParameters.Faces + 1)];
				mTexCoords[currentIdx + is + 1] = MyVec2f(0, is / (float)mRenderingParameters.Faces);
				mValues[currentIdx + is + 1] = fa;
			}
		}

		// add back cap
		currentIdx += mRenderingParameters.Faces + 1;
		{
			float fa = mTracts->GetTensor(it, npoints - 1).GetFA();
			if (fa > 1) fa = 1;
			else if (fa < 0) fa = 0;
			MyVec3f p = mTracts->GetCoord(it, npoints - 1);
			MyVec3f d = mTracts->GetCoord(it, npoints - 2) - p;
			d.normalize();
			MyVec3f perpend1 = (pole^d).normalized();
			MyVec3f perpend2 = (perpend1^d).normalized();
			mVertices[currentIdx] = p;
			mNormals[currentIdx] = -d;
			mColors[currentIdx] = mColors[currentIdx - (mRenderingParameters.Faces + 2)];
			mTexCoords[currentIdx] = MyVec2f(0, 0.5);
			mValues[currentIdx] = fa;
			for (int is = 0; is < mRenderingParameters.Faces; is++){
				float angle = dangle*is;
				MyVec3f pt = sin(angle)*perpend1 + cos(angle)*perpend2;
				MyVec3f pe = pt*R + p;
				mVertices[currentIdx + is + 1] = pe;
				mNormals[currentIdx + is + 1] = pt;
				//mNormals[currentIdx + is + 1] = -d;
				mColors[currentIdx + is + 1] = mColors[currentIdx - (mRenderingParameters.Faces + 2)];
				mTexCoords[currentIdx + is + 1] = MyVec2f(0, is / (float)mRenderingParameters.Faces);
				mValues[currentIdx + is + 1] = fa;
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
			mIndices << MyVec3i(offset, offset + (j + 1) % (mRenderingParameters.Faces)+1, offset + j + 1);
		}
		offset += 1 + mRenderingParameters.Faces;
		for (int j = 0; j < mRenderingParameters.Faces; j++){
			mIndices << MyVec3i(offset, offset + (j + 1) % (mRenderingParameters.Faces)+1, offset + j + 1);
		}
	}
	cout << "Computing completed.\n";
}

void MyTractVisBase::ComputeLineGeometry(){

	int currentIdx = 0;
	mIdxOffset.clear();
	int totalPoints = 0;
	for (int it = 0; it < mTracts->GetNumTracks(); it++){
		totalPoints += mTracts->At(it).Size();
	}

	cout << "Allocating Storage for Geometry...\r";

	mVertices.clear();
	mNormals.clear();
	mVertices.reserve(totalPoints);
	mNormals.reserve(totalPoints);
	mColors.reserve(totalPoints);
	mTexCoords.reserve(totalPoints);
	mValues.reserve(totalPoints);


	mIdxOffset.clear();
	mIdxOffset.reserve(mTracts->GetNumTracks());
	for (int it = 0; it < mTracts->GetNumTracks(); it++){
		PrintProgress(it, mTracts->GetNumTracks(), 1);
		int npoints = mTracts->At(it).Size();

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
			float size = 0.4;
			//float size = 0;
			MyVec3f d;
			if (i == npoints - 1) d = p - mTracts->GetCoord(it, i - 1);
			else if (i == 0) d = mTracts->GetCoord(it, i + 1) - p;
			else d = mTracts->GetCoord(it, i + 1) - mTracts->GetCoord(it, i - 1);

			MyVec3f perpend1 = (pole^d).normalized();
			MyVec3f perpend2 = (perpend1^d).normalized();

			//mVertices[currentIdx + i] = p;
			//mNormals[currentIdx + i] = perpend1;
			mVertices << p;
			// normal is tangent
			//mNormals << perpend1;
			mNormals << d.normalized();
			mColors << mTracts->GetTrackColor(it);
			mTexCoords << MyVec2f(i, 0);
			float fa = mTracts->GetTensor(it, i).GetFA();
			if (fa > 1) fa = 1;
			mValues << fa;
		}

		mIdxOffset << currentIdx;
		currentIdx += npoints;
	}

	mLineIndices.clear();
	mLineIndices.reserve(totalPoints);
	// index
	for (int it = 0; it<mTracts->GetNumTracks(); it++){
		int offset = mIdxOffset[it];
		for (int i = 0; i<mTracts->GetNumVertex(it); i++){
			mLineIndices << i + offset;
		}
	}
	cout << "Computing completed.\n";
}

void MyTractVisBase::ComputeSuperquadricGeometry(){

	mIdxOffset.clear();
	int totalPoints = 0;
	MyArrayi prevVertexCount;
	prevVertexCount << 0;
	for (int it = 0; it < mTracts->GetNumTracks(); it++){
		totalPoints += mTracts->At(it).Size() / (1 + mRenderingParameters.SuperquadricSkip);
		prevVertexCount << totalPoints;
	}

	cout << "Allocating Storage for Geometry...\r";
	int vCount = MySuperquadric::GetVertexCount();
	int tCount = MySuperquadric::GetTriangleCount();
	int totalVertices = totalPoints*vCount;
	mVertices.resize(totalVertices);
	mNormals.resize(totalVertices);
	mColors.resize(totalVertices);
	mTexCoords.resize(totalVertices);
	mValues.resize(totalVertices);
	mIndices.resize(totalPoints*tCount);

	mIdxOffset.clear();
	mIdxOffset.resize(mTracts->GetNumTracks());

#pragma omp parallel for default(shared)
	for (int it = 0; it < mTracts->GetNumTracks(); it++){
		PrintProgress(it, mTracts->GetNumTracks(), 1);
		int vOffset = prevVertexCount[it] * vCount;
		int tOffset = prevVertexCount[it] * tCount;
		mIdxOffset[it] = tOffset * 3;
		MySuperquadric sqDrawer;
		sqDrawer.SetScale(0.5);
		for (int i = 0; i < mTracts->At(it).Size();
			i += (mRenderingParameters.SuperquadricSkip + 1)){
			int idx = i / (mRenderingParameters.SuperquadricSkip + 1);
			int thisVOffset = vOffset + idx*vCount;
			int thistOffset = tOffset + idx*tCount;
			MyTensor3f t = mTracts->GetTensor(it, i);
			t.NormalizeEigenVectors();
			sqDrawer.SetTensor(&t);
			sqDrawer.SetCenter(mTracts->At(it)[i]);
			sqDrawer.BuildOn(mVertices, mNormals, mTexCoords, mIndices, thisVOffset, thistOffset);
			// add color
			MyColor4f color = mTracts->GetTrackColor(it);
			for (int ic = 0; ic < vCount; ic++) mColors[thisVOffset + ic] = color;
			// add value
			float fa = t.GetFA();
			if (fa>1) fa = 1;
			else if (fa < 0) fa = 0;
			for (int ic = 0; ic < vCount; ic++) mValues[thisVOffset + ic] = fa;
		}
	}
	mIdxOffset << prevVertexCount.back()* tCount * 3;
	cout << "Computing completed.\n";
}

void MyTractVisBase::SetToInfluence(int idx){
	MyArray<float*> influences = { &mRenderingParameters.ColorInfluence, &mRenderingParameters.ValueToTextureInfluence,
		&mRenderingParameters.ValueToSizeInfluence, &mRenderingParameters.ValueToTextureRatioInfluence};
	for (int i = 0; i < influences.size(); i++){
		*(influences[i]) = 0;
	}
	if (idx >= 0 && idx < 4){
		*(influences[idx]) = 1;
	}
}

MyArrayf MyTractVisBase::GetInfluences() const{
	MyArrayf influences = { mRenderingParameters.ColorInfluence, 
		mRenderingParameters.ValueToTextureInfluence,
		mRenderingParameters.ValueToSizeInfluence, 
		mRenderingParameters.ValueToTextureRatioInfluence };
	return influences;
}

void MyTractVisBase::SetInfluences(const MyArrayf& influences){
	MyArray<float*> ifPtr = { &mRenderingParameters.ColorInfluence, 
		&mRenderingParameters.ValueToTextureInfluence,
		&mRenderingParameters.ValueToSizeInfluence, 
		&mRenderingParameters.ValueToTextureRatioInfluence };
	for (int i = 0; i < influences.size() && i<4; i++){
		*(ifPtr[i]) = influences[i];
	}
}

void MyTractVisBase::UpdateBoundingBox(){
	mBoundingBox = mTracts->ComputeBoundingBox(mFiberToDraw);
}

void MyTractVisBase::ClearGeometry(){
	mVertices.clear();
	mNormals.clear();
	mTexCoords.clear();
	//mRadius.clear();
	mColors.clear();
	mValues.clear();
	mIndices.clear();
	mLineIndices.clear();

}

void MyTractVisBase::ComputeGeometry(){
	ClearGeometry();
	//mShape = TRACK_SHAPE_SUPERQUADRIC;
	TrackShape shape = this->GetShape();
	if (shape == TRACK_SHAPE_TUBE) this->ComputeTubeGeometry();
	else if (shape == TRACK_SHAPE_LINE) this->ComputeLineGeometry();
	else if (shape == TRACK_SHAPE_SUPERQUADRIC) this->ComputeSuperquadricGeometry();
	else cerr << "Unresolved track shape: " << shape << endl;
}

void MyTractVisBase::Show(){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	float pixelHaloWidth = this->GetPixelHaloWidth();
	if (pixelHaloWidth == 0) DrawGeometry();
	else{
		MyColor4f baseColor = this->GetBaseColor();
		float ab = GetAmbient();
		float lineWidth;
		glGetFloatv(GL_LINE_WIDTH, &lineWidth);
		glLineWidth(lineWidth + pixelHaloWidth);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glPolygonMode(GL_BACK, GL_LINE);
		if (this->GetShape() == TRACK_SHAPE_TUBE
			|| this->GetShape() == TRACK_SHAPE_SUPERQUADRIC){
			glLineWidth(pixelHaloWidth);
		}
		SetBaseColor(this->GetPixelHaloColor());
		// wash out to white
		SetAmbient(100);
		DrawGeometry();
		glDisable(GL_BLEND);
		glPolygonMode(GL_FRONT, GL_FILL);
		glCullFace(GL_BACK);
		glDepthFunc(GL_LEQUAL);
		SetBaseColor(baseColor);
		SetAmbient(ab);
		glLineWidth(lineWidth);
		DrawGeometry();
	}
	glPopAttrib();
}

void MyTractVisBase::LoadGeometry(){

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
	// texCoord
	if (mTexCoords.size() > 0){
		if (glIsBuffer(mTexCoordBuffer)){
			glDeleteBuffers(1, &mTexCoordBuffer);
		}
		glGenBuffers(1, &mTexCoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, mTexCoordBuffer);
		glBufferData(GL_ARRAY_BUFFER, mTexCoords.size() * sizeof(MyVec2f), &mTexCoords[0][0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(mTexCoordAttribute);
		glVertexAttribPointer(mTexCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}
	// value
	if (mValues.size() > 0){
		if (glIsBuffer(mValueBuffer)){
			glDeleteBuffers(1, &mValueBuffer);
		}
		glGenBuffers(1, &mValueBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, mValueBuffer);
		glBufferData(GL_ARRAY_BUFFER, mValues.size() * sizeof(float), &mValues[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(mValueAttribute);
		glVertexAttribPointer(mValueAttribute, 1, GL_FLOAT, GL_FALSE, 0, 0);
	}
	/*
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
	if ((this->GetShape() == TRACK_SHAPE_TUBE || this->GetShape() == TRACK_SHAPE_SUPERQUADRIC) && mIndices.size()>0){
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(MyVec3i), &mIndices[0][0], GL_STATIC_DRAW);
	}
	else if (this->GetShape() == TRACK_SHAPE_LINE && mLineIndices.size()>0){
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mLineIndices.size() * sizeof(int), &mLineIndices[0], GL_STATIC_DRAW);
	}
	else{
		cerr << "Unresoved shape, no geometry index loaded!" << endl;
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
	}
	// unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// now free everything
	ClearGeometry();
}

void MyTractVisBase::LoadShader(){
	if(glIsProgram(mShaderProgram)) glDeleteProgram(mShaderProgram);
	mShaderProgram = InitShader("..\\SSAO\\Shaders\\geom.vert", "..\\SSAO\\Shaders\\geom.frag", "fragColour", "position");
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
	mTexCoordAttribute = glGetAttribLocation(mShaderProgram, "texCoord");
	if (mTexCoordAttribute < 0) {
		cerr << "Shader did not contain the 'texCoord' attribute." << endl;
	}
	mValueAttribute = glGetAttribLocation(mShaderProgram, "value");
	if (mValueAttribute < 0) {
		cerr << "Shader did not contain the 'value' attribute." << endl;
	}
}

void MyTractVisBase::DrawGeometry(){
	//if (mShape == TRACK_SHAPE_TUBE){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
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
	int shapeLocation = glGetUniformLocation(mShaderProgram, "shape");
	glUniform1i(shapeLocation, int(this->GetShape()));

	int textureLocation = glGetUniformLocation(mShaderProgram, "colorTex");
	glUniform1i(textureLocation, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	//unsigned int texture = MyBlackBodyColor::GetTexture();
	//glBindTexture(GL_TEXTURE_1D, texture);
	glBindTexture(GL_TEXTURE_2D, mRenderingParameters.Texture);

	glUniform4fv(glGetUniformLocation(mShaderProgram, "baseColor"), 1, &mRenderingParameters.BaseColor.r);
	glUniform1f(glGetUniformLocation(mShaderProgram, "lightIntensity"), mRenderingParameters.LightIntensity);
	glUniform1f(glGetUniformLocation(mShaderProgram, "ambient"), mRenderingParameters.Ambient);
	glUniform1f(glGetUniformLocation(mShaderProgram, "diffuse"), mRenderingParameters.Diffuse);
	glUniform1f(glGetUniformLocation(mShaderProgram, "specular"), mRenderingParameters.Specular);
	glUniform1f(glGetUniformLocation(mShaderProgram, "shininess"), mRenderingParameters.Shininess);

	glUniform1f(glGetUniformLocation(mShaderProgram, "colorInfluence"), mRenderingParameters.ColorInfluence);
	glUniform1f(glGetUniformLocation(mShaderProgram, "valueToSizeInfluence"), mRenderingParameters.ValueToSizeInfluence);
	glUniform1f(glGetUniformLocation(mShaderProgram, "valueToTextureInfluence"), mRenderingParameters.ValueToTextureInfluence);
	glUniform1f(glGetUniformLocation(mShaderProgram, "valueToTextureRatioInfluence"), mRenderingParameters.ValueToTextureRatioInfluence);

#ifdef RIC
	glEnable(GL_TEXTURE_3D);
	int filterVolLocation = glGetUniformLocation(mShaderProgram, "filterVol");
	glUniform1i(filterVolLocation, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_3D, mFilterVolumeTexture);
#endif
	if (this->GetShape() == TRACK_SHAPE_TUBE){
		glUniform1f(radiusLocation, mTrackRadius);
		//this has no cap considered
		for (int i = 0; i < mFiberToDraw.size(); i++){
			int fiberIdx = mFiberToDraw[i];
			//int offset = (mIdxOffset[fiberIdx] / (mRenderingParameters.Faces + 1) - fiberIdx)*mRenderingParameters.Faces * 6;
			//int numVertex = (this->GetNumVertex(fiberIdx) - 1)*(mRenderingParameters.Faces + 0) * 6;
			// add cap offset
			int offset = (mIdxOffset[fiberIdx] / (mRenderingParameters.Faces + 1) - fiberIdx * 2)*mRenderingParameters.Faces * 6;
			int numVertex = (mTracts->GetNumVertex(fiberIdx) - 1)*(mRenderingParameters.Faces + 0) * 6 + mRenderingParameters.Faces * 6;
			glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
			//glDrawElements(GL_LINE_STRIP, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
		}
	}
	else if (this->GetShape() == TRACK_SHAPE_SUPERQUADRIC){
		glUniform1f(radiusLocation, 0);
		for (int i = 0; i < mFiberToDraw.size(); i++){
			int fiberIdx = mFiberToDraw[i];
			int offset = mIdxOffset[fiberIdx];
			int numIndices = (mIdxOffset[fiberIdx + 1] - mIdxOffset[fiberIdx]);
			glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
		}
		//glDrawElements(GL_LINE_STRIP, mIdxOffset.back() * 3, GL_UNSIGNED_INT, 0);
	}
	else if (this->GetShape() == TRACK_SHAPE_LINE){
		glUniform1f(radiusLocation, 0);
		for (int i = 0; i < mFiberToDraw.size(); i++){
			int fiberIdx = mFiberToDraw[i];
			int offset = mIdxOffset[fiberIdx];
			int numVertex = mTracts->GetNumVertex(fiberIdx);
			glDrawElements(GL_LINE_STRIP, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
		}
	}
	else cerr << "Unresolved shape, no elements drawn!" << endl;

	if (mSphereGeometry){
		glUniform1f(radiusLocation, 0);
		glUniform1i(shapeLocation, 2);
		glUniform1f(glGetUniformLocation(mShaderProgram, "colorInfluence"), 0);
		glUniform1f(glGetUniformLocation(mShaderProgram, "valueToSizeInfluence"), 0);
		glUniform1f(glGetUniformLocation(mShaderProgram, "valueToTextureInfluence"), 0);
		glUniform1f(glGetUniformLocation(mShaderProgram, "valueToTextureRatioInfluence"), 0);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);
		//glPolygonMode(GL_BACK, GL_LINE);
		glLineWidth(mRenderingParameters.PixelHaloWidth);
		mSphereGeometry->DrawGeometry();
		//glPolygonMode(GL_FRONT, GL_FILL);
		//glCullFace(GL_BACK);
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


void MyTractVisBase::ShowCapsOnly(){
	//if (mShape == TRACK_SHAPE_TUBE){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_DEPTH_TEST);
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
	if (this->GetShape() == TRACK_SHAPE_LINE){
		glUniform1f(radiusLocation, 0);
	}
	else{
		glUniform1f(radiusLocation, mTrackRadius);
	}

#ifdef RIC
	glEnable(GL_TEXTURE_3D);
	int filterVolLocation = glGetUniformLocation(mShaderProgram, "filterVol");
	glUniform1i(filterVolLocation, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_3D, mFilterVolumeTexture);
#endif
	if (this->GetShape() == TRACK_SHAPE_TUBE){
		//this has no cap considered
		for (int i = 0; i < mFiberToDraw.size(); i++){
			int fiberIdx = mFiberToDraw[i];
			int offset = (mIdxOffset[fiberIdx] / (mRenderingParameters.Faces + 1) - fiberIdx * 2)*mRenderingParameters.Faces * 6;
			offset += (mTracts->GetNumVertex(fiberIdx) - 1)*(mRenderingParameters.Faces + 0) * 6;
			int numVertex = mRenderingParameters.Faces * 6;
			glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
		}
	}

	glUseProgram(0);
	glBindVertexArray(0);
	glPopAttrib();
}

void MyTractVisBase::PrintProgress(float current, float all, float step){
	if ((int)((current + step) * 100 / all)
		- (int)(current * 100 / all) >= 1){
		cout << "Computing: " << current * 100 / all << "%.          \r";
	}
}

#ifdef RIC
void MyTracks::GetVoxelIndex(const MyVec3f vertex, long &x, long &y, long &z) const{
	x = 181 - (long)vertex[0];
	y = 217 - (long)vertex[1];
	z = 181 - (long)vertex[2];
}

void MyTracks::MaskFiber(MyTracks* tracks, Array3D<float>* mask, int startIdx, int endIdx){
	for (int i = startIdx; i <= endIdx; i++){
		for (int j = 0; j < tracks->GetNumVertex(i); j++){
			long x, y, z;
			tracks->GetVoxelIndex(tracks->GetCoord(i, j), x, y, z);
			if (mask->operator()(x, y, z) > 0.5){
				tracks->mFiberDraw[i] = true;
				break;
			}
		}
	}
}

void MyTracks::FiberVolumeDensity(MyTracks* tracks,
	Array3D<atomic<int>>* density, const MyArrayi* indices, int startIdx, int endIdx){
	for (int i = startIdx; i <= endIdx; i++){
		int it = indices->at(i);
		long lastX = -1, lastY = -1, lastZ = -1;
		for (int j = 0; j < tracks->GetNumVertex(it); j++){
			long x, y, z;
			tracks->GetVoxelIndex(tracks->GetCoord(it, j), x, y, z);
			if (lastX != x || lastY != y || lastZ != z){
				density->operator()(x, y, z)++;
				lastX = x; lastY = y; lastZ = z;
			}
		}
	}
}

void MyTracks::FilterByVolumeMask(Array3D<float>& mask){
	if (mTracks.empty()) return;
	mFiberToDraw.clear();
	mFiberDraw = MyArrayb(mTracts->GetNumTracks(), false);
	// mask the fibers
	// serial edition
	/*
	for (int i = 0; i < this->GetNumTracks(); i++){
	for (int j = 0; j < this->GetNumVertex(i); j++){
	long x, y, z;
	this->GetVoxelIndex(mTracts->GetCoord(i, j), x, y, z);
	if (mask(x, y, z) > 0.5){
	mFiberToDraw << i;
	break;
	}
	}
	}
	*/

	//MaskFiber(this, &mask, 0, mTracts->GetNumTracks() - 1);

	// multi-thread edition
	int numThread = std::thread::hardware_concurrency() - 1;
	numThread = min(numThread, (int)mTracts->GetNumTracks());
	std::thread *tt = new std::thread[numThread - 1];
	float fiberPerThread = mTracts->GetNumTracks() / (float)numThread;
	for (int i = 0; i < numThread - 1; i++){
		int startIdx = fiberPerThread*i;
		int endIdx = fiberPerThread*(i + 1) - 1;
		tt[i] = std::thread(MaskFiber, this, &mask, startIdx, endIdx);
	}
	MaskFiber(this, &mask, fiberPerThread*(numThread - 1), mTracts->GetNumTracks() - 1);
	for (int i = 0; i < numThread - 1; i++){
		tt[i].join();
	}
	delete[] tt;
	for (int i = 0; i < mFiberDraw.size(); i++){
		if (mFiberDraw[i]){
			mFiberToDraw << i;
		}
	}

	//std::cout << "Filter: " << mFiberToDraw.size() << " fibers to be drawn.\n";
	/*
	// updating indices
	if (mShape == TRACK_SHAPE_TUBE){
	mIndices.clear();
	for (int itt = 0; itt<mFiberToDraw.size(); itt++){
	int it = mFiberToDraw[itt];
	int offset = mIdxOffset[it];
	for (int i = 1; i<this->GetNumVertex(it); i++){
	for (int j = 0; j <= mRenderingParameters.Faces; j++){
	mIndices << MyVec3i((i - 1)*(mRenderingParameters.Faces + 1) + j % (mRenderingParameters.Faces + 1) + offset,
	(i)*(mRenderingParameters.Faces + 1) + j % (mRenderingParameters.Faces + 1) + offset,
	(i)*(mRenderingParameters.Faces + 1) + (j + 1) % (mRenderingParameters.Faces + 1) + offset);
	mIndices << MyVec3i((i - 1)*(mRenderingParameters.Faces + 1) + j % (mRenderingParameters.Faces + 1) + offset,
	(i)*(mRenderingParameters.Faces + 1) + (j + 1) % (mRenderingParameters.Faces + 1) + offset,
	(i - 1)*(mRenderingParameters.Faces + 1) + (j + 1) % (mRenderingParameters.Faces + 1) + offset);
	}
	}
	}
	glBindVertexArray(mVertexArray);
	if (glIsBuffer(mIndexBuffer)){
	glDeleteBuffers(1, &mIndexBuffer);
	}
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	if (mIndices.size()>0){
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(MyVec3i), &mIndices[0][0], GL_DYNAMIC_DRAW);
	}
	else{
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
	}
	glBindVertexArray(0);
	}
	else{
	mLineIndices.clear();
	for (int itt = 0; itt<mFiberToDraw.size(); itt++){
	int it = mFiberToDraw[itt];
	int offset = mIdxOffset[it];
	for (int i = 0; i<this->GetNumVertex(it); i++){
	mLineIndices << offset + i;
	}
	}
	glBindVertexArray(mVertexArray);
	if (glIsBuffer(mIndexBuffer)){
	glDeleteBuffers(1, &mIndexBuffer);
	}
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	if (mLineIndices.size()>0){
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mLineIndices.size() * sizeof(int), &mLineIndices[0], GL_DYNAMIC_DRAW);
	}
	else{
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
	}
	glBindVertexArray(0);
	}

	*/
}

void MyTracks::SetFiberToDraw(const MyArrayi* fiberToDraw){
	mFiberToDraw = *fiberToDraw;
}

void MyTracks::AddVolumeFilter(RicVolume& vol){
	if (glIsTexture(mFilterVolumeTexture)){
		glDeleteTextures(1, &mFilterVolumeTexture);
	}
	glGenTextures(1, &mFilterVolumeTexture);
	glBindTexture(GL_TEXTURE_3D, mFilterVolumeTexture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// map x,y,z to z,y,x
	float *d = new float[vol.nvox];
	for (int i = 0; i < vol.get_numx(); i++){
		for (int j = 0; j < vol.get_numy(); j++){
			for (int k = 0; k < vol.get_numz(); k++){
				//if (vol.vox[i][j][k] > 0.1) cout << vol.vox[i][j][k] << endl;
				d[k*vol.get_numx()*vol.get_numy() + j*vol.get_numx() + i]
					= vol.vox[i][j][k];
			}

		}
	}
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, vol.get_numx(), vol.get_numy(), vol.get_numz(), 0, GL_RED, GL_FLOAT, d);
	delete[]d;

	glBindTexture(GL_TEXTURE_3D, 0);
}

void MyTracks::ToDensityVolume(float* densityVol, int x, int y, int z){
	if (mFiberToDraw.size() == 0){
		memset(densityVol, 0, x*y*z*sizeof(float));
		return;
	}
	Array3D<atomic<int>> density;
	density.Construct(x, y, z);
	for (int i = 0; i < x; i++){
		for (int j = 0; j < y; j++){
			for (int k = 0; k < z; k++){
				density(i, j, k).store(0);
			}
		}
	}
	int numThread = std::thread::hardware_concurrency() - 1;
	numThread = min(numThread, (int)mFiberToDraw.size());
	std::thread *tt = new std::thread[numThread - 1];
	float fiberPerThread = mFiberToDraw.size() / (float)numThread;
	for (int i = 0; i < numThread - 1; i++){
		int startIdx = fiberPerThread*i;
		int endIdx = fiberPerThread*(i + 1) - 1;
		tt[i] = std::thread(FiberVolumeDensity, this, &density, &mFiberToDraw, startIdx, endIdx);
	}
	FiberVolumeDensity(this, &density, &mFiberToDraw, fiberPerThread*(numThread - 1), mFiberToDraw.size() - 1);
	for (int i = 0; i < numThread - 1; i++){
		tt[i].join();
	}
	delete[] tt;

	int idx = 0;
	float maxValue = 0;
	for (int i = 0; i < x; i++){
		for (int j = 0; j < y; j++){
			for (int k = 0; k < z; k++){
				densityVol[idx] = (float)density(i, j, k).load();
				maxValue = max(maxValue, densityVol[idx]);
				idx++;
			}
		}
	}
	if (maxValue > 0){
		for (int i = 0; i < x*y*z; i++){
			densityVol[i] /= maxValue;
		}
	}
}
#endif