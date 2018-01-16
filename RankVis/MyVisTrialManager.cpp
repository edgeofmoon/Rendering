#include "MyVisTrialManager.h"
#include "MyMathHelper.h"

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
		dataCombinations << DataCombination(dataBundles[j % 4], dataCovers[j / 4], 0);
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
			for (int iTrial = 0; iTrial < 12; iTrial++){
				DataCombination dataCombination = dataCombinations[iTrial % 8];
				if (iTrial % 2 == 0){
					MyVisInfo legendChangeInfo(thisTask);
					legendChangeInfo.SetShowLegend(colors[iTrial / 2]);
					mVisInfos << legendChangeInfo;
				}
				MyVisInfo visInfo(false, true, thisTask, COLOR, colors[iTrial / 2], TUBE,
					BASIC, dataCombination.bundle, dataCombination.cover,
					dataCombination.quest, TRAINING_RES, iTrial % 2);
				mVisInfos << visInfo;
			}
		}
		else{
			//shapeCues.Permute(mUserIndex);
			MyArrayi colors = { 4, 3, 1, 0 };
			for (int iTrial = 0; iTrial < 8; iTrial++){
				if (iTrial % 2 == 0){
					MyVisInfo legendChangeInfo(thisTask);
					legendChangeInfo.SetShowLegend(colors[iTrial / 2]);
					mVisInfos << legendChangeInfo;
				}
				DataCombination dataCombination = dataCombinations[iTrial % 8];
				MyVisInfo visInfo(false, true, thisTask, COLOR, colors[iTrial / 2], TUBE,
					ENCODING, dataCombination.bundle, dataCombination.cover,
					dataCombination.quest, TRAINING_RES, iTrial % 2);
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
	MyArray<pair<FiberCover, Bundle>> coverBundles = {
		make_pair(WHOLE, CC), make_pair(BUNDLE, CC),
		make_pair(WHOLE, CST), make_pair(BUNDLE, CST),
		make_pair(WHOLE, IFO),// make_pair(BUNDLE, IFO),
		make_pair(WHOLE, ILF),// make_pair(BUNDLE, ILF),
	};

	/****************dcap****************/
	for (int i = 0; i < 6; i++){
		for (int k = 0; k < 12; k++){
			mVisInfos
				<< MyVisInfo(false, false, FA_VALUE, COLOR, 0, TUBE, BASIC,
				coverBundles[i].second, coverBundles[i].first, k, EXPERIMENT_RES);
		}
	}
	return;
	/*
	for (int j = 0; j < 6; j++){
		for (int i = 0; i < 6; i++){
			for (int k = 0; k < 12; k++){
				mVisInfos
					<< MyVisInfo(false, false, FA_VALUE, COLOR, j, TUBE, BASIC,
					coverBundles[i].second, coverBundles[i].first, k, EXPERIMENT_RES);
			}
		}
	}
	return;
	for (int i = 0; i < 6; i++){
		for (int k = 0; k < 6; k++){
			//mVisInfos
			//	<< MyVisInfo(false, false, TRACE, COLOR, k, TUBE, ENCODING,
			//	dataBundles[j], dataCovers[i], 0, EXPERIMENT_RES);
			mVisInfos
				<< MyVisInfo(false, false, FA_VALUE, COLOR, k, TUBE, BASIC,
				coverBundles[i].second, coverBundles[i].first, 5, EXPERIMENT_RES);
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
	mVisInfos
		<< MyVisInfo(false, false, TRACE, COLOR, 0, TUBE, ENCODING, CC, WHOLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TRACE, COLOR, 1, TUBE, ENCODING, CC, WHOLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TRACE, COLOR, 3, TUBE, ENCODING, CC, WHOLE, 0, EXPERIMENT_RES)
		<< MyVisInfo(false, false, TRACE, COLOR, 4, TUBE, ENCODING, CC, WHOLE, 0, EXPERIMENT_RES);
	return;
	int methods[] = { 0, 1, 3, 4 };
	for (int ibd = 0; ibd < 8; ibd++){
		for (int quest = 0; quest < 4; quest++){
				for (int m = 0; m < 4; m++){
				mVisInfos <<
					MyVisInfo(false, false, TRACE, COLOR, methods[m], TUBE, ENCODING,
					dataBundles[ibd % 4], dataCovers[ibd / 4], quest, EXPERIMENT_RES);
			}
		}
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
			// ref: http://www.plantsciences.ucdavis.edu/agr205/Lectures/2010%20Iago/Topic%207/Useful%20Latin%20Squares.pdf
			int visLatin[6][6] = {
				{ 0, 1, 2, 3, 4, 5 },
				{ 1, 0, 5, 4, 2, 3 },
				{ 2, 5, 1, 0, 3, 4, },
				{ 3, 2, 4, 1, 5, 0 },
				{ 4, 3, 0, 5, 1, 2, },
				{ 5, 4, 3, 2, 0, 1 },
			};

			int dataBlock[6][12] = {};
			for (int quest = 0; quest < 12; quest++){
				MyArrayi dataSequence = MyArrayi::GetSequence(0, 5);
				dataSequence.Permute(20170809 + quest);
				for (int iRow = 0; iRow < 6; iRow++){
					dataBlock[iRow][quest] = dataSequence[iRow] * 12 + quest;
				}
			}

			int infoSize = mVisInfos.size();
			mVisInfos.resize(infoSize + 78, MyVisInfo(thisTask));
			int* visOrder = visLatin[mUserIndex % 6];
			for (int ivis = 0; ivis < 6; ivis++){
				int dataBlockRowIdx = (ivis + mUserIndex) % 6;
				int visLocIdx = visOrder[ivis];
				MyVisInfo legendChangeInfo(thisTask);
				legendChangeInfo.SetShowLegend(ivis);
				mVisInfos[infoSize + visLocIdx * 13] = legendChangeInfo;
				for (int idata = 0; idata < 12; idata++){
					int dataIdx = dataBlock[dataBlockRowIdx][idata];
					int trialLocIdx = visLocIdx * 13 + idata + 1;
					mVisInfos[infoSize + trialLocIdx] =
						MyVisInfo(false, false, FA_VALUE, COLOR, ivis, TUBE, BASIC,
						coverBundles[dataIdx / 12].second, coverBundles[dataIdx / 12].first,
						dataIdx % 12, EXPERIMENT_RES);
				}
			}
			for (int ivis = 0; ivis < 6; ivis++){
				mVisInfos.Permute(mUserIndex*ivis * 123, infoSize + ivis * 13 + 1, infoSize + ivis * 13 + 12);
				mVisInfos[infoSize + ivis * 13 + 12].SetNeedMoreInput(true);
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
				MyVisInfo legendChangeInfo(thisTask);
				legendChangeInfo.SetShowLegend(color);
				mVisInfos << legendChangeInfo;
				int infoSize = mVisInfos.size();
				for (int ibd = 0; ibd < 8; ibd++){
					mVisInfos <<
						MyVisInfo(false, false, TRACE, COLOR, color, TUBE, ENCODING,
						dataBundles[ibd % 4], dataCovers[ibd / 4], quest, EXPERIMENT_RES);
				}
				mVisInfos.Permute(mUserIndex*ilatin * 320, infoSize, mVisInfos.size() - 1);
				mVisInfos[mVisInfos.size() - 1].SetNeedMoreInput(true);
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

void MyVisTrialManager::PrintHistogramTables(
	const MyString& fileName, const MyString& decimer) const{
	ofstream outFile(fileName);
	if (!outFile.is_open()){
		cerr << "Cannot open file to write: " << fileName << endl;
		return;
	}
	MyArrayf values;
	outFile << "USERIDX" << decimer
		<< "TRIALIDX" << decimer
		<< MyVisInfo::GetStringHeader(decimer) << decimer
		<< "MEAN" << decimer
		<< "STDDEV" << decimer;
	for (int i = 0; i < 16; i++){
		outFile << 0.2 + i*0.05 << decimer;
	}
	outFile << endl;
	for (int i = 0; i < mVisInfos.size(); i++){
		if (mVisInfos[i].IsEmpty()) continue;
		if (mVisInfos[i].GetVisTask() != FA_VALUE) continue;
		MyVisData* visData = new MyVisData(mVisInfos[i]);
		visData->SetTracts(mTracts);
		visData->LoadFromDirectory(mDataRootDir);
		outFile << mUserIndex << decimer
			<< i << decimer
			<< mVisInfos[i].GetString(decimer) << decimer;
		float mean;
		if (visData->GetCorrectAnswers().size() > 0){
			outFile << (mean = visData->GetCorrectAnswers()[0]) << decimer;
		}
		else {
			outFile << (mean = visData->GetAnswerInfo()) << decimer;
		}
		values.clear();
		mTracts->GetSampleClampedValues(visData->GetBoxes()[0], 0.2, 1, visData->GetTractIndices(), values);

		// compute std deviance
		float var = 0;
		for (float v : values){
			float diff = v - mean;
			var += diff * diff;
		}
		outFile << var / values.size() << decimer;

		// to bins
		vector<int> bins(16, 0);
		for (float v : values){
			int idx = (v - 0.2) / 0.05;
			idx = min(idx, 15);
			idx = max(0, idx);
			bins[idx]++;
		}
		for (int i = 0; i < 16; i++){
			outFile << bins[i] << decimer;
		}
		outFile << endl;
		delete visData;
	}
	cout << "Histogram table written." << endl;
}

void MyVisTrialManager::PrintFABySegments(const MyString& fileNamePrefix) const{
	float minv = 0.2f;
	float maxv = 1.0f;
	for (int i = 0; i < mVisInfos.size(); i++){
		if (mVisInfos[i].IsEmpty()) continue;
		if (mVisInfos[i].GetVisTask() != FA_VALUE) continue;
		MyVisData* visData = new MyVisData(mVisInfos[i]);
		visData->SetTracts(mTracts);
		visData->LoadFromDirectory(mDataRootDir);
		const MyArrayi& indices = visData->GetTractIndices();
		const MyBoundingBox& box = visData->GetBoxes()[0];
		MyArray<MyArrayf> valueArrays;
		float vSum = 0;
		int count = 0;
		for (int i = 0; i < indices.size(); i++){
			int it = indices[i];
			MyArrayf values;
			for (int is = 0; is < mTracts->GetNumVertex(it); is++){
				MyVec3f p = mTracts->GetCoord(it, is);
				if (box.IsIn(p)){
					float value = mTracts->GetValue(it, is);
					if (value < minv) value = minv;
					if (value > maxv) value = maxv;
					values << value;
					vSum += value;
					count++;
				}
			}
			if (!values.empty()){
				valueArrays << values;
			}
		}
		MyVec3f boxSize = box.GetSize();
		delete visData;
		int idx = mVisInfos[i].GetDataIndex();
		MyString fileName = fileNamePrefix + MyString(idx) + ".txt";
		ofstream outFile(fileName);
		if (!outFile.is_open()){
			cerr << "Cannot open file to write: " << fileName << endl;
			return;
		}
		else{
			// print #tracts
			outFile << valueArrays.size() << endl;
			// print box size
			outFile << boxSize[0] << " " << boxSize[1] << " " << boxSize[2] << endl;
			//outFile << vSum / count << endl;
			for (int i = 0; i < valueArrays.size(); i++){
				for (int j = 0; j < valueArrays[i].size(); j++){
					outFile << i << " " << j << " " << valueArrays[i][j] << endl;
				}
			}
			outFile.close();
		}
	}
	cout << "FA value files written" << endl;
}

void MyVisTrialManager::PrintBoxPairWiseDistances(const MyString& fileName) const{
	ofstream outFile(fileName);
	if (!outFile.is_open()){
		cerr << "Cannot open file to write: " << fileName << endl;
		return;
	}
	else{
		for (int i = 0; i < mVisInfos.size(); i++){
			if (mVisInfos[i].IsEmpty()) continue;
			if (mVisInfos[i].GetVisTask() != TRACE) continue;
			MyVisData* visData = new MyVisData(mVisInfos[i]);
			visData->SetTracts(mTracts);
			visData->LoadFromDirectory(mDataRootDir);
			MyArrayf distances = visData->GetBoxPairWiseDistances();
			int idx = mVisInfos[i].GetDataIndex();
			outFile << idx;
			for (int j = 0; j < distances.size(); j++){
				outFile << " " << distances[j];
			}
			outFile << endl;
			delete visData;
		}
		outFile.close();
	}
}

void MyVisTrialManager::PrintFAVarianceTable(const MyString& fileName) const{
	float minv = 0.2f;
	float maxv = 1.0f;
	ofstream outFile(fileName);
	if (!outFile.is_open()){
		cerr << "Cannot open file to write: " << fileName << endl;
		return;
	}
	else{
		for (int i = 0; i < mVisInfos.size(); i++){
			if (mVisInfos[i].IsEmpty()) continue;
			if (mVisInfos[i].GetVisTask() != FA_VALUE) continue;
			MyVisData* visData = new MyVisData(mVisInfos[i]);
			visData->SetTracts(mTracts);
			visData->LoadFromDirectory(mDataRootDir);
			const MyArrayi& indices = visData->GetTractIndices();
			const MyBoundingBox& box = visData->GetBoxes()[0];
			MyArrayf values;
			mTracts->GetSampleClampedValues(box, minv, maxv, indices, values);
			float mean = visData->GetAnswerInfo();
			float stdev = MyMathHelper::ComputeStandardDeviation(values, mean);
			int idx = mVisInfos[i].GetDataIndex();
			// print #tracts
			outFile << idx << " " << mean << " " << stdev << endl;
			delete visData;
		}
		outFile.close();
	}
	cout << "FA value files written" << endl;

}