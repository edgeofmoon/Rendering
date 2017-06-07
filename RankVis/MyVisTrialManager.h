#pragma once

#include "MyVisData.h"
#include "MyVisInfo.h"
#include "MyTracks.h"

class MyVisTrialManager
{
public:
	MyVisTrialManager();
	~MyVisTrialManager();

	void GenerateVisInfo_Training();
	void GenerateVisInfo_Experiment_Random();
	void GenerateVisInfo_LightingProfile();
	void GenerateVisInfo_OcclusionProfile();
	void SetDataRootDir(const MyString& root){ mDataRootDir = root; };
	void SetTracts(const MyTracks* tract){ mTracts = tract; };
	void SetUserIndex(int idx){ mUserIndex = idx; };
	void SetDataIndex(int idx){ mVisDataIndex = idx; };
	bool IsLast() const { return mVisDataIndex == mVisInfos.size() - 1; };
	bool IsFirst() const{ return mVisDataIndex == 0; };

	int GetCurrentVisDataIndex() const { return mVisDataIndex; };
	MyVisData* GotoVisData(int index);
	MyVisData* GetCurrentVisData();
	const MyVisData* GetCurrentVisData() const;
	MyVisData* GotoNextVisData();
	MyVisData* GotoPreviousVisData();

	void GetProgressInfo(int& cur, int& total) const;
	MyString GetProgresInfoString() const;

	void PrintAllCase(const MyString& fileName, const MyString& decimer = "\t") const;

protected:
	int mUserIndex;
	int mVisDataIndex;
	MyArray<MyVisInfo> mVisInfos;
	const MyTracks* mTracts;
	MyVisData* mCurrent;

	// data related
	MyString mDataRootDir;
};

