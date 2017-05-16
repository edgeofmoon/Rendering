#include "MyTractVisLabelled.h"
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
using namespace std;

#include "GL\glew.h"
#include <GL/freeglut.h>


MyTractVisLabelled::MyTractVisLabelled()
	:MyTractVisBase(){
}


MyTractVisLabelled::~MyTractVisLabelled()
{
}


void MyTractVisLabelled::ComputeGeometry(){
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

	MyArrayi solids;
	MyArrayi transps;

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

		MyColor4f color = mTracts->GetPointColor(it, 0);
		if (color.getMin()>0.9){
			color.a = 0.2;
			transps << it;
		}
		else solids << it;

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
				mColors[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = color;
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
			mIndices << MyVec3i(offset, offset + (j + 1) % (mRenderingParameters.Faces) + 1, offset + j + 1);
		}
		offset += 1 + mRenderingParameters.Faces;
		for (int j = 0; j < mRenderingParameters.Faces; j++){
			mIndices << MyVec3i(offset, offset + (j + 1) % (mRenderingParameters.Faces) + 1, offset + j + 1);
		}
	}

	mFiberToDraw.clear();
	for (int i : solids) mFiberToDraw << i;
	for (int i : transps)mFiberToDraw << i;
	cout << "Computing completed.\n";
}
