#include "MyVisRankingApp.h"
#include "MyBitmap.h"
#include "MyTexture.h"
#include "MyPrimitiveDrawer.h"
#include "MyConstants.h"
#include "MyBlackBodyColor.h"
#include "MyVisEnum.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>

using namespace std;
using namespace MyConstants;
using namespace MyVisEnum;


MyVisRankingApp::MyVisRankingApp()
{

	mCanvasScaleX = mCanvasScaleY = 2;
	mLineThickness = mCanvasScaleX * 4.f;
	mTargetBrightness = 0.4;

	mAppMode = APP_MODE_STUDY;
	mCurrentBoxIndex = -1;
	mbDrawTracts = true;
	mbDrawIndicators = true;
	mbDrawHighlighted = false;
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
	//mAppMode = APP_MODE_STUDY | APP_MODE_DEBUG;
	//mAppMode = APP_MODE_OCCLUSION;
	//mAppMode = APP_MODE_LIGHTING;
	MyRenderingLog::InitEyePositions(100, 20, 5);
	MyRenderingLog::SetLogDirectory("logs\\");
	MyLogTable::SetLogDirectory("logs\\");
	if (IsOnMode(APP_MODE_TRAINING))
		mTracts.Read("C:\\Users\\GuohaoZhang\\Dropbox\\data\\normal_s4_tensorboy_RevZ.trk");
	else mTracts.Read("C:\\Users\\GuohaoZhang\\Dropbox\\data\\normal_s3_tensorboy_RevZ.trk");
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
	mTrialManager.SetDataIndex(tidx);
	// load textures
	MyBitmap bitmap;
	bitmap.Open("..\\SSAO\\data\\diverging.bmp");
	//mColorTexture = MyTexture::MakeGLTexture(&bitmap);
	mColorTexture = MyBlackBodyColor::GetTexture();
	//mColorTexture = MyTexture::MakeGLTexture((float*)IsoluminanceMap, 33, 1);
	//MyBitmap bitmap2;
	//bitmap2.Open("..\\SSAO\\data\\diverging.bmp");
	//mSaturationTexture = MyTexture::MakeGLTexture(&bitmap2);
	MyArrayf hsv;
	for (int i = 0; i < 100; i++){
		MyColor4f c;
		c.fromHSV(10, i / 99.f, 1.f);
		hsv << c.r << c.g << c.b;
	}
	mSaturationTexture = MyTexture::MakeGLTexture(&hsv[0], 100, 1);
	// tract vis setup
	mVisTract.SetColorTextures({ mColorTexture });
	mVisTract.SetValueTextures({ mSaturationTexture });

	UIInit();
	mLogs.SetEnabled(IsOnMode(APP_MODE_TRAINING) || IsOnMode(APP_MODE_STUDY));
	mLogs.SetUserIndex(uidx);
	mLogs.StartLog(IsOnMode(APP_MODE_TRAINING) ? "training" : "log");
	mRenderingLog.SetEnabled(IsOnMode(APP_MODE_LIGHTING) || IsOnMode(APP_MODE_OCCLUSION));
	mRenderingLog.SetUserIndex(uidx);
	if (IsOnMode(APP_MODE_LIGHTING)){
		mRenderingLog.SetRenderingValueNames({ "ALLVALUE", "OBJVALUE" });
		mRenderingLog.StartLog("Lighting");
	}
	else if (IsOnMode(APP_MODE_OCCLUSION)){
		mRenderingLog.SetRenderingValueNames({ "NUMPIXL", "PIXLFRGS" });
		mRenderingLog.StartLog("Occlusion");
	}
	mTrialManager.SetUserIndex(uidx);

	mTrackBall.SetScaleRange(pow(1.05f, -5), pow(1.05f, 20));
	//mTrackBall.SetScaleRange(pow(1.05f, -5), pow(1.05f, 50));
}

void MyVisRankingApp::Next(){
	mLogs.EndTrial();
	mRenderingLog.EndTrial();
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
	}
	else {
		if (IsOnMode(APP_MODE_DEBUG)) cerr << "End of trial." << endl;
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
	}
	else {
		if (IsOnMode(APP_MODE_DEBUG)) cerr << "First of trial." << endl;
		//exit(1);
	}
}

void MyVisRankingApp::PrintTrialInfo(){
	const MyVisInfo& visInfo = mTrialManager.GetCurrentVisData()->GetVisInfo();
	int index = mTrialManager.GetCurrentVisDataIndex();
	VisTask task = visInfo.GetVisTask();
	if (task == MyVisEnum::FA){
		cout << index << ": "
			<< toString(visInfo.GetCover()) << ", "
			<< toString(visInfo.GetBundle()) << ", "
			<< toString(visInfo.GetShape()) << ", "
			<< toString(visInfo.GetEncoding()) << ", "
			<< visInfo.GetQuest() << ", "
			<< endl;
	}
	else if (task == MyVisEnum::TRACE){
		cout << index << ": "
			<< toString(visInfo.GetCover()) << ", "
			<< toString(visInfo.GetBundle()) << ", "
			<< toString(visInfo.GetShape()) << ", "
			<< toString(visInfo.GetEncoding()) << ", "
			<< visInfo.GetQuest() << ", "
			<< endl;
	}
	else if (task == MyVisEnum::TUMOR){
		CollisionStatus cs = (CollisionStatus)(visInfo.GetQuest() % 3 + 1);
		cout << index << ": "
			<< toString(visInfo.GetBundle()) << ", "
			<< toString(visInfo.GetShape()) << ", "
			<< toString(visInfo.GetVisCue()) << ", "
			<< toString(cs) << ", "
			<< endl;
	}
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
		MyGraphicsTool::Translate(-box.GetCenter());
		glLineWidth(mLineThickness);
		if (mbDrawTracts) mVisTract.Show();
		if (mbDrawHighlighted) DrawHighlighted();
		if (mbDrawIndicators) {
			DrawBoxes();
			DrawTractIndicators();
		}
		glPopMatrix();
	}

	BrightnessBalance();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (IsOnMode(APP_MODE_OCCLUSION)) glClearColor(0, 0, 0, 0);
	else glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, mWindowWidth, mWindowHeight);
	MyPrimitiveDrawer::DrawTextureOnViewport(mFrameBuffer.GetColorTexture());
	if(mbDrawUI) UIDraw();
}