#pragma once

#include "MyVisTrialManager.h"
#include "MyVisTract.h"
#include "MyTrackBall.h"
#include "MyFrameBuffer.h"
#include "MyUIObject.h"
#include "MyNumberArea.h"
#include "MyButton.h"
#include "MyToggleButton.h"
#include "MyLogTable.h"
#include "MyRenderingLog.h"

class MyVisRankingApp
{
public:
	MyVisRankingApp();
	~MyVisRankingApp();

	enum APP_MODE{
		APP_MODE_TRAINING = (1 << 0),
		APP_MODE_STUDY = (1 << 1),
		APP_MODE_LIGHTING = (1 << 2),
		APP_MODE_OCCLUSION = (1 << 3),

		APP_MODE_DEBUG = (1 << 8),
	};
	void Init(int uidx, int tidx = -1, int mode = APP_MODE_STUDY);
	void Next();
	void Previous();
	void PrintTrialInfo();
	void Show();

	// gl call back
	void HandleGlutDisplay();
	void HandleGlutKeyboard(unsigned char key, int x, int y);
	void HandleGlutSpecialKeyboard(unsigned char key, int x, int y);
	void HandleGlutMouseWheel(int button, int dir, int x, int y);
	void HandleGlutMouse(int button, int state, int x, int y);
	void HandleGlutMotion(int x, int y);
	void HandleGlutPassiveMotion(int x, int y);
	void HandleGlutReshape(int x, int y);
	void RequestRedisplay();

	void ResetCamera();
	bool IsOnMode(APP_MODE mode) const { return (mAppMode & mode) != 0; };

protected:
	// control
	int mAppMode;
	bool mbDrawTracts;
	bool mbDrawIndicators;
	bool mbDrawHighlighted;
	bool mbDrawUI;
	bool mbComputeBrightness;
	bool mbLightnessBalance;
	bool mbComputeTotalAlpha;
	bool mbComputeTotalPixelDrawn;
	int HandleDebugKey(unsigned char key);

	// log
	MyLogTable mLogs;
	MyRenderingLog mRenderingLog;

	// UI
	MyArray<MyUIObject*> mTaskUIs;
	void ActivateUI(MyUIObject* ui);
	void DeactivateUI(MyUIObject* ui);

	// UI events
	int mAnswerSelected;
	int mConfidenceSelected;
	bool mbPaused;
	void UI_Pause();
	void UI_Input();
	void UI_Resume();
	void UI_Back();
	void UI_AnswerSelect(int idx);
	void UI_ConfidenceSelected(int idx);
	void UI_Next();
	void UI_Process(int uid);
	bool UI_CanNext();

	// UI components;
	float mButtonWidth, mButtonHeight;
	float mButtonInterval;
	MyButton mButton_Next;
	MyTextArea mTextArea_ConfidenceHint;
	MyToggleButtonGroup mButtonGroup_Confidence;
	MyToggleButtonGroup mButtonGroup_Answer;
	MyButton mButton_Input;
	MyButton mButton_Back;
	MyButton mButton_Pause;
	MyButton mButton_Resume;
	MyTextArea mTextArea_Answer;
	MyTextArea mTextArea_InputHint;
	MyNumberArea mTextArea_Input;
	MyTextArea mTextArea_Transition;
	MyTextArea mTextArea_Hint;
	MyTextArea mTextArea_Progress;
	int UIProcessMouseUp(int x, int y);
	int UIProcessMouseDown(int x, int y);
	int UIProcessMouseMove(int x, int y);
	int UIProcessKey(unsigned char key, int x, int y);
	int UIProcessSpecialKey(unsigned char key, int x, int y);
	void UIInit();
	void UIUpdate();
	void UIResize(int w, int h);
	void UIDestory();
	void UIDraw();

	// indicators
	void DrawHighlightedTracts(MyTractVisBase* tractVis,
		const MyArrayi& tractIndices, const MyColor4f& color);
	void DrawHighlightedTracts(MyTractVisBase* tractVis);
	void DrawHighlighted();
	void DrawBoxes();
	void DrawTractIndicators();

	// data gen
	void ProcessKey_DataGen(unsigned char key);
	int mCurrentBoxIndex;
	void ProcessKey_FA(unsigned char key);
	MyArray<MySphere> mSphereSelectors;
	MyArrayc mOperators;
	MyArrayi mSelected;
	int mCurrentSelectorIndex;
	void ProcessKey_TRACE(unsigned char key);
	void ProcessKey_TUMOR(unsigned char key);

protected:
	// data
	MyVisTrialManager mTrialManager;
	MyVisTract mVisTract;
	MyTracks mTracts;
	unsigned int mColorTexture;
	unsigned int mSaturationTexture;

	// framebuffer
	MyFrameBuffer mFrameBuffer;
	MyArray<MyColor4f> mColorBuffer;
	void ResizeRenderBuffer(int w, int h);

	// statistics
	float mBrightness;
	float mTargetBrightness;
	float mRawBightness;
	float mLineThickness;
	float mTotalPixelAlpha;
	float mTotalPixelDrawn;
	void BrightnessBalance();
	float ComputeTotalAlpha();
	float ComputeTotalPixelDraw();
	float ComputeAverageValue();
	float ComputeGeometryAverageValue();
	void ScaleImageLightness(float scale);

	MyTractVisBase::RenderingParameters mOcclusionRenderingParamters[2];
	bool mDisplayEndToNext;
	void OcclusionProfilePrepare(MyTractVisBase* tracts, int idx);
	void OcclusionProfileRestore(MyTractVisBase* tracts, int idx);
	void StartProfileRendering();
	void EndProfileRendering();

	// interaction
	MyTrackBall mTrackBall;
	int mWindowWidth, mWindowHeight;
	int mCanvasWidth, mCanvasHeight;
	float mCanvasScaleX, mCanvasScaleY;

private:
	MyVec3f GetWorldOffset(const MyVec3f& viewoffset);
	bool ProcessBoundingObjectKey(MyBoundingObject& bobj, unsigned char key);
	void BoundObjectSize(MyBoundingObject& bobj);
	void ModifyToStable(MyBoundingObject& bobj, float step, int maxItr = 10) const;
	MyArrayi ComputedSelected(const MyArray<MySphere>& selectors,
		const MyArrayc& operators) const;
	MyArrayi ComputedFilteredByBox(const MyBoundingBox& box,
		const MyArrayi& selected) const;
	MyColor4f GetSphereStatusColor(const MyTractVisBase* tractVis) const;
	MyColor4f GetCollisionStatusColor(MyVisEnum::CollisionStatus st) const;
};

