#include "MyVisRankingApp.h"
#include "MyBitmap.h"
#include "MyTexture.h"
#include "MyPrimitiveDrawer.h"
#include "MyConstants.h"
#include "MyBlackBodyColor.h"
#include "MyVisEnum.h"
#include "MyColorLegend.h"
#include "MyConstants.h"
#include "MyColorConverter.h"
#include "MySuperquadric.h"
#include "MyColorTextureMaker.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>

using namespace std;
using namespace MyConstants;
using namespace MyVisEnum;

#define BOXSCALING 6.f

MyVisRankingApp::MyVisRankingApp()
{

	mCanvasScaleX = mCanvasScaleY = 2;
	mLineThickness = mCanvasScaleX * 3.f;
	mTargetBrightness = 4.0f;

	mAppMode = APP_MODE_STUDY;
	mCurrentBoxIndex = -1;
	mbDrawTracts = true;
	mbDrawIndicators = true;
	mbDrawHighlighted = false;
	mbDrawLegend = true;
	mbDrawUI = true;
	mbComputeBrightness = false;
	mbLightnessBalance = false;
	mbComputeTotalAlpha = false;
	mbComputeTotalPixelDrawn = false;
	mDisplayEndToNext = false;
	UIInit();

}


MyVisRankingApp::~MyVisRankingApp()
{
	UIDestory();
}

void MyVisRankingApp::Init(int uidx, int tidx, int mode){
	mAppMode = mode;
	mTrialManager.SetUserIndex(uidx);
	mTrialManager.SetDataIndex(tidx);
	mLogs.SetUserIndex(uidx);
	mEventLog.SetUserIndex(uidx);
	mRenderingLog.SetUserIndex(uidx);
	//mAppMode = APP_MODE_STUDY | APP_MODE_DEBUG;
	//mAppMode = APP_MODE_OCCLUSION;
	//mAppMode = APP_MODE_LIGHTING | APP_MODE_DEBUG;
	MyRenderingLog::InitEyePositions(100, 20, 5);
	MyRenderingLog::SetLogDirectory("logs\\");
	MyLogTable::SetLogDirectory("logs\\");
	MyEventLog::SetLogDirectory("logs\\");
	// s3 does not need reverse z but s4 needs
	// but data selected are for reversed...
	// so we reverse s3 and keep s4...
	// also remember to reverse rp[2] is MySuperquadric.cpp


	//mTracts.Read("C:\\Users\\GuohaoZhang\\Dropbox\\data\\normal_s2.tensorinfo");
	//mTracts.AppendTrackColor("C:\\Users\\GuohaoZhang\\Dropbox\\data\\normal_s2_boy.data");
	//mTracts.Save("C:\\Users\\GuohaoZhang\\Dropbox\\data\\normal_s2_tensorboy.trk");
	//exit(0);

	if (IsOnMode(APP_MODE_TRAINING)){
		mTracts.Read("C:\\Users\\GuohaoZhang\\Dropbox\\data\\normal_s4_tensorboy.trk");
		MySuperquadric::ZREV = 1;
	}
	else {
		//mTracts.Read("C:\\Users\\GuohaoZhang\\Dropbox\\data\\normal_s3_tensorboy_RevZ.trk");
		mTracts.Read("C:\\Users\\GuohaoZhang\\Dropbox\\data\\normal_s2_tensorboy_RevZ.trk");
		MySuperquadric::ZREV = -1;


	}
	// generate trials
	mTrialManager.SetDataRootDir("C:\\Users\\GuohaoZhang\\Dropbox\\data\\traces");
	mTrialManager.SetTracts(&mTracts);
	if (IsOnMode(APP_MODE_TRAINING))
		mTrialManager.GenerateVisInfo_Training();
	else if (IsOnMode(APP_MODE_STUDY))
		mTrialManager.GenerateVisInfo_Experiment_Random();
	else if (IsOnMode(APP_MODE_LIGHTING))
		mTrialManager.GenerateVisInfo_LightingProfile();
	else if (IsOnMode(APP_MODE_OCCLUSION))
		mTrialManager.GenerateVisInfo_OcclusionProfile();
	if (IsOnMode(APP_MODE_PRINTDATA)){
		mTrialManager.PrintAllCase("dataTable_" + MyString(uidx) + ".txt");
	}

	//MyVisTract::UseNormalizedLighting(!IsOnMode(APP_MODE_LIGHTING));
	MyVisTract::UseNormalizedLighting(false);
	//mTrialManager.SetDataIndex(4);
	// load textures
	MyBitmap bitmap;
	MyColorLegend legends[2];
	legends[0].SetColorsFromData(&MyConstants::BlackBodyExtended[0][0], 3, 1024, 1);
	bitmap.Open("..\\SSAO\\data\\isoluminant.bmp");
	legends[1].SetColorsFromData(bitmap.GetPixelBufferRGB(), 3, bitmap.GetWidth(), bitmap.GetHeight());
	MyArray<MyColor4f> colors = legends[1].GetColors();
	for (int i = 0; i < colors.size(); i++){
		MyColorConverter::Lab lab = MyColorConverter::rgb2lab(colors[i]);
		lab.l = 50;
		colors[i] = MyColorConverter::lab2rgb(lab);
	}
	legends[1].SetColors(colors);
	legends[0].CutFromCenterByArcLength(160.f);
	legends[1].CutFromCenterByArcLength(160.f);
	MyTexture::SetInterpolateMethod(GL_NEAREST);
	unsigned int valueTexture = MyTexture::MakeGLTexture(legends[0].GetColors(), legends[0].GetColors().size(), 1);
	unsigned int colorTexture = MyTexture::MakeGLTexture(legends[1].GetColors(), legends[1].GetColors().size(), 1);
	/*
	// get more color maps
	MyArrayui colorTextures;
	MyTexture::SetInterpolateMethod(GL_NEAREST);
	colorTextures << MyTexture::MakeGLTexture(&MyConstants::SpiralColorHenan[0][0], 1000, 1);
	for (int i = 0; i < 14; i++){
		MyString colorFileName = "scale_s" + MyString(i) + ".bmp";
		bitmap.Open("..\\RankVis\\colors\\bmp_files\\" + colorFileName);
		colorTextures << MyTexture::MakeGLTexture(&bitmap);
	}
	mVisTract.SetColorTextures({ colorTextures });
	*/
	//mVisTract.SetColorTextures({ colorTexture });
	mVisTract.SetColorTextures(MyColorTextureMaker::MakeColorTextures());
	mVisTract.SetValueTextures({ valueTexture });
	MyTexture::SetInterpolateMethod(GL_LINEAR);

	UIInit();
	mLogs.SetEnabled(IsOnMode(APP_MODE_TRAINING) || IsOnMode(APP_MODE_STUDY));
	mLogs.StartLog(IsOnMode(APP_MODE_TRAINING) ? "training" : "formal");
	mEventLog.SetEnabled(IsOnMode(APP_MODE_TRAINING) || IsOnMode(APP_MODE_STUDY));
	mEventLog.StartLog(IsOnMode(APP_MODE_TRAINING) ? "trainingEvent" : "formalEvent");
	mRenderingLog.SetEnabled(IsOnMode(APP_MODE_LIGHTING) || IsOnMode(APP_MODE_OCCLUSION));
	if (IsOnMode(APP_MODE_LIGHTING)){
		mRenderingLog.SetRenderingValueNames({ "ALLVALUE", "OBJVALUE" });
		mRenderingLog.StartLog("Lighting");
	}
	else if (IsOnMode(APP_MODE_OCCLUSION)){
		mRenderingLog.SetRenderingValueNames({ "NUMPIXL", "PIXLFRGS" });
		mRenderingLog.StartLog("Occlusion_"+MyString(MyTractVisBase::DefaultTrackRadius));
	}

	if (!IsOnMode(APP_MODE_DEBUG)){
		mTrackBall.SetScaleRange(pow(1.05f, -5), pow(1.05f, 20));
	}

	mTractLegend.LoadShader();
	mTractLegend.ComputeGeometry();
	mTractLegend.LoadGeometry();
}

bool MyVisRankingApp::IsOnMode(APP_MODE mode) const { 
	int mm = mode & APP_MODE_MODE_MASK;
	int mf = mode & APP_MODE_FLAG_MASK;
	int xmm = mAppMode & APP_MODE_MODE_MASK;
	int xmf = mAppMode & APP_MODE_FLAG_MASK;
	return mm == xmm || mf & xmf;
}

void MyVisRankingApp::Next(){
	mLogs.EndTrial();
	mRenderingLog.EndTrial();
	mEventLog.LogItem("TrialEnd");
	if (!mTrialManager.IsLast()){
		MyVisData* visData = mTrialManager.GotoNextVisData();
		mVisTract.SetVisData(visData);
		mVisTract.Update();
		UIUpdate();
		if (!IsOnMode(APP_MODE_DEBUG)) ResetCamera();
		else PrintTrialInfo();

		mLogs.SetVisData(visData);
		mLogs.SetTrialIndex(mTrialManager.GetCurrentVisDataIndex());
		mRenderingLog.SetVisData(visData);
		mRenderingLog.SetTrialIndex(mTrialManager.GetCurrentVisDataIndex());
		mEventLog.LogItem("TrialStart\t" + MyString(mTrialManager.GetCurrentVisDataIndex()));
	}
	else {
		if (IsOnMode(APP_MODE_DEBUG) && !IsOnMode(APP_MODE_LIGHTING)) cerr << "End of trial." << endl;
		else exit(1);
	}
}

void MyVisRankingApp::Previous(){
	mLogs.EndTrial();
	mRenderingLog.EndTrial();
	if (!mTrialManager.IsFirst()){
		MyVisData* visData = mTrialManager.GotoPreviousVisData();
		mVisTract.SetVisData(visData);
		mVisTract.Update();
		UIUpdate();
		if (!IsOnMode(APP_MODE_DEBUG)) ResetCamera();
		else PrintTrialInfo();

		mLogs.SetVisData(visData);
		mLogs.SetTrialIndex(mTrialManager.GetCurrentVisDataIndex());
		mRenderingLog.SetVisData(visData);
		mRenderingLog.SetTrialIndex(mTrialManager.GetCurrentVisDataIndex());
		mEventLog.LogItem("TrialStart\t" + MyString(mTrialManager.GetCurrentVisDataIndex()));
	}
	else {
		if (IsOnMode(APP_MODE_DEBUG)) cerr << "First of trial." << endl;
		//exit(1);
	}
}

void MyVisRankingApp::PrintTrialInfo(){
	const MyVisData* visData = mTrialManager.GetCurrentVisData();
	const MyVisInfo& visInfo = visData->GetVisInfo();
	int index = mTrialManager.GetCurrentVisDataIndex();
	VisTask task = visInfo.GetVisTask();
	if (task == MyVisEnum::FA){
		cout << index << ": "
			<< toString(visInfo.GetCover()) << ", "
			<< toString(visInfo.GetBundle()) << ", "
			<< toString(visInfo.GetShape()) << ", "
			<< toString(visInfo.GetEncoding()) << ", "
			<< visInfo.GetQuest() << ", ";
		if (visData->GetCorrectAnswers().size()>0)
				cout << visData->GetCorrectAnswers()[0] << ":"
				<< visData->GetCorrectAnswerString();
		cout << endl;
	}
	else if (task == MyVisEnum::TRACE){
		cout << index << ": "
			<< toString(visInfo.GetCover()) << ", "
			<< toString(visInfo.GetBundle()) << ", "
			<< toString(visInfo.GetShape()) << ", "
			<< toString(visInfo.GetEncoding()) << ", "
			<< visInfo.GetQuest() << ", ";
		if (visData->GetCorrectAnswers().size()>0)
			cout << visData->GetCorrectAnswers()[0] << ":"
			<< visData->GetCorrectAnswerString();
		cout << endl;
	}
	else if (task == MyVisEnum::TUMOR){
		CollisionStatus cs = (CollisionStatus)(visInfo.GetQuest() % 3 + 1);
		cout << index << ": "
			<< toString(visInfo.GetBundle()) << ", "
			<< toString(visInfo.GetShape()) << ", "
			<< toString(visInfo.GetVisCue()) << ", "
			<< toString(cs) << ", ";
		if (visData->GetCorrectAnswers().size()>0)
			cout << visData->GetCorrectAnswers()[0] << ":"
			<< visData->GetCorrectAnswerString();
		cout << endl;
	}
}


bool MyVisRankingApp::IsOnBoxView() const{
	if (mTrialManager.GetCurrentVisData()->GetVisInfo().GetVisTask() == FA_VALUE){
		return !mVisTract.GetIgnoreBoxVis();
	}
	return false;
}


void MyVisRankingApp::Show(){
	glViewport(0, 0, mCanvasWidth, mCanvasHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer.GetFrameBuffer());
	if (IsOnMode(APP_MODE_OCCLUSION)) glClearColor(0, 0, 0, 0);
	else glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (mVisTract.GetTractVis() != NULL && !mbPaused){
		glPushMatrix();
		MyGraphicsTool::LoadTrackBall(&mTrackBall);
		MyBoundingBox box = mTrialManager.GetCurrentVisData()->GetBoundingBox();
		if (IsOnBoxView()){
			box = mTrialManager.GetCurrentVisData()->GetBoxes()[0];
			glScalef(BOXSCALING, BOXSCALING, BOXSCALING);
		}
		MyGraphicsTool::Translate(-box.GetCenter());
		glLineWidth(mLineThickness);
		if (mbDrawTracts) mVisTract.Show();
		if (mbDrawHighlighted) DrawHighlighted();
		if (mbDrawIndicators) {
			DrawBoxes();
			DrawTractIndicators();
		}
		if (IsOnBoxView()){
			glScalef(1 / BOXSCALING, 1 / BOXSCALING, 1 / BOXSCALING);
		}
		glPopMatrix();
		if (mbDrawLegend) {
			glClear(GL_DEPTH_BUFFER_BIT);
			DrawTractLegend();
		}
	}

	BrightnessBalance();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (IsOnMode(APP_MODE_OCCLUSION)) glClearColor(0, 0, 0, 0);
	else glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, mWindowWidth, mWindowHeight);
	MyPrimitiveDrawer::DrawTextureOnViewport(mFrameBuffer.GetColorTexture());
	if (mbDrawUI) UIDraw();
	if (mbDrawLegend) {
		DrawColorLegend();
		DrawTextureRatioLegend();
		DrawTractLegendText();
	}
}