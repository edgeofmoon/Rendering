#include "MyVisRankingApp.h"
#include "MyBitmap.h"
#include "MyTexture.h"
#include "MyPrimitiveDrawer.h"
#include "MyColorConverter.h"
#include "MyColorTextureMaker.h"
#include "MyLineAO.h"
#include "MyTrackDDH.h"
#include "MyMathHelper.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <omp.h>
#include <iostream>
using namespace std;

void MyVisRankingApp::HandleGlutDisplay(){
	StartProfileRendering();
	Show();
	//ShowForLegendMaking();
	SaveInitScreenShots();
	glutSwapBuffers();
	EndProfileRendering();
	mLogs.StartTrial();
	if (mDisplayEndToNext) Next();
	mEventLog.LogItem( "Display" );
}

void MyVisRankingApp::HandleGlutKeyboard(unsigned char key, int x, int y){
	if (key == 'h' || key == 'H') ResetCamera();
	else if (UIProcessKey(key, x, y)) {}
	else if (HandleDebugKey(key)) {}
	else ProcessKey_DataGen(key);
	RequestRedisplay();
	mEventLog.LogItem(
		"Key" + MyEventLog::Decimer
		+ MyString(key) + MyEventLog::Decimer
		+ MyString(x) + MyEventLog::Decimer
		+ MyString(y));
}

void MyVisRankingApp::HandleGlutSpecialKeyboard(unsigned char key, int x, int y){
	UIProcessSpecialKey(key, x, y);
	RequestRedisplay();
	mEventLog.LogItem(
		"SpecialKey" + MyEventLog::Decimer
		+ MyString(key) + MyEventLog::Decimer
		+ MyString(x) + MyEventLog::Decimer
		+ MyString(y));
}

void MyVisRankingApp::HandleGlutMouseWheel(int button, int dir, int x, int y){
	if (dir > 0){
		if (!mbPaused){
			mTrackBall.ScaleMultiply(1.05);
			//mTrackBall.ScaleMultiply(sqrt(2));
			//glMatrixMode(GL_PROJECTION);
			//glScalef(sqrt(2), sqrt(2), 1);
			//glMatrixMode(GL_MODELVIEW);
		}
	}
	else{
		if (!mbPaused){
			mTrackBall.ScaleMultiply(1 / 1.05);
			//mTrackBall.ScaleMultiply(sqrt(0.5));
			//glMatrixMode(GL_PROJECTION);
			//glScalef(1 / sqrt(2), 1 / sqrt(2), 1);
			//glMatrixMode(GL_MODELVIEW);
		}
	}
	UpdateSampePerFragmentAtScale(mTrackBall.GetScale());

	RequestRedisplay();
	mEventLog.LogItem(
		"MouseWheel" + MyEventLog::Decimer
		+ MyString(button) + MyEventLog::Decimer
		+ MyString(dir) + MyEventLog::Decimer
		+ MyString(x) + MyEventLog::Decimer
		+ MyString(y));
}

void MyVisRankingApp::HandleGlutMouse(int button, int state, int x, int y){
	if(button == GLUT_LEFT_BUTTON){
		if (state == GLUT_DOWN){
			mTrackBall.StartRotation(x, y);
			UIProcessMouseDown(x, y);
		}
		else if (state == GLUT_UP){
			mTrackBall.EndRotation(x, y);
			UIProcessMouseUp(x, y);
		}
	}
	else if (IsOnMode(APP_MODE_DEBUG) && button == GLUT_RIGHT_BUTTON){
		mTrackBall.SetTranslateScale(0.1, 0.1);
		if (state == GLUT_DOWN){
			mTrackBall.StartTranslation(x, y);
			UIProcessMouseDown(x, y);
		}
		else if (state == GLUT_UP){
			mTrackBall.EndTranslation(x, y);
			UIProcessMouseUp(x, y);
		}
	}
	mEventLog.LogItem(
		"MouseButton" + MyEventLog::Decimer
		+ MyString(button) + MyEventLog::Decimer
		+ MyString(state) + MyEventLog::Decimer
		+ MyString(x) + MyEventLog::Decimer
		+ MyString(y));
	RequestRedisplay();
}

void MyVisRankingApp::HandleGlutMotion(int x, int y){
	if (!UIProcessMouseMove(x, y)){
		if (!mbPaused){
			mTrackBall.Motion(x, y);
		}
	}
	mEventLog.LogItem(
		"MouseMotion" + MyEventLog::Decimer
		+ MyString(x) + MyEventLog::Decimer
		+ MyString(y));
	RequestRedisplay();
}

void MyVisRankingApp::HandleGlutPassiveMotion(int x, int y){
	if (UIProcessMouseMove(x, y)){
		RequestRedisplay();
	}
	/*
	mEventLog.LogItem(
		"MousePassive" + MyEventLog::Decimer
		+ MyString(x) + MyEventLog::Decimer
		+ MyString(y));
		*/
}

void MyVisRankingApp::HandleGlutReshape(int x, int y){
	mWindowWidth = x;
	mWindowHeight = y;
	mCanvasWidth = x*mCanvasScaleX;
	mCanvasHeight = y*mCanvasScaleY;
	mTrackBall.Reshape(x, y);
	ResetCamera(false);
	ResizeRenderBuffer(mCanvasWidth, mCanvasHeight);
	UIResize(x, y);
	mVisTract.Resize(mCanvasWidth, mCanvasHeight);
	mEventLog.LogItem(
		"Reshape" + MyEventLog::Decimer
		+ MyString(x) + MyEventLog::Decimer
		+ MyString(y));
	RequestRedisplay();
}

void MyVisRankingApp::RequestRedisplay(){
	glutPostRedisplay();
}

void MyVisRankingApp::ResetCamera(bool resetTractBall){
	MyMatrixf projectionMatrix = MyMatrixf::PerspectiveMatrix(60, mWindowWidth / (float)mWindowHeight, 1, 300);
	//MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-100,100,-100,100, 1, 200);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	gluLookAt(0, 0, 150, 0, 0, 0, 0, 1, 0);
	if (resetTractBall){
		mTrackBall.ResetRotate();
		mTrackBall.ResetScale();
		mTrackBall.ResetTranslate();
	}
	if (mTrialManager.GetCurrentVisData()->GetVisInfo().IsTraining()){
		mTrackBall.SetRotationMatrix(MyMatrixf::RotateMatrix(180, 0, 1, 0));
	}
}

int MyVisRankingApp::HandleDebugKey(unsigned char key){
	if (!IsOnMode(APP_MODE_DEBUG)) return 0;

	MyTrackDDH* p = static_cast<MyTrackDDH*>(mVisTract.GetTractVis());

	switch (key){
		case 27:
			exit(0);
			break;
		case 'r':
		case 'R':
			mVisTract.GetTractVis()->LoadShader();
			break;
		case 'n':
		case 'N':
			if (!mLogs.NeedAtLeastOneFrame()) Next();
			break;
		case 'p':
		case 'P':
			if (!mLogs.NeedAtLeastOneFrame()) Previous();
			break;
		case 'k':
		case 'K':
			mbDrawHighlighted = !mbDrawHighlighted;
			break;
		case 'l':
		case 'L':
			mVisTract.SetIgnoreBoxVis(!mVisTract.GetIgnoreBoxVis());
			mVisTract.Update();
			break;
		case 'b':
		case 'B':
			mbLightnessBalance = !mbLightnessBalance;
			cout << "Lightness balance: " << (mbLightnessBalance ? "ON" : "OFF") << endl;
			break;
		case ']':
			mTargetBrightness += 1.0;
			cout << "Brightness: " << mBrightness << endl;
			if (p) {
				p->mStripDepth += 0.02;
				cout << "StripDepth: " << p->mStripDepth << endl;
			}
			break;
		case '[':
			mTargetBrightness -= 1.0;
			cout << "Brightness: " << mBrightness << endl;
			if (p) {
				p->mStripDepth -= 0.02;
				cout << "StripDepth: " << p->mStripDepth << endl;
			}
			break;
		case '0':
			mLineThickness += 0.5;
			cout << "mLineThickness: " << mLineThickness << endl;
			break;
		case '9':
			mLineThickness -= 0.5;
			cout << "mLineThickness: " << mLineThickness << endl;
			break;
		case 't':
			if (mVisTract.GetTractVis()) {
				mVisTract.GetTractVis()->mTrackRadius -= 0.01;
				cout << "Tube Radius: " << mVisTract.GetTractVis()->mTrackRadius << endl;
			}
			break;
		case 'T':
			if (mVisTract.GetTractVis()) {
				mVisTract.GetTractVis()->mTrackRadius += 0.01;
				cout << "Tube Radius: " << mVisTract.GetTractVis()->mTrackRadius << endl;
			}
			break;
		case 'w':
		case 'W':
			// top view
			this->ResetCamera();
			//glRotatef(15, 1, 0, 0);
			//glRotatef(80, 0, 1, 0);
			//glRotatef(-90, 1, 0, 0);
			break;
		case 'q':
		case 'Q':
			// left side view
			this->ResetCamera();
			glRotatef(-90, 1, 0, 0);
			glRotatef(90, 0, 0, 1);
			break;
		case 'e':
		case 'E':
			// front view
			this->ResetCamera();
			glRotatef(-90, 1, 0, 0);
			break;
		case 's':
		case 'S':
			// save picture
			this->SaveColorBufferToImage("images\\bmp\\test.bmp");
			break;
		default:
			return 0;
			break;
	};
	return 1;
}

void MyVisRankingApp::UpdateSampePerFragmentAtScale(float scale){
	MyLineAO::SamplesPerFragment = 32 / pow(scale, 1.7);
	MyLineAO::SamplesPerFragment = min(MyLineAO::SamplesPerFragment, 32);
	MyLineAO::SamplesPerFragment = max(MyLineAO::SamplesPerFragment, 6);
}

void MyVisRankingApp::ResizeRenderBuffer(int w, int h){
	if (mFrameBuffer.GetWidth() != w || mFrameBuffer.GetHeight() != h){
		mFrameBuffer.SetSize(w, h);
		mFrameBuffer.Build();
	}
}

void MyVisRankingApp::BrightnessBalance(){
	if (mbComputeBrightness || mbComputeTotalAlpha || mbLightnessBalance){
		mColorBuffer.resize(mCanvasWidth*mCanvasHeight);
		//glReadPixels(0, 0, mCanvasWidth, mCanvasHeight, GL_RGBA, GL_FLOAT, &mColorBuffer[0]);
		glBindTexture(GL_TEXTURE_2D, mFrameBuffer.GetColorTexture());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &mColorBuffer[0]);
		if (mbComputeBrightness || mbLightnessBalance){
			mBrightness = mRawBightness = ComputeGeometryAverageValue();
			cout << "Brightness: " << mRawBightness << endl;
		}
		if (mbComputeTotalPixelDrawn){
			mTotalPixelAlpha = ComputeTotalAlpha();
			cout << "TotalAlpha: " << mTotalPixelAlpha << endl;
		}
		if (mbComputeTotalAlpha){
			mTotalPixelDrawn = ComputeTotalPixelDraw();
			cout << "TotalPixels: " << mTotalPixelDrawn << endl;
			if (mbComputeTotalPixelDrawn){
				float overlappintRatio = mTotalPixelAlpha*100.f / mTotalPixelDrawn;
				cout << "Overlapping: " << overlappintRatio << endl;
			}
		}
		if (mbLightnessBalance){
			ScaleImageLightness(mTargetBrightness / mRawBightness);
			mBrightness = ComputeGeometryAverageValue();
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mCanvasWidth, mCanvasHeight, 0,
				GL_RGBA, GL_FLOAT, &mColorBuffer[0]);
			cout << "Ajusted Brightness: " << mBrightness << endl;
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

float MyVisRankingApp::ComputeTotalAlpha(){
	float sum = 0;
#pragma omp parallel for default(shared) reduction(+:sum)
	for (int i = 0; i < mColorBuffer.size(); i++){
		sum += mColorBuffer[i].a;
	}
	return sum;
}

int MyVisRankingApp::ComputeTotalPixelDraw(){
	int sum = 0;
#pragma omp parallel for default(shared) reduction(+:sum)
	for (int i = 0; i < mColorBuffer.size(); i++){
		sum += (mColorBuffer[i].a > 0 ? 1 : 0);
	}
	return sum;
}

float MyVisRankingApp::ComputeAverageValue(){
	float sum = 0;
#pragma omp parallel for default(shared) reduction(+:sum)
	for (int i = 0; i < mColorBuffer.size(); i++){
		//MyColorConverter::Lab lab = MyColorConverter::rgb2lab(mColorBuffer[i]);
		//sum += lab.l;
		sum += (mColorBuffer[i].r + mColorBuffer[i].g + mColorBuffer[i].b) / 3;
	}
	return sum / mColorBuffer.size();
}

float MyVisRankingApp::ComputeGeometryAverageValue(){
	float sum = 0;
#pragma omp parallel for default(shared) reduction(+:sum)
	for (int i = 0; i < mColorBuffer.size(); i++){
		if (mColorBuffer[i].a > 0){
			//MyColorConverter::Lab lab = MyColorConverter::rgb2lab(mColorBuffer[i]);
			//sum += lab.l*mColorBuffer[i].a;
			float value = (mColorBuffer[i].r + mColorBuffer[i].g + mColorBuffer[i].b) / 3;
			sum += value*mColorBuffer[i].a;
		}
	}
	float count = 0;
#pragma omp parallel for default(shared) reduction(+:count)
	for (int i = 0; i < mColorBuffer.size(); i++){
		if (mColorBuffer[i].a > 0){
			count += mColorBuffer[i].a;
		}
	}
	return sum / count;
}

void MyVisRankingApp::ScaleImageLightness(float scale){
#pragma omp parallel for
	for (int i = 0; i < mColorBuffer.size(); i++){
		if (mColorBuffer[i].a > 0){
			//float a = mColorBuffer[i].a;
			//MyColorConverter::Lab lab = MyColorConverter::rgb2lab(mColorBuffer[i]);
			//lab.l *= scale;
			//mColorBuffer[i] = MyColorConverter::lab2rgb(lab);
			//mColorBuffer[i].a = a;
			mColorBuffer[i].r *= scale;
			mColorBuffer[i].g *= scale;
			mColorBuffer[i].b *= scale;
		}
	}
}

void MyVisRankingApp::OcclusionProfilePrepare(MyTractVisBase* tracts, int idx){
	if (!IsOnMode(APP_MODE_OCCLUSION)) return;
	if (!tracts) return;
	mOcclusionRenderingParamters[idx] = tracts->GetRenderingParamters();
	tracts->ClearInfluences();
	tracts->SetBaseColor(MyColor4f(0.01, 0.01, 0.01, 0.01));
	tracts->SetAmbient(1);
	tracts->SetDiffuse(0);
	tracts->SetLightIntensity(1);
}

void MyVisRankingApp::OcclusionProfileRestore(MyTractVisBase* tracts, int idx){
	if (!IsOnMode(APP_MODE_OCCLUSION)) return;
	if (!tracts) return;
	tracts->SetRenderingParamters(mOcclusionRenderingParamters[idx]);
}
void MyVisRankingApp::StartProfileRendering(){
	if (IsOnMode(APP_MODE_OCCLUSION) || IsOnMode(APP_MODE_LIGHTING)){
		mRenderingLog.StartTrial();
		mbDrawTracts = true;
		mbDrawIndicators = false;
		mbDrawUI = false;
		mbDrawIndicators = false;
		mbDrawHighlighted = false;
		mbDrawLegend = false;
		mbComputeBrightness = false;
		mbLightnessBalance = false;
		mbComputeTotalAlpha = false;
		mbComputeTotalPixelDrawn = false;
		mbDrawUI = false;
		mDisplayEndToNext = mRenderingLog.IsLastRendering();
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();
		glLoadIdentity();
		MyVec3f eyepos = mRenderingLog.GetCameraPosition();
		gluLookAt(eyepos[0], eyepos[1], eyepos[2], 0, 0, 0, 0, 0, 1);
		float scale = 150.f / eyepos.norm();
		UpdateSampePerFragmentAtScale(scale);
		if (IsOnMode(APP_MODE_OCCLUSION)){
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			glDepthFunc(GL_ALWAYS);
			OcclusionProfilePrepare(mVisTract.GetTractVis(), 0);
			OcclusionProfilePrepare(mVisTract.GetTractVisAux(), 1);
		}
		mRenderingLog.StartFrame();
	}
}

void MyVisRankingApp::EndProfileRendering(){
	// statistics
	if (IsOnMode(APP_MODE_OCCLUSION) || IsOnMode(APP_MODE_LIGHTING)){
		mRenderingLog.PauseFrameTimer();
		glPopAttrib();
		glPopMatrix();
		if (IsOnMode(APP_MODE_OCCLUSION)){
			OcclusionProfileRestore(mVisTract.GetTractVis(), 0);
			OcclusionProfileRestore(mVisTract.GetTractVisAux(), 1);
		}
		mColorBuffer.resize(mCanvasWidth*mCanvasHeight);
		glBindTexture(GL_TEXTURE_2D, mFrameBuffer.GetColorTexture());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &mColorBuffer[0]);
		float v1, v2;
		if (IsOnMode(APP_MODE_LIGHTING)){
			v1 = ComputeAverageValue();
			v2 = ComputeGeometryAverageValue();
		}
		else if (IsOnMode(APP_MODE_OCCLUSION)){
			v1 = ComputeTotalPixelDraw();
			v2 = ComputeTotalAlpha()*100.f / v1;
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		mRenderingLog.SetRenderingValues({ v1, v2 });
		mRenderingLog.ResumeFrameTimer();
		mRenderingLog.EndFrame();
		RequestRedisplay();
	}

}

void MyVisRankingApp::SaveInitScreenShots(){
	// save init pics
	bool saveInit = false;
	if (saveInit){
		// check when all reshape event processed
		static int w = -1, h = -1;
		if (mWindowWidth != w || mWindowHeight != h){
			w = mWindowWidth;
			h = mWindowHeight;
			RequestRedisplay();
			return;
		}
		// start save screenshots
		const MyVisData* visData = mTrialManager.GetCurrentVisData();
		const MyVisInfo* visInfo = &(visData->GetVisInfo());
		static int idx = -1;
		static int mIdx = -1;
		//if (idx == visInfo->GetDataIndex() && mIdx == visInfo->GetMappingMethod()){
		//	RequestRedisplay();
		//	return;
		//}
		idx = visInfo->GetDataIndex();
		mIdx = visInfo->GetMappingMethod();
		if (!visInfo->IsEmpty()){
			if (visInfo->GetVisTask() == MyVisEnum::FA_VALUE){
				float avg = visData->GetAnswerInfo();
				MyArrayf values;
				mTracts.GetSampleClampedValues(visData->GetBoxes()[0], 0.2, 1.0, visData->GetTractIndices(), values);
				float stdev = MyMathHelper::ComputeStandardDeviation(values, avg);
				MyString colorName = MyColorTextureMaker::GetColorName(mIdx);
				char avgStr[128], stdevStr[128];
				sprintf(avgStr, "%f", avg);
				sprintf(stdevStr, "%f", stdev);
				SaveScreenToImage("images\\bmp\\" + colorName + "_" + MyString(idx) + "_" +
					MyString(avgStr) + "_" + MyString(stdevStr) + ".bmp");
			}
			else if (visInfo->GetVisTask() == MyVisEnum::TRACE && MyVisInfo::IsColorStudy){
				MyString ansStr = visData->GetCorrectAnswerString();
				MyArrayStr mappingNames = { "boys", "eigen", "similarity", "absolute", "gray" };
				MyString colorName = mappingNames[mIdx];
				SaveScreenToImage("images\\bmp\\" + colorName + "_" + MyString(idx) + "_" + ansStr + ".bmp");
			}
			else if (visInfo->GetVisTask() == MyVisEnum::FA){
				int ans = visData->GetCorrectAnswers()[0];
				SaveScreenToImage("images\\bmp\\t1_" + 
					toString(visInfo->GetShape()) + "_" + 
					toString(visInfo->GetEncoding()) + "_" +
					MyString(idx) + "_" +
					MyString(ans) + ".bmp");
			}
			else if (visInfo->GetVisTask() == MyVisEnum::TRACE && !MyVisInfo::IsColorStudy){
				int ans = visData->GetCorrectAnswers()[0];
				SaveScreenToImage("images\\bmp\\t2_" +
					toString(visInfo->GetShape()) + "_" +
					toString(visInfo->GetVisCue()) + "_" +
					MyString(idx) + "_" +
					MyString(ans) + ".bmp");
			}
			else if (visInfo->GetVisTask() == MyVisEnum::TUMOR){
				int ans = visData->GetCorrectAnswers()[0];
				SaveScreenToImage("images\\bmp\\t3_" +
					toString(visInfo->GetShape()) + "_" +
					toString(visInfo->GetVisCue()) + "_" +
					MyString(idx) + "_" + 
					MyString(ans) + ".bmp");
			}
		}
		mDisplayEndToNext = true;
		RequestRedisplay();
	}
}

void MyVisRankingApp::SaveColorBufferToImage(const MyString& fileName){
	MyArray<MyColor4f> colorBuffer(mCanvasWidth*mCanvasHeight);
	glBindTexture(GL_TEXTURE_2D, mFrameBuffer.GetColorTexture());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &colorBuffer[0]);
	MyBitmap bitmap;
	bitmap.SetFromData(&colorBuffer[0], mCanvasWidth, mCanvasHeight);
	bitmap.Save(fileName);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void MyVisRankingApp::SaveScreenToImage(const MyString& fileName){
	MyArray<MyColor4f> colorBuffer(mWindowWidth * mWindowHeight);
	glReadPixels(0, 0, mWindowWidth, mWindowHeight, GL_RGBA, GL_FLOAT, &colorBuffer[0].r);
	MyBitmap bitmap;
	bitmap.SetFromData(&colorBuffer[0], mWindowWidth, mWindowHeight);
	bitmap.Save(fileName);
}