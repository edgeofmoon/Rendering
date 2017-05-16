#include "MyVisTrialManager.h"

#include <iostream>
using namespace std;
using namespace MyVisEnum;

#define EXPERIMENT_RES 3
#define TRAINING_RES 4

MyVisTrialManager::MyVisTrialManager(){
	mCurrent = NULL;
	mVisDataIndex = -1;
	mDataRootDir = "./";
	mUserIndex = 0;
}


MyVisTrialManager::~MyVisTrialManager(){
	if (mCurrent) delete mCurrent;
}

void MyVisTrialManager::GenerateVisInfo_Training(){
	MyArray<VisTask> visTasks = { FA, TRACE, TUMOR };
	// Vis
	MyArray<RetinalChannel> visEncodings = { COLOR, VALUE, ESIZE, TEXTURE };
	MyArray<Shape> visShapes = { TUBE, SUPERQUADRIC, LINE };
	MyArray<VisCue> visCues = { BASIC, /*CURV_MIN,*/ DEPTH, AMBIENT_OCCULUSION, ENCODING };
	// Data
	MyArray<Bundle> dataBundles = { CC, CST, IFO, ILF };
	MyArray<FiberCover> dataCovers = { BUNDLE, WHOLE };
	MyArrayi dataQuests = { 0 };
	// database element
	class DataCombination{
	public:
		DataCombination(Bundle b, FiberCover c, int q) :
			bundle(b), cover(c), quest(q){};
		Bundle bundle;
		FiberCover cover;
		int quest;
	};

	MyArray<pair<Shape, RetinalChannel>> shapeEncodings = {
		make_pair(TUBE, COLOR), make_pair(TUBE, VALUE),
		make_pair(TUBE, ESIZE), make_pair(TUBE, TEXTURE),
		make_pair(SUPERQUADRIC, COLOR), make_pair(SUPERQUADRIC, VALUE) };
	MyArray<pair<Shape, VisCue>> shapeCues = {
		make_pair(TUBE, BASIC), make_pair(LINE, BASIC),
		//make_pair(TUBE, CURV_MIN),	make_pair(LINE, CURV_MIN),
		make_pair(TUBE, DEPTH), make_pair(LINE, DEPTH),
		make_pair(TUBE, AMBIENT_OCCULUSION), make_pair(LINE, AMBIENT_OCCULUSION),
		make_pair(TUBE, ENCODING), make_pair(LINE, ENCODING),
	};
	MyArray<DataCombination> dataCombinations;
	for (int j = 0; j < 8; j++){
		dataCombinations << DataCombination(dataBundles[j % 4], dataCovers[j / 4], dataQuests[0]);
	}
	mVisInfos << MyVisInfo(START, true);
	for (int iTask = 0; iTask < 3; iTask++){
		// random seed as user index
		//dataCombinations.Permute(mUserIndex);
		VisTask thisTask = visTasks[iTask];
		mVisInfos << MyVisInfo(thisTask, true);
		if (thisTask == FA){
			//shapeEncodings.Permute(mUserIndex);
			for (int iTrial = 0; iTrial < 6; iTrial++){
				pair<Shape, RetinalChannel>shapeEncoding = shapeEncodings[iTrial % 6];
				DataCombination dataCombination = dataCombinations[iTrial % 8];
				MyVisInfo visInfo(false, true, thisTask, shapeEncoding.second, 0, shapeEncoding.first,
					BASIC, dataCombination.bundle, dataCombination.cover,
					dataCombination.quest, TRAINING_RES);
				mVisInfos << visInfo;
			}
		}
		else{
			//shapeCues.Permute(mUserIndex);
			for (int iTrial = 0; iTrial < 8; iTrial++){
				pair<Shape, VisCue> shapeCue = shapeCues[iTrial % 8];
				DataCombination dataCombination = dataCombinations[iTrial % 8];
				MyVisInfo visInfo(false, true, thisTask, COLOR, 0, shapeCue.first,
					shapeCue.second, dataCombination.bundle, dataCombination.cover,
					dataCombination.quest, TRAINING_RES);
				mVisInfos << visInfo;
			}
		}
	}
	mVisInfos << MyVisInfo(END, true);
}

void MyVisTrialManager::GenerateVisInfo_Experiment_Random(){
	MyArray<VisTask> visTasks = { FA, TRACE, TUMOR };
	// Vis
	MyArray<RetinalChannel> visEncodings = {COLOR, VALUE, ESIZE, TEXTURE};
	MyArray<Shape> visShapes = {TUBE, SUPERQUADRIC, LINE};
	MyArray<VisCue> visCues = { BASIC, /*CURV_MIN,*/ DEPTH, AMBIENT_OCCULUSION, ENCODING};
	// Data
	MyArray<Bundle> dataBundles = {CC, CST, IFO, ILF};
	MyArray<FiberCover> dataCovers = {BUNDLE, WHOLE};
	MyArrayi dataQuests = { 0, 1, 2, 3, 4, 5 };
	// database element
	class DataCombination{
	public:
		DataCombination(Bundle b, FiberCover c, int q) :
			bundle(b), cover(c), quest(q){};
		Bundle bundle;
		FiberCover cover;
		int quest;
	};

	MyArray<pair<Shape, RetinalChannel>> shapeEncodings = {
		make_pair(TUBE, COLOR), make_pair(TUBE, VALUE),
		make_pair(TUBE, ESIZE), make_pair(TUBE, TEXTURE),
		make_pair(SUPERQUADRIC, COLOR), make_pair(SUPERQUADRIC, VALUE) };
	MyArray<pair<Shape, VisCue>> shapeCues = {
		make_pair(TUBE, BASIC), make_pair(LINE, BASIC),
		//make_pair(TUBE, CURV_MIN),	make_pair(LINE, CURV_MIN),
		make_pair(TUBE, DEPTH), make_pair(LINE, DEPTH),
		make_pair(TUBE, AMBIENT_OCCULUSION), make_pair(LINE, AMBIENT_OCCULUSION),
		make_pair(TUBE, ENCODING), make_pair(LINE, ENCODING),
	};
	MyArray<DataCombination> dataCombinations;
	for (int j = 0; j < 48; j++){
		dataCombinations << DataCombination(dataBundles[(j % 24) / 6], dataCovers[j / 24], dataQuests[j % 6]);
	}
	mVisInfos << MyVisInfo(START);
	for (int iTask = 0; iTask < 3; iTask++){
		// random seed as user index
		dataCombinations.Permute(mUserIndex);
		VisTask thisTask = visTasks[iTask];
		mVisInfos << MyVisInfo(thisTask);
		if (thisTask == FA){

			shapeEncodings.Permute(mUserIndex);
			for (int iTrial = 0; iTrial < 48; iTrial++){
				pair<Shape, RetinalChannel>shapeEncoding = shapeEncodings[iTrial % 6];
				DataCombination dataCombination = dataCombinations[iTrial];
				MyVisInfo visInfo(false, false, thisTask, shapeEncoding.second, 0, shapeEncoding.first,
					BASIC, dataCombination.bundle, dataCombination.cover,
					dataCombination.quest, EXPERIMENT_RES);
				mVisInfos << visInfo;
			}
		}
		else{
			shapeCues.Permute(mUserIndex);
			for (int iTrial = 0; iTrial < 48; iTrial++){
				pair<Shape, VisCue> shapeCue = shapeCues[iTrial % 8];
				DataCombination dataCombination = dataCombinations[iTrial];
				MyVisInfo visInfo(false, false, thisTask, COLOR, 0, shapeCue.first,
					shapeCue.second, dataCombination.bundle, dataCombination.cover,
					dataCombination.quest, EXPERIMENT_RES);
				mVisInfos << visInfo;
			}
		}
	}
	mVisInfos << MyVisInfo(END);
}

void MyVisTrialManager::GenerateVisInfo_LightingProfile(){
	MyArray<Shape> visShapes = { LINE, TUBE };
	MyArray<VisCue> visCues = { BASIC, AMBIENT_OCCULUSION, DEPTH, ENCODING };
	MyArray<Bundle> dataBundles = { CC, CST, IFO, ILF };
	MyArray<FiberCover> dataCovers = { WHOLE, BUNDLE };
	for (int i = 0; i < 8; i++){
		for (int j = 0; j < 8; j++){
			MyVisInfo visInfo(false, false, TRACE, COLOR, 0, visShapes[i / 4], visCues[i % 4],
				dataBundles[j % 4], dataCovers[j / 4], 0, EXPERIMENT_RES);
			mVisInfos << visInfo;
		}
	}
}

void MyVisTrialManager::GenerateVisInfo_OcclusionProfile(){
	MyArray<Bundle> dataBundles = { CC, CST, IFO, ILF };
	MyArray<FiberCover> dataCovers = { WHOLE, BUNDLE };
	MyArray<Shape> visShapes = { TUBE, SUPERQUADRIC };

	for (int j = 0; j < 8; j++){
		for (int i = 0; i < 2; i++){
			MyVisInfo visInfo(false, false, FA, COLOR, 0, visShapes[i], BASIC,
				dataBundles[j % 4], dataCovers[j / 4], 0, EXPERIMENT_RES);
			mVisInfos << visInfo;
		}
	}
}

MyVisData* MyVisTrialManager::GotoVisData(int index){
	if (mCurrent) delete mCurrent;
	mCurrent = new MyVisData(mVisInfos[index]);
	mCurrent->SetTracts(mTracts);
	mCurrent->LoadFromDirectory(mDataRootDir);
	return mCurrent;
}

MyVisData* MyVisTrialManager::GetCurrentVisData(){
	return mCurrent;
}

const MyVisData* MyVisTrialManager::GetCurrentVisData() const{
	return mCurrent;
}

MyVisData* MyVisTrialManager::GotoNextVisData(){
	return GotoVisData(++mVisDataIndex);
}

MyVisData* MyVisTrialManager::GotoPreviousVisData(){
	return GotoVisData(--mVisDataIndex);
}

void MyVisTrialManager::GetProgressInfo(int& cur, int& total) const{
	int a = 0;
	int b = 0;
	for (int i = 0; i < mVisInfos.size(); i++){
		if (!mVisInfos[i].IsEmpty()){
			b++;
			if (i < mVisDataIndex) a++;
		}
	}
	cur = a+1;
	total = b;
}

MyString MyVisTrialManager::GetProgresInfoString() const{
	int cur, total;
	GetProgressInfo(cur, total);
	return MyString(cur) + "/" + MyString(total);
}