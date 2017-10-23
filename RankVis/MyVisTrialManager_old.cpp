#include "MyVisTrialManager.h"

#include <iostream>
#include <fstream>
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
	MyArrayi dataQuests = { 0, 1, 2, 3, 4, 5, 6 };
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
	MyArray<pair<FiberCover, Bundle>> CoverBundles = {
		make_pair(BUNDLE, CC), make_pair(WHOLE, CC),
		make_pair(BUNDLE, CST), make_pair(WHOLE, CST),
		make_pair(BUNDLE, IFO), make_pair(WHOLE, IFO),
		make_pair(BUNDLE, ILF)
	};
	/**********dcap*****************/
	/*
	for (int i = 0; i < 8; i++){
		MyVisInfo visInfo(false, false, TRACE, COLOR, 0, shapeCues[i].first,
			shapeCues[i].second, CC, BUNDLE, 0, EXPERIMENT_RES);
		//MyVisInfo visInfo(false, false, FA, shapeEncodings[i].second, 0, shapeEncodings[i].first,
		//	BASIC, IFO, BUNDLE, 0, EXPERIMENT_RES);
		mVisInfos << visInfo;
	}
	return;
	*/
	/*
	for (int i = 0; i < 56; i++){
		//MyVisInfo visInfo(false, false, TRACE, COLOR, 0, shapeCues[i].first,
		//	shapeCues[i].second, CC, BUNDLE, 0, EXPERIMENT_RES);
		MyVisInfo visInfo(false, false, FA, COLOR, i%14, TUBE,
			BASIC, dataBundles[(i / 7) % 4], dataCovers[i / 28], dataQuests[i % 7], EXPERIMENT_RES);
		mVisInfos << visInfo;
	}
	*/

	/*
	for (int i = 0; i < 6; i++){
		for (int j = 0; j < 56; j+=7){
			MyVisInfo visInfo(false, false, FA, shapeEncodings[i].second, 0, shapeEncodings[i].first,
				BASIC, dataBundles[(j / 7) % 4], dataCovers[j / 28], dataQuests[j % 7], EXPERIMENT_RES);
			mVisInfos << visInfo;
		}
	}
	*/
	/*
	for (int i = 0; i < 8; i++){
		for (int j = 0; j < 48; j+=6){
			MyVisInfo visInfo(false, false, TRACE, COLOR, 0, shapeCues[i].first, shapeCues[i].second, 
				dataBundles[(j / 6) % 4], dataCovers[j / 6 / 4], dataQuests[j % 6], EXPERIMENT_RES);
			mVisInfos << visInfo;
		}
	}
	mVisInfos
		<< MyVisInfo(false, false, FA, COLOR, 0, TUBE, BASIC, CC, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, FA, VALUE, 0, TUBE, BASIC, CC, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, FA, ESIZE, 0, TUBE, BASIC, CC, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, FA, TEXTURE, 0, TUBE, BASIC, CC, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, FA, COLOR, 0, SUPERQUADRIC, BASIC, CC, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, FA, VALUE, 0, SUPERQUADRIC, BASIC, CC, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TRACE, COLOR, 0, TUBE, BASIC, CC, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TRACE, COLOR, 0, TUBE, DEPTH, CC, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TRACE, COLOR, 0, TUBE, AMBIENT_OCCULUSION, CC, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TRACE, COLOR, 0, TUBE, ENCODING, CC, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TRACE, COLOR, 0, LINE, BASIC, CC, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TRACE, COLOR, 0, LINE, DEPTH, CC, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TRACE, COLOR, 0, LINE, AMBIENT_OCCULUSION, CC, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TRACE, COLOR, 0, LINE, ENCODING, CC, BUNDLE, 0, EXPERIMENT_RES);
	*/
	/*
	mVisInfos
		<< MyVisInfo(false, false, FA, VALUE, 0, SUPERQUADRIC, BASIC, CC, WHOLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, FA, ESIZE, 0, TUBE, BASIC, ILF, BUNDLE, 2, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TRACE, COLOR, 0, TUBE, ENCODING, CC, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TRACE, COLOR, 0, LINE, BASIC, CST, WHOLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TUMOR, COLOR, 0, TUBE, BASIC, IFO, BUNDLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TUMOR, COLOR, 0, TUBE, DEPTH, ILF, WHOLE, 5, EXPERIMENT_RES);
	*/
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
	/**********dcap*****************/
	/*******Experiment Used*********/
	shapeEncodings.Permute(mUserIndex);
	shapeCues.Permute(mUserIndex);
	mVisInfos << MyVisInfo(START);
	for (int iTask = 0; iTask < 3; iTask++){
		// random seed as user index
		VisTask thisTask = visTasks[iTask];
		mVisInfos << MyVisInfo(thisTask);
		if (thisTask == FA){
			// make sure under each vis
			// there always will be 7 quest from 0~7
			// and 7 different bunlde*cover combins
			MyArray<DataCombination> dataTable;
			for (int iq = 0; iq < 7; iq++){
				for (int idb = 0; idb < 7; idb++){
					if (iq == idb) continue;
					dataTable << DataCombination(CoverBundles[idb].second, CoverBundles[idb].first, iq);
				}
			}
			for (int iPermute = 0; iPermute < 7; iPermute++){
				dataTable.Permute(mUserIndex * 100 * iPermute, iPermute * 6, iPermute * 6 + 5);
			}

			int visInfoIdxOffset = mVisInfos.size();
			for (int iTrial = 0; iTrial < 42; iTrial++){
				int visIndex = iTrial / 7;
				int dataIndex = (iTrial % 7) * 6 + visIndex;
				MyVisInfo visInfo(false, false, thisTask, shapeEncodings[visIndex].second, 0, 
					shapeEncodings[visIndex].first, BASIC, dataTable[dataIndex].bundle,
					dataTable[dataIndex].cover, dataTable[dataIndex].quest, EXPERIMENT_RES);
				mVisInfos << visInfo;
			}
			for (int iPermute = 0; iPermute < 6; iPermute++){
				mVisInfos.Permute(mUserIndex * 101 * iPermute, 
					visInfoIdxOffset + iPermute * 7, visInfoIdxOffset + iPermute * 7 + 6);
			}
		}
		else{
			MyArray<DataCombination> dataTable;
			for (int iq = 0; iq < 6; iq++){
				for (int idb = 0; idb < 8; idb++){
					dataTable << DataCombination(dataBundles[idb % 4], dataCovers[idb / 4], iq);
				}
			}
			for (int iPermute = 0; iPermute < 7; iPermute++){
				dataTable.Permute(mUserIndex * 100 * iPermute, iPermute * 8, iPermute * 8 + 7);
			}
			int visInfoIdxOffset = mVisInfos.size();
			for (int iTrial = 0; iTrial < 48; iTrial++){
				int visIndex = iTrial / 6;
				int dataIndex = (iTrial % 6) * 8 + visIndex;
				MyVisInfo visInfo(false, false, thisTask, COLOR, 0, shapeCues[visIndex].first,
					shapeCues[visIndex].second, dataTable[dataIndex].bundle,
					dataTable[dataIndex].cover, dataTable[dataIndex].quest, EXPERIMENT_RES);
				mVisInfos << visInfo;
			}
			for (int iPermute = 0; iPermute < 8; iPermute++){
				mVisInfos.Permute(mUserIndex * 101 * iPermute,
					visInfoIdxOffset + iPermute * 6, visInfoIdxOffset + iPermute * 6 + 5);
			}
		}
	}
	mVisInfos << MyVisInfo(END);
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
			<< mVisInfos[i].GetString(decimer) << decimer
			<< visData->GetCorrectAnswers()[0] << endl;
		delete visData;
	}
}