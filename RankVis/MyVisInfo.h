#pragma once
#include "MyVisEnum.h"
#include "MyArray.h"

class MyVisInfo
{

public:
	MyVisInfo(MyVisEnum::VisTask task, bool isTraining = false);
	MyVisInfo(bool isEmpty, bool isTraining, MyVisEnum::VisTask task, 
		MyVisEnum::RetinalChannel encoding, int method, MyVisEnum::Shape sp, 
		MyVisEnum::VisCue visCue, MyVisEnum::Bundle bundle, 
		MyVisEnum::FiberCover cover, int quest, int resolution, bool needMoreInput = false);
	~MyVisInfo();

	bool IsEmpty() const { return mIsEmpty; };
	bool IsTraining() const{ return mIsTraining; };
	bool NeedMoreInput() const { return mNeedMoreInput; };
	bool IsShowLegend() const{ return mDoShowLegend; };

	void SetShowLegend(int method){ mDoShowLegend = true, mMappingMethod = method; mEncoding = MyVisEnum::COLOR; };
	void SetNeedMoreInput(bool b){ mNeedMoreInput = b; };

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
	MyString GetLegendString() const;
	MyString GetTaskTransitionString() const;
	MyString GetTaskHintString() const;
	int GetNumberAnswerOption() const;
	MyString GetAnswerOptionString(int idx) const;
	MyString GetAnswerHintString() const;

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
	MyString GetTractColorFileName(int idx) const;
	MyString GetBoxFileName(int idx) const;
	MyString GetSphereFileName(int idx) const;

	// for log use
	MyString GetString(const MyString& decimer = "\t") const;
	static MyString GetStringHeader(const MyString& decimer = "\t");

	static MyString GetTaskHintString(MyVisEnum::VisTask task);
	static MyString GetFAAnswerOptionRangeString(int idx);
	static MyString GetTaskAnswerOptionString(MyVisEnum::VisTask task, int idx);
	static MyString GetTaskAnswerHintString(MyVisEnum::VisTask task);
	static const MyArray2f& GetFAAnswerOptionRanges();

protected:
	// transition trial
	bool mIsEmpty;
	// training session
	bool mIsTraining;
	// need confidence
	bool mNeedMoreInput;
	// show legend on empty screen
	bool mDoShowLegend;

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

	static MyArray2f FAAnswerOptionRanges;
};

