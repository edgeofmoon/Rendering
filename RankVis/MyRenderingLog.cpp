#include "MyRenderingLog.h"

#include <iostream>
using namespace std;

MyString MyRenderingLog::LogDirectory = ".\\";
MyString MyRenderingLog::Decimer = "\t";
MyArray3f MyRenderingLog::CameraDirections;
MyArrayf MyRenderingLog::CameraDistances;

MyRenderingLog::MyRenderingLog(){
	mCurrentIndex = 0;
	mbEnabled = true;
}


MyRenderingLog::~MyRenderingLog()
{
}

MyVec3f MyRenderingLog::GetCameraPosition() const{
	return GetCameraDirection() * GetCameraDistance();
}

const MyVec3f& MyRenderingLog::GetCameraDirection() const{
	int j = mCurrentIndex / CameraDistances.size();
	return CameraDirections[j];
}

float MyRenderingLog::GetCameraDistance() const{
	int i = mCurrentIndex % CameraDistances.size();
	return CameraDistances[i];
}

void MyRenderingLog::StartLog(const MyString& pref){
	if (!mbEnabled) return;
	MyString dir = LogDirectory;
	if (dir.back() != '\\') dir.push_back('\\');
	MyString logFileName = dir + pref + "_" + MyString(mUserIndex) + ".txt";
	mOutStream.open(logFileName);
	if (!mOutStream.is_open()){
		cerr << "Cannot open log file to write: " << logFileName << endl;
	}
	else{
		mOutStream << GetTableHeader() << endl;
	}
}

void MyRenderingLog::EndLog(){
	if (mOutStream.is_open())
		mOutStream.close();
}

void MyRenderingLog::StartTrial(){
	if (!mbStarted && mOutStream.is_open() && mbEnabled){
		mbStarted = true;
		mCurrentIndex = 0;
		mLastTime = clock();
		mbPaused = false;
	}
}

void MyRenderingLog::StartFrame(){
	if (mbStarted){
		clock_t currentTime = clock();
		mRenderingClock = currentTime - mLastTime;
		mLastTime = currentTime;
		mPausedClock = 0;
	}
}

void MyRenderingLog::PauseFrameTimer(){
	if (mbStarted && !mbPaused){
		mPauseTime = clock();
		mbPaused = true;
	}
}

void MyRenderingLog::ResumeFrameTimer(){
	if (mbStarted && mbPaused){
		mPausedClock += clock() - mPauseTime;
		mbPaused = false;
	}
}

void MyRenderingLog::EndFrame(){
	if (mbStarted){
		float fps = CLOCKS_PER_SEC;
		int timeUsed = GetTimeUserd();
		if (timeUsed != 0)
			fps = CLOCKS_PER_SEC / float(timeUsed);
		MyVec3f cameraDir = GetCameraDirection();
		mStreamBuffer << mUserIndex << Decimer
			<< mCurrentTrialIndex << Decimer
			<< mCurrentVisData->GetVisInfo().GetString() << Decimer
			<< cameraDir[0] << Decimer
			<< cameraDir[1] << Decimer
			<< cameraDir[2] << Decimer
			<< GetCameraDistance() << Decimer
			<< float(timeUsed) / CLOCKS_PER_SEC << Decimer
			<< fps;
		for (int i = 0; i < mRenderingValues.size(); i++){
			mStreamBuffer << Decimer << mRenderingValues[i];
		}
		mStreamBuffer << endl;
		++mCurrentIndex;

		mOutStream << mStreamBuffer.str();
		mStreamBuffer.str(std::string());
	}
}

bool MyRenderingLog::IsLastRendering() const{
	return mCurrentIndex >= CameraDirections.size()*CameraDistances.size() - 1;
}

void MyRenderingLog::EndTrial(){
	if (mbStarted){
		mbStarted = false;
		mOutStream << mStreamBuffer.str();
		mStreamBuffer.str(std::string());
	}
}

int MyRenderingLog::GetTimeUserd() const{
	clock_t current = clock();
	int totalClock = current - mLastTime;
	totalClock -= mPausedClock;
	if (mbPaused){
		totalClock -= (current - mPauseTime);
	}
	return totalClock;
}

MyString MyRenderingLog::GetTableHeader(){
	MyString header;
	header = "USER" + Decimer
		+ "TRIAL" + Decimer
		+ MyVisInfo::GetStringHeader(Decimer) + Decimer
		+ "EYEDIR_X" + Decimer
		+ "EYEDIR_Y" + Decimer
		+ "EYEDIR_Z" + Decimer
		+ "EYEDIST" + Decimer
		+ "SECONDS" + Decimer
		+ "FPS";
	for (int i = 0; i < mRenderingValueNames.size(); i++){
		header += (Decimer + mRenderingValueNames[i]);
	}
	return header;
}

void MyRenderingLog::InitEyePositions(
	int numDirs, int numDistIn, int numDistOut, float scaleStep){
	int i = 0;
	CameraDirections.clear();
	while (i < numDirs){
		float x = rand() / float(RAND_MAX) - 0.5f;
		float y = rand() / float(RAND_MAX) - 0.5f;
		float z = rand() / float(RAND_MAX) - 0.5f;
		MyVec3f eyePos(x, y, z);
		if (eyePos.norm() > 0.01){
			eyePos.normalize();
			CameraDirections << eyePos;
			i++;
		}
	}
	CameraDistances.clear();
	double baseDist = 150;
	double base = scaleStep;
	for (int i = numDistOut; i >= 1; i--){
		CameraDistances << baseDist * pow(base, i);
	}
	CameraDistances << baseDist;
	for (int i = 1; i <= numDistIn; i++){
		CameraDistances << baseDist / pow(base, i);
	}

}