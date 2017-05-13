#pragma once

#include <ctime>
#include <fstream>
#include <sstream>

#include "MyVisInfo.h"
#include "MyVisData.h"
#include "MyMatrix.h"

class MyRenderingLog
{
public:
	MyRenderingLog();
	~MyRenderingLog();

	void SetRenderingValues(const MyArrayf& v){ mRenderingValues = v; };
	void SetRenderingValueNames(const MyArrayStr& v){ mRenderingValueNames = v; };
	MyVec3f GetCameraPosition() const;
	const MyVec3f& GetCameraDirection() const;
	float GetCameraDistance() const;
	bool IsEnabled() const { return mbEnabled; };

	void SetEnabled(bool b){ mbEnabled = b; };
	void SetUserIndex(int uid){ mUserIndex = uid; };
	void SetVisData(const MyVisData* visData){ mCurrentVisData = visData; };
	void SetTrialIndex(int idx){ mCurrentTrialIndex = idx; };
	void StartTrial();
	void StartFrame();
	void PauseFrameTimer();
	void ResumeFrameTimer();
	void EndFrame();
	bool IsLastRendering() const;
	void EndTrial();
	void StartLog(const MyString& pref);
	void EndLog();

	static void InitEyePositions(int numDirs = 100, 
		int numDistIn = 10, int numDistOut = 5, float scaleStep = 1.05f);
	static void SetLogDirectory(const MyString& dir){ LogDirectory = dir; };

protected:
	bool mbEnabled;
	std::ofstream mOutStream;
	std::stringstream mStreamBuffer;
	int mUserIndex;
	int mCurrentTrialIndex;
	const MyVisData* mCurrentVisData;
	bool mbStarted;
	bool mbPaused;
	MyArrayf mRenderingValues;
	MyArrayStr mRenderingValueNames;
	int mRenderingClock;
	int mPausedClock;
	int mCurrentIndex;
	clock_t mLastTime;
	clock_t mPauseTime;
	int GetTimeUserd() const;
	MyString GetTableHeader();

	static MyString LogDirectory;
	static MyString Decimer;
	static MyArray3f CameraDirections;
	static MyArrayf CameraDistances;
};

