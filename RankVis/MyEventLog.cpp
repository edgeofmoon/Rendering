#include "MyEventLog.h"

#include <iostream>
using namespace std;

MyString MyEventLog::LogDirectory = ".\\";
MyString MyEventLog::Decimer = "\t";

MyEventLog::MyEventLog()
{
}


MyEventLog::~MyEventLog()
{
	if (mOutStream.is_open()){
		mOutStream.close();
	}
}

void MyEventLog::StartLog(const MyString& prefix){
	if (!mbEnabled) return;
	MyString dir = LogDirectory;
	if (dir.back() != '\\') dir.push_back('\\');
	MyString logFileName = dir + prefix + "_" + MyString(mUserIndex) + ".txt";
	mOutStream.open(logFileName);
	if (!mOutStream.is_open()){
		cerr << "Cannot open log file to write: " << logFileName << endl;
	}
	mStartClock = clock();
}

void MyEventLog::LogItem(const MyString& item){
	if (!mbEnabled) return;
	clock_t current = clock();
	int totalTime = current - mStartClock;
	float seconds = float (totalTime) / CLOCKS_PER_SEC;
	mOutStream << seconds << '\t' << item << endl;
}