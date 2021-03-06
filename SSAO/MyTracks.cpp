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
#include <iomanip>
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
		else printf("Loading %d tracts from file %s\r", mHeader.n_count, filename.c_str());

		//cout << "Allocating Storage...\r";
		mTracks.clear();
		mTracks.resize(mHeader.n_count);
		mBoundingBox = MyBoundingBox(MyVec3f(INT_MAX, INT_MAX, INT_MAX), MyVec3f(-INT_MAX, -INT_MAX, -INT_MAX));

		std::streamsize ss = std::cout.precision();
		std::cout.precision(2);
		for (int i = 0; i < mHeader.n_count; i++){
			if ((int)((i + 1) * 100 / (float)mHeader.n_count)
				- (int)(i * 100 / (float)mHeader.n_count) >= 1){
				cout << "Loading: " << i + 1 << "/" << mHeader.n_count
					<< " (" << i*100.f / mHeader.n_count << "%).\r";
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
		std::cout.precision(ss);
		cout << mHeader.n_count << " tracks successfully loaded.\t\t\n";
		fclose(fp);
	}
	else if (fileExt == "data"){
		ifstream fs(filename);
		fs >> mHeader.n_count;
		printf("Loading %d tracts from file %s\r", mHeader.n_count, filename.c_str());

		//cout << "Allocating Storage...\r";
		mTracks.clear();
		mTracks.resize(mHeader.n_count);
		mBoundingBox = MyBoundingBox(MyVec3f(INT_MAX, INT_MAX, INT_MAX), MyVec3f(-INT_MAX, -INT_MAX, -INT_MAX));

		std::streamsize ss = std::cout.precision();
		std::cout.precision(2);
		mHeader.n_properties = 0;
		mHeader.n_scalars = 3;
		for (int i = 0; i < mHeader.n_count; i++){
			if ((int)((i + 1) * 100 / (float)mHeader.n_count)
				- (int)(i * 100 / (float)mHeader.n_count) >= 1){
				cout << "Loading: " << i + 1 << "/" << mHeader.n_count
					<< " (" << i*100.f / mHeader.n_count << "%).\r";
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
		std::cout.precision(ss);
		cout << mHeader.n_count << " tracks successfully loaded.\t\t\n";
		fs.close();
	}
	else if (fileExt == "tensorinfo"){
		ifstream fs(filename);
		fs >> mHeader.n_count;
		printf("Loading %d tracts from file %s\r", mHeader.n_count, filename.c_str());

		//cout << "Allocating Storage...\r";
		mTracks.clear();
		mTracks.resize(mHeader.n_count);
		mBoundingBox = MyBoundingBox(MyVec3f(INT_MAX, INT_MAX, INT_MAX), MyVec3f(-INT_MAX, -INT_MAX, -INT_MAX));

		std::streamsize ss = std::cout.precision();
		std::cout.precision(2);
		for (int i = 0; i < mHeader.n_count; i++){
			if ((int)((i + 1) * 100 / (float)mHeader.n_count)
				- (int)(i * 100 / (float)mHeader.n_count) >= 1){
				cout << "Loading: " << i + 1 << "/" << mHeader.n_count
					<< " (" << i*100.f / mHeader.n_count << "%).\r";
			}
			MySingleTrackData& track = mTracks[i];
			int tractSize = 0;
			fs >> tractSize;
			mHeader.n_properties = 0;
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
		std::cout.precision(ss);
		cout << mHeader.n_count << " tracks successfully loaded.\t\t\n";
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

void MyTracks::BoxSubsetFrom(const MyTracks& tract, 
	const MyBoundingObject& bobj, const MyArrayi& indices, int minSeg){
	memcpy(&this->mHeader, &tract.mHeader, sizeof(MyTrackHeader_Trk));
	for (int i = 0; i < indices.size(); i++){
		int tidx = indices[i];
		int st = -1, ed = -1;
		int n = tract.GetNumVertex(tidx);
		for (int j = 0; j < n; j++){
			const MyVec3f& p = tract.GetCoord(tidx, j);
			if (bobj.IsIn(p)){
				if (st == -1) st = ed = j;
				else ed = j;
			}
			else {
				if (st >= 0 && ed - st + 1 >= minSeg){
					MySingleTrackData d;
					d.CopyFrom(tract.GetTracts()[tidx], st, ed);
					mTracks << d;
					st = ed = -1;
				}
			}
		}
		if (st >= 0 && ed - st + 1 >= minSeg){
			MySingleTrackData d;
			d.CopyFrom(tract.GetTracts()[tidx], st, ed);
			mTracks << d;
			st = ed = -1;
		}
	}
	mHeader.n_count = mTracks.size();
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

bool MyTracks::IsTractIntersected(const MyBoundingObject& bobj, int tractIndex) const{
	for (int i = 1; i < this->GetNumVertex(tractIndex); i++){
		if (bobj.IsIntersected(GetCoord(tractIndex, i - 1), GetCoord(tractIndex, i)))
			return true;
	}
	return false;
}

int MyTracks::CountTractsIntersected(const MyBoundingObject& bobj) const{
	int rst = 0;
	for (int it = 0; it < this->GetNumTracks(); it++){
		if (IsTractIntersected(bobj, it)) rst++;
	}
	return rst;

}

int MyTracks::CountTractsIntersected(const MyBoundingObject& bobj, const MyArrayi& indices) const{
	int rst = 0;
	for (int it = 0; it < indices.size(); it++){
		if (IsTractIntersected(bobj, indices[it])) rst++;
	}
	return rst;

}

MyArrayi MyTracks::GetTractsIntersected(const MyBoundingObject& bobj) const{
	MyArrayi rst;
	for (int it = 0; it < this->GetNumTracks(); it++){
		if (IsTractIntersected(bobj, it)){
			rst << it;
		}
	}
	return rst;
}

MyArrayi MyTracks::GetTractsIntersected(const MyBoundingObject& bobj, const MyArrayi& indices) const{
	MyArrayi rst;
	for (int it = 0; it < indices.size(); it++){
		if (IsTractIntersected(bobj, indices[it])){
			rst << indices[it];
		}
	}
	return rst;
}

int MyTracks::CountVertexInBox(const MyBoundingObject& bobj) const{
	int rst = 0;
	for (int it = 0; it < this->GetNumTracks(); it++){
		for (int is = 0; is < this->GetNumVertex(it); is++){
			MyVec3f p = this->GetCoord(it, is);
			if (bobj.IsIn(p)){
				rst++;
			}
		}
	}
	return rst;
}

int MyTracks::CountVertexInBox(const MyBoundingObject& bobj, const MyArrayi& indices) const{
	int rst = 0;
	for (int it = 0; it < indices.size(); it++){
		for (int is = 0; is < this->GetNumVertex(indices[it]); is++){
			MyVec3f p = this->GetCoord(indices[it], is);
			if (bobj.IsIn(p)){
				rst++;
			}
		}
	}
	return rst;
}

MyArray2i MyTracks::GetVertexInBox(const MyBoundingObject& bobj) const{
	MyArray2i rst;
	for (int it = 0; it < this->GetNumTracks(); it++){
		for (int is = 0; is < this->GetNumVertex(it); is++){
			MyVec3f p = this->GetCoord(it, is);
			if (bobj.IsIn(p)){
				rst << MyVec2i(it, is);
			}
		}
	}
	return rst;
}

MyArray2i MyTracks::GetVertexInBox(const MyBoundingObject& bobj, const MyArrayi& indices) const{
	MyArray2i rst;
	for (int it = 0; it < indices.size(); it++){
		for (int is = 0; is < this->GetNumVertex(indices[it]); is++){
			MyVec3f p = this->GetCoord(indices[it], is);
			if (bobj.IsIn(p)){
				rst << MyVec2i(indices[it], is);
			}
		}
	}
	return rst;
}

void MyTracks::GetSampleValueInfo(const MyBoundingObject& bobj, int& nSample, float& valueSum) const{
	nSample = 0;
	valueSum = 0;
	for (int it = 0; it < this->GetNumTracks(); it++){
		for (int is = 0; is < this->GetNumVertex(it); is++){
			MyVec3f p = this->GetCoord(it, is);
			if (bobj.IsIn(p)){
				float value = this->GetValue(it, is);
				++nSample;
				valueSum += value;
			}
		}
	}
}

void MyTracks::GetSampleValueInfo(const MyBoundingObject& bobj, const MyArrayi& indices, int& nSample, float& valueSum) const{
	nSample = 0;
	valueSum = 0;
	for (int it = 0; it < indices.size(); it++){
		for (int is = 0; is < this->GetNumVertex(indices[it]); is++){
			MyVec3f p = this->GetCoord(indices[it], is);
			if (bobj.IsIn(p)){
				float value = this->GetValue(indices[it], is);
				++nSample;
				valueSum += value;
			}
		}
	}
}

void MyTracks::GetSampleClampedValueInfo(const MyBoundingObject& bobj,
	float minv, float maxv, int& nSample, float& valueSum) const{
	nSample = 0;
	valueSum = 0;
	for (int it = 0; it < this->GetNumTracks(); it++){
		for (int is = 0; is < this->GetNumVertex(it); is++){
			MyVec3f p = this->GetCoord(it, is);
			if (bobj.IsIn(p)){
				float value = this->GetValue(it, is);
				if (value < minv) value = minv;
				if (value > maxv) value = maxv;
				++nSample;
				valueSum += value;
			}
		}
	}
}

void MyTracks::GetSampleClampedValueInfo(const MyBoundingObject& bobj, 
	float minv, float maxv, const MyArrayi& indices, int& nSample, float& valueSum) const{
	nSample = 0;
	valueSum = 0;
	for (int it = 0; it < indices.size(); it++){
		for (int is = 0; is < this->GetNumVertex(indices[it]); is++){
			MyVec3f p = this->GetCoord(indices[it], is);
			if (bobj.IsIn(p)){
				float value = this->GetValue(indices[it], is);
				if (value < minv) value = minv;
				if (value > maxv) value = maxv;
				++nSample;
				valueSum += value;
			}
		}
	}
}

void MyTracks::GetSampleClampedValues(const MyBoundingObject& bobj, float minv, float maxv, MyArrayf& values) const{
	for (int it = 0; it < this->GetNumTracks(); it++){
		for (int is = 0; is < this->GetNumVertex(it); is++){
			MyVec3f p = this->GetCoord(it, is);
			if (bobj.IsIn(p)){
				float value = this->GetValue(it, is);
				if (value < minv) value = minv;
				if (value > maxv) value = maxv;
				values << value;
			}
		}
	}
}

void MyTracks::GetSampleClampedValues(const MyBoundingObject& bobj, float minv, float maxv, const MyArrayi& indices, MyArrayf& values) const{
	for (int i = 0; i < indices.size(); i++){
		int it = indices[i];
		for (int is = 0; is < this->GetNumVertex(it); is++){
			MyVec3f p = this->GetCoord(it, is);
			if (bobj.IsIn(p)){
				float value = this->GetValue(it, is);
				if (value < minv) value = minv;
				if (value > maxv) value = maxv;
				values << value;
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

MyVec3f MyTracks::GetCoord(const MyVec2i& idx) const{
	return GetCoord(idx[0], idx[1]);
}

MyColor4f MyTracks::GetTrackColor(int trackIdx) const{
	int n = mHeader.n_properties;
	if (mTracks[trackIdx].mTrackProperties.size() >= 3){
		return MyColor4f(
			mTracks[trackIdx].mTrackProperties[n - 3],
			mTracks[trackIdx].mTrackProperties[n - 2],
			mTracks[trackIdx].mTrackProperties[n - 1], 1);
	}
	else return MyColor4f(0.5, 0.5, 0.5, 1);
}

MyColor4f MyTracks::GetPointColor(int trackIdx, int vIdx) const{
	return MyColor4f(
		mTracks[trackIdx].mPointScalars[vIdx][0],
		mTracks[trackIdx].mPointScalars[vIdx][1],
		mTracks[trackIdx].mPointScalars[vIdx][2], 1);
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
	if (mTracks[trackIdx].mPointScalars[pointIdx].size() >= 13){
		t.SetEigenValues(&(mTracks[trackIdx].mPointScalars[pointIdx][1]));
		t.SetEigenVectors(&(mTracks[trackIdx].mPointScalars[pointIdx][4]));
	}
	//assert(t.CheckEigenValueOrder());
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
