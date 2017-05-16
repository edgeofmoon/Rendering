#pragma once

#include <ctime>
#include <fstream>
#include "MyString.h"

class MyEventLog
{
public:
	MyEventLog();
	~MyEventLog();

	void SetEnabled(bool b){ mbEnabled = b; };
	void StartLog(const MyString& pref);
	void SetUserIndex(int uid){ mUserIndex = uid; };
	void LogItem(const MyString& item);
	static void SetLogDirectory(const MyString& dir){ LogDirectory = dir; };

	static MyString Decimer;
protected:
	bool mbEnabled;
	int mUserIndex;
	clock_t mStartClock;

	std::ofstream mOutStream;
	static MyString LogDirectory;
};

