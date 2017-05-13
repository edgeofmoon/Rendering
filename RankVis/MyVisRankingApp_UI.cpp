#include "MyVisRankingApp.h"
#include "MyTaskBoxDrawer.h"
#include "MyPrimitiveDrawer.h"
#include "MyButton.h"
#include "MyTextArea.h"

#include "GL\glew.h"
#include <GL/freeglut.h>

using namespace MyUI;
using namespace MyVisEnum;

#define UI_INDEX_PAUSE 1
#define UI_INDEX_INPUT 2
#define UI_INDEX_RESUME 3
#define UI_INDEX_BACK 4
#define UI_INDEX_NEXT 5

#define UI_INDEX_ANSWER_GROUP 100
#define UI_INDEX_ANSWER_0 101
#define UI_INDEX_ANSWER_1 102
#define UI_INDEX_ANSWER_2 103

#define UI_INDEX_CONFIDENCE_GROUP 200
#define UI_INDEX_CONFIDENCE_0 201
#define UI_INDEX_CONFIDENCE_1 202
#define UI_INDEX_CONFIDENCE_2 203
#define UI_INDEX_CONFIDENCE_3 204
#define UI_INDEX_CONFIDENCE_4 205
#define UI_INDEX_CONFIDENCE_5 206
#define UI_INDEX_CONFIDENCE_6 207

#define UI_INVALID_ANSWER -10

#define UI_ANSWER_HIGH 1
#define UI_ANSWER_LOW -1

void MyVisRankingApp::ActivateUI(MyUIObject* ui){
	if (ui && mTaskUIs.HasOne(ui)){
		ui->ResetStatus();
	}
}
void MyVisRankingApp::DeactivateUI(MyUIObject* ui){
	if (ui && mTaskUIs.HasOne(ui)){
		ui->Disable();
		ui->SetHidden(true);
	}
}

void MyVisRankingApp::UI_Pause(){
	if (!mbPaused){
		mbPaused = true;
		mLogs.PauseTrial();
		ActivateUI(&mButton_Resume);
		DeactivateUI(&mButton_Pause);
		DeactivateUI(&mButton_Input);
		DeactivateUI(&mButtonGroup_Answer);
		//DeactivateUI(&mButton_Next);
	}
}

void MyVisRankingApp::UI_Input(){
	if (!mbPaused){
		mbPaused = true;
		mLogs.PauseTrial();
		ActivateUI(&mButton_Next);
		ActivateUI(&mButton_Back);
		ActivateUI(&mTextArea_Input);
		ActivateUI(&mTextArea_InputHint);
		DeactivateUI(&mButton_Pause);
		DeactivateUI(&mButton_Input);
		ActivateUI(&mTextArea_ConfidenceHint);
		ActivateUI(&mButtonGroup_Confidence);
	}
	if (UI_CanNext()) mButton_Next.Enable();
	else mButton_Next.Disable();
	if (mConfidenceSelected >= 0){
		mButtonGroup_Confidence.SetToSelectedIndex(UI_INDEX_CONFIDENCE_0 + mConfidenceSelected);
	}
}

void MyVisRankingApp::UI_Resume(){
	if (mbPaused){
		mbPaused = false;
		mLogs.ResumeTrial();
		DeactivateUI(&mButton_Resume);
		ActivateUI(&mButton_Pause);
		ActivateUI(&mButton_Input);
		ActivateUI(&mButtonGroup_Answer);
		//ActivateUI(&mButton_Next);
	}
	// resume answer
	if (mAnswerSelected >= 0){
		mButtonGroup_Answer.SetToSelectedIndex(UI_INDEX_ANSWER_0 + mAnswerSelected);
	}
}

void MyVisRankingApp::UI_Back(){
	if (mbPaused){
		mbPaused = false;
		mLogs.ResumeTrial();
		DeactivateUI(&mButton_Next);
		DeactivateUI(&mButton_Back);
		DeactivateUI(&mTextArea_Input);
		DeactivateUI(&mTextArea_InputHint);
		ActivateUI(&mButton_Pause);
		ActivateUI(&mButton_Input);
		DeactivateUI(&mTextArea_ConfidenceHint);
		DeactivateUI(&mButtonGroup_Confidence);
	}
}

void MyVisRankingApp::UI_AnswerSelect(int idx){
	MyButton* button = mButtonGroup_Answer.GetButton(UI_INDEX_ANSWER_0 + idx);
	if (button && button->GetEventBit() & PUSHED_BIT){
		button->ClearEventBit();
		mAnswerSelected = idx;
		if (mConfidenceSelected >= 0){
			mButton_Next.Enable();
		}
	}
	else{
		mAnswerSelected = UI_INVALID_ANSWER;
		mButton_Next.Disable();
	}
	//mButton_Answer[idx].ClearEventBit();
	mLogs.SetUserAnswer(mAnswerSelected);
	if (IsOnMode(APP_MODE_DEBUG)) cout << "Answer: " << mAnswerSelected << endl;
}

void MyVisRankingApp::UI_ConfidenceSelected(int idx){
	MyButton* button = mButtonGroup_Confidence.GetButton(UI_INDEX_CONFIDENCE_0 + idx);
	if (button && button->GetEventBit() & PUSHED_BIT){
		button->ClearEventBit();
		mConfidenceSelected = idx + 1;
		if (mAnswerSelected >= 0 || mTextArea_Input.IsEnabled()){
			mButton_Next.Enable();
		}
	}
	else{
		mConfidenceSelected = UI_INVALID_ANSWER;
		mButton_Next.Disable();
	}
	mLogs.SetUserConfidence(mConfidenceSelected);
	if (IsOnMode(APP_MODE_DEBUG)) cout << "Confidence: " << mConfidenceSelected << endl;
}

void MyVisRankingApp::UI_Next(){
	if (mTrialManager.GetCurrentVisData()->GetVisInfo().IsEmpty()){
		Next();
	}
	else if (mAnswerSelected != UI_INVALID_ANSWER){
		if (mTrialManager.GetCurrentVisData()->GetVisInfo().IsTraining()
			&& !mbDrawHighlighted) {
			mbDrawHighlighted = true;
			MyString correctAnswer =
				mTrialManager.GetCurrentVisData()->GetCorrectAnswerString();
			MyString answerStr = "Correct: " + correctAnswer;
			mTextArea_Answer.SetText(answerStr);
		}
		else Next();
	}
	else if (!mTextArea_Input.IsHidden()){
		float ans;
		bool success = mTextArea_Input.GetFloat(ans);
		if (success){
			if (ans > UI_ANSWER_HIGH || ans < UI_ANSWER_LOW){
				MyString warningStr;
				warningStr = MyString(ans) + " isn't in range!";
				mTextArea_InputHint.SetText(warningStr);

			}
			else{
				mLogs.SetUserAnswer(ans);
				if (IsOnMode(APP_MODE_DEBUG)) cout << "Answer: " << ans << endl;
				if (mTrialManager.GetCurrentVisData()->GetVisInfo().IsTraining()
					&& !mbDrawHighlighted) {
					mbDrawHighlighted = true;
					MyString correctAnswer = 
						mTrialManager.GetCurrentVisData()->GetCorrectAnswerString();
					MyString answerStr = "Correct: " + correctAnswer;
					mTextArea_Answer.SetText(answerStr);
				}
				else Next();
			}
		}
		else{
			MyString warningStr("Not a number!");
			mTextArea_InputHint.SetText(warningStr);
		}
	}
}

void MyVisRankingApp::UI_Process(int uid){
	switch (uid){
		case UI_INDEX_PAUSE:
			UI_Pause();
			break;
		case UI_INDEX_INPUT:
			UI_Input();
			break;
		case UI_INDEX_RESUME:
			UI_Resume();
			break;
		case UI_INDEX_BACK:
			UI_Back();
			break;
		case UI_INDEX_NEXT:
			UI_Next();
			break;
		case UI_INDEX_ANSWER_GROUP:
			UI_AnswerSelect(
				mButtonGroup_Answer.GetSelectedIndex() - UI_INDEX_ANSWER_0);
			break;
		case UI_INDEX_CONFIDENCE_GROUP:
			UI_ConfidenceSelected(
				mButtonGroup_Confidence.GetSelectedIndex() - UI_INDEX_CONFIDENCE_0);
			break;
	}
}

bool MyVisRankingApp::UI_CanNext(){
	if (mConfidenceSelected >= 0 && (!mTextArea_Input.IsHidden()
		|| mAnswerSelected >= 0)){
		return true;
	}
	return false;
}

int MyVisRankingApp::UIProcessMouseUp(int x, int y){
	float fx = x;
	float fy = mWindowHeight - y;
	for (int i = 0; i < mTaskUIs.size(); i++){
		if (mTaskUIs[i]->HandleMouseUp(fx, fy)) return 1;
	}
	return 0;
}

int MyVisRankingApp::UIProcessMouseDown(int x, int y){
	float fx = x;
	float fy = mWindowHeight - y;
	for (int i = 0; i < mTaskUIs.size(); i++){
		if (mTaskUIs[i]->HandleMouseDown(fx, fy)) {
			UI_Process(mTaskUIs[i]->GetIndex());
			return 1;
		}
	}
	return 0;
}

int MyVisRankingApp::UIProcessMouseMove(int x, int y){
	float fx = x;
	float fy = mWindowHeight - y;
	for (int i = 0; i < mTaskUIs.size(); i++){
		if (mTaskUIs[i]->HandleMouseMove(fx, fy)) return 1;
	}
	return 0;
}

int MyVisRankingApp::UIProcessKey(unsigned char key, int x, int y){
	float fx = x;
	float fy = mWindowHeight - y;
	for (int i = 0; i < mTaskUIs.size(); i++){
		if (mTaskUIs[i]->HandleKey(key, fx, fy)) return 1;
	}
	return 0;
}

int MyVisRankingApp::UIProcessSpecialKey(unsigned char key, int x, int y){
	float fx = x;
	float fy = mWindowHeight - y;
	for (int i = 0; i < mTaskUIs.size(); i++){
		if (mTaskUIs[i]->HandleSpecialKey(key, fx, fy)) return 1;
	}
	return 0;
}

void MyVisRankingApp::UIInit(){
	mbPaused = false;
	mButtonWidth = 100;
	mButtonHeight = 60;
	mButtonInterval = 20;
	mButton_Next.SetIndex(UI_INDEX_NEXT);
	mButtonGroup_Answer.ClearOptions();
	mButtonGroup_Answer.AddOption("3", UI_INDEX_ANSWER_2);
	mButtonGroup_Answer.AddOption("2", UI_INDEX_ANSWER_1);
	mButtonGroup_Answer.AddOption("1", UI_INDEX_ANSWER_0);
	mButtonGroup_Confidence.ClearOptions();
	mButtonGroup_Confidence.AddOption("1", UI_INDEX_CONFIDENCE_0);
	mButtonGroup_Confidence.AddOption("2", UI_INDEX_CONFIDENCE_1);
	mButtonGroup_Confidence.AddOption("3", UI_INDEX_CONFIDENCE_2);
	mButtonGroup_Confidence.AddOption("4", UI_INDEX_CONFIDENCE_3);
	mButtonGroup_Confidence.AddOption("5", UI_INDEX_CONFIDENCE_4);
	mButtonGroup_Confidence.AddOption("6", UI_INDEX_CONFIDENCE_5);
	mButtonGroup_Confidence.AddOption("7", UI_INDEX_CONFIDENCE_6);
	mButton_Input.SetIndex(UI_INDEX_INPUT);
	mButton_Back.SetIndex(UI_INDEX_BACK);
	mButton_Pause.SetIndex(UI_INDEX_PAUSE);
	mButton_Resume.SetIndex(UI_INDEX_RESUME);
	mButtonGroup_Answer.SetIndex(UI_INDEX_ANSWER_GROUP);
	mButtonGroup_Confidence.SetIndex(UI_INDEX_CONFIDENCE_GROUP);
	mButton_Next.SetSize(MyVec2f(mButtonWidth, mButtonHeight));
	mButton_Input.SetSize(MyVec2f(mButtonWidth, mButtonHeight));
	mButton_Back.SetSize(MyVec2f(mButtonWidth, mButtonHeight));
	mButton_Pause.SetSize(MyVec2f(mButtonWidth, mButtonHeight));
	mButton_Resume.SetSize(MyVec2f(mButtonWidth, mButtonHeight));
	mButtonGroup_Confidence.SetButtonSize(MyVec2f(mButtonWidth, mButtonHeight));
	mButtonGroup_Confidence.SetButtonInterval(MyVec2f(mButtonInterval, mButtonInterval));
	mButtonGroup_Answer.SetButtonSize(MyVec2f(mButtonWidth, mButtonHeight));
	mButtonGroup_Answer.SetButtonInterval(MyVec2f(mButtonInterval, mButtonInterval));
	mButtonGroup_Confidence.SetLayout(MyToggleButtonGroup::Layout_Horizontal);
	mButtonGroup_Confidence.UpdateLayout();
	mButtonGroup_Answer.SetLayout(MyToggleButtonGroup::Layout_Vertical);
	mButtonGroup_Answer.UpdateLayout();
	mButton_Next.SetText("Next");
	mButton_Input.SetText("Input");
	mButton_Back.SetText("Back");
	mButton_Pause.SetText("Pause");
	mButton_Resume.SetText("Resume");
	mTextArea_Input.SetConstant(false);
	mTextArea_Input.SetBlankText("Please key in a number between "
		+ MyString(UI_ANSWER_LOW) + " and " + MyString(UI_ANSWER_HIGH));
	mTextArea_ConfidenceHint.SetText("Please select your confidence level in your answer: 1 = least confident, 7 = most confident.");
	mTextArea_Input.SetMaxSize(20);
	mTextArea_InputHint.SetConstant(true);
	mTextArea_Transition.SetConstant(true);
	mTextArea_Hint.SetConstant(true);
	mTextArea_Progress.SetConstant(true);
	mTextArea_Answer.SetConstant(true);
	mTextArea_ConfidenceHint.SetConstant(true);
}

void MyVisRankingApp::UIUpdate(){
	const MyVisInfo& visInfo = mTrialManager.GetCurrentVisData()->GetVisInfo();
	mTaskUIs.clear();
	if (visInfo.IsEmpty()){
		mTaskUIs << &mButton_Next
			<< &mTextArea_Transition
			<< &mTextArea_Progress;
		mTextArea_Transition.SetText(visInfo.GetTaskTransitionString());
	}
	else{
		if (visInfo.GetVisTask() == FA){
			mTaskUIs << &mButton_Next
				<< &mButton_Input
				<< &mButton_Back
				<< &mButton_Pause
				<< &mButton_Resume
				<< &mTextArea_Hint
				<< &mTextArea_Progress
				<< &mTextArea_Input
				<< &mTextArea_InputHint
				<< &mTextArea_ConfidenceHint
				<< &mButtonGroup_Confidence;
			DeactivateUI(&mButton_Next);
			ActivateUI(&mButton_Input);
			DeactivateUI(&mButton_Back);
			ActivateUI(&mButton_Pause);
			DeactivateUI(&mButton_Resume);
			ActivateUI(&mTextArea_Hint);
			ActivateUI(&mTextArea_Progress);
			DeactivateUI(&mTextArea_Input);
			DeactivateUI(&mTextArea_InputHint);
			DeactivateUI(&mTextArea_ConfidenceHint);
			DeactivateUI(&mButtonGroup_Confidence);
			mButton_Next.Disable();
			mTextArea_Input.ClearText();
			mTextArea_InputHint.SetText("Input a number representing averge FA of box 1 minus that of box 2");
		}
		else if (visInfo.GetVisTask() == TRACE){
			mButtonGroup_Answer.GetButton(UI_INDEX_ANSWER_0)->SetText("1");
			mButtonGroup_Answer.GetButton(UI_INDEX_ANSWER_1)->SetText("2");
			mButtonGroup_Answer.GetButton(UI_INDEX_ANSWER_2)->SetText("3");
			mTaskUIs << &mButton_Next
				<< &mButton_Input
				<< &mButton_Back
				<< &mButton_Pause
				<< &mButton_Resume
				<< &mButtonGroup_Answer
				<< &mTextArea_Hint
				<< &mTextArea_Progress
				<< &mTextArea_ConfidenceHint
				<< &mButtonGroup_Confidence;
			DeactivateUI(&mButton_Next);
			ActivateUI(&mButton_Input);
			DeactivateUI(&mButton_Back);
			ActivateUI(&mButton_Pause);
			DeactivateUI(&mButton_Resume);
			ActivateUI(&mButtonGroup_Answer);
			ActivateUI(&mTextArea_Hint);
			ActivateUI(&mTextArea_Progress);
			DeactivateUI(&mTextArea_ConfidenceHint);
			DeactivateUI(&mButtonGroup_Confidence);
			mButton_Next.Disable();
		}
		else if (visInfo.GetVisTask() == TUMOR){
			mButtonGroup_Answer.GetButton(UI_INDEX_ANSWER_0)->SetText("Intersect");
			mButtonGroup_Answer.GetButton(UI_INDEX_ANSWER_1)->SetText("Touch");
			mButtonGroup_Answer.GetButton(UI_INDEX_ANSWER_2)->SetText("Away");
			mTaskUIs << &mButton_Next
				<< &mButton_Input
				<< &mButton_Back
				<< &mButton_Pause
				<< &mButton_Resume
				<< &mButtonGroup_Answer
				<< &mTextArea_Hint
				<< &mTextArea_Progress
				<< &mTextArea_ConfidenceHint
				<< &mButtonGroup_Confidence;
			DeactivateUI(&mButton_Next);
			ActivateUI(&mButton_Input);
			DeactivateUI(&mButton_Back);
			ActivateUI(&mButton_Pause);
			DeactivateUI(&mButton_Resume);
			ActivateUI(&mButtonGroup_Answer);
			ActivateUI(&mTextArea_Hint);
			ActivateUI(&mTextArea_Progress);
			DeactivateUI(&mTextArea_ConfidenceHint);
			DeactivateUI(&mButtonGroup_Confidence);
			mButton_Next.Disable();
		}
		mTextArea_Hint.SetText(visInfo.GetTaskHintString());
		mTaskUIs << &mTextArea_Answer;
		mTextArea_Answer.ClearText();
	}
	mTextArea_Progress.SetText(mTrialManager.GetProgresInfoString());
	mbPaused = false;
	mAnswerSelected = UI_INVALID_ANSWER;
	mConfidenceSelected = UI_INVALID_ANSWER;
	mbDrawHighlighted = false;
}

void MyVisRankingApp::UIResize(int w, int h){
	MyUI::SetViewport(MyVec4i(0, 0, w, h));
	float boarder = 15;
	float xPos = w - mButtonWidth - boarder;
	float yOffset = mButtonInterval + mButtonHeight;
	float yPos = boarder;
	mButton_Input.SetPosition(MyVec2f(xPos, yPos + 1 * yOffset));
	mButton_Back.SetPosition(MyVec2f(xPos, yPos + 1 * yOffset));
	mButton_Next.SetPosition(MyVec2f(xPos, yPos));
	mTextArea_Answer.SetPosition(MyVec2f(xPos, yPos + 5 * yOffset));
	mButton_Pause.SetPosition(MyVec2f(xPos, h - yPos - yOffset));
	mButton_Resume.SetPosition(MyVec2f(xPos, h - yPos - yOffset));
	mTextArea_Hint.SetPosition(MyVec2f(w / 2, boarder));
	mTextArea_Transition.SetPosition(MyVec2f(w / 2, h / 2));
	mTextArea_Progress.SetPosition(MyVec2f(boarder + mButtonWidth / 2, h - boarder - mButtonHeight));
	mTextArea_InputHint.SetPosition(MyVec2f(w / 2, h / 2 + mButtonHeight * 2));
	mTextArea_Input.SetPosition(MyVec2f(w / 2, h / 2 + mButtonHeight * 1));
	mTextArea_ConfidenceHint.SetPosition(MyVec2f(w / 2, h / 2 - mButtonHeight * 1));
	MyVec2f gSize = mButtonGroup_Confidence.GetSize();
	mButtonGroup_Confidence.SetPosition(MyVec2f(w / 2 - gSize[0] / 2, h / 2 - mButtonHeight * 3));
	mButtonGroup_Confidence.UpdateLayout();
	mButtonGroup_Answer.SetPosition(MyVec2f(xPos, yPos + 2 * yOffset));
	mButtonGroup_Answer.UpdateLayout();
}

void MyVisRankingApp::UIDestory(){
	//for (int i = 0; i < mUIs.size(); i++){
	//	delete mUIs[i];
	//}
	//mUIs.clear();
}

void MyVisRankingApp::UIDraw(){
	UIBeginDrawer();
	for (int i = 0; i < mTaskUIs.size(); i++){
		mTaskUIs[i]->Show();
	}

	UIEndDrawer();
}

void MyVisRankingApp::DrawHighlightedTracts(MyTractVisBase* tractVis,
	const MyArrayi& tractIndices, const MyColor4f& color){
	if (!tractVis) return;
	MySphereGeometry* sg = sg = tractVis->GetSphereGeometry();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDepthFunc(GL_LEQUAL);
	MyColor4f baseColor;
	MyArrayi tractShow;
	MyArrayf influences;
	float ambient;
	MyColor4f baseSphereColor;
	baseColor = tractVis->GetBaseColor();
	tractShow = tractVis->GetTractsShown();
	ambient = tractVis->GetAmbient();
	influences = tractVis->GetInfluences();
	if(sg) baseSphereColor = sg->GetColor();
	tractVis->SetTractsShown(tractIndices);
	tractVis->SetBaseColor(color);
	tractVis->SetAmbient(1);
	tractVis->ClearInfluences();
	if (sg) sg->SetColor(color);
	tractVis->Show();
	tractVis->SetBaseColor(baseColor);
	tractVis->SetTractsShown(tractShow);
	tractVis->SetAmbient(ambient);
	tractVis->SetInfluences(influences);
	if (sg) sg->SetColor(baseSphereColor);
	glPopAttrib();
}

void MyVisRankingApp::DrawHighlightedTracts(MyTractVisBase* tractVis){
	if (tractVis){
		MyVisData* visData = mTrialManager.GetCurrentVisData();
		DrawHighlightedTracts(tractVis, visData->GetSelectIndices(), MyColor4f::yellow());
		if (visData->GetSpheres().size() > 0){
			const MySphere& sphere = visData->GetSphere(0);
			CollisionStatus cs = visData->ComputeSphereCollusionStatus(sphere);
			MyColor4f color = GetCollisionStatusColor(cs);
			MyArrayi indices;
			if (cs == TOUCH) indices = visData->ComputeTouchedIndices(sphere);
			else if (cs == INTERSECT) indices = visData->ComputeIntersectedIndices(sphere);
			DrawHighlightedTracts(tractVis, indices, color);
		}
	}
}

void MyVisRankingApp::DrawHighlighted(){
	MyTractVisBase* tractVis = mVisTract.GetTractVis();
	MyTractVisBase* tractVisAux = mVisTract.GetTractVisAux();
	DrawHighlightedTracts(tractVis);
	DrawHighlightedTracts(tractVisAux);
}

void MyVisRankingApp::DrawBoxes(){
	const MyVisData* visData = mTrialManager.GetCurrentVisData();
	const MyArray<MyBoundingBox>& boxes = visData->GetBoxes();
	MyTaskBoxDrawer boxDrawer;
	for (int i = 0; i < boxes.size(); i++){
		boxDrawer.SetHighlighted(i == mCurrentBoxIndex);
		boxDrawer.SetBox(&boxes[i]);
		boxDrawer.Draw();
	}
	// draw text
	for (int i = 0; i < boxes.size(); i++){
		if (i == mCurrentBoxIndex) MyGraphicsTool::Color(MyColor4f::red());
		else MyGraphicsTool::Color(MyColor4f::green());
		MyPrimitiveDrawer::DrawStrokeTextOrtho(boxes[i].GetHighPos(), MyString(1, char('1' + i)));
	}
}

void MyVisRankingApp::DrawTractIndicators(){
	const MyVisData* visData = mTrialManager.GetCurrentVisData();
	const MyTracks* tracts = visData->GetTracts();
	const MyArrayi& tractIndices = visData->GetSelectIndices();
	const MyArrayb& sphereAtHead = visData->GetSphereAtHead();
	MyGraphicsTool::PushAllAttributes();
	MyGraphicsTool::Color(MyColor4f::yellow());
	for (int i = 0; i < tractIndices.size(); i++){
		MyVec3f pos;
		int tractIndex = tractIndices[i];
		if (sphereAtHead[i]) pos = tracts->GetCoord(tractIndex, 0);
		else pos = tracts->GetCoord(tractIndex, tracts->GetNumVertex(tractIndex) - 1);
		MyPrimitiveDrawer::DrawSphereAt(pos, 1);
	}
	// draw spheres
	for (int i = 0; i < mSphereSelectors.size(); i++){
		if (mOperators[i] == '+') MyGraphicsTool::Color(MyColor4f::green());
		else MyGraphicsTool::Color(MyColor4f::blue());
		MyPrimitiveDrawer::DrawSphereAt(
			mSphereSelectors[i].GetCenter(), mSphereSelectors[i].GetRadius());
		// draw sphere text
		if (i == mCurrentSelectorIndex) MyGraphicsTool::Color(MyColor4f::red());
		else MyGraphicsTool::Color(MyColor4f::green());
		MyVec3f charPos = mSphereSelectors[i].GetCenter() 
			+ MyVec3f(0, 0, 1)*mSphereSelectors[i].GetRadius();
		MyPrimitiveDrawer::DrawStrokeTextOrtho( charPos, MyString(1, char('1' + i)));
	}
	MyGraphicsTool::PopAttributes();
}