#include "MyVisTrialManager.h"

#include <iostream>
#include <fstream>
using namespace std;
using namespace MyVisEnum;

#define EXPERIMENT_RES 2
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
	MyArray<VisTask> visTasks = { FA_VALUE, TRACE };
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
	MyArray<DataCombination> dataCombinations;
	for (int j = 0; j < 8; j++){
		dataCombinations << DataCombination(dataBundles[j % 4], dataCovers[j / 4], dataQuests[0]);
	}


	mVisInfos << MyVisInfo(START, true);
	for (int iTask = 0; iTask < 2; iTask++){
		// random seed as user index
		//dataCombinations.Permute(mUserIndex);
		VisTask thisTask = visTasks[iTask];
		mVisInfos << MyVisInfo(thisTask, true);
		MyArrayi colors = { 1, 2, 3, 0, 4, 5 };
		if (thisTask == FA_VALUE){
			//shapeEncodings.Permute(mUserIndex);
			for (int iTrial = 0; iTrial < 6; iTrial++){
				DataCombination dataCombination = dataCombinations[iTrial % 8];
				MyVisInfo visInfo(false, true, thisTask, COLOR, colors[iTrial], TUBE,
					BASIC, dataCombination.bundle, dataCombination.cover,
					dataCombination.quest, TRAINING_RES);
				mVisInfos << visInfo;
			}
		}
		else{
			//shapeCues.Permute(mUserIndex);
			MyArrayi colors = { 4, 3, 1, 0 };
			for (int iTrial = 0; iTrial < 4; iTrial++){
				DataCombination dataCombination = dataCombinations[iTrial % 8];
				MyVisInfo visInfo(false, true, thisTask, COLOR, colors[iTrial], TUBE,
					ENCODING, dataCombination.bundle, dataCombination.cover,
					dataCombination.quest, TRAINING_RES);
				mVisInfos << visInfo;
			}
		}
	}
	mVisInfos << MyVisInfo(END, true);
}

void MyVisTrialManager::GenerateVisInfo_Experiment_Random(){
	MyArray<VisTask> visTasks = { FA_VALUE, TRACE };
	// vis
	int numValueColors = 6;
	int numTractColors = 5;
	// Data
	MyArray<Bundle> dataBundles = { CC, CST, IFO, ILF };
	MyArray<FiberCover> dataCovers = { WHOLE, BUNDLE };
	// database element
	class DataCombination{
	public:
		DataCombination(Bundle b, FiberCover c, int q) :
			bundle(b), cover(c), quest(q){};
		Bundle bundle;
		FiberCover cover;
		int quest;
	};
	MyArray<pair<FiberCover, Bundle>> CoverBundles = {
		make_pair(WHOLE, CC), make_pair(BUNDLE, CC),
		make_pair(WHOLE, CST), make_pair(BUNDLE, CST),
		make_pair(WHOLE, IFO), make_pair(BUNDLE, IFO),
		make_pair(WHOLE, ILF), make_pair(BUNDLE, ILF),
	};

	/****************dcap****************/
	/*
	for (int k = 0; k < 6; k++){
	for (int i = 0; i < 2; i++){
	for (int j = 0; j < 4; j++){
	//mVisInfos
	//	<< MyVisInfo(false, false, TRACE, COLOR, k, TUBE, ENCODING,
	//	dataBundles[j], dataCovers[i], 0, EXPERIMENT_RES);
	mVisInfos
	<< MyVisInfo(false, false, FA_VALUE, COLOR, k, TUBE, BASIC,
	dataBundles[j], dataCovers[i], 0, EXPERIMENT_RES);
	}
	}
	}
	return;
	for (int k = 0; k < 6; k++){
	mVisInfos
	<< MyVisInfo(false, false, FA_VALUE, COLOR, k, TUBE, BASIC, IFO, BUNDLE, 0, EXPERIMENT_RES);
	}
	MyArrayi colors = { 0, 1, 3, 4 };
	for (int k = 0; k < 4; k++){
	mVisInfos
	<< MyVisInfo(false, false, TRACE, COLOR, colors[k], TUBE, ENCODING, CC, BUNDLE, 0, EXPERIMENT_RES);
	}
	return;
	*/
	/****************dcap****************/
	mVisInfos << MyVisInfo(START);
	for (int iTask = 0; iTask < 2; iTask++){
		// random seed as user index
		VisTask thisTask = visTasks[iTask];
		mVisInfos << MyVisInfo(thisTask);
		if (thisTask == FA_VALUE){
			int dataBlock[6][8] = {
				{ 39, 12, 30, 42, 43, 20, 36, 6, },
				{ 44, 33, 13, 14, 9, 35, 32, 26, },
				{ 10, 4, 34, 40, 21, 8, 45, 47, },
				{ 1, 19, 28, 29, 24, 16, 15, 38, },
				{ 0, 17, 23, 46, 5, 41, 37, 11, },
				{ 25, 3, 2, 18, 31, 27, 7, 22 },
			};
			// ref: http://www.plantsciences.ucdavis.edu/agr205/Lectures/2010%20Iago/Topic%207/Useful%20Latin%20Squares.pdf
			int visLatin[6][6] = {
				{ 0, 1, 2, 3, 4, 5 },
				{ 1, 0, 5, 4, 2, 3 },
				{ 2, 5, 1, 0, 3, 4, },
				{ 3, 2, 4, 1, 5, 0 },
				{ 4, 3, 0, 5, 1, 2, },
				{ 5, 4, 3, 2, 0, 1 },
			};
			int infoSize = mVisInfos.size();
			mVisInfos.resize(infoSize + 48, MyVisInfo(thisTask));
			int* visOrder = visLatin[mUserIndex % 6];
			for (int ivis = 0; ivis < 6; ivis++){
				int dataBlockRowIdx = (ivis + mUserIndex) % 6;
				int visLocIdx = visOrder[ivis];
				for (int idata = 0; idata < 8; idata++){
					int dataIdx = dataBlock[dataBlockRowIdx][idata];
					int trialLocIdx = visLocIdx * 8 + idata;
					mVisInfos[infoSize + trialLocIdx] =
						MyVisInfo(false, false, FA_VALUE, COLOR, ivis, TUBE, BASIC,
						dataBundles[(dataIdx / 6) % 4], dataCovers[dataIdx / 24], dataIdx % 6, EXPERIMENT_RES);
				}
			}
			for (int ivis = 0; ivis < 6; ivis++){
				mVisInfos.Permute(mUserIndex*ivis * 123, infoSize + ivis * 8, infoSize + ivis * 8 + 7);
			}
		}
		else{
			// ref: http://www.plantsciences.ucdavis.edu/agr205/Lectures/2010%20Iago/Topic%207/Useful%20Latin%20Squares.pdf
			int latin0[4][4] = {
				{ 0, 1, 2, 3 },
				{ 1, 0, 3, 2 },
				{ 2, 3, 0, 1 },
				{ 3, 2, 1, 0 },
			};
			int latin1[4][4] = {
				{ 0, 1, 2, 3 },
				{ 3, 2, 1, 0 },
				{ 1, 0, 3, 2 },
				{ 2, 3, 0, 1 },
			};
			MyArrayi colors = { 0, 1, 3, 4 }; // remove color 2
			for (int ilatin = 0; ilatin < 4; ilatin++){
				int color = colors[latin0[mUserIndex % 4][ilatin]];
				int quest = latin1[mUserIndex % 4][ilatin];
				int infoSize = mVisInfos.size();
				for (int ibd = 0; ibd < 8; ibd++){
					mVisInfos <<
						MyVisInfo(false, false, TRACE, COLOR, color, TUBE, ENCODING,
						dataBundles[ibd % 4], dataCovers[ibd / 4], quest, EXPERIMENT_RES);
				}
				mVisInfos.Permute(mUserIndex*ilatin * 320, infoSize, mVisInfos.size() - 1);
			}
		}
	}
	mVisInfos << MyVisInfo(END);

	/*******dumpled 7/18/2017*********/
	/*
	mVisInfos << MyVisInfo(START);
	for (int iTask = 0; iTask < 2; iTask++){
	// random seed as user index
	VisTask thisTask = visTasks[iTask];
	mVisInfos << MyVisInfo(thisTask);
	if (thisTask == FA_VALUE){
	MyArray<pair<FiberCover, Bundle>> cbds = CoverBundles;
	cbds.Permute(mUserIndex);
	MyArray<DataCombination> dataCombinations;
	for (int i = 0; i < numValueColors * CoverBundles.size(); i++){
	dataCombinations << DataCombination(cbds[i / numValueColors].second,
	cbds[i / numValueColors].first, i % numValueColors);
	if (i % numValueColors == numValueColors - 1){
	dataCombinations.Permute(mUserIndex * 101 * i, i - numValueColors + 1, i);
	}
	}
	MyArrayi colorMaps = MyArrayi::GetSequence(0, numValueColors - 1);
	colorMaps.Permute(mUserIndex);
	for (int i = 0; i < numValueColors; i++){
	int infoSize = mVisInfos.size();
	for (int idata = 0; idata < CoverBundles.size(); idata++){
	DataCombination combo = dataCombinations[idata * numValueColors + i];
	MyVisInfo visInfo(false, false, FA_VALUE, COLOR, colorMaps[i], TUBE, BASIC,
	combo.bundle, combo.cover, combo.quest, EXPERIMENT_RES);
	mVisInfos << visInfo;
	}
	mVisInfos.Permute(mUserIndex*i * 144, infoSize, mVisInfos.size() - 1);
	}
	}
	else{
	MyArray<pair<FiberCover, Bundle>> cbds = CoverBundles;
	cbds.Permute(mUserIndex * 153);
	MyArray<DataCombination> dataCombinations;
	for (int i = 0; i < numTractColors * CoverBundles.size(); i++){
	dataCombinations << DataCombination(cbds[i / numTractColors].second,
	cbds[i / numTractColors].first, i % numTractColors);
	if (i % numTractColors == numTractColors - 1){
	dataCombinations.Permute(mUserIndex * 191 * i, i - numTractColors + 1, i);
	}
	}
	MyArrayi tractColors = MyArrayi::GetSequence(0, numTractColors - 1);
	tractColors.Permute(mUserIndex);
	for (int i = 0; i < numTractColors; i++){
	int infoSize = mVisInfos.size();
	for (int idata = 0; idata < CoverBundles.size(); idata++){
	DataCombination combo = dataCombinations[idata * numTractColors + i];
	MyVisInfo visInfo(false, false, TRACE, COLOR, tractColors[i], TUBE, ENCODING,
	combo.bundle, combo.cover, combo.quest, EXPERIMENT_RES);
	mVisInfos << visInfo;
	}
	mVisInfos.Permute(mUserIndex*i * 144, infoSize, mVisInfos.size() - 1);
	}
	}
	}
	mVisInfos << MyVisInfo(END);
	*/
}

void MyVisTrialManager::GenerateVisInfo_LightingProfile(){
	MyArray<Shape> visShapes = { TUBE, LINE };
	MyArray<VisCue> visCues = { AMBIENT_OCCULUSION, DEPTH, BASIC, ENCODING };
	//MyArray<Shape> visShapes = { LINE };
	//MyArray<VisCue> visCues = { DEPTH };
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
		for (int i = 0; i < 1; i++){
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
			if (i <= mVisDataIndex) a++;
		}
	}
	cur = a;
	total = b;
}

MyString MyVisTrialManager::GetProgresInfoString() const{
	int cur, total;
	GetProgressInfo(cur, total);
	return MyString(cur) + "/" + MyString(total);
}

void MyVisTrialManager::PrintAllCase(const MyString& fileName, const MyString& decimer) const{
	ofstream outFile(fileName);
	if (!outFile.is_open()){
		cerr << "Cannot open file to write: " << fileName << endl;
		return;
	}
	outFile << "USERIDX" << decimer
		<< "TRIALIDX" << decimer
		<< MyVisInfo::GetStringHeader(decimer) << decimer
		<< "CRT_ANS" << endl;
	for (int i = 0; i < mVisInfos.size(); i++){
		if (mVisInfos[i].IsEmpty()) continue;
		MyVisData* visData = new MyVisData(mVisInfos[i]);
		visData->SetTracts(mTracts);
		visData->LoadFromDirectory(mDataRootDir);
		outFile << mUserIndex << decimer
			<< i << decimer
			<< mVisInfos[i].GetString(decimer) << decimer;

		if (visData->GetCorrectAnswers().size() > 0){
			outFile << visData->GetCorrectAnswers()[0] << endl;
		}
		else {
			outFile << visData->GetAnswerInfo() << endl;
		}
		delete visData;
	}
}