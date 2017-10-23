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
#define UI_INDEX_RESUME 2
#define UI_INDEX_INPUT 3
#define UI_INDEX_CHECK 4
#define UI_INDEX_NEXT 5

#define UI_INDEX_ANSWER_GROUP 100
#define UI_INDEX_ANSWER_0 101
#define UI_INDEX_ANSWER_1 102
#define UI_INDEX_ANSWER_2 103
#define UI_INDEX_ANSWER_3 104
#define UI_INDEX_ANSWER_4 105
#define UI_INDEX_ANSWER_5 106
#define UI_INDEX_ANSWER_6 107

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

#define LEGEND_LEFT 0.8
#define LEGEND_BOTTOM 0.2
#define LEGEND_WIDTH 0.02
#define LEGEND_HEIGHT 0.6
#define LEGEND_TICKSIZE 0.005

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
		DeactivateUI(&mTextArea_AnswerHint0);
		DeactivateUI(&mTextArea_AnswerHint1);
		//DeactivateUI(&mButton_Next);
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
		ActivateUI(&mTextArea_AnswerHint0);
		ActivateUI(&mTextArea_AnswerHint1);
		//ActivateUI(&mButton_Next);
	}
	if (mConfidenceSelected >= 0){
		mButtonGroup_Confidence.SetToSelectedIndex(UI_INDEX_CONFIDENCE_0 + mConfidenceSelected);
	}
	if (mAnswerSelected >= 0){
		mButtonGroup_Answer.SetToSelectedIndex(UI_INDEX_ANSWER_0 + mAnswerSelected);
	}
}

void MyVisRankingApp::UI_Input(){
	if (!mbPaused){
		mbPaused = true;
		mLogs.PauseTrial();
		if (UI_CanCheck()) ActivateUI(&mButton_Check);
		else ActivateUI(&mButton_Next);
		DeactivateUI(&mButton_Pause);
		DeactivateUI(&mButton_Input);
		//ActivateUI(&mButtonGroup_Answer);
		ActivateUI(&mTextArea_ConfidenceHint);
		ActivateUI(&mButtonGroup_Confidence);
		ActivateUI(&mNumberArea_Input);
		ActivateUI(&mNumberArea_InputHint);

		if (UI_Answered()) {
			if (UI_CanCheck()) mButton_Check.Enable();
			else mButton_Next.Enable();
		}
		else {
			if (UI_CanCheck()) mButton_Check.Disable();
			else mButton_Next.Disable();
		}
	}
	if (mConfidenceSelected >= 0){
		mButtonGroup_Confidence.SetToSelectedIndex(UI_INDEX_CONFIDENCE_0 + mConfidenceSelected);
	}
	if (mAnswerSelected >= 0){
		mButtonGroup_Answer.SetToSelectedIndex(UI_INDEX_ANSWER_0 + mAnswerSelected);
	}
}

void MyVisRankingApp::UI_Check(){
	if (mbPaused){
		mbPaused = false;
		mLogs.ResumeTrial();
		DeactivateUI(&mButton_Check);
		ActivateUI(&mButton_Next);
		DeactivateUI(&mTextArea_AnswerHint0);
		DeactivateUI(&mTextArea_AnswerHint1);
		DeactivateUI(&mButtonGroup_Answer);
		DeactivateUI(&mButton_Pause);
		DeactivateUI(&mButton_Input);
		DeactivateUI(&mTextArea_ConfidenceHint);
		DeactivateUI(&mButtonGroup_Confidence);
		DeactivateUI(&mNumberArea_Input);
		DeactivateUI(&mNumberArea_InputHint);
	}
	MyVisInfo visInfo = mTrialManager.GetCurrentVisData()->GetVisInfo();
	if (visInfo.IsTraining() && !mbDrawHighlighted) {
		mbDrawHighlighted = true;
		if (visInfo.GetVisTask() != FA_VALUE){
			if (mTrialManager.GetCurrentVisData()->IsAnswerCorrect(mAnswerSelected)){
				mTextArea_Answer.SetTextColor(MyColor4f::green());
				mTextArea_Answer.SetText("Correct!");
			}
			else{
				mTextArea_Answer.SetTextColor(MyColor4f::red());
				MyString correctAnswer =
					mTrialManager.GetCurrentVisData()->GetCorrectAnswerString();
				MyString answerStr = "Answer: " + correctAnswer;
				mTextArea_Answer.SetText(answerStr);
			}
		}
		else{
			float crtAns = mTrialManager.GetCurrentVisData()->GetAnswerInfo();
			float error = mTrialManager.GetCurrentVisData()->GetError(mAnswerInput);
			if (error <= 0.1){
				mTextArea_Answer.SetTextColor(MyColor4f::green());
				mTextArea_Answer.SetText("Very Close!\n" + MyString(crtAns));
			}
			else if (error <= 0.2){
				mTextArea_Answer.SetTextColor(MyColor4f::blue());
				mTextArea_Answer.SetText("Not really!\n" + MyString(crtAns));
			}
			else{
				mTextArea_Answer.SetTextColor(MyColor4f::red());
				mTextArea_Answer.SetText("Wrong!\n" + MyString(crtAns));
			}
		}
		ActivateUI(&mButton_Next);
	}
}

void MyVisRankingApp::UI_Next(){
	if (UI_Answered()){
		Next();
	}
}

void MyVisRankingApp::UI_AnswerSelect(int idx, MyButton* button){
	if (button && button->GetEventBit() & PUSHED_BIT){
		button->ClearEventBit();
		mAnswerSelected = idx;
		if (mConfidenceSelected >= 0){
			if (UI_CanCheck()) mButton_Check.Enable();
			else mButton_Next.Enable();
		}
		if (!mTaskUIs.HasOne(&mButton_Check) || mButton_Check.IsHidden()) mButton_Input.Enable();
	}
	else{
		mAnswerSelected = UI_INVALID_ANSWER;
		if (UI_CanCheck()) mButton_Check.Disable();
		else mButton_Next.Disable();
		mButton_Input.Disable();
	}
	//mButton_Answer[idx].ClearEventBit();
	mLogs.SetUserAnswer(mAnswerSelected);
	if (IsOnMode(APP_MODE_DEBUG)) cout << "Answer: " << mAnswerSelected << endl;
}

void MyVisRankingApp::UI_AnswerInput(float value, bool valid){
	if (valid && value > 0.2 && value < 1){
		mAnswerInput = value;
		mNumberArea_InputHint.SetTextColor(MyColor4f::green());
		mNumberArea_InputHint.SetText("Acceptable answer.");

		if (mConfidenceSelected >= 0){
			if (UI_CanCheck()) mButton_Check.Enable();
			else mButton_Next.Enable();
		}
		if (!mTaskUIs.HasOne(&mButton_Check) || mButton_Check.IsHidden()) mButton_Input.Enable();
	}
	else{
		if (valid) mNumberArea_InputHint.SetText("Value should be between 0.2 and 1.");
		else mNumberArea_InputHint.SetText("Input is not a number.");
		mAnswerInput = -1;
		mNumberArea_InputHint.SetTextColor(MyColor4f::red());

		if (UI_CanCheck()) mButton_Check.Disable();
		else mButton_Next.Disable();
		mButton_Input.Disable();
	}
	mLogs.SetUserAnswer(value);
	if (IsOnMode(APP_MODE_DEBUG)) cout << "Answer: " << mAnswerInput << endl;
}

void MyVisRankingApp::UI_ConfidenceSelected(int idx){
	MyButton* button = mButtonGroup_Confidence.GetButton(UI_INDEX_CONFIDENCE_0 + idx);
	if (button && button->GetEventBit() & PUSHED_BIT){
		button->ClearEventBit();
		mConfidenceSelected = idx + 1;
		if (mAnswerSelected >= 0){
			if (UI_CanCheck()) mButton_Check.Enable();
			else mButton_Next.Enable();
		}
		else if (mAnswerInput >= 0){
			if (UI_CanCheck()) mButton_Check.Enable();
			else mButton_Next.Enable();
		}
	}
	else{
		mConfidenceSelected = UI_INVALID_ANSWER;
		if (UI_CanCheck()) mButton_Check.Disable();
		else mButton_Next.Disable();
	}
	mLogs.SetUserConfidence(mConfidenceSelected);
	if (IsOnMode(APP_MODE_DEBUG)) cout << "Confidence: " << mConfidenceSelected << endl;
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
		case UI_INDEX_CHECK:
			UI_Check();
			break;
		case UI_INDEX_NEXT:
			UI_Next();
			break;
		case UI_INDEX_ANSWER_GROUP:
			UI_AnswerSelect(
				mButtonGroup_Answer.GetSelectedIndex() - UI_INDEX_ANSWER_0,
				mButtonGroup_Answer.GetSelectedButton());
			break;
		case UI_INDEX_CONFIDENCE_GROUP:
			UI_ConfidenceSelected(
				mButtonGroup_Confidence.GetSelectedIndex() - UI_INDEX_CONFIDENCE_0);
			break;
	}
}

bool MyVisRankingApp::UI_Answered() const{
	if (mTrialManager.GetCurrentVisData()->GetVisInfo().IsEmpty()
		|| (mConfidenceSelected >= 0 && mAnswerSelected >= 0)
		|| (mConfidenceSelected >= 0 && mAnswerInput >= 0)){
		return true;
	}
	return false;
}

bool MyVisRankingApp::UI_CanCheck() const{
	return mTrialManager.GetCurrentVisData()->GetVisInfo().IsTraining();
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
		if (mTaskUIs[i]->HandleKey(key, fx, fy)){
			MyNumberArea* numberArea = dynamic_cast<MyNumberArea*>(mTaskUIs[i]);
			if (numberArea){
				float ans;
				bool valid = numberArea->GetFloat(ans);
				UI_AnswerInput(ans, valid);
			}
			return 1;
		}
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
	mButtonWidth = 110;
	mButtonHeight = 50;
	mButtonIntervalX = 20;
	mButtonIntervalY = 15;
	mButton_Next.SetIndex(UI_INDEX_NEXT);
	mButtonGroup_Confidence.ClearOptions();
	mButton_Input.SetIndex(UI_INDEX_INPUT);
	mButton_Check.SetIndex(UI_INDEX_CHECK);
	mButton_Pause.SetIndex(UI_INDEX_PAUSE);
	mButton_Resume.SetIndex(UI_INDEX_RESUME);
	mButtonGroup_Answer.SetIndex(UI_INDEX_ANSWER_GROUP);
	mButtonGroup_Confidence.SetIndex(UI_INDEX_CONFIDENCE_GROUP);
	for (int i = 0; i < 7; i++){
		mButtonGroup_Confidence.AddOption(MyString(i + 1), UI_INDEX_CONFIDENCE_0 + i);
	}
	mButton_Next.SetSize(MyVec2f(mButtonWidth, mButtonHeight));
	mButton_Input.SetSize(MyVec2f(mButtonWidth, mButtonHeight));
	mButton_Check.SetSize(MyVec2f(mButtonWidth, mButtonHeight));
	mButton_Pause.SetSize(MyVec2f(mButtonWidth, mButtonHeight));
	mButton_Resume.SetSize(MyVec2f(mButtonWidth, mButtonHeight));
	mButtonGroup_Answer.SetButtonSize(MyVec2f(mButtonWidth, mButtonHeight));
	mButtonGroup_Answer.SetButtonInterval(MyVec2f(mButtonIntervalX, mButtonIntervalY));
	mButtonGroup_Answer.SetLayout(MyToggleButtonGroup::Layout_Vertical);
	mButtonGroup_Answer.SetAlighment(MyToggleButtonGroup::Alignment_Down);
	mButtonGroup_Answer.UpdateLayout();
	mButtonGroup_Confidence.SetButtonSize(MyVec2f(mButtonWidth, mButtonHeight));
	mButtonGroup_Confidence.SetButtonInterval(MyVec2f(mButtonIntervalX, mButtonIntervalY));
	mButtonGroup_Confidence.SetLayout(MyToggleButtonGroup::Layout_Horizontal);
	mButtonGroup_Confidence.SetAlighment(MyToggleButtonGroup::Alignment_Middle);
	mButtonGroup_Confidence.UpdateLayout();
	mButton_Next.SetText("Next");
	mButton_Input.SetText("Answer");
	mButton_Check.SetText("Check");
	mButton_Pause.SetText("Pause");
	mButton_Resume.SetText("Resume");
	mTextArea_AnswerHint0.SetText("Box 1 Higher");
	mTextArea_AnswerHint1.SetText("Box 1 Lower");
	mTextArea_ConfidenceHint.SetText(
		"Please select your confidence level in your answer: \
		1 = least confident, 7 = most confident.");
	mTextArea_AnswerHint0.SetConstant(true);
	mTextArea_AnswerHint1.SetConstant(true);
	mTextArea_Transition.SetConstant(true);
	mTextArea_Hint.SetConstant(true);
	mTextArea_Progress.SetConstant(true);
	mTextArea_Answer.SetConstant(true);
	mTextArea_ConfidenceHint.SetConstant(true);
	mNumberArea_InputHint.SetText("What is the average FA value of the tracts in  box 1?");
	mNumberArea_InputHint.SetConstant(true);
	mNumberArea_Input.SetBlankText("Please type in the average FA value.");
	mNumberArea_Input.SetConstant(false);
}

void MyVisRankingApp::UIUpdate(){
	const MyVisInfo& visInfo = mTrialManager.GetCurrentVisData()->GetVisInfo();
	for (auto ui : mTaskUIs) DeactivateUI(ui);
	mTaskUIs.clear();
	if (visInfo.IsEmpty()){
		mTaskUIs << &mButton_Next
			<< &mTextArea_Transition
			<< &mTextArea_Progress;
		ActivateUI(&mButton_Next);
		ActivateUI(&mTextArea_Transition);
		ActivateUI(&mTextArea_Progress);
		mTextArea_Transition.SetText(visInfo.GetTaskTransitionString());
		if (mTrialManager.IsLast()) mButton_Next.SetText("Finish");
	}
	else{
		mButtonGroup_Answer.ClearOptions();
		for (int i = 0; i < visInfo.GetNumberAnswerOption(); i++){
			if (visInfo.GetVisTask() == FA){
				mButtonGroup_Answer.AddOption(
					visInfo.GetAnswerOptionString(i), UI_INDEX_ANSWER_0 + i);
			}
			else{
				mButtonGroup_Answer.AddOption(
					visInfo.GetAnswerOptionString(visInfo.GetNumberAnswerOption() - i - 1),
					UI_INDEX_ANSWER_0 + visInfo.GetNumberAnswerOption() - i - 1);
			}
		}
		mButtonGroup_Answer.UpdateLayout();
		mTaskUIs << &mButton_Next
			<< &mButton_Input
			<< &mButton_Pause
			<< &mButton_Resume
			<< &mButtonGroup_Answer
			<< &mTextArea_ConfidenceHint
			<< &mButtonGroup_Confidence
			<< &mTextArea_Hint
			<< &mTextArea_Progress;
		DeactivateUI(&mButton_Next);
		ActivateUI(&mButton_Input);
		ActivateUI(&mButton_Pause);
		DeactivateUI(&mButton_Resume);
		//DeactivateUI(&mButtonGroup_Answer);
		ActivateUI(&mButtonGroup_Answer);
		DeactivateUI(&mTextArea_ConfidenceHint);
		DeactivateUI(&mButtonGroup_Confidence);
		ActivateUI(&mTextArea_Hint);
		ActivateUI(&mTextArea_Progress);
		if (UI_CanCheck()) {
			mTaskUIs << &mButton_Check;
			DeactivateUI(&mButton_Check);
		}
		if (visInfo.GetVisTask() == FA){
			mTaskUIs << &mTextArea_AnswerHint0
				<< &mTextArea_AnswerHint1;
			ActivateUI(&mTextArea_AnswerHint0);
			ActivateUI(&mTextArea_AnswerHint1);
		}
		mTextArea_Hint.SetText(visInfo.GetTaskHintString());
		mTaskUIs << &mTextArea_Answer;
		ActivateUI(&mTextArea_Answer);
		mTextArea_Answer.ClearText();
	}
	mTextArea_Progress.SetText(mTrialManager.GetProgresInfoString());
	mbPaused = false;
	mAnswerInput = -1;
	mAnswerSelected = UI_INVALID_ANSWER;
	mConfidenceSelected = UI_INVALID_ANSWER;
	mbDrawHighlighted = false;
	if (visInfo.GetVisTask() == FA_VALUE && !visInfo.IsEmpty()) {
		mButton_Input.Enable();
		mTaskUIs << &mNumberArea_Input
			<< &mNumberArea_InputHint;
		DeactivateUI(&mNumberArea_Input);
		DeactivateUI(&mNumberArea_InputHint);
		mNumberArea_Input.ClearText();
		mNumberArea_InputHint.SetText("What is the average FA value of the tracts in  box 1?");
		mNumberArea_InputHint.SetTextColor(MyColor4f::black());
	}
	else mButton_Input.Disable();
}

void MyVisRankingApp::UIResize(int w, int h){
	MyUI::SetViewport(MyVec4i(0, 0, w, h));
	float boarder = 15;
	float xPos = w - mButtonWidth - boarder;
	float yOffset = mButtonIntervalY + mButtonHeight;
	float xOffset = mButtonWidth;
	float yPos = boarder;
	MyVec2f gSize;
	mButton_Input.SetPosition(MyVec2f(xPos, yPos + yOffset));
	mButton_Check.SetPosition(MyVec2f(xPos, yPos + yOffset));
	mButton_Next.SetPosition(MyVec2f(xPos, yPos));
	mTextArea_Answer.SetPosition(MyVec2f(xPos, yPos + 9 * yOffset));
	mButton_Pause.SetPosition(MyVec2f(xPos, h - yPos - yOffset));
	mButton_Resume.SetPosition(MyVec2f(xPos, h - yPos - yOffset));
	mTextArea_Hint.SetPosition(MyVec2f(w / 2, boarder));
	mTextArea_Transition.SetPosition(MyVec2f(w / 2, h / 2));
	mTextArea_Progress.SetPosition(MyVec2f(boarder + mButtonWidth / 2, h - boarder - mButtonHeight));
	mTextArea_AnswerHint0.SetPosition(MyVec2f(xPos - xOffset / 2, yPos + yOffset * 8.5 - 6));
	mTextArea_AnswerHint1.SetPosition(MyVec2f(xPos - xOffset / 2, yPos + yOffset * 2.5 - 6));
	mButtonGroup_Answer.SetPosition(MyVec2f(xPos, yPos + yOffset * 2));
	mTextArea_ConfidenceHint.SetPosition(MyVec2f(w / 2, h / 2 - mButtonHeight * 1));
	mButtonGroup_Confidence.SetPosition(MyVec2f(w / 2, h / 2 - mButtonHeight * 2 - mButtonIntervalY));
	mButtonGroup_Answer.UpdateLayout();
	mButtonGroup_Confidence.UpdateLayout();
	mNumberArea_InputHint.SetPosition(MyVec2f(w / 2, h / 2 + mButtonHeight * 2 + mButtonIntervalY));
	mNumberArea_Input.SetPosition(MyVec2f(w / 2, h / 2 + mButtonHeight * 1));
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
	float ambient, diffuse;
	const MyArray<MyColor4f>* perTractColor;
	//MyColor4f baseSphereColor;
	baseColor = tractVis->GetBaseColor();
	tractShow = tractVis->GetTractsShown();
	ambient = tractVis->GetAmbient();
	diffuse = tractVis->GetDiffuse();
	influences = tractVis->GetInfluences();
	perTractColor = tractVis->GetPerTractColor();
	//if(sg) baseSphereColor = sg->GetColor();
	tractVis->SetTractsShown(tractIndices);
	tractVis->SetBaseColor(color);
	tractVis->SetAmbient(1);
	tractVis->SetDiffuse(0);
	tractVis->ClearInfluences();
	tractVis->SetPerTractColor(NULL);
	tractVis->SetSphereGeometry(NULL);
	//if (sg) sg->SetColor(color);
	tractVis->Show();
	tractVis->SetBaseColor(baseColor);
	tractVis->SetTractsShown(tractShow);
	tractVis->SetAmbient(ambient);
	tractVis->SetDiffuse(diffuse);
	tractVis->SetInfluences(influences);
	tractVis->SetPerTractColor(perTractColor);
	tractVis->SetSphereGeometry(sg);
	//if (sg) sg->SetColor(baseSphereColor);
	glPopAttrib();
}

void MyVisRankingApp::DrawHighlightedTracts(MyTractVisBase* tractVis){
	if (tractVis){
		MyVisData* visData = mTrialManager.GetCurrentVisData();
		DrawHighlightedTracts(tractVis, visData->GetSelectIndices(), MyColor4f::yellow());
		if (visData->GetSpheres().size() > 0){
			const MySphere& sphere = visData->GetSphere(0);
			CollisionStatus cs = visData->ComputeSphereCollusionStatus(sphere);
			//MyColor4f color = GetCollisionStatusColor(cs);
			MyColor4f color = MyColor4f::yellow();
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
	if (IsOnBoxView()){ return; };
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
	//MyGraphicsTool::Color(MyColor4f::yellow());
	MyGraphicsTool::Color(MyColor4f::red());
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

void MyVisRankingApp::DrawColorLegend(){
	const MyVisInfo& visInfo = mTrialManager.GetCurrentVisData()->GetVisInfo();
	if ((visInfo.GetVisTask() != FA && visInfo.GetVisTask() != FA_VALUE)
		|| visInfo.IsEmpty() || mbPaused) return;
	unsigned int texture;
	if (visInfo.GetEncoding() == COLOR) texture = mVisTract.GetColorTextures()[visInfo.GetMappingMethod()];
	else if (visInfo.GetEncoding() == VALUE) texture = mVisTract.GetValueTextures()[visInfo.GetMappingMethod()];
	else return;
	MyVec4i viewport = MyGraphicsTool::GetViewport();

	MyGraphicsTool::PushAllAttributes();
	MyGraphicsTool::PushProjectionMatrix();
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::LoadProjectionMatrix(
		//&MyMatrixf::OrthographicMatrix(0, 1, 0, 1, 0, 1));
		&MyMatrixf::OrthographicMatrix(0, 1, 0, 1, 0, 1));
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glColor3f(1, 1, 1);
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0, 1);
	glVertex2f(LEGEND_LEFT, LEGEND_BOTTOM);
	glTexCoord2f(0, 0);
	glVertex2f(LEGEND_LEFT + LEGEND_WIDTH, LEGEND_BOTTOM);
	glTexCoord2f(1, 0);
	glVertex2f(LEGEND_LEFT + LEGEND_WIDTH, LEGEND_BOTTOM + LEGEND_HEIGHT);
	glTexCoord2f(1, 1);
	glVertex2f(LEGEND_LEFT, LEGEND_BOTTOM + LEGEND_HEIGHT);
	glEnd();

	/*
	glLineWidth(2 * mCanvasScaleX);
	glColor3f(0, 0, 0);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINE_LOOP);
	glVertex2f(LEGEND_LEFT, LEGEND_BOTTOM);
	glVertex2f(LEGEND_LEFT + LEGEND_WIDTH, LEGEND_BOTTOM);
	glVertex2f(LEGEND_LEFT + LEGEND_WIDTH, LEGEND_BOTTOM + LEGEND_HEIGHT);
	glVertex2f(LEGEND_LEFT, LEGEND_BOTTOM + LEGEND_HEIGHT);
	glEnd();
	*/

	glColor3f(0, 0, 0);
	for (int i = 0; i <= 8; i++){
		glLineWidth(1 * mCanvasScaleX);
		glBegin(GL_LINES);
		glVertex2f(LEGEND_LEFT + LEGEND_WIDTH - LEGEND_TICKSIZE, LEGEND_BOTTOM + i / 8.f*LEGEND_HEIGHT);
		glVertex2f(LEGEND_LEFT + LEGEND_WIDTH, LEGEND_BOTTOM + i / 8.f*LEGEND_HEIGHT);
		glEnd();
		MyString numStr((i + 2) / 10.f);
		glLineWidth(2 * mCanvasScaleX);
		MyPrimitiveDrawer::DrawBitMapTextLarge(MyVec3f(LEGEND_LEFT + LEGEND_WIDTH + LEGEND_TICKSIZE,
			LEGEND_BOTTOM + i / 8.f*LEGEND_HEIGHT - MyPrimitiveDrawer::GetBitmapHeight('0') / (2.f*viewport[3]), 0),
			numStr, 0);
	}
	MyGraphicsTool::PopMatrix();
	MyGraphicsTool::PopProjectionMatrix();
	MyGraphicsTool::PopAttributes();
}

void MyVisRankingApp::DrawTextureRatioLegend(){
	const MyVisInfo& visInfo = mTrialManager.GetCurrentVisData()->GetVisInfo();
	if ((visInfo.GetVisTask() != FA && visInfo.GetVisTask() != FA_VALUE)
		|| visInfo.IsEmpty() || mbPaused) return;
	if (visInfo.GetEncoding() != TEXTURE) return;
	MyVec4i viewport = MyGraphicsTool::GetViewport();
	MyGraphicsTool::PushAllAttributes();
	MyGraphicsTool::PushProjectionMatrix();
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::LoadProjectionMatrix(
		//&MyMatrixf::OrthographicMatrix(0, 1, 0, 1, 0, 1));
		&MyMatrixf::OrthographicMatrix(0, 1, 0, 1, 0, 1));
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor3f(0.8, 0.8, 0.8);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(LEGEND_LEFT, LEGEND_BOTTOM);
	glVertex2f(LEGEND_LEFT + LEGEND_WIDTH, LEGEND_BOTTOM);
	glVertex2f(LEGEND_LEFT + LEGEND_WIDTH, LEGEND_BOTTOM + LEGEND_HEIGHT);
	glVertex2f(LEGEND_LEFT, LEGEND_BOTTOM + LEGEND_HEIGHT);
	glEnd();

	float unitHeight = LEGEND_HEIGHT / 9;
	glColor3f(0, 0, 0);
	glLineWidth(1 * mCanvasScaleX);
	for (int i = 0; i <= 8; i++){
		float value = i / 10.f + 0.2;
		float thisBase = LEGEND_BOTTOM + i * unitHeight;
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(LEGEND_LEFT, thisBase);
		glVertex2f(LEGEND_LEFT + LEGEND_WIDTH, thisBase);
		glVertex2f(LEGEND_LEFT + LEGEND_WIDTH, thisBase + value*unitHeight);
		glVertex2f(LEGEND_LEFT, thisBase + value*unitHeight);
		glEnd();
		MyString numStr((i + 2) / 10.f);
		glLineWidth(2 * mCanvasScaleX);
		MyPrimitiveDrawer::DrawBitMapTextLarge(MyVec3f(LEGEND_LEFT + LEGEND_WIDTH + LEGEND_TICKSIZE,
			thisBase + unitHeight / 2 - MyPrimitiveDrawer::GetBitmapHeight('0') / (2.f*viewport[3]), 0),
			numStr, 0);
	}
	MyGraphicsTool::PopMatrix();
	MyGraphicsTool::PopProjectionMatrix();
	MyGraphicsTool::PopAttributes();
}

void MyVisRankingApp::DrawTractLegend(){
	const MyVisInfo& visInfo = mTrialManager.GetCurrentVisData()->GetVisInfo();
	if ((visInfo.GetVisTask() != FA && visInfo.GetVisTask() != FA_VALUE)
		|| visInfo.IsEmpty() || mbPaused) return;
	mTractLegend.ClearInfluences();
	if (visInfo.GetEncoding() == ESIZE){
		mTractLegend.SetValueToSizeInfluence(2 * mTractLegend.mTrackRadius);
	}
	//else if (visInfo.GetEncoding() == TEXTURE){
	//	mTractLegend.SetValueToTextureRatioInfluence(14);
	//}
	else return;

	MyGraphicsTool::PushAllAttributes();
	MyGraphicsTool::PushProjectionMatrix();
	MyGraphicsTool::PushMatrix();
	double mvMat[16], projMat[16];
	int viewport[4];
	double wx, wy, wz;
	double ox, oy, oz;
	glGetDoublev(GL_MODELVIEW_MATRIX, mvMat);
	glGetDoublev(GL_PROJECTION_MATRIX, projMat);
	glGetIntegerv(GL_VIEWPORT, viewport);
	gluProject(0, 0, 0, mvMat, projMat, viewport, &ox, &oy, &oz);
	gluUnProject((LEGEND_LEFT + LEGEND_WIDTH / 2)*viewport[2], viewport[3]/2, oz,
		mvMat, projMat, viewport, &wx, &wy, &wz);
	glTranslatef(wx, 0, 0);

	gluUnProject(viewport[2] / 2, viewport[3] / 2 + 2, oz, mvMat, projMat, viewport, &wx, &wy, &wz);

	MyGraphicsTool::MultiplyMatrix(&mTrackBall.GetScaleMatrix());
	//MyGraphicsTool::LoadTrackBall(&mTrackBall);

	mTractLegend.Show();

	MyGraphicsTool::PopMatrix();
	MyGraphicsTool::PopProjectionMatrix();
	MyGraphicsTool::PopAttributes();
}

void MyVisRankingApp::DrawTractLegendText(){
	const MyVisInfo& visInfo = mTrialManager.GetCurrentVisData()->GetVisInfo();
	if ((visInfo.GetVisTask() != FA && visInfo.GetVisTask() != FA_VALUE)
		|| visInfo.IsEmpty() || mbPaused) return;
	mTractLegend.ClearInfluences();
	if (visInfo.GetEncoding() == ESIZE){
		MyGraphicsTool::PushAllAttributes();
		MyGraphicsTool::PushProjectionMatrix();
		MyGraphicsTool::PushMatrix();
		double mvMat[16], projMat[16];
		int viewport[4];
		double wx, wy, wz;
		double ox, oy, oz;
		double x, y, z;
		glGetDoublev(GL_MODELVIEW_MATRIX, mvMat);
		glGetDoublev(GL_PROJECTION_MATRIX, projMat);
		glGetIntegerv(GL_VIEWPORT, viewport);
		float h = MyPrimitiveDrawer::GetBitmapHeight('0');
		gluProject(0, 0, 0, mvMat, projMat, viewport, &ox, &oy, &oz);
		gluUnProject((LEGEND_LEFT + LEGEND_WIDTH / 2)*viewport[2], viewport[3] / 2, oz,
			mvMat, projMat, viewport, &wx, &wy, &wz);
		glTranslatef(wx, 0, 0);
		MyGraphicsTool::MultiplyMatrix(&mTrackBall.GetScaleMatrix());
		glGetDoublev(GL_MODELVIEW_MATRIX, mvMat);
		glDisable(GL_DEPTH_TEST);
		MyVec3f offset(1, 0, 0);
		glLineWidth(2);
		glColor3f(0, 0, 0);
		for (int i = 0; i < mTractLegend.GetNumSamples(); i++){
			glBegin(GL_LINES);
			MyVec3f p = mTractLegend.GetBase() + i*mTractLegend.GetDir();
			MyGraphicsTool::Vertex(p);
			MyGraphicsTool::Vertex(p + offset);
			glEnd();
			MyVec3f tp = p + 1.5* offset;
			MyVec3d tpd;
			gluProject(tp[0], tp[1], tp[2], mvMat, projMat, viewport, &x, &y, &z);
			y -= h / 2;
			gluUnProject(x, y, z, mvMat, projMat, viewport, &tpd[0], &tpd[1], &tpd[2]);

			MyPrimitiveDrawer::DrawBitMapTextLarge(tpd.toType<float>(),
				MyString(i / float(mTractLegend.GetNumSamples() - 1)*0.8 + 0.2));
		}
		MyGraphicsTool::PopMatrix();
		MyGraphicsTool::PopProjectionMatrix();
		MyGraphicsTool::PopAttributes();
	};
}
