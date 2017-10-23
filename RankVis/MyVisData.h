#pragma once

#include "MyArray.h"
#include "MyTracks.h"
#include "MyVisInfo.h"
#include "MySphere.h"
#include "MyVisEnum.h"

class MyVisData
{
public:
	MyVisData(MyVisInfo info);
	~MyVisData();

	void LoadFromDirectory(const MyString& dir);
	void UpdateAnswers();
	int GetError(int userAnswer) const;
	float GetError(float userAnswer) const;
	MyString GetCorrectAnswerString() const;
	const MyTracks* GetTracts() const { return mTracts; };
	void SetTracts(const MyTracks* tracts) { mTracts = tracts; };

	const MyVisInfo& GetVisInfo() const {return mVisInfo;};

	const MyArrayi& GetTractIndices() const { return mTractIndices; };
	const MyArrayi& GetSelectIndices() const { return mTractSelectIndices; };
	const MyArray<MyColor4f>& GetTractColors() const { return mTractColors; };
	const MyArray<MyBoundingBox>& GetBoxes() const { return mBoxes; };
	const MyArray<MySphere>& GetSpheres() const { return mSpheres; };
	const MySphere& GetSphere(int idx) const { return mSpheres[idx]; };
	const MyArrayi& GetCorrectAnswers() const { return mCorrectAnswers; };
	const MyArrayb& GetSphereAtHead() const { return mSphereAtHead; };
	const MyBoundingBox& GetBoundingBox() const { return mBoundingBox; };
	const float GetAnswerInfo() const { return mAnswerInfo; };
	const bool IsAnswerCorrect(int ans) const { return mCorrectAnswers.HasOne(ans); };

	void Clear();

	static float GetTouchRange() { return TouchRange; };
	static float GetMinSphereRadius() { return mMinSphereRadius; };
	static float GetMaxSphereRadius() { return mMaxSphereRadius; };

protected:
	// preset data
	const MyTracks* mTracts;
	MyVisInfo mVisInfo;
	static float TouchRange;
	static float mMinSphereRadius, mMaxSphereRadius;

	// data read from files
	MyArrayi mTractIndices;
	MyArrayi mTractSelectIndices;
	MyArray<MyColor4f> mTractColors;
	MyArray<MyBoundingBox> mBoxes;
	MyArray<MySphere> mSpheres;
	MyArrayi mCorrectAnswers;

	// derive info
	MyArrayb mSphereAtHead;
	MyBoundingBox mBoundingBox;
	float mAnswerInfo;

private:
	MyString mBaseDirectory;
	MyString mDataPath;
	// Per task loading functions
	int LoadTractIndices(const MyString& fileName);
	void LoadData_FA();
	void LoadData_TRACE();
	void LoadData_SAME();
	void LoadData_TUMOR();
	void LoadData_FA_VALUE();

	void LoadData_Box(int numBox);
	void LoadData_SelectIndices();
	void LoadData_TractColors();
	void LoadData_Sphere();
	void ComputeAnswer_FA();
	void ComputeAnswer_TRACE();
	void ComputeAnswer_SAME();
	void ComputeAnswer_TUMOR();
	void ComputeAnswer_FA_VALUE();

public:
	// for adding data
	void CreateNewQuestFolder() const;
	// FA
	void SetBox(const MyBoundingBox& box, int idx);
	void SaveBoxFiles(const MyArray<MyBoundingBox>& boxes) const;
	float GetBoxValues(const MyBoundingBox& box) const;

	// TRACE
	void SetSelectedIndices(const MyArrayi& selected);
	void CheckSelectedValidness();
	void SaveSelectedIndicesFile(const MyArrayi& selected) const;
	void PermuteBoxes(int seed) { mBoxes.Permute(seed); };

	// TUMOR
	void SetSphere(const MySphere& sphere);
	void SaveTumorFile(const MySphere& sphere) const;
	bool MoveSphereToStatus(MySphere& sphere, MyVisEnum::CollisionStatus st,
		int maxItr = 10) const;
	MyVisEnum::CollisionStatus ComputeSphereCollusionStatus(
		const MySphere& sphere) const;
	MyVec2i ComputeClosestSegment(const MyVec3f& p) const;
	bool ComputeNearbyPointAtDist(MyVec3f& rst, const MyVec3f& p,
		float minDist, float distRange, int maxItr = 10) const;
	MyArrayi ComputeIntersectedIndices(const MySphere& sphere) const;
	MyArrayi ComputeTouchedIndices(const MySphere& sphere) const;

	// FA_VALUE
	float GetExpectedValueFromQuestIdx() const;
	void ComputeBoxesWithValue(MyArray<MyBoundingBox>& boxes, MyArray2f& avgs, float v, float err, int minSegs = 20, int maxSegs = 9999);
	static void WriteVectorToFile(const MyString& fn, const MyArray2f& vecs);

protected:
	static MyVisEnum::CollisionStatus SphereCollusionStatus(float radius,
		float closetCenterDist);
};

