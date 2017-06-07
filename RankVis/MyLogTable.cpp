#include "MyLogTable.h"

#include <iostream>
using namespace std;

MyString MyLogTable::LogDirectory = ".\\";
MyString MyLogTable::Decimer = "\t";

MyLogTable::MyLogTable()
{
	mbPaused = false;
	mbStarted = false;
	mCurrentTrialTotalPausedClock = 0;
}


MyLogTable::~MyLogTable()
{
}

void MyLogTable::StartLog(MyString prefix){
	if (!mbEnabled) return;
	MyString dir = LogDirectory;
	if (dir.back() != '\\') dir.push_back('\\');
	MyString logFileName = dir + prefix+"_" + MyString(mUserIndex) + ".txt";
	mOutStream.open(logFileName);
	if (!mOutStream.is_open()){
		cerr << "Cannot open log file to write: " << logFileName << endl;
	}
	else{
		mOutStream << GetTableHeader() << endl;
	}
	mStartTime = clock();
}

void MyLogTable::EndLog(){
	if (mOutStream.is_open())
		mOutStream.close();
}

void MyLogTable::StartTrial(){
	if (!mbStarted && mbEnabled && !mCurrentVisData->GetVisInfo().IsEmpty()){
		mCureentTrialStartTime = clock();
		mCurrentTrialTotalPausedClock = 0;
		mbStarted = true;
		mbPaused = false;
	}
}

void MyLogTable::EndTrial(){
	if (!mbStarted) return;
	mbStarted = false;
	if (!mOutStream.is_open()) return;
	mOutStream << GetTimeString(time(0)) << Decimer
		<< float(clock() - mStartTime) / CLOCKS_PER_SEC << Decimer
		<< mUserIndex << Decimer
		<< mCurrentTrialIndex << Decimer
		<< mCurrentVisData->GetVisInfo().GetString() << Decimer
		<< GetTimeUsed() << Decimer
		<< GetTimePaused() << Decimer
		<< GetError(mUserAnswer) << Decimer
		<< mUserAnswer << Decimer
		<< mUserConfidence << Decimer
		<< GetCorrectAnswer() << endl;
}

void MyLogTable::PauseTrial(){
	if (!mbStarted) return;
	if (!mbPaused){
		mbPaused = true;
		mCureentTrialPauseStartTime = clock();
	}
}

void MyLogTable::ResumeTrial(){
	if (!mbStarted) return;
	if (mbPaused){
		mCurrentTrialTotalPausedClock += (clock() - mCureentTrialPauseStartTime);
		mbPaused = false;
	}
}

float MyLogTable::GetTimeUsed() const{
	clock_t current = clock();
	int totalTime = current - mCureentTrialStartTime;
	int timePaused = mCurrentTrialTotalPausedClock;
	if (mbPaused){
		timePaused += (current - mCureentTrialPauseStartTime);
	}
	return float(totalTime - timePaused) / CLOCKS_PER_SEC;
}

float MyLogTable::GetTimePaused() const{
	clock_t current = clock();
	int timePaused = mCurrentTrialTotalPausedClock;
	if (mbPaused){
		timePaused += (current - mCureentTrialPauseStartTime);
	}
	return float(timePaused) / CLOCKS_PER_SEC;
}

int MyLogTable::GetError(int userAnswer) const{
	if (mCurrentVisData->GetCorrectAnswers().size() > 0){
		return mCurrentVisData->GetError(userAnswer);
	}
	else return -100;
}

float MyLogTable::GetCorrectAnswer() const{
	if (mCurrentVisData->GetCorrectAnswers().size() > 0){
		return mCurrentVisData->GetCorrectAnswers()[0];
	}
	else return -100;
}

MyString MyLogTable::GetTimeString(std::time_t t){
	struct tm * now = localtime(&t);
	return MyString(now->tm_year + 1900) + '/'
		+ MyString(now->tm_mon + 1) + '/'
		+ MyString(now->tm_mday) + Decimer
		+ MyString(now->tm_hour) + ':'
		+ MyString(now->tm_min) + ':'
		+ MyString(now->tm_sec);
}

MyString MyLogTable::GetTableHeader(){
	MyString header;
	header = "DATE" + Decimer
		+ "LOCALTIME" + Decimer
		+ "EXPTIME" + Decimer
		+ "USER" + Decimer
		+ "TRIAL" + Decimer
		+ MyVisInfo::GetStringHeader(Decimer) + Decimer
		+ "ACT_TIME" + Decimer
		+ "PAUSE_TIME" + Decimer
		+ "ERROR" + Decimer
		+ "USER_ANS" + Decimer
		+ "USER_CNF" + Decimer
		+ "CRT_ANS";
	return header;
}