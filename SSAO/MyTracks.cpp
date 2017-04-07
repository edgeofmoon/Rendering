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
	mBoundingBox = MyBoundingBox(MyVec3f(INT_MAX, INT_MAX, INT_MAX), MyVec3f(-INT_MAX, -INT_MAX, -INT_MAX));
}

MyTracks::MyTracks(const string& filename){
	Read(filename);
}

int MyTracks::Read(const std::string& filename){

	string fileExt = filename.substr(filename.find_last_of(".") + 1);
	if (fileExt == "trk") {
		FILE* fp;
		if ((fp = fopen(filename.c_str(), "rb")) == NULL){
			printf("Error: cannot open file: %s\n", filename.c_str());
			return 0;
		}
		if (fread(&mHeader, sizeof(MyTrackHeader_Trk), 1, fp) != 1){
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
			int tractSize = 0;
			fread(&tractSize, sizeof(int), 1, fp);
			track.mPoints.resize(tractSize);
			track.mPointScalars.resize(tractSize);
			track.mTrackProperties.resize(mHeader.n_properties);
			for (int j = 0; j< tractSize; j++){
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
			int tractSize = 0;
			fs >> tractSize;
			mHeader.n_scalars = 3;
			track.mPoints.resize(tractSize);
			track.mPointScalars.resize(tractSize);
			for (int j = 0; j< tractSize; j++){
				track.mPointScalars[j].resize(3);
				fs >> track.mPoints[j][0] >> track.mPoints[j][1] >> track.mPoints[j][2]
					>> track.mPointScalars[j][0] >> track.mPointScalars[j][1] >> track.mPointScalars[j][2];
				mBoundingBox.Engulf(track.mPoints[j]);
			}
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
			int tractSize = 0;
			fs >> tractSize;
			mHeader.n_scalars = 13;
			track.mPoints.resize(tractSize);
			track.mPointScalars.resize(tractSize);
			for (int j = 0; j< tractSize; j++){
				track.mPointScalars[j].resize(13);
				fs >> track.mPoints[j][0] >> track.mPoints[j][1] >> track.mPoints[j][2]
					>> track.mPointScalars[j][0]
					>> track.mPointScalars[j][1] >> track.mPointScalars[j][2] >> track.mPointScalars[j][3]
					>> track.mPointScalars[j][4] >> track.mPointScalars[j][5] >> track.mPointScalars[j][6]
					>> track.mPointScalars[j][7] >> track.mPointScalars[j][8] >> track.mPointScalars[j][9]
					>> track.mPointScalars[j][10] >> track.mPointScalars[j][11] >> track.mPointScalars[j][12];
				mBoundingBox.Engulf(track.mPoints[j]);
			}
		}
		cout << "Tracks loading completed.\n";
		fs.close();
	}

	return 1;
}

int MyTracks::AppendTrackColor(const std::string& filename){
	string fileExt = filename.substr(filename.find_last_of(".") + 1);
	if (fileExt != "data") {
		cout << "Unknown color format.\n";
		return 0;
	}
	ifstream fs(filename);
	int tractNum = 0;
	fs >> tractNum;
	if (tractNum != mHeader.n_count){
		cout << "New file tract count " << tractNum 
			<< " mismatches current count " << mHeader.n_count << endl;
		return 0;
	}
	float skipCoord;
	float tr, tg, tb;
	string skipLineStr;
	for (int i = 0; i < mHeader.n_count; i++){
		if ((int)((i + 1) * 100 / (float)mHeader.n_count)
			- (int)(i * 100 / (float)mHeader.n_count) >= 1){
			cout << "Loading: " << i*100.f / mHeader.n_count << "%.\r";
		}
		MySingleTrackData& track = mTracks[i];
		int tractSize = 0;
		fs >> tractSize;
		int pStart = mTracks[i].mTrackProperties.size();
		mTracks[i].mTrackProperties.reserve(pStart + 3);
		if (tractSize > 0)
			fs >> skipCoord >> skipCoord >> skipCoord >> tr >> tg >> tb;
		else{
			cout << "Track "<< i << " has no vertex, append black color!" << endl;
			tr = tg = tb = 0;
		}
		mTracks[i].mTrackProperties.push_back(tr);
		mTracks[i].mTrackProperties.push_back(tg);
		mTracks[i].mTrackProperties.push_back(tb);
		// skip the rest of the current line: j = 0
		// and skip the rest vertices: j = 1 ~ traceSize
		for (int j = 0; j< tractSize; j++){
			getline(fs, skipLineStr);
		}
	}
	mHeader.n_properties += 3;
	cout << "Tracks color appending completed.\n";
	fs.close();
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

		if (fwrite(&mHeader, sizeof(MyTrackHeader_Trk), 1, fp) != 1){
			printf("Error: cannot write header from: %s\n", filename.c_str());
			fclose(fp);
			return 0;
		}

		printf("Writing %d tracks...\n", mHeader.n_count);

		for (int i = 0; i<mHeader.n_count; i++){
			const MySingleTrackData& track = mTracks[i];
			int tractSize = track.Size();
			fwrite(&tractSize, sizeof(int), 1, fp);

			for (int j = 0; j< tractSize; j++){
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
		MyTrackHeader_Trk newHeader;
		memcpy(&newHeader, &mHeader, sizeof(MyTrackHeader_Trk));
		newHeader.n_count = saveTrackIdx.size();
		if (fwrite(&newHeader, sizeof(MyTrackHeader_Trk), 1, fp) != 1){
			printf("Error: cannot write header from: %s\n", filename.c_str());
			fclose(fp);
			return 0;
		}


		printf("Writing %d tracks...\n", newHeader.n_count);

		for (int i = 0; i<newHeader.n_count; i++){
			const MySingleTrackData& track = mTracks[saveTrackIdx[i]];
			int tractSize = track.Size();
			fwrite(&tractSize, sizeof(int), 1, fp);

			for (int j = 0; j< tractSize; j++){
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
	for(unsigned int i = 0;i<tracks.mTracks.size(); i++){
		mTracks.push_back(tracks.mTracks[i]);
	}
	mBoundingBox.Engulf(tracks.GetBoundingBox());
}

void MyTracks::CopyTracksFrom(const MyTracks& track){
	memcpy(&this->mHeader, &track.mHeader, sizeof(MyTrackHeader_Trk));
	this->mTracks = track.mTracks;
}

int MyTracks::GetNumTracks() const{
	return mHeader.n_count;
}

int MyTracks::GetNumVertex(int trackIdx) const{
	return mTracks[trackIdx].Size();
}

MyBoundingBox MyTracks::ComputeBoundingBox(const MyArrayi tractIndices) const{
	MyBoundingBox box(MyVec3f(FLT_MAX, FLT_MAX, FLT_MAX), 
		MyVec3f(-FLT_MAX, -FLT_MAX, -FLT_MAX));
	for (int i = 0; i < tractIndices.size(); i++){
		const MySingleTrackData& tract = mTracks[tractIndices[i]];
		for (int j = 0; j < tract.Size(); j++){
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

MyColor4f MyTracks::GetTrackColor(int trackIdx) const{
	int n = mHeader.n_properties;
	return MyColor4f(
		mTracks[trackIdx].mTrackProperties[n - 3],
		mTracks[trackIdx].mTrackProperties[n - 2],
		mTracks[trackIdx].mTrackProperties[n - 1], 1);
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

MyTensor3f MyTracks::GetTensor(int trackIdx, int pointIdx) const{
	MyTensor3f t;
	t.SetEigenValues(&(mTracks[trackIdx].mPointScalars[pointIdx][1]));
	t.SetEigenVectors(&(mTracks[trackIdx].mPointScalars[pointIdx][4]));
	assert(t.CheckEigenValueOrder());
	return t;
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
