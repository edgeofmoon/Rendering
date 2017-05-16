#include "MyVisRankingApp.h"
#include "MyBitmap.h"
#include "MyTexture.h"
#include "MyPrimitiveDrawer.h"
#include "MyColorConverter.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <omp.h>
#include <iostream>
using namespace std;


void MyVisRankingApp::HandleGlutDisplay(){
	StartProfileRendering();
	Show();
	glutSwapBuffers();
	EndProfileRendering();
	mLogs.StartTrial();
	if (mDisplayEndToNext) Next();
	mEventLog.LogItem( "Display" );
}

void MyVisRankingApp::HandleGlutKeyboard(unsigned char key, int x, int y){
	if (UIProcessKey(key, x, y)) {}
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
		if (!mbPaused)
			mTrackBall.ScaleMultiply(1.05);
	}
	else{
		if (!mbPaused)
			mTrackBall.ScaleMultiply(1 / 1.05);
	}
	RequestRedisplay();
	mEventLog.LogItem(
		"MouseWheel" + MyEventLog::Decimer
		+ MyString(button) + MyEventLog::Decimer
		+ MyString(dir) + MyEventLog::Decimer
		+ MyString(x) + MyEventLog::Decimer
		+ MyString(y));
}

void MyVisRankingApp::HandleGlutMouse(int button, int state, int x, int y){
	if (button == GLUT_RIGHT_BUTTON){
		if (state == GLUT_DOWN){
			ResetCamera();
		}
		RequestRedisplay();
	}
	else if(button == GLUT_LEFT_BUTTON){
		if (state == GLUT_DOWN){
			mTrackBall.StartMotion(x, y);
			UIProcessMouseDown(x, y);
		}
		else if (state == GLUT_UP){
			mTrackBall.EndMotion(x, y);
			UIProcessMouseUp(x, y);
		}
		RequestRedisplay();
	}
	mEventLog.LogItem(
		"MouseButton" + MyEventLog::Decimer
		+ MyString(button) + MyEventLog::Decimer
		+ MyString(state) + MyEventLog::Decimer
		+ MyString(x) + MyEventLog::Decimer
		+ MyString(y));
}

void MyVisRankingApp::HandleGlutMotion(int x, int y){
	if (!UIProcessMouseMove(x, y)){
		if (!mbPaused){
			mTrackBall.RotateMotion(x, y);
		}
	}
	RequestRedisplay();
	mEventLog.LogItem(
		"MouseMotion" + MyEventLog::Decimer
		+ MyString(x) + MyEventLog::Decimer
		+ MyString(y));
}

void MyVisRankingApp::HandleGlutPassiveMotion(int x, int y){
	if (UIProcessMouseMove(x, y)){
		RequestRedisplay();
	}
	mEventLog.LogItem(
		"MousePassive" + MyEventLog::Decimer
		+ MyString(x) + MyEventLog::Decimer
		+ MyString(y));
}

void MyVisRankingApp::HandleGlutReshape(int x, int y){
	mWindowWidth = x;
	mWindowHeight = y;
	mCanvasWidth = x*mCanvasScaleX;
	mCanvasHeight = y*mCanvasScaleY;
	mTrackBall.Reshape(x, y);
	ResetCamera();
	ResizeRenderBuffer(mCanvasWidth, mCanvasHeight);
	UIResize(x, y);
	mVisTract.Resize(mCanvasWidth, mCanvasHeight);
	RequestRedisplay();
	mEventLog.LogItem(
		"Reshape" + MyEventLog::Decimer
		+ MyString(x) + MyEventLog::Decimer
		+ MyString(y));
}

void MyVisRankingApp::RequestRedisplay(){
	glutPostRedisplay();
}

void MyVisRankingApp::ResetCamera(){
	MyMatrixf projectionMatrix = MyMatrixf::PerspectiveMatrix(60, mWindowWidth / (float)mWindowHeight, 1, 300);
	//MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-100,100,-100,100, 1, 200);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	gluLookAt(0, 0, 150, 0, 0, 0, 0, 1, 0);
	mTrackBall.ResetRotate();
	mTrackBall.ResetScale();
}

int MyVisRankingApp::HandleDebugKey(unsigned char key){
	if (!IsOnMode(APP_MODE_DEBUG)) return 0;
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
			Next();
			break;
		case 'p':
		case 'P':
			Previous();
			break;
		case 'b':
		case 'B':
			mbLightnessBalance = !mbLightnessBalance;
			cout << "Lightness balance: " << (mbLightnessBalance ? "ON" : "OFF") << endl;
			break;
		case ']':
			mTargetBrightness += 1.0;
			cout << "Brightness: " << mBrightness << endl;
			break;
		case '[':
			mTargetBrightness -= 1.0;
			cout << "Brightness: " << mBrightness << endl;
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
		default:
			return 0;
			break;
	};
	return 1;
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
			v2 = ComputeTotalAlpha()*100.f / v1;;
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		mRenderingLog.SetRenderingValues({ v1, v2 });
		mRenderingLog.ResumeFrameTimer();
		mRenderingLog.EndFrame();
		RequestRedisplay();
	}

}