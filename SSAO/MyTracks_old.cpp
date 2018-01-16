/*
 * MyTracks.cpp
 *
 *  Created on: Dec 12, 2014
 *      Author: GuohaoZhang
 */

#include "MyTracks.h"
#ifdef RIC
#include "RicPoint.h"
#endif
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

MyTracks::MyTracks(){
	ResetRenderingParameters();
	mBoundingBox = MyBoundingBox(MyVec3f(INT_MAX, INT_MAX, INT_MAX), MyVec3f(-INT_MAX, -INT_MAX, -INT_MAX));
}

MyTracks::MyTracks(const string& filename){
	Read(filename);
	ResetRenderingParameters();
}

void MyTracks::ResetRenderingParameters(){
	mTrackRadius = 0.4;
	mFaces = 20;
	mShape = TRACK_SHAPE_TUBE;
	mBoxOffset0 = MyVec3f(0, 0, 0);
	mBoxOffset1 = mBoxOffset0;
}

int MyTracks::Read(const std::string& filename){

	string fileExt = filename.substr(filename.find_last_of(".") + 1);
	if (fileExt == "trk") {
		FILE* fp;
		if ((fp = fopen(filename.c_str(), "rb")) == NULL){
			printf("Error: cannot open file: %s\n", filename.c_str());
			return 0;
		}
		if (fread(&mHeader, sizeof(MyTrackHeader), 1, fp) != 1){
			printf("Error: cannot read header from: %s\n", filename.c_str());
			fclose(fp);
			return 0;
		}
		else printf("Info: %d tracts from file %s\n", mHeader.n_count, filename.c_str());

		cout << "Allocating Storage...\r";
		mTracks.clear();
		mTracks.resize(mHeader.n_count);
		mBoundingBox = MyBoundingBox(MyVec3f(INT_MAX, INT_MAX, INT_MAX), MyVec3f(-INT_MAX, -INT_MAX, -INT_MAX));

		for (int i = 0; i<mHeader.n_count; i++){
			if ((int)((i + 1) * 100 / (float)mHeader.n_count)
				- (int)(i * 100 / (float)mHeader.n_count) >= 1){
				cout << "Loading: " << i*100.f / mHeader.n_count << "%.\r";
			}
			MySingleTrackData& track = mTracks[i];
			fread(&track.mSize, sizeof(int), 1, fp);
			track.mPoints.resize(track.mSize);
			track.mPointScalars.resize(track.mSize);
			track.mTrackProperties.resize(track.mSize);
			for (int j = 0; j< track.mSize; j++){
				track.mPointScalars[j].resize(mHeader.n_scalars);
				fread(&track.mPoints[j], sizeof(MyVec3f), 1, fp);
				mBoundingBox.Engulf(track.mPoints[j]);
				if (mHeader.n_scalars>0){
					fread(&track.mPointScalars[j][0], mHeader.n_scalars*sizeof(float), 1, fp);
				}
			}
			if (mHeader.n_properties>0){
				fread(&track.mTrackProperties[0], mHeader.n_properties*sizeof(float), 1, fp);
			}

			// init draw all
			mFiberToDraw.push_back(i);
		}
		cout << "Tracks loading completed.\n";
		fclose(fp);
	}
	else if (fileExt == "data"){
		ifstream fs(filename);
		fs >> mHeader.n_count;
		printf("Info: %d tracts from file %s\n", mHeader.n_count, filename.c_str());

		cout << "Allocating Storage...\r";
		mTracks.clear();
		mTracks.resize(mHeader.n_count);
		mBoundingBox = MyBoundingBox(MyVec3f(INT_MAX, INT_MAX, INT_MAX), MyVec3f(-INT_MAX, -INT_MAX, -INT_MAX));

		for (int i = 0; i < mHeader.n_count; i++){
			if ((int)((i + 1) * 100 / (float)mHeader.n_count)
				- (int)(i * 100 / (float)mHeader.n_count) >= 1){
				cout << "Loading: " << i*100.f / mHeader.n_count << "%.\r";
			}
			MySingleTrackData& track = mTracks[i];
			fs >> track.mSize;
			mHeader.n_scalars = 3;
			track.mPoints.resize(track.mSize);
			track.mPointScalars.resize(track.mSize);
			for (int j = 0; j< track.mSize; j++){
				track.mPointScalars[j].resize(3);
				fs >> track.mPoints[j][0] >> track.mPoints[j][1] >> track.mPoints[j][2]
					>> track.mPointScalars[j][0] >> track.mPointScalars[j][1] >> track.mPointScalars[j][2];
				mBoundingBox.Engulf(track.mPoints[j]);
			}

			// init draw all
			mFiberToDraw.push_back(i);
		}
		cout << "Tracks loading completed.\n";
		fs.close();
	}
	else if (fileExt == "tensorinfo"){
		ifstream fs(filename);
		fs >> mHeader.n_count;
		printf("Info: %d tracts from file %s\n", mHeader.n_count, filename.c_str());

		cout << "Allocating Storage...\r";
		mTracks.clear();
		mTracks.resize(mHeader.n_count);
		mBoundingBox = MyBoundingBox(MyVec3f(INT_MAX, INT_MAX, INT_MAX), MyVec3f(-INT_MAX, -INT_MAX, -INT_MAX));

		for (int i = 0; i < mHeader.n_count; i++){
			if ((int)((i + 1) * 100 / (float)mHeader.n_count)
				- (int)(i * 100 / (float)mHeader.n_count) >= 1){
				cout << "Loading: " << i*100.f / mHeader.n_count << "%.\r";
			}
			MySingleTrackData& track = mTracks[i];
			fs >> track.mSize;
			mHeader.n_scalars = 13;
			track.mPoints.resize(track.mSize);
			track.mPointScalars.resize(track.mSize);
			for (int j = 0; j< track.mSize; j++){
				track.mPointScalars[j].resize(13);
				fs >> track.mPoints[j][0] >> track.mPoints[j][1] >> track.mPoints[j][2]
					>> track.mPointScalars[j][0]
					>> track.mPointScalars[j][1] >> track.mPointScalars[j][2] >> track.mPointScalars[j][3]
					>> track.mPointScalars[j][4] >> track.mPointScalars[j][5] >> track.mPointScalars[j][6]
					>> track.mPointScalars[j][7] >> track.mPointScalars[j][8] >> track.mPointScalars[j][9]
					>> track.mPointScalars[j][10] >> track.mPointScalars[j][11] >> track.mPointScalars[j][12];
				mBoundingBox.Engulf(track.mPoints[j]);
			}

			// init draw all
			mFiberToDraw.push_back(i);
		}
		cout << "Tracks loading completed.\n";
		fs.close();
	}

	return 1;
}


int MyTracks::Save(const std::string& filename) const{
	string ext = filename.substr(filename.find_last_of(".") + 1);
	if (ext == "trk"){
		FILE* fp;
		if ((fp = fopen(filename.c_str(), "wb")) == NULL){
			printf("Error: cannot open file: %s\n", filename.c_str());
			return 0;
		}

		if (fwrite(&mHeader, sizeof(MyTrackHeader), 1, fp) != 1){
			printf("Error: cannot write header from: %s\n", filename.c_str());
			fclose(fp);
			return 0;
		}

		printf("Writing %d tracks...\n", mHeader.n_count);

		for (int i = 0; i<mHeader.n_count; i++){
			const MySingleTrackData& track = mTracks[i];
			fwrite(&track.mSize, sizeof(int), 1, fp);

			for (int j = 0; j< track.mSize; j++){
				fwrite(&track.mPoints[j], sizeof(MyVec3f), 1, fp);
				if (mHeader.n_scalars>0){
					fwrite(&track.mPointScalars[j][0], mHeader.n_scalars*sizeof(float), 1, fp);
				}
			}
			if (mHeader.n_properties>0){
				fwrite(&track.mTrackProperties[0], mHeader.n_properties*sizeof(float), 1, fp);
			}
		}
		fclose(fp);
		return 1;
	}
	else if (ext == "data"){
		ofstream outfile(filename);
		if (!outfile.is_open()){
			printf("Error: cannot open file: %s\n", filename.c_str());
			return 0;
		}
		char space = ' ';
		outfile << mTracks.size() << endl;
		for (int i = 0; i < mTracks.size(); i++){
			outfile << mTracks[i].mPoints.size() << endl;
			for (int j = 0; j < mTracks[i].mPoints.size(); j++){
				const MyVec3f& p = mTracks[i].mPoints[j];
				// vertex coordinates
				outfile << p[0] << space
					<< p[1] << space
					<< p[2] << space;
				// vertex color
				const vector<float>& s = mTracks[i].mPointScalars[j];
				if (s.size() == 3){
					outfile << s[0] << space
						<< s[1] << space
						<< s[2] << endl;
				}
				else if (s.size() > 3){
					outfile << s[1] << space
						<< s[2] << space
						<< s[3] << endl;
				}
				else{
					outfile << "1 1 1\n";
				}
			}
		}
		return 1;
	}
	printf("Error: unknown file extension %s\n", ext.c_str());
	return 0;
}

int MyTracks::SavePartial(const std::string& filename, const std::vector<int>& saveTrackIdx) const{
	string ext = filename.substr(filename.find_last_of(".") + 1);
	if (ext == "trk"){
		FILE* fp;
		if ((fp = fopen(filename.c_str(), "wb")) == NULL){
			printf("Error: cannot open file: %s\n", filename.c_str());
			return 0;
		}

		float count = mHeader.n_count;
		MyTrackHeader newHeader;
		memcpy(&newHeader, &mHeader, sizeof(MyTrackHeader));
		newHeader.n_count = saveTrackIdx.size();
		if (fwrite(&newHeader, sizeof(MyTrackHeader), 1, fp) != 1){
			printf("Error: cannot write header from: %s\n", filename.c_str());
			fclose(fp);
			return 0;
		}


		printf("Writing %d tracks...\n", newHeader.n_count);

		for (int i = 0; i<newHeader.n_count; i++){
			const MySingleTrackData& track = mTracks[saveTrackIdx[i]];
			fwrite(&track.mSize, sizeof(int), 1, fp);

			for (int j = 0; j< track.mSize; j++){
				fwrite(&track.mPoints[j], sizeof(MyVec3f), 1, fp);
				if (mHeader.n_scalars>0){
					fwrite(&track.mPointScalars[j][0], mHeader.n_scalars*sizeof(float), 1, fp);
				}
			}
			if (mHeader.n_properties>0){
				fwrite(&track.mTrackProperties[0], mHeader.n_properties*sizeof(float), 1, fp);
			}
		}
		fclose(fp);
		return 1;
	}
	else if (ext == "data"){
		ofstream outfile(filename);
		if (!outfile.is_open()){
			printf("Error: cannot open file: %s\n", filename.c_str());
			return 0;
		}
		char space = ' ';
		outfile << saveTrackIdx.size() << endl;
		for (int i = 0; i < saveTrackIdx.size(); i++){
			outfile << mTracks[saveTrackIdx[i]].mPoints.size() << endl;
			for (int j = 0; j < mTracks[saveTrackIdx[i]].mPoints.size(); j++){
				const MyVec3f& p = mTracks[saveTrackIdx[i]].mPoints[j];
				// vertex coordinates
				outfile << p[0] << space
					<< p[1] << space
					<< p[2] << space;
				// vertex color
				const vector<float>& s = mTracks[saveTrackIdx[i]].mPointScalars[j];
				if (s.size() == 3){
					outfile << s[0] << space
						<< s[1] << space
						<< s[2] << endl;
				}
				else if (s.size() > 3){
					outfile << s[1] << space
						<< s[2] << space
						<< s[3] << endl;
				}
				else{
					outfile << "1 1 1\n";
				}
			}
		}
		return 1;
	}
	printf("Error: unknown file extension %s\n", ext.c_str());
	return 0;
}

MyTracks MyTracks::Subset(const std::vector<int>& trackIndices) const{
	MyTracks subset;
	subset.mHeader = this->mHeader;
	subset.mHeader.n_count = trackIndices.size();
	for(unsigned int i = 0;i<trackIndices.size(); i++){
		int trackIndex = trackIndices[i];
		subset.mTracks.push_back(this->mTracks[trackIndex]);
	}
	return subset;
}

void MyTracks::AddTracks(const MyTracks& tracks){
	mHeader.n_count += tracks.mHeader.n_count;
	
	int n = mFiberToDraw.size();
	for(unsigned int i = 0;i<tracks.mTracks.size(); i++){
		mTracks.push_back(tracks.mTracks[i]);
		mFiberToDraw.push_back(n + i);
	}
	mBoundingBox.Engulf(tracks.GetBoundingBox());
}

void MyTracks::CopyTracksFrom(const MyTracks& track){
	memcpy(&this->mHeader, &track.mHeader, sizeof(MyTrackHeader));
	this->mTracks = track.mTracks;
	this->mFiberToDraw = track.mFiberToDraw;
}

int MyTracks::GetNumTracks() const{
	return mHeader.n_count;
}

int MyTracks::GetNumVertex(int trackIdx) const{
	return mTracks[trackIdx].mSize;
}

MyBoundingBox MyTracks::ComputeBoundingBox(const MyArrayi tractIndices) const{
	MyBoundingBox box(MyVec3f(FLT_MAX, FLT_MAX, FLT_MAX), 
		MyVec3f(-FLT_MAX, -FLT_MAX, -FLT_MAX));
	for (int i = 0; i < tractIndices.size(); i++){
		const MySingleTrackData& tract = mTracks[tractIndices[i]];
		for (int j = 0; j < tract.mSize; j++){
			const MyVec3f& p = tract.mPoints[j];
			box.Engulf(p);
		}
	}
	return box;
}

void MyTracks::GetSampleValueInfo(MyBoundingBox& box, int& nSample, float& valueSum) const{
	nSample = 0;
	valueSum = 0;
	for (int it = 0; it < this->GetNumTracks(); it++){
		for (int is = 0; is < this->GetNumVertex(it); is++){
			MyVec3f p = this->GetCoord(it, is);
			if (box.IsIn(p)){
				float value = this->GetValue(it, is);
				++nSample;
				valueSum += value;
			}
		}
	}
}

void MyTracks::GetSampleValueInfo(MyBoundingBox& box, const MyArrayi& indices, int& nSample, float& valueSum) const{
	nSample = 0;
	valueSum = 0;
	for (int it = 0; it < indices.size(); it++){
		for (int is = 0; is < this->GetNumVertex(it); is++){
			MyVec3f p = this->GetCoord(indices[it], is);
			if (box.IsIn(p)){
				float value = this->GetValue(indices[it], is);
				++nSample;
				valueSum += value;
			}
		}
	}
}

void MyTracks::FilterByTrackLength(const std::vector<int>& inset, float threshold[2], std::vector<int>& outset) const{
	for (int i = 0; i < inset.size(); i++){
		int idx = inset[i];
		float length = this->ComputeTrackLength(idx);
		if (length >= threshold[0] && length <= threshold[1]){
			outset.push_back(idx);
		}
	}
}

void MyTracks::FilterByIndexSkip(const std::vector<int>& inset, float skipRatio, std::vector<int>& outset) const{
	float rest= 0;
	for (int i = 0; i < inset.size(); i++){
		rest += skipRatio;
		if (rest >= 0){
			// skip
			rest -= 1.f;
			continue;
		}
		else{
			outset.push_back(inset[i]);
		}
	}
}

#ifdef RIC
Point MyTracks::GetPoint(int trackIdx, int pointIdx) const{
	return mTracks[trackIdx].mPoints[pointIdx];
}
#endif

MyVec3f MyTracks::GetCoord(int trackIdx, int pointIdx) const{
	return mTracks[trackIdx].mPoints[pointIdx];
}

float MyTracks::GetValue(int trackIdx, int pointIdx) const{
	if (mTracks[trackIdx].mPointScalars[pointIdx].size() == 3){
		return 1-mTracks[trackIdx].mPointScalars[pointIdx][2];
	}
	else if (mTracks[trackIdx].mPointScalars[pointIdx].size() == 13){
		return mTracks[trackIdx].mPointScalars[pointIdx][0];
	}
	else return -1;
}

float MyTracks::ComputeTrackLength(int trackIdx) const{
	const MySingleTrackData& track = mTracks[trackIdx];
	if (track.mPoints.size() == 0) return 0;
	float acc = 0;
	MyVec3f cur = track.mPoints[0];
	for (int i = 1; i < track.mPoints.size(); i++){
		const MyVec3f& next = track.mPoints[i];
		float dist = (next - cur).norm();
		acc += dist;
		cur = next;
	}
	return acc;
}

void MyTracks::ComputeTubeGeometry(){
	int currentIdx = 0;
	mIdxOffset.clear();
	int totalPoints = 0;
	for (int it = 0; it < mTracks.size(); it++){
		totalPoints += mTracks[it].mSize;
	}
	totalPoints *= (mFaces + 1);

	// for caps
	totalPoints += mTracks.size()*(1 + mFaces) * 2;

	cout << "Allocating Storage for Geometry...\r";

	mVertices.resize(totalPoints);
	mNormals.resize(totalPoints);
	//mTexCoords.resize(totalPoints);
	//mRadius.resize(totalPoints);
	mColors.resize(totalPoints);

	float R = 0;

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
				mVertices[currentIdx + i*(mFaces + 1) + is] = pt * R + p;
				mNormals[currentIdx + i*(mFaces + 1) + is] = pt;
				//mTexCoords[currentIdx + i*(mFaces + 1) + is] = MyVec2f(i, is / (float)mFaces);
				//mRadius[currentIdx + i*(mFaces + 1) + is] = size;
				if (mHeader.n_scalars == 3){
					mColors[currentIdx + i*(mFaces + 1) + is] = MyColor4f(
						mTracks[it].mPointScalars[i][0], mTracks[it].mPointScalars[i][1],
						mTracks[it].mPointScalars[i][2], 1);
				}
				else if (mHeader.n_scalars == 13){
					mColors[currentIdx + i*(mFaces + 1) + is] = MyColor4f(
						1, 1, 1 - mTracks[it].mPointScalars[i][0], 1);
				}
				else mColors[currentIdx + i*(mFaces + 1) + is] = MyColor4f(1, 1, 1, 1);
			}
			mVertices[currentIdx + i*(mFaces + 1) + mFaces] = mVertices[currentIdx + i*(mFaces + 1)];
			mNormals[currentIdx + i*(mFaces + 1) + mFaces] = mNormals[currentIdx + i*(mFaces + 1)];
			//mTexCoords[currentIdx + i*(mFaces + 1) + mFaces] = MyVec2f(i, 1);
			//mRadius[currentIdx + i*(mFaces + 1) + mFaces] = mRadius[currentIdx + i*(mFaces + 1)];
			mColors[currentIdx + i*(mFaces + 1) + mFaces] = mColors[currentIdx + i*(mFaces + 1)];
		}

		mIdxOffset << currentIdx;
		currentIdx += npoints*(mFaces + 1);

		// add front cap
		{
			MyVec3f p = this->GetCoord(it, 0);
			MyVec3f d = this->GetCoord(it, 1) - p;
			d.normalize();
			MyVec3f perpend1 = (pole^d).normalized();
			MyVec3f perpend2 = (perpend1^d).normalized();
			mVertices[currentIdx] = p;
			mNormals[currentIdx] = -d;
			mColors[currentIdx] = mColors[currentIdx - npoints*(mFaces + 1)];
			for (int is = 0; is < mFaces; is++){
				float angle = dangle*is;
				MyVec3f pt = sin(angle)*perpend1 + cos(angle)*perpend2;
				MyVec3f pe = pt*R + p;
				mVertices[currentIdx + is + 1] = pe;
				mNormals[currentIdx + is + 1] = pt;
				//mNormals[currentIdx + is + 1] = -d;
				mColors[currentIdx + is + 1] = mColors[currentIdx - npoints*(mFaces + 1)];
			}
		}

		// add back cap
		currentIdx += mFaces + 1;
		{
			MyVec3f p = this->GetCoord(it, npoints - 1);
			MyVec3f d = this->GetCoord(it, npoints - 2) - p;
			d.normalize();
			MyVec3f perpend1 = (pole^d).normalized();
			MyVec3f perpend2 = (perpend1^d).normalized();
			mVertices[currentIdx] = p;
			mNormals[currentIdx] = -d;
			mColors[currentIdx] = mColors[currentIdx - (mFaces + 2)];
			for (int is = 0; is < mFaces; is++){
				float angle = dangle*is;
				MyVec3f pt = sin(angle)*perpend1 + cos(angle)*perpend2;
				MyVec3f pe = pt*R + p;
				mVertices[currentIdx + is + 1] = pe;
				mNormals[currentIdx + is + 1] = pt;
				//mNormals[currentIdx + is + 1] = -d;
				mColors[currentIdx + is + 1] = mColors[currentIdx - (mFaces + 2)];
			}
		}
		currentIdx += mFaces + 1;
	}
	// index
	mIndices.clear();
	for (int it = 0; it<this->GetNumTracks(); it++){
		int offset = mIdxOffset[it];
		for (int i = 1; i<this->GetNumVertex(it); i++){
			for (int j = 0; j < mFaces; j++){
				/*
				// wrong direction!
				mIndices << MyVec3i((i - 1)*(mFaces + 1) + j + offset,
					(i)*(mFaces + 1) + j + offset,
					(i)*(mFaces + 1) + (j + 1) + offset);
				mIndices << MyVec3i((i - 1)*(mFaces + 1) + j + offset,
					(i)*(mFaces + 1) + (j + 1) + offset,
					(i - 1)*(mFaces + 1) + (j + 1) + offset);
					*/
				// fixed direction
				mIndices << MyVec3i((i - 1)*(mFaces + 1) + j + offset,
					(i)*(mFaces + 1) + (j + 1) + offset,
					(i)*(mFaces + 1) + j + offset);
				mIndices << MyVec3i((i - 1)*(mFaces + 1) + j + offset,
					(i - 1)*(mFaces + 1) + (j + 1) + offset,
					(i)*(mFaces + 1) + (j + 1) + offset);
			}
		}
		// add caps
		offset += this->GetNumVertex(it)*(mFaces + 1);
		for (int j = 0; j < mFaces; j++){
			mIndices << MyVec3i(offset, offset + (j + 1) % (mFaces) + 1, offset + j + 1);
		}
		offset += 1 + mFaces;
		for (int j = 0; j < mFaces; j++){
			mIndices << MyVec3i(offset, offset + (j + 1) % (mFaces) + 1, offset + j + 1);
		}
	}
	cout << "Computing completed.\n";
}

void MyTracks::ComputeLineGeometry(){

	int currentIdx = 0;
	mIdxOffset.clear();
	int totalPoints = 0;
	for (int it = 0; it < mTracks.size(); it++){
		totalPoints += mTracks[it].mSize;
	}

	cout << "Allocating Storage for Geometry...\r";

	mVertices.clear();
	mNormals.clear();
	mVertices.reserve(totalPoints);
	mNormals.reserve(totalPoints);
	mColors.reserve(totalPoints);


	mIdxOffset.clear();
	mIdxOffset.reserve(mTracks.size());
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

			//mVertices[currentIdx + i] = p;
			//mNormals[currentIdx + i] = perpend1;
			mVertices.push_back(p);
			mNormals.push_back(perpend1);
			if (mHeader.n_scalars == 3){
				 MyColor4f color (
					mTracks[it].mPointScalars[i][0], mTracks[it].mPointScalars[i][1],
					mTracks[it].mPointScalars[i][2], 1);
				 mColors.push_back(color);
			}
			else {
				mColors.push_back(MyColor4f(1, 1, 1, 1));
			}
		}

		mIdxOffset << currentIdx;
		currentIdx += npoints;
	}

	mLineIndices.clear();
	mLineIndices.reserve(totalPoints);
	// index
	for (int it = 0; it<this->GetNumTracks(); it++){
		int offset = mIdxOffset[it];
		for (int i = 0; i<this->GetNumVertex(it); i++){
			mLineIndices << i + offset;
		}
	}
	cout << "Computing completed.\n";
}

void MyTracks::SetNumberFaces(int f){
	mFaces = f;

}

void MyTracks::ClearGeometry(){
	mVertices.clear();
	mNormals.clear();
	//mTexCoords.clear();
	//mRadius.clear();
	mColors.clear();
	mIndices.clear();
	mLineIndices.clear();
}

void MyTracks::ComputeGeometry(){
	ClearGeometry();
	if (mShape == TRACK_SHAPE_TUBE){
		this->ComputeTubeGeometry();
	}
	else{
		this->ComputeLineGeometry();
	}
}

void MyTracks::LoadGeometry(){
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

void MyTracks::LoadShader(){
	if (mShape == TRACK_SHAPE_LINE){
	//	return;
	}

	glDeleteProgram(mShaderProgram);
	mShaderProgram = InitShader("Shaders\\geom.vert", "Shaders\\geom.frag", "fragColour", "position");
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
}

void MyTracks::Show(){
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
		glUniform1f(radiusLocation, mTrackRadius);

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
				//int offset = (mIdxOffset[fiberIdx] / (mFaces + 1) - fiberIdx)*mFaces * 6;
				//int numVertex = (this->GetNumVertex(fiberIdx) - 1)*(mFaces + 0) * 6;
				// add cap offset
				int offset = (mIdxOffset[fiberIdx] / (mFaces + 1) - fiberIdx*2)*mFaces * 6;
				int numVertex = (this->GetNumVertex(fiberIdx) - 1)*(mFaces + 0) * 6 + mFaces * 6;
				glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
			}
		}
		else{
			for (int i = 0; i < mFiberToDraw.size(); i++){
				int fiberIdx = mFiberToDraw[i];
				int offset = mIdxOffset[fiberIdx];
				int numVertex = this->GetNumVertex(fiberIdx);
				glDrawElements(GL_LINE_STRIP, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
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


void MyTracks::ShowCapsOnly(){
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
	if (mShape == TRACK_SHAPE_LINE){
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
	if (mShape == TRACK_SHAPE_TUBE){
		//this has no cap considered
		for (int i = 0; i < mFiberToDraw.size(); i++){
			int fiberIdx = mFiberToDraw[i];
			int offset = (mIdxOffset[fiberIdx] / (mFaces + 1) - fiberIdx*2)*mFaces * 6;
			offset += (this->GetNumVertex(fiberIdx) - 1)*(mFaces + 0) * 6;
			int numVertex = mFaces * 6;
			glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
		}
	}

	glUseProgram(0);
	glBindVertexArray(0);
	glPopAttrib();
}

#ifdef RIC
void MyTracks::GetVoxelIndex(const MyVec3f vertex, long &x, long &y, long &z) const{
	x = 181-(long)vertex[0];
	y = 217-(long)vertex[1];
	z = 181-(long)vertex[2];
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
	mFiberDraw = MyArrayb(mTracks.size(), false);
	// mask the fibers
	// serial edition
	/*
	for (int i = 0; i < this->GetNumTracks(); i++){
		for (int j = 0; j < this->GetNumVertex(i); j++){
			long x, y, z;
			this->GetVoxelIndex(this->GetCoord(i, j), x, y, z);
			if (mask(x, y, z) > 0.5){
				mFiberToDraw << i;
				break;
			}
		}
	}
	*/

	//MaskFiber(this, &mask, 0, mTracks.size() - 1);

	// multi-thread edition
	int numThread = std::thread::hardware_concurrency() - 1;
	numThread = min(numThread, (int)mTracks.size());
	std::thread *tt = new std::thread[numThread - 1];
	float fiberPerThread = mTracks.size() / (float)numThread;
	for (int i = 0; i < numThread-1; i++){
		int startIdx = fiberPerThread*i;
		int endIdx = fiberPerThread*(i+1)-1;
		tt[i] = std::thread(MaskFiber, this, &mask, startIdx, endIdx);
	}
	MaskFiber(this, &mask, fiberPerThread*(numThread - 1), mTracks.size() - 1);
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
				for (int j = 0; j <= mFaces; j++){
					mIndices << MyVec3i((i - 1)*(mFaces + 1) + j % (mFaces + 1) + offset,
						(i)*(mFaces + 1) + j % (mFaces + 1) + offset,
						(i)*(mFaces + 1) + (j + 1) % (mFaces + 1) + offset);
					mIndices << MyVec3i((i - 1)*(mFaces + 1) + j % (mFaces + 1) + offset,
						(i)*(mFaces + 1) + (j + 1) % (mFaces + 1) + offset,
						(i - 1)*(mFaces + 1) + (j + 1) % (mFaces + 1) + offset);
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