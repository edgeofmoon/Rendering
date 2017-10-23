#include "MyVisRankingApp.h"
#include "MyMathHelper.h"

#include <unordered_map>
#include <algorithm>
using namespace std;
using namespace MyVisEnum;

#define SMALLMOV 0.1f
#define LARGEMOV 0.5f
#define MAX_SELECTOR_RADIUS 15.f

MyVec3f MyVisRankingApp::GetWorldOffset(const MyVec3f& viewoffset){
	MyMatrixf mat = mTrackBall.Matrix();
	MyMatrixf invMat = MyMathHelper::InverseMatrix4x4ColMaj(mat);
	MyVec4f vo = viewoffset.toDim<4>(0);
	MyVec4f worldOffset = MyMathHelper::MatMulVec(invMat, vo);
	return worldOffset.toDim<3>(0);
}

bool MyVisRankingApp::ProcessBoundingObjectKey(MyBoundingObject& bobj, unsigned char key){
	// adjust radius
	if (key == '+' || key == '='){
		if (key == '=') bobj.Expand(SMALLMOV);
		else bobj.Expand(LARGEMOV);
		BoundObjectSize(bobj);
	}
	else if (key == '-' || key == '_'){
		if (key == '-') bobj.Expand(-SMALLMOV);
		else bobj.Expand(-LARGEMOV);
		BoundObjectSize(bobj);
	}
	// adjust center
	else if (key == 'w' || key == 'W'){
		MyVec3f c(0, 0, 0);
		if (key >= 'A') c[1] += LARGEMOV;
		else c[1] += SMALLMOV;
		c = GetWorldOffset(c);
		bobj.Translate(c);
	}
	else if (key == 's' || key == 'S'){
		MyVec3f c(0, 0, 0);
		if (key >= 'A') c[1] -= LARGEMOV;
		else c[1] -= SMALLMOV;
		c = GetWorldOffset(c);
		bobj.Translate(c);
	}
	else if (key == 'a' || key == 'A'){
		MyVec3f c(0, 0, 0);
		if (key >= 'A') c[0] -= LARGEMOV;
		else c[0] -= SMALLMOV;
		c = GetWorldOffset(c);
		bobj.Translate(c);
	}
	else if (key == 'd' || key == 'D'){
		MyVec3f c(0, 0, 0);
		if (key >= 'A') c[0] += LARGEMOV;
		else c[0] += SMALLMOV;
		c = GetWorldOffset(c);
		bobj.Translate(c);
	}
	else if (key == 'q' || key == 'Q'){
		MyVec3f c(0, 0, 0);
		if (key >= 'A') c[2] -= LARGEMOV;
		else c[2] -= SMALLMOV;
		c = GetWorldOffset(c);
		bobj.Translate(c);
	}
	else if (key == 'e' || key == 'E'){
		MyVec3f c(0, 0, 0);
		if (key >= 'A') c[2] += LARGEMOV;
		else c[2] += SMALLMOV;
		c = GetWorldOffset(c);
		bobj.Translate(c);
	}
	else return false;
	return true;
}

void MyVisRankingApp::BoundObjectSize(MyBoundingObject& bobj){
	MySphere* sphere = dynamic_cast<MySphere*>(&bobj);
	if (sphere){
		float r = sphere->GetRadius();
		if (r > MAX_SELECTOR_RADIUS)
			r = MAX_SELECTOR_RADIUS;
		else if (r < MyVisData::GetMinSphereRadius())
			r = MyVisData::GetMinSphereRadius();
		sphere->SetRadius(r);
	}
}

void MyVisRankingApp::ModifyToStable(
	MyBoundingObject& bobj, float step, int maxItr) const{
	const MyVisData* visData = mTrialManager.GetCurrentVisData();
	int np = mTracts.CountVertexInBox(bobj, visData->GetTractIndices());
	int newNp = 0;
	int nItr = 0;
	MyBoundingObject* bkup = bobj.MakeCopy();
	int i = 0;
	for (; i < maxItr; i++){
		bobj.Expand(step * 2);
		int newNp = mTracts.CountVertexInBox(bobj, visData->GetTractIndices());
		if (newNp == np){
			bobj.Expand(-step);
			cout << (step>0 ? "Expand " : "Shrink ") << "stable at level: " << step << endl;
			break;
		}
		np = newNp;
	}
	if (i >= maxItr){
		bobj = *bkup;
	}
	delete bkup;
}

MyArrayi MyVisRankingApp::ComputedSelected(
	const MyArray<MySphere>& selectors, const MyArrayc& operators) const{
	const MyVisData* visData = mTrialManager.GetCurrentVisData();
	const MyArrayi& indices = visData->GetTractIndices();
	MyArrayi rst;
	for (int i = 0; i < indices.size(); i++){
		int tractIndex = indices[i];
		for (int j = 0; j < selectors.size(); j++){
			if (operators[j] == '+'){
				if (mTracts.IsTractIntersected(selectors[j], tractIndex)){
					bool add = true;
					for (int k = 0; k < selectors.size(); k++){
						if (operators[k] == '-'){
							if (mTracts.IsTractIntersected(selectors[k], tractIndex)){
								add = false;
								break;
							}
						}
					}
					if (add) rst << tractIndex;
				}
			}
		}
	}
	return rst;
}

MyArrayi MyVisRankingApp::ComputedFilteredByBox(
	const MyBoundingBox& box, const MyArrayi& selected) const{
	MyArrayi rst;
	for (int i = 0; i < selected.size(); i++){
		int tractIndex = selected[i];
		if (mTracts.IsTractIntersected(box, tractIndex)){
			rst << tractIndex;
		}
	}
	return rst;
}

MyArrayi MyVisRankingApp::ComputedEndFilteredByBox(
	const MyBoundingBox& box, const MyArrayi& selected) const{
	MyArrayi rst;
	for (int i = 0; i < selected.size(); i++){
		int tractIndex = selected[i];
		int n = mTracts.GetNumVertex(tractIndex);
		if (box.IsIn(mTracts.GetCoord(tractIndex, 0))
			|| box.IsIn(mTracts.GetCoord(tractIndex, n-1))){
			rst << tractIndex;
		}
	}
	return rst;
}

MyBoundingBox MyVisRankingApp::ComputedEndingBox(const MyArrayi& selected) const{
	MyBoundingBox box;
	box.SetNull();
	const MyVisData* visData = mTrialManager.GetCurrentVisData();
	for (int i = 0; i < selected.size(); i++){
		if (!visData->GetSphereAtHead()[i]){
			box.Engulf(visData->GetTracts()->GetCoord(selected[i], 0));
		}
		else{
			int n = visData->GetTracts()->GetNumVertex(selected[i]);
			box.Engulf(visData->GetTracts()->GetCoord(selected[i], n - 1));
		}
	}
	return box;
}

MyColor4f MyVisRankingApp::GetSphereStatusColor(const MyTractVisBase* tractVis) const{
	if (!tractVis) return MyColor4f(0.5, 0.5, 0.5, 1);
	const MySphereGeometry* sg = tractVis->GetSphereGeometry();
	const MyVisData* visData = mTrialManager.GetCurrentVisData();
	if (sg && visData){
		CollisionStatus st = visData->ComputeSphereCollusionStatus(*sg->GetSphere());
		return GetCollisionStatusColor(st);
	}
	return MyColor4f(0.5, 0.5, 0.5, 1);
}

MyColor4f MyVisRankingApp::GetCollisionStatusColor(CollisionStatus st) const{
	if (st == INTERSECT) return (MyColor4f::red());
	else if (st == TOUCH) return (MyColor4f::green());
	else if (st == AWAY) return (MyColor4f::blue());
	else return (MyColor4f::black());
}

void MyVisRankingApp::ProcessKey_DataGen(unsigned char key){
	if (!IsOnMode(APP_MODE_DEBUG)) return;
	switch (mTrialManager.GetCurrentVisData()->GetVisInfo().GetVisTask()){
	case FA:
		ProcessKey_FA(key);
		break;
	case TRACE:
		ProcessKey_TRACE(key);
		break;
	case TUMOR:
		ProcessKey_TUMOR(key);
		break;
	case FA_VALUE:
		ProcessKey_FA_VALUE(key);
		break;
	default:
		break;
	}
}
void MyVisRankingApp::ProcessKey_FA(unsigned char key){
	MyVisData* visData = mTrialManager.GetCurrentVisData();
	const float minDist = 0.1;
	const int maxItr = 10;
	if (key == 'v' || key == 'V'){
		if (visData->GetBoxes().size() > mCurrentBoxIndex){
			MyVec3f c = visData->GetBoundingBox().GetCenter();
			MyBoundingBox box = visData->GetBoxes()[mCurrentBoxIndex];
			box = MyBoundingBox(box.GetCenter(), powf(box.GetVolume(), 1.f/3));
			visData->SetBox(box, mCurrentBoxIndex);
		}
		else{
			if (mCurrentBoxIndex < 0) mCurrentBoxIndex = 1;
			float r = (rand() % 1001) / (float)1000
				* (MyVisData::GetMaxSphereRadius() - MyVisData::GetMinSphereRadius())
				+ MyVisData::GetMinSphereRadius();
			visData->SetBox(MyBoundingBox(visData->GetBoundingBox().GetCenter(), r),
				mCurrentBoxIndex);
		}
	}
	else if (key == '1'){
		mCurrentBoxIndex = 0;
	}
	else if (key == '2'){
		mCurrentBoxIndex = 1;
	}
	else if (key == '3'){
		mCurrentBoxIndex = -1;
	}
	else if (visData->GetBoxes().size() <= mCurrentBoxIndex) return;
	else{
		MyBoundingBox box = visData->GetBoxes()[mCurrentBoxIndex];
		if (ProcessBoundingObjectKey(box, key)){
			visData->SetBox(box, mCurrentBoxIndex);
		}
		// snap
		else if (key == 'z' || key == 'Z'){
			ModifyToStable(box, SMALLMOV, maxItr);
			visData->SetBox(box, mCurrentBoxIndex);
		}
		else if (key == 'x' || key == 'X'){
			ModifyToStable(box, -SMALLMOV, maxItr);
			visData->SetBox(box, mCurrentBoxIndex);
		}
		else if (key == 'c' || key == 'C'){
			MyArray2i points = mTracts.GetVertexInBox(box, visData->GetTractIndices());
			if (points.empty()) cerr << "Empty box. Nothing done." << endl;
			else{
				MyBoundingBox newBox(mTracts.GetCoord(points[0]), 0);
				for (int i = 1; i < points.size(); i++){
					newBox.Engulf(mTracts.GetCoord(points[i]));
				}
				visData->SetBox(newBox, mCurrentBoxIndex);
			}
		}
		else if (key == 'j' || key == 'G'){
			if (mCurrentBoxIndex >= 0){
				visData->PermuteBoxes(mCurrentBoxIndex);
			}
		}
		// save files
		else if (key == 'm' || key == 'M'){
			MyArray<MyBoundingBox> boxes = visData->GetBoxes();
			if (boxes.size() > 0){
				visData->SaveBoxFiles(boxes);
			}
			// skip output
			return;
		}
	}

	// out info
	if (mTrialManager.GetCurrentVisData()->GetVisInfo().GetVisTask() == FA){
		if (visData->GetBoxes().size() >= 2){
			float faSum0, faSum1;
			int nSample0, nSample1;
			mTracts.GetSampleValueInfo(visData->GetBoxes()[0],
				visData->GetTractIndices(), nSample0, faSum0);
			mTracts.GetSampleValueInfo(visData->GetBoxes()[1],
				visData->GetTractIndices(), nSample1, faSum1);
			float fa0 = faSum0 / nSample0;
			float fa1 = faSum1 / nSample1;
			cout << fa0 << "-" << fa1 << "=" << fa0 - fa1 << endl;

			visData->UpdateAnswers();
			PrintTrialInfo();
		};
	}
}


void MyVisRankingApp::ProcessKey_FA_VALUE(unsigned char key){
	MyVisData* visData = mTrialManager.GetCurrentVisData();
	const float minDist = 0.1;
	const int maxItr = 10;
	if (key == 'v' || key == 'V'){
		if (visData->GetBoxes().size() > mCurrentBoxIndex){
			MyVec3f c = visData->GetBoundingBox().GetCenter();
			MyBoundingBox box = visData->GetBoxes()[mCurrentBoxIndex];
			box = MyBoundingBox(box.GetCenter(), powf(box.GetVolume(), 1.f / 3));
			visData->SetBox(box, mCurrentBoxIndex);
		}
		else{
			if (mCurrentBoxIndex < 0) mCurrentBoxIndex = 1;
			float r = (rand() % 1001) / (float)1000
				* (MyVisData::GetMaxSphereRadius() - MyVisData::GetMinSphereRadius())
				+ MyVisData::GetMinSphereRadius();
			visData->SetBox(MyBoundingBox(visData->GetBoundingBox().GetCenter(), r),
				mCurrentBoxIndex);
		}
	}
	else if (key == '1'){
		mCurrentBoxIndex = 0;
	}
	else if (key == '2'){
		mCurrentBoxIndex = -1;
	}
	else if (visData->GetBoxes().size() <= mCurrentBoxIndex) return;
	else{
		MyBoundingBox box = visData->GetBoxes()[mCurrentBoxIndex];
		if (ProcessBoundingObjectKey(box, key)){
			visData->SetBox(box, mCurrentBoxIndex);
		}
		// snap
		else if (key == 'z' || key == 'Z'){
			ModifyToStable(box, SMALLMOV, maxItr);
			visData->SetBox(box, mCurrentBoxIndex);
		}
		else if (key == 'x' || key == 'X'){
			ModifyToStable(box, -SMALLMOV, maxItr);
			visData->SetBox(box, mCurrentBoxIndex);
		}
		else if (key == 'c' || key == 'C'){
			MyArray2i points = mTracts.GetVertexInBox(box, visData->GetTractIndices());
			if (points.empty()) cerr << "Empty box. Nothing done." << endl;
			else{
				MyBoundingBox newBox(mTracts.GetCoord(points[0]), 0);
				for (int i = 1; i < points.size(); i++){
					newBox.Engulf(mTracts.GetCoord(points[i]));
				}
				visData->SetBox(newBox, mCurrentBoxIndex);
			}
		}
		else if (key == 'f' || key == 'F'){
			static Bundle lastBd = CG;
			static FiberCover lastCv = BUNDLE;
			static int lastQuest = -1;
			static MyArray<MyBoundingBox> boxes;
			Bundle bd = visData->GetVisInfo().GetBundle();
			FiberCover cv = visData->GetVisInfo().GetCover();
			int quest = visData->GetVisInfo().GetQuest();
			if (bd == lastBd && cv == lastCv && quest == lastQuest && boxes.size()>0){
				float target = visData->GetExpectedValueFromQuestIdx();
				int idx = rand() % boxes.size();
				cerr << "target:" << target << endl;
				cerr << "options:" << boxes.size() << endl;
				visData->SetBox(boxes[idx], 0);
			}
			else {
				float target = visData->GetExpectedValueFromQuestIdx();
				MyArray2f avgs;
				boxes.clear();
				visData->ComputeBoxesWithValue(boxes, avgs, target, 0.02, 100, 500);
				lastBd = bd;
				lastCv = cv;
				lastQuest = quest;
				if (boxes.size()>0){
					int idx = rand() % boxes.size();
					visData->SetBox(box, 0);
					cerr << "target:" << target << endl;
					cerr << "options:" << boxes.size() << endl;
					visData->SetBox(boxes[idx], 0);
				}
				else{
					class compare{
					public:
						bool operator()(const MyVec2f& a, const MyVec2f& b){
							return a[0] < b[0];
						}
					} comp;
					sort(avgs.begin(), avgs.end(), comp);
					cerr << "Auto find failed." << "target:" << target << endl;
					MyVisData::WriteVectorToFile("avgs.txt", avgs);
				}
			}

		}
		else if (key == 'j' || key == 'G'){
			if (mCurrentBoxIndex >= 0){
				visData->PermuteBoxes(mCurrentBoxIndex);
			}
		}
		// save files
		else if (key == 'm' || key == 'M'){
			MyArray<MyBoundingBox> boxes = visData->GetBoxes();
			if (boxes.size() > 0){
				visData->SaveBoxFiles(boxes);
			}
			// skip output
			return;
		}
	}

	// out info
	if (mTrialManager.GetCurrentVisData()->GetVisInfo().GetVisTask() == FA_VALUE){
		float target = visData->GetExpectedValueFromQuestIdx();
		cout << "target: " << target << endl;
		if (visData->GetBoxes().size() >= 1){
			float faSum;
			int nSample;
			mTracts.GetSampleValueInfo(visData->GetBoxes()[0],
				visData->GetTractIndices(), nSample, faSum);
			float fa = faSum / nSample;
			cout << "FA_VALUE: " << fa << endl;

			visData->UpdateAnswers();
			PrintTrialInfo();
		};
	}
}

void MyVisRankingApp::ProcessKey_TRACE(unsigned char key){
	MyVisData* visData = mTrialManager.GetCurrentVisData();
	unordered_map<unsigned char, int> sdict;
	sdict['!'] = 1; sdict['@'] = 2; sdict['#'] = 3;
	sdict['$'] = 4; sdict['%'] = 5; sdict['^'] = 6;
	sdict['&'] = 7; sdict['*'] = 8;
	if (key >= '1' && key <= '3'){
		mCurrentBoxIndex = key - '1';
		mCurrentSelectorIndex = -1;
	}
	else if (sdict.find(key) != sdict.end()){
		mCurrentSelectorIndex = sdict[key] - 1;
		mCurrentBoxIndex = -1;
	}
	else if (key == 'm' || key == 'M'){
		visData->SaveBoxFiles(visData->GetBoxes());
		visData->SaveSelectedIndicesFile(visData->GetSelectIndices());
	}
	else if (key == 'f' || key == 'F'){
		if (mCurrentBoxIndex >= 0){
			const MyBoundingBox& box =
				visData->GetBoxes().at(mCurrentBoxIndex);
			mSelected = ComputedEndFilteredByBox(box, visData->GetSelectIndices());
			visData->SetSelectedIndices(mSelected);
			visData->CheckSelectedValidness();
		}
	}
	else if (key == 'j' || key == 'J'){
		if (mCurrentBoxIndex >= 0){
			visData->PermuteBoxes(mCurrentBoxIndex);
			visData->CheckSelectedValidness();
		}
	}
	else if (key == 'g' || key == 'G'){
		if (visData->GetCorrectAnswers().size() > 0){
			MyBoundingBox box = ComputedEndingBox(visData->GetSelectIndices());
			visData->SetBox(box, visData->GetCorrectAnswers()[0]);
			visData->CheckSelectedValidness();
		}
	}
	else if (mCurrentSelectorIndex >= 0){
		// selector mod
		if (key == 'v' || key == 'V'){
			if (mCurrentSelectorIndex >= mSphereSelectors.size()){
				mSphereSelectors << MySphere(visData->GetBoundingBox().GetCenter(), 10);
				mOperators << '+';
			}
			else{
				MyVec3f c = visData->GetBoundingBox().GetCenter();
				MySphere& sphere = mSphereSelectors[mCurrentSelectorIndex];
				sphere.Translate(c - sphere.GetCenter());
			}
			mSelected = ComputedSelected(mSphereSelectors, mOperators);
			visData->SetSelectedIndices(mSelected);
			visData->CheckSelectedValidness();
		}
		else if (mCurrentSelectorIndex >= mSphereSelectors.size()) return;
		else {
			MySphere& sphere = mSphereSelectors[mCurrentSelectorIndex];
			if (ProcessBoundingObjectKey(sphere, key)){
			}
			else if (key == '\b'){
				mSphereSelectors.EraseAt(mCurrentSelectorIndex);
				mOperators.EraseAt(mCurrentSelectorIndex);
				mCurrentSelectorIndex = max(mCurrentSelectorIndex - 1, 0);
			}
			else if (key == 'z' || key == 'Z'){
				mOperators[mCurrentSelectorIndex] = '+';
			}
			else if (key == 'x' || key == 'X'){
				mOperators[mCurrentSelectorIndex] = '-';
			}
			mSelected = ComputedSelected(mSphereSelectors, mOperators);
			visData->SetSelectedIndices(mSelected);
			visData->CheckSelectedValidness();
		}
	}
	else if (mCurrentBoxIndex >= 0){
		ProcessKey_FA(key);
		visData->CheckSelectedValidness();
	}
	if (visData->GetCorrectAnswers().size() > 0){
		cout << "Answer: " << visData->GetCorrectAnswers()[0] + 1 << endl;
	}
	else{
		cout << "Cannot resolve task answer!" << endl;
	}
}

void MyVisRankingApp::ProcessKey_TUMOR(unsigned char key){
	MyVisData* visData = mTrialManager.GetCurrentVisData();
	int maxItr = 10;

	// new sphere
	if (key == 'v' || key == 'V'){
		if (visData->GetSpheres().size() > 0){
			MySphere sphere = visData->GetSpheres()[0];
			visData->SetSphere(MySphere(MyVec3f(
				visData->GetBoundingBox().GetCenter()), sphere.GetRadius()));
		}
		else{
			float r = (rand() % 1001) / (float)1000
				* (MyVisData::GetMaxSphereRadius() - MyVisData::GetMinSphereRadius())
				+ MyVisData::GetMinSphereRadius();
			visData->SetSphere(MySphere(MyVec3f(
				visData->GetBoundingBox().GetCenter()), r));
		}
	}
	else if (visData->GetSpheres().size() == 0) return;
	// modify sphere
	else {
		MySphere sphere = visData->GetSphere(0);
		if (ProcessBoundingObjectKey(sphere, key)){
			visData->SetSphere(sphere);
		}
		else if (key == 'z' || key == 'Z'){
			if (visData->MoveSphereToStatus(sphere, INTERSECT, maxItr)){
				visData->SetSphere(sphere);
			}
		}
		else if (key == 'x' || key == 'X'){
			if (visData->MoveSphereToStatus(sphere, TOUCH, maxItr)){
				visData->SetSphere(sphere);
			}
		}
		else if (key == 'c' || key == 'C'){
			if (visData->MoveSphereToStatus(sphere, AWAY, maxItr)){
				visData->SetSphere(sphere);
			}
		}
		// save files
		else if (key == 'm' || key == 'M'){
			visData->SaveTumorFile(sphere);
		}
	}

	mVisTract.Update();
}