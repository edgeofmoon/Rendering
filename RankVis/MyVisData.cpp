#include "MyVisData.h"
#include "MyMathHelper.h"
#include "OSCB.h"

#include <iostream>
#include <fstream>
#include <cassert>
#include <algorithm>
using namespace std;
using namespace MyVisEnum;

float MyVisData::TouchRange = 0.4f;
float MyVisData::mMinSphereRadius = 5.f;
float MyVisData::mMaxSphereRadius = 8.f;

MyVisData::MyVisData(MyVisInfo info):
	mVisInfo(info){
	mTracts = NULL;
}


MyVisData::~MyVisData()
{
}

void MyVisData::Clear(){
	mTractIndices.clear();
	mTractSelectIndices.clear();
	mSphereAtHead.clear();
	mBoxes.clear();
}

void MyVisData::LoadFromDirectory(const MyString& dir){
	if ( mVisInfo.IsEmpty()){
		//cerr << "Empty trial, no data loaded." << endl;
		return;
	}

	if (mTracts == NULL){
		cerr << "Cannot load with empty tract data!" << endl;
		return;
	}
	mBaseDirectory = dir;
	MyString slash("\\");
	MyString imDir = mBaseDirectory + slash
		+ mVisInfo.GetCoverFolderName() + slash
		+ mVisInfo.GetTaskFolderName() + slash
		+ mVisInfo.GetResolutionFolderName() + slash
		+ mVisInfo.GetBundleFolderName() + slash;
	mDataPath = imDir + mVisInfo.GetQuestFolderName() + slash;
	Clear();

	if (LoadTractIndices(mDataPath+mVisInfo.GetTractIndexFileName()) != 1){
		cerr << "Cannot load tract index file: "
			<< mDataPath + mVisInfo.GetTractIndexFileName() << endl;
		if (LoadTractIndices(imDir + mVisInfo.GetTractIndexFileName()) != 1){
			cerr << "Cannot load alternative file: "
				<< imDir + mVisInfo.GetTractIndexFileName() << endl
				<< "Will load default set (all tracts)." << endl;
			mTractIndices = MyArrayi::GetSequence(0, mTracts->GetNumTracks() - 1);
		}
	}

	mBoundingBox = mTracts->ComputeBoundingBox(mTractIndices);

	if (mVisInfo.GetVisTask() == FA){
		LoadData_FA();
	}
	else if (mVisInfo.GetVisTask() == TRACE){
		LoadData_TRACE();
	}
	else if (mVisInfo.GetVisTask() == TUMOR){
		LoadData_TUMOR();
	}
	else if (mVisInfo.GetVisTask() == FA_VALUE){
		LoadData_FA_VALUE();
	}
	else{
		cerr << "Cannot resolve task type!" << endl;
	}
}

void MyVisData::UpdateAnswers(){
	if (mVisInfo.GetVisTask() == FA){
		ComputeAnswer_FA();
	}
	else if (mVisInfo.GetVisTask() == TRACE){
		ComputeAnswer_TRACE();
	}
	else if (mVisInfo.GetVisTask() == TUMOR){
		ComputeAnswer_TUMOR();
	}
	else if (mVisInfo.GetVisTask() == FA_VALUE){
		ComputeAnswer_FA_VALUE();
	}
	else{
		cerr << "Cannot resolve task type!" << endl;
	}
}

int MyVisData::GetError(int userAnswer) const{
	if (mVisInfo.GetVisTask() == FA){
		if (mCorrectAnswers.size()>0){
			return abs(userAnswer - mCorrectAnswers[0]);
		}
		else return -1;
	}
	else if (mVisInfo.GetVisTask() == TRACE
		|| mVisInfo.GetVisTask() == TUMOR){
		if (mCorrectAnswers.size()>0)
			return (userAnswer == mCorrectAnswers[0] ? 0 : 1);
		else return 1;
	}
	else return -100;
}

float MyVisData::GetError(float userAnswer) const{
	if (mVisInfo.GetVisTask() == FA_VALUE){
		return abs(userAnswer - mAnswerInfo);
	}
	else return -100;
}

MyString MyVisData::GetCorrectAnswerString() const{
	if (mVisInfo.GetVisTask() == FA
		|| mVisInfo.GetVisTask() == TRACE
		|| mVisInfo.GetVisTask() == TUMOR){
		if (mCorrectAnswers.size() > 0){
			int idx = this->GetCorrectAnswers()[0];
			return mVisInfo.GetAnswerOptionString(idx);
		}
		return "Unknown answer";
	}
	return "Unknown task";
}

float MyVisData::GetStandardDeviation() const{
	if (mVisInfo.IsEmpty()) return -1;
	if (mVisInfo.GetVisTask() == FA || mVisInfo.GetVisTask() == FA_VALUE){
		MyArrayf values;
		mTracts->GetSampleClampedValues(mBoxes[0], 0.2, 1, values);
		float m = MyMathHelper::ComputeMean(values);
		float stdev = MyMathHelper::ComputeStandardDeviation(values, m);
		return stdev;
	}
	return -1;
}

MyArrayf MyVisData::GetBoxPairWiseDistances() const{
	if (mBoxes.size() != 3) return MyArrayf();
	float d0 = (mBoxes[0].GetCenter() - mBoxes[1].GetCenter()).norm();
	float d1 = (mBoxes[1].GetCenter() - mBoxes[2].GetCenter()).norm();
	float d2 = (mBoxes[2].GetCenter() - mBoxes[0].GetCenter()).norm();
	return MyArrayf{ d0, d1, d2 };
}

int MyVisData::LoadTractIndices(const MyString& fileName){
	// load tract index
	ifstream infile(fileName);
	if (infile.is_open()){
		int numFibers;
		infile >> numFibers;
		mTractIndices.reserve(numFibers);
		for (unsigned int i = 0; i<numFibers; i++){
			int fiberIdx;
			infile >> fiberIdx;
			mTractIndices.push_back(fiberIdx);
		}
		infile.close();
		return 1;
	}
	else{
		return 0;
	}
}

void MyVisData::LoadData_FA(){
	LoadData_Box(2);
	ComputeAnswer_FA();
}

void MyVisData::LoadData_TRACE(){
	LoadData_Box(3);
	LoadData_SelectIndices();
	LoadData_TractColors();
	ComputeAnswer_TRACE();

}
void MyVisData::LoadData_SAME(){
	LoadData_SelectIndices();
	ComputeAnswer_SAME();
}

void MyVisData::LoadData_TUMOR(){
	LoadData_Sphere();
	ComputeAnswer_TUMOR();
}

void MyVisData::LoadData_FA_VALUE(){
	LoadData_Box(1);
	ComputeAnswer_FA_VALUE();
}

void MyVisData::LoadData_Box(int numBox){
	// load boxes
	mBoxes.clear();
	// ensure correct index-filename corresponse
	mBoxes.resize(numBox, MyBoundingBox(mBoundingBox.GetCenter(), 10));
	for (int ibox = 0; ibox<numBox; ibox++){
		MyString boxFileName = mDataPath + mVisInfo.GetBoxFileName(ibox);
		ifstream infile(boxFileName);
		if (infile.is_open()){
			char tmp[200];
			infile.getline(tmp, 200, '\n');
			MyVec3f low, high;
			infile >> low[0] >> low[1] >> low[2]
				>> high[0] >> high[1] >> high[2];
			// box shift to center
			MyVec3f offset = mBoundingBox.GetCenter();
			//MyVec3f offset = mTracts->GetBoundingBox().GetCenter();
			MyBoundingBox box(low + offset, high + offset);
			if (mBoundingBox.IsIn(box.GetCenter())){
				mBoxes[ibox] = box;
			}
			else {
				cerr << "Error loading box file: " << boxFileName << endl
					<< "Load default box." << endl;
			}
			infile.close();
		}
		else{
			cerr << "Cannot open box file: " << boxFileName << endl
				<< "Load default box." << endl;
		}
	}
}

void MyVisData::LoadData_SelectIndices(){
	MyString selectIndexFileName = mDataPath + mVisInfo.GetTractSelectFileName();
	ifstream infile(selectIndexFileName.c_str());
	mTractSelectIndices.clear();
	if (infile.is_open()){
		unsigned int numIdxs;
		if (mVisInfo.GetVisTask() == SAME){
			// first line is the bundle number
			infile >> numIdxs;

			// bundle_same task: answer depend on questset
			// 1 is yes, 2 is no
			mCorrectAnswers = MyArrayi(1, mVisInfo.GetQuest() == 1 ? 1 : 0);
		}
		infile >> numIdxs;
		mTractSelectIndices.reserve(numIdxs);
		for (unsigned int i = 0; i<numIdxs; i++){
			int tmpIdx;
			infile >> tmpIdx;
			assert(tmpIdx < mTractIndices.size());
			int highlightFiberIdx_inWhole = mTractIndices[tmpIdx];
			mTractSelectIndices.push_back(highlightFiberIdx_inWhole);
		}
		infile.close();
	}
	else{
		cerr << "Cannot read tract index file: " << selectIndexFileName << endl;
	}
}

void MyVisData::LoadData_TractColors(){
	if (mVisInfo.GetMappingMethod() == 0) return;
	MyString slash("\\");
	MyString imDir = mBaseDirectory + slash
		+ mVisInfo.GetCoverFolderName() + slash
		+ mVisInfo.GetTaskFolderName() + slash
		+ mVisInfo.GetResolutionFolderName() + slash
		+ mVisInfo.GetBundleFolderName() + slash;
	MyString fileName = imDir + mVisInfo.GetTractColorFileName(mVisInfo.GetMappingMethod());
	ifstream infile(fileName.c_str());
	mTractColors.clear();
	if (infile.is_open()){
		unsigned int numIdxs = mTractIndices.size();
		mTractColors.reserve(numIdxs);
		for (unsigned int i = 0; i<numIdxs; i++){
			MyColor4f c(0, 0, 0, 1);
			infile >> c.r >> c.g >> c.b;
			mTractColors << (c);
		}
		infile.close();
	}
	else{
		cerr << "Cannot read tract color file: " << fileName << endl;
	}
}

void MyVisData::LoadData_Sphere(){
	MyString sphereFileName = mDataPath + mVisInfo.GetSphereFileName(mVisInfo.GetQuest());
	ifstream infile(sphereFileName);
	mSpheres.clear();
	if (infile.is_open()){
		MyVec3f pos;
		float r;
		infile >> pos[0] >> pos[1] >> pos[2] >> r;
		mSpheres.resize(1);
		// sphere shift to center
		mSpheres[0].SetCenter(pos+mBoundingBox.GetCenter());
		mSpheres[0].SetRadius(r);
		infile.close();
	}
	else{
		cerr << "Cannot load sphere file: " << sphereFileName << endl;
	}
}

void MyVisData::ComputeAnswer_FA(){
	float faSum0, faSum1;
	int nSample0, nSample1;
	if (mBoxes.size() >= 2){
		mTracts->GetSampleClampedValueInfo(mBoxes[0], 0.2, 1.0, mTractIndices, nSample0, faSum0);
		mTracts->GetSampleClampedValueInfo(mBoxes[1], 0.2, 1.0, mTractIndices, nSample1, faSum1);
		float fa0 = faSum0 / nSample0;
		float fa1 = faSum1 / nSample1;

		mAnswerInfo = fa0 - fa1;
		for (int i = 0; i < mVisInfo.GetNumberAnswerOption(); i++){
			MyVec2f ranges = mVisInfo.GetFAAnswerOptionRanges()[i];
			if (mAnswerInfo >= ranges[0] && mAnswerInfo < ranges[1]){
				mCorrectAnswers = { i };
				break;
			}
		}
	}

	if (mCorrectAnswers.empty()){
		cerr << "Cannot resolve FA task answer!" << endl;
	}
}

void MyVisData::ComputeAnswer_TRACE(){
	// bundle trace task
	//assert(!spheres.empty());
	mSphereAtHead.resize(mTractSelectIndices.size());
	for (int i = 0; i<mTractSelectIndices.size(); i++){
		int t = mTractSelectIndices[i];
		int numSegs = mTracts->GetNumVertex(t);
		MyVec3f head = mTracts->GetCoord(t, 0);
		MyVec3f tail = mTracts->GetCoord(t, numSegs - 1);
		for (int j = 0; j<mBoxes.size(); j++){
			if (mBoxes[j].IsIn(head)){
				mCorrectAnswers = { j };
				// change the spheres to tail
				mSphereAtHead[i] = false;
			}
			else if (mBoxes[j].IsIn(tail)){
				mCorrectAnswers = { j };
				// change the spheres to head
				mSphereAtHead[i] = true;
			}
		}
	}
	//assert(correctAnswer >= 0);
	mAnswerInfo = 3.f;
	// push to further side in case neither end is in box but near box
	if (mCorrectAnswers.size() > 0){
		int correctAnswer = mCorrectAnswers[0];
		MyVec3f correctCenter = mBoxes[correctAnswer].GetCenter();
		for (unsigned int i = 0; i<mTractSelectIndices.size(); i++){
			int t = mTractSelectIndices[i];
			int numSegs = mTracts->GetNumVertex(t);
			MyVec3f head = mTracts->GetCoord(t, 0);
			MyVec3f tail = mTracts->GetCoord(t, numSegs - 1);
			if ((correctCenter - head).norm() < (correctCenter - tail).norm()){
				// set to tail since tail is further from box
				mSphereAtHead[i] = false;
			}
			else{
				// set to head since head is further from box
				mSphereAtHead[i] = true;
			}
		}
	}
	else{
		cerr << "Cannot resolve TRACE task answer!" << endl;
	}
}

void MyVisData::ComputeAnswer_SAME(){
	// do nothing, answer appears in loading
	if (mCorrectAnswers.size() > 0){
	}
	else{
		cerr << "Cannot resolve SAME task answer!" << endl;
	}
}

void MyVisData::ComputeAnswer_TUMOR(){
	// do nothing, answer appears in loading
	if (mSpheres.size() > 0){
		CollisionStatus cs = ComputeSphereCollusionStatus(mSpheres[0]);
		int ans = int(cs) - 1;
		mCorrectAnswers = MyArrayi(1, ans);
		if (ans != mVisInfo.GetQuest()%3 && !mVisInfo.IsTraining())
			cerr << "TUMOR task answer doesn't match quest index!" << endl;
	}
	else{
		cerr << "No tumor to resolve TUMOR task answer!" << endl;
	}
}

void MyVisData::ComputeAnswer_FA_VALUE(){
	float faSum;
	int nSample;
	if (mBoxes.size() >= 1){
		mTracts->GetSampleClampedValueInfo(mBoxes[0], 0.2, 1.0, mTractIndices, nSample, faSum);
		float fa = faSum / nSample;
		mAnswerInfo = fa;
		mCorrectAnswers.clear();
	}
	else{
		cerr << "No Box to resolve FA_VALUE task answer!" << endl;
	}

}

// for adding data
void MyVisData::CreateNewQuestFolder() const{
	MyString slash("\\");
	OSCB::MakePath(mDataPath);
	cerr << "New folder created: " << mDataPath << endl;
	MyString imDir = mBaseDirectory + slash
		+ mVisInfo.GetCoverFolderName() + slash
		+ mVisInfo.GetTaskFolderName() + slash
		+ mVisInfo.GetResolutionFolderName() + slash
		+ mVisInfo.GetBundleFolderName() + slash;
	OSCB::MakeCopy(imDir + mVisInfo.GetTractIndexFileName(),
		mDataPath + mVisInfo.GetTractIndexFileName());
	cerr << "Index file copied: "
		<< mDataPath + mVisInfo.GetTractIndexFileName() << endl;
}

// FA
void MyVisData::SetBox(const MyBoundingBox& box, int idx){
	if (mBoxes.size() < idx + 1) mBoxes.resize(idx + 1, box);
	else mBoxes[idx] = box;
}

void MyVisData::SaveBoxFiles(const MyArray<MyBoundingBox>& boxes) const{
	for (int i = 0; i < boxes.size(); i++){
		MyString boxFileName = mDataPath + mVisInfo.GetBoxFileName(i);
		const MyBoundingBox& box = boxes[i];
		ofstream outfile(boxFileName);
		if (!outfile.is_open()) {
			CreateNewQuestFolder();
			outfile.open(boxFileName);
		}
		if (!outfile.is_open()){
			cerr << "Cannot open box file to write: " << boxFileName <<endl;
		}
		else{
			MyVec3f offset = mBoundingBox.GetCenter();
			outfile << "Exclusive" << endl;
			// box unshift to center
			MyVec3f low = box.GetLowPos() - offset;
			MyVec3f high = box.GetHighPos() - offset;
			outfile << low[0] << " " << low[1] << " " << low[2] << " " << endl
				<< high[0] << " " << high[1] << " " << high[2] << " ";
			outfile.close();
			cout << "Box saved to: " << boxFileName << endl;
		}
	}
}
float MyVisData::GetBoxValues(const MyBoundingBox& box) const{
	int nSample;
	float faSum;
	mTracts->GetSampleValueInfo(box, mTractSelectIndices, nSample, faSum);
	if (nSample != 0) return faSum / nSample;
	else return 0;
}

// TRACE
void MyVisData::SetSelectedIndices(const MyArrayi& selected){
	mTractSelectIndices = selected;
}

void MyVisData::CheckSelectedValidness(){
	ComputeAnswer_TRACE();
}

void MyVisData::SaveSelectedIndicesFile(const MyArrayi& selected) const{
	MyString slash("\\");
	MyString selectIndexFileName = mDataPath + mVisInfo.GetTractSelectFileName();
	ofstream outfile(selectIndexFileName.c_str());
	if (!outfile.is_open()) {
		CreateNewQuestFolder();
		outfile.open(selectIndexFileName);
	}
	if (!outfile.is_open()){
		cerr << "Cannot read tract index file: " << selectIndexFileName << endl;
	}
	else{
		int n = selected.size();
		outfile << n << endl;
		for (unsigned int i = 0; i<n; i++){
			int tmpIdx = mTractIndices.IndexOf(selected[i]);
			assert(tmpIdx >= 0);
			outfile << tmpIdx << endl;
		}
		outfile.close();
		cout << "Selected saved to: " << selectIndexFileName << endl;
	}
}

// TUMOR
void MyVisData::SetSphere(const MySphere& sphere){ 
	mSpheres = MyArray<MySphere>(1, sphere);
}

void MyVisData::SaveTumorFile(const MySphere& sphere) const{
	CollisionStatus  cs = ComputeSphereCollusionStatus(sphere);
	int idx = 3;
	if (cs == INTERSECT) idx = 0;
	else if (cs == TOUCH) idx = 1;
	else if (cs == AWAY) idx = 2;
	else cerr << "Unknow collision status!" << endl;
	if (idx != mVisInfo.GetQuest() % 3 && !mVisInfo.IsTraining()){
		cerr << "Collusion status doesn't match quest index!" << endl;
		return;
	}
	MyString sphereFileName = mDataPath + mVisInfo.GetSphereFileName(mVisInfo.GetQuest());
	ofstream outfile(sphereFileName);
	if (!outfile.is_open()){
		CreateNewQuestFolder();
		outfile.open(sphereFileName);
	}
	if (outfile.is_open()){
		MyVec3f pos = sphere.GetCenter()-mBoundingBox.GetCenter();
		outfile << pos[0] << ' ' << pos[1] << ' ' << pos[2] << ' '
			<< sphere.GetRadius() << endl;
		outfile.close();
		cout << "Sphere saved to: " << sphereFileName << endl;
	}
	else{
		cerr << "Cannot open sphere file to write: " << sphereFileName << endl;
	}
}

bool MyVisData::MoveSphereToStatus(MySphere& sphere, 
	CollisionStatus st, int maxItr) const{
	MyVec3f newCenter;
	MyVec3f oldCenter = sphere.GetCenter();
	float r = sphere.GetRadius();
	float minDist, distRange;
	float range = TouchRange / 2;
	if (st == INTERSECT) minDist = 0, distRange = r - TouchRange - range;
	else if (st == TOUCH) minDist = r - TouchRange/10, distRange = 2 * TouchRange/10;
	else minDist = r + TouchRange + range, distRange = 2 * range;
	bool rst = ComputeNearbyPointAtDist(newCenter, oldCenter, minDist, distRange, maxItr);
	if (rst) sphere.SetCenter(newCenter);
	return rst;
}

CollisionStatus MyVisData::ComputeSphereCollusionStatus(const MySphere& sphere) const{
	float intersectionMax = sphere.GetRadius() - TouchRange;
	float touchMax = sphere.GetRadius() + TouchRange;
	CollisionStatus cs = AWAY;
	for (int i = 0; i < mTractIndices.size(); i++){
		int tractIndex = mTractIndices[i];
		for (int j = 1; j < mTracts->GetNumVertex(tractIndex); j++){
			float dist = MyMathHelper::PointToLineSegmentDistance(sphere.GetCenter(),
				mTracts->GetCoord(tractIndex, j - 1), mTracts->GetCoord(tractIndex, j));
			if (dist <= intersectionMax) return INTERSECT;
			else if (dist < touchMax) cs = TOUCH;
		}
	}
	return cs;
}

MyVec2i MyVisData::ComputeClosestSegment(const MyVec3f& p) const{
	MyVec2i rst(-1, -1);
	float minDist = FLT_MAX;
	for (int i = 0; i < mTractIndices.size(); i++){
		int tractIndex = mTractIndices[i];
		for (int j = 1; j < mTracts->GetNumVertex(tractIndex); j++){
			float dist = MyMathHelper::PointToLineSegmentDistance(p,
				mTracts->GetCoord(tractIndex, j - 1), mTracts->GetCoord(tractIndex, j));
			if (dist < minDist) {
				minDist = dist;
				rst = MyVec2i(tractIndex, j - 1);
			}
		}
	}
	assert(rst[0] >= 0);
	return rst;
}

bool MyVisData::ComputeNearbyPointAtDist(MyVec3f& rst, const MyVec3f& p,
	float minDist, float distRange, int maxItr) const{
	MyVec3f cur = p;
	for (int i = 0; i < maxItr; i++){
		MyVec2i seg = ComputeClosestSegment(cur);
		MyVec3f lineSeg[] = { mTracts->GetCoord(seg[0], seg[1]), 
			mTracts->GetCoord(seg[0], seg[1] + 1) };
		float d = MyMathHelper::PointToLineSegmentDistance(cur, lineSeg[0], lineSeg[1]);
		if (d>minDist && d < minDist + distRange) {
			rst = cur; 
			return true;
		}
		MyVec3f cp = MyMathHelper::ClosestPointOnLineSegment(cur, lineSeg[0], lineSeg[1]);
		float diff = minDist + (distRange / 2) - d;
		MyVec3f dir = (cur - cp).normalized();
		cur += dir*diff;
	}
	return false;
}

MyArrayi MyVisData::ComputeIntersectedIndices(const MySphere& sphere) const{
	return mTracts->GetTractsIntersected(sphere, mTractIndices);
}

MyArrayi MyVisData::ComputeTouchedIndices(const MySphere& sphere) const{
	MyArrayi rst;
	for (int t = 0; t < mTractIndices.size(); t++){
		int tractIndex = mTractIndices[t];
		for (int i = 1; i < mTracts->GetNumVertex(tractIndex); i++){
			float dist = MyMathHelper::PointToLineSegmentDistance(sphere.GetCenter(), 
				mTracts->GetCoord(tractIndex, i - 1), mTracts->GetCoord(tractIndex, i));
			if (SphereCollusionStatus(sphere.GetRadius(), dist) == TOUCH)
				rst << tractIndex;
		}
	}
	return rst;
}

// FA_VALUE
float MyVisData::GetExpectedValueFromQuestIdx() const{
	Bundle bd = mVisInfo.GetBundle();
	FiberCover cv = mVisInfo.GetCover();
	if (cv == BUNDLE && (bd == IFO || bd == ILF)){
		float values[] = { 0.26, 0.3, 0.34, 0.38, 0.42, 0.46, 0.5, 0.54, 0.58, 0.62, 0.68, 0.7 };
		return values[mVisInfo.GetQuest() % 12];
	}
	else {
		float values[] = { 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8 };
		return values[mVisInfo.GetQuest() % 12];
	}
}

void MyVisData::ComputeBoxesWithValue(MyArray<MyBoundingBox>& boxes, MyArray2f& avgs, float v, float err, int minSegs, int maxSegs){
	for (int w = 15; w >= 5; w--){
		for (int t = 0; t < mTractIndices.size(); t++){
			int tractIndex = mTractIndices[t];
			int m = mTracts->GetNumVertex(tractIndex);
			MyArrayi ctns(m, 0);
			MyArrayf sums(m, 0);
#pragma omp parallel for default(shared)
			for (int i = 0; i < m; i++){
				MyBoundingBox tbox(mTracts->GetCoord(tractIndex, i), w);
				float sum = 0;
				int ctn = 0;
				mTracts->GetSampleClampedValueInfo(tbox, 0.1, 1, mTractIndices, ctns[i], sums[i]);
			}
			for (int i = 0; i < m; i++){
				avgs << MyVec2f(((ctns[i]!=0) ? (sums[i] / ctns[i]) : 0), ctns[i]);
				if (ctns[i]<minSegs || ctns[i]>maxSegs) continue;
				float fa = sums[i] / ctns[i];
				if (fabs(fa - v) <= err) {
					boxes << MyBoundingBox(mTracts->GetCoord(tractIndex, i), w);
				}
			}
			if (boxes.size()>10) break;
		}
	}
}

void MyVisData::WriteVectorToFile(const MyString& fn, const MyArray2f& vecs){
	ofstream outfile(fn);
	if (!outfile.is_open()){
		cerr << "Cannot Open File: " << fn << endl;
		return;
	}
	for (auto f : vecs) outfile << f[0] << '\t' << f[1] << endl;
	outfile.close();
}

CollisionStatus MyVisData::SphereCollusionStatus(float radius, float closetCenterDist){
	float intersectionMax = max(0.f, radius - TouchRange);
	float touchMax = radius + TouchRange;
	if (closetCenterDist <= intersectionMax) return INTERSECT;
	else if (closetCenterDist <= touchMax) return TOUCH;
	else return AWAY;
}