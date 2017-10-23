#pragma once

#include <ctime>
#include <fstream>
#include "MyVisInfo.h"
#include "MyVisData.h"

typedef struct LogItem_Analysis_t{
	// date and time 2 fields
	std::time_t time;
	int userIndex;
	int trialIndex;
	MyVisInfo visInfo;
	float secondUsed;
	float answerError;
	float userAnswer;
	float correctAnswer;
}LogItem_Analysis;

class MyLogTable
{
public:
	MyLogTable();
	~MyLogTable();

	bool IsStarted() const { return mbStarted; };
	bool NeedAtLeastOneFrame() const;

	void SetEnabled(bool b){ mbEnabled = b; };
	bool IsEnabled() const { return mbEnabled; };
	void SetUserIndex(int uid){ mUserIndex = uid; };
	void SetVisData(const MyVisData* visData){ mCurrentVisData = visData; };
	void SetTrialIndex(int idx){ mCurrentTrialIndex = idx; };
	void SetUserAnswer(int ua){ mUserAnswerType = 1;  mUserAnswerInt = ua; };
	void SetUserAnswer(float ua){ mUserAnswerType = 2; mUserAnswerFloat = ua; };
	void SetUserConfidence(int cf){ mUserConfidence = cf; };
	void StartLog(MyString prefix = "log");
	void EndLog();
	void StartTrial();
	void EndTrial();
	void PauseTrial();
	void ResumeTrial();

	static void SetLogDirectory(const MyString& dir){ LogDirectory = dir; };

protected:
	bool mbEnabled;
	std::ofstream mOutStream;
	int mUserIndex;
	int mCurrentTrialIndex;
	int mUserAnswerType;
	int mUserAnswerInt;
	float mUserAnswerFloat;
	int mUserConfidence;
	const MyVisData* mCurrentVisData;
	bool mbPaused, mbStarted;
	clock_t mCureentTrialStartTime;
	clock_t mCureentTrialPauseStartTime;
	clock_t mStartTime;
	int mCurrentTrialTotalPausedClock;
	float GetTimeUsed() const;
	float GetTimePaused() const;
	float GetError() const;
	float GetCorrectAnswer() const;

	static MyString LogDirectory;
	static MyString Decimer;
	static MyString GetTimeString(std::time_t t);
	static MyString GetTableHeader();
};

