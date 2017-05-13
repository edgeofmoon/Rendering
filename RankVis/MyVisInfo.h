#pragma once
#include "MyVisEnum.h"
#include "MyArray.h"

class MyVisInfo
{

public:
	MyVisInfo(MyVisEnum::VisTask task, bool isTraining = false);
	MyVisInfo(bool isEmpty, bool isTraining, MyVisEnum::VisTask task, MyVisEnum::RetinalChannel encoding, int method,
		MyVisEnum::Shape sp, MyVisEnum::VisCue visCue, MyVisEnum::Bundle bundle, MyVisEnum::FiberCover cover, int quest, int resolution);
	~MyVisInfo();

	// transition trial
	bool mIsEmpty;
	// training session
	bool mIsTraining;

	bool IsEmpty() const { return mIsEmpty; };
	bool IsTraining() const{ return mIsTraining; };

	MyVisEnum::RetinalChannel GetEncoding() const { return mEncoding; };
	int GetMappingMethod() const { return mMappingMethod; };
	MyVisEnum::Shape GetShape() const { return mShape; };
	MyVisEnum::VisCue GetVisCue() const { return mVisCue; };
	MyVisEnum::VisTask GetVisTask() const { return mTask; };
	MyVisEnum::Bundle GetBundle() const { return mBundle; };
	MyVisEnum::FiberCover GetCover() const { return mCover; };
	int GetResolution() const { return mResolution; };
	int GetQuest() const { return mQuest; };

	// task questions
	MyString GetTaskTransitionString() const;
	MyString GetTaskHintString() const;
	static MyString GetTaskHintString(MyVisEnum::VisTask task);

	// folder names
	int GetTaskRawIndex() const;
	MyString GetTaskFolderName() const;
	MyString GetCoverFolderName() const;
	MyString GetResolutionFolderName() const;
	MyString GetBundleFolderName() const;
	MyString GetQuestFolderName() const;

	// file names
	MyString GetTractIndexFileName() const;
	MyString GetTractSelectFileName() const;
	MyString GetBoxFileName(int idx) const;
	MyString GetSphereFileName(int idx) const;

	// for log use
	MyString GetString(const MyString& decimer = "\t") const;
	static MyString GetStringHeader(const MyString& decimer = "\t");

protected:
	// vis related
	MyVisEnum::RetinalChannel mEncoding;
	int mMappingMethod;
	MyVisEnum::Shape mShape;
	MyVisEnum::VisCue mVisCue;
	MyVisEnum::VisTask mTask;

	// data related
	MyVisEnum::Bundle mBundle;
	MyVisEnum::FiberCover mCover;
	// resolution 1~5 : high~low
	int mResolution;
	// each has two different quests related data 1 or 2
	int mQuest;
};

