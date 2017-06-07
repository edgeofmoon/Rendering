#include "MyTractVisBaseLegend.h"
#include "MyMathHelper.h"
#include "MyPrimitiveDrawer.h"
#include <iostream>

#include "GL\glew.h"
#include <GL/freeglut.h>
using namespace std;


MyTractVisBaseLegend::MyTractVisBaseLegend()
	:MyTractVisBase(){
	mNumberSamples = 9;
	mDir = MyVec3f(0, 7, 0);
	mBase = - mDir * (mNumberSamples - 1.f) / 2;
}


MyTractVisBaseLegend::~MyTractVisBaseLegend()
{
}

void MyTractVisBaseLegend::ComputeGeometry(){
	int currentIdx = 0;
	mIdxOffset.clear();
	int totalPoints = mNumberSamples;

	totalPoints *= (mRenderingParameters.Faces + 1);
	// for caps
	totalPoints += (1 + mRenderingParameters.Faces) * 2;

	mVertices.resize(totalPoints);
	mNormals.resize(totalPoints);
	mTexCoords.resize(totalPoints);
	//mRadius.resize(totalPoints);
	mColors.resize(totalPoints);
	mValues.resize(totalPoints);

	float R = 0;

	int npoints = mNumberSamples;

	const float myPI = 3.1415926f;
	float dangle = 2 * myPI / mRenderingParameters.Faces;
	MyVec3f pole(0.6, 0.8, 0);
	mIdxOffset << 0;
	for (int i = 0; i < npoints; i++){
		MyVec3f p = mBase + i*mDir;
		MyVec3f d = mDir;

		MyVec3f perpend1 = (pole^d).normalized();
		MyVec3f perpend2 = (perpend1^d).normalized();
		//if ((perpend1^perpend2)*d < 0) dangle = -dangle;
		float fa = i / float(npoints - 1)*0.8 + 0.2f;
		for (int is = 0; is < mRenderingParameters.Faces; is++){
			float angle = dangle*is;
			MyVec3f pt = sin(angle)*perpend1 + cos(angle)*perpend2;
			mVertices[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = pt * R + p;
			mNormals[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = pt;
			mColors[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = MyColor4f(0.5,0.5,0.5);
			mTexCoords[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = MyVec2f(i, is / (float)mRenderingParameters.Faces);
			mValues[currentIdx + i*(mRenderingParameters.Faces + 1) + is] = fa;
		}
		mVertices[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = mVertices[currentIdx + i*(mRenderingParameters.Faces + 1)];
		mNormals[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = mNormals[currentIdx + i*(mRenderingParameters.Faces + 1)];
		//mTexCoords[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = MyVec2f(i, 1);
		//mRadius[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = mRadius[currentIdx + i*(mRenderingParameters.Faces + 1)];
		mColors[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = mColors[currentIdx + i*(mRenderingParameters.Faces + 1)];
		mTexCoords[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = MyVec2f(i, 1);
		mValues[currentIdx + i*(mRenderingParameters.Faces + 1) + mRenderingParameters.Faces] = fa;
	}

	currentIdx += npoints*(mRenderingParameters.Faces + 1);

	// add front cap
	{
		float fa = 0.2;
		MyVec3f p = mBase;
		MyVec3f d = mDir;
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
		float fa = 1.0f;
		MyVec3f p = mBase + (npoints-1)*mDir;
		MyVec3f d = -mDir;
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
	mIdxOffset << currentIdx;
	// index
	mIndices.clear();
	int offset = mIdxOffset[0];
	for (int i = 1; i < npoints; i++){
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
	offset += npoints*(mRenderingParameters.Faces + 1);
	for (int j = 0; j < mRenderingParameters.Faces; j++){
		mIndices << MyVec3i(offset, offset + (j + 1) % (mRenderingParameters.Faces) + 1, offset + j + 1);
	}
	offset += 1 + mRenderingParameters.Faces;
	for (int j = 0; j < mRenderingParameters.Faces; j++){
		mIndices << MyVec3i(offset, offset + (j + 1) % (mRenderingParameters.Faces) + 1, offset + j + 1);
	}

	mFiberToDraw = { 0 };
}


void MyTractVisBaseLegend::Show(){
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
	glUniform1f(radiusLocation, mTrackRadius);
	//this has no cap considered
	for (int i = 0; i < mFiberToDraw.size(); i++){
		int fiberIdx = mFiberToDraw[i];
		//int offset = (mIdxOffset[fiberIdx] / (mRenderingParameters.Faces + 1) - fiberIdx)*mRenderingParameters.Faces * 6;
		//int numVertex = (this->GetNumVertex(fiberIdx) - 1)*(mRenderingParameters.Faces + 0) * 6;
		// add cap offset
		int offset = (mIdxOffset[fiberIdx] / (mRenderingParameters.Faces + 1) - fiberIdx * 2)*mRenderingParameters.Faces * 6;
		//int numVertex = (mNumberSamples - 1)*(mRenderingParameters.Faces + 0) * 6 + mRenderingParameters.Faces * 6;
		// no cap
		int numVertex = (mNumberSamples - 1)*(mRenderingParameters.Faces + 0) * 6;
		//int numVertex = (mIdxOffset[fiberIdx + 1] - mIdxOffset[fiberIdx]);
		glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
		//glDrawElements(GL_LINE_STRIP, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
	}

	glUseProgram(0);
	glBindVertexArray(0);

	glPopAttrib();


}
