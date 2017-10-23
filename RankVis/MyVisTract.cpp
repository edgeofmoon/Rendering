#include "MyVisTract.h"

#include "MyTubeDDH.h"
#include "MyTrackDDH.h"
#include "MyLineAO.h"
#include "MyColor4.h"

#include "MySphereAOGeometry.h"
#include "MySphereDDHGeometry.h"

#include <iostream>
using namespace std;
using namespace MyVisEnum;

#define BASIC_SCALE_LINE_AO			1
#define BASIC_SCALE_LINE_BASIC		1
#define BASIC_SCALE_LINE_DEPTH		1
#define BASIC_SCALE_LINE_ENCODING	1
#define BASIC_SCALE_TUBE_AO			1
#define BASIC_SCALE_TUBE_BASIC		1
#define BASIC_SCALE_TUBE_DEPTH		1
#define BASIC_SCALE_TUBE_ENCODING	1

#define NORMALIZED_SCALE_LINE_AO		1.764
#define NORMALIZED_SCALE_LINE_BASIC		1
#define NORMALIZED_SCALE_LINE_DEPTH		1.022
#define NORMALIZED_SCALE_LINE_ENCODING	1.000
#define NORMALIZED_SCALE_TUBE_AO		1.677
#define NORMALIZED_SCALE_TUBE_BASIC		1
#define NORMALIZED_SCALE_TUBE_DEPTH		1.009
#define NORMALIZED_SCALE_TUBE_ENCODING	0.990

float MyVisTract::SCALE_LINE_AO = BASIC_SCALE_LINE_AO;
float MyVisTract::SCALE_LINE_BASIC = BASIC_SCALE_LINE_BASIC;
float MyVisTract::SCALE_LINE_DEPTH = BASIC_SCALE_LINE_DEPTH;
float MyVisTract::SCALE_LINE_ENCODING = BASIC_SCALE_LINE_ENCODING;
float MyVisTract::SCALE_TUBE_AO = BASIC_SCALE_TUBE_AO;
float MyVisTract::SCALE_TUBE_BASIC = BASIC_SCALE_TUBE_BASIC;
float MyVisTract::SCALE_TUBE_DEPTH = BASIC_SCALE_TUBE_DEPTH;
float MyVisTract::SCALE_TUBE_ENCODING = BASIC_SCALE_TUBE_ENCODING;

void MyVisTract::UseNormalizedLighting(bool b){
	if (b){
		SCALE_LINE_AO = NORMALIZED_SCALE_LINE_AO;
		SCALE_LINE_BASIC = NORMALIZED_SCALE_LINE_BASIC;
		SCALE_LINE_DEPTH = NORMALIZED_SCALE_LINE_DEPTH;
		SCALE_LINE_ENCODING = NORMALIZED_SCALE_LINE_ENCODING;
		SCALE_TUBE_AO = NORMALIZED_SCALE_TUBE_AO;
		SCALE_TUBE_BASIC = NORMALIZED_SCALE_TUBE_BASIC;
		SCALE_TUBE_DEPTH = NORMALIZED_SCALE_TUBE_DEPTH;
		SCALE_TUBE_ENCODING = NORMALIZED_SCALE_TUBE_ENCODING;
	}
	else {
		SCALE_LINE_AO = BASIC_SCALE_LINE_AO;
		SCALE_LINE_BASIC = BASIC_SCALE_LINE_BASIC;
		SCALE_LINE_DEPTH = BASIC_SCALE_LINE_DEPTH;
		SCALE_LINE_ENCODING = BASIC_SCALE_LINE_ENCODING;
		SCALE_TUBE_AO = BASIC_SCALE_TUBE_AO;
		SCALE_TUBE_BASIC = BASIC_SCALE_TUBE_BASIC;
		SCALE_TUBE_DEPTH = BASIC_SCALE_TUBE_DEPTH;
		SCALE_TUBE_ENCODING = BASIC_SCALE_TUBE_ENCODING;
	}
}

MyVisTract::MyVisTract()
	:mVisData(NULL), mTractVis(NULL), mTractVis_Aux(NULL){
	mTubeVis = NULL;
	mLineVis = NULL;
	mTubeVisFlatCap = NULL;
	mSuperquadricVis = NULL;
	mLineDDHVis = NULL;
	mTubeDDHVis = NULL;
	mLineAOVis = NULL;
	mTubeAOVis = NULL;
	mCanvasWidth = 0;
	mCanvasHeight = 0;
	mBoxTubeVis = NULL;
	mBoxTracts = NULL;
	mIgnoreBoxVis = false;
}


MyVisTract::~MyVisTract(){
	if (mTubeVis) delete mTubeVis;
	if (mLineVis) delete mLineVis;
	if (mSuperquadricVis) delete mSuperquadricVis;
	if (mLineDDHVis) delete mLineDDHVis;
	if (mTubeDDHVis) delete mTubeDDHVis;
	if (mLineAOVis) delete mLineAOVis;
	if (mTubeAOVis) delete mTubeAOVis;
	if (mBoxTubeVis) delete mBoxTubeVis;
	if (mBoxTracts) delete mBoxTracts;
}

void MyVisTract::Show(){
	if (mTractVis) mTractVis->Show();
	if (mTractVis_Aux) mTractVis_Aux->Show();
}

void MyVisTract::Resize(int w, int h){
	mCanvasWidth = w;
	mCanvasHeight = h;
	MyLineAO* lineAOVis = dynamic_cast<MyLineAO*>(mTractVis);
	if (lineAOVis){
		lineAOVis->Resize(w, h);
	}
};


void MyVisTract::Update(){
	const MyVisData& visData = *mVisData;
	const MyVisInfo& visInfo = visData.GetVisInfo();

	mTractVis = NULL;
	mTractVis_Aux = NULL;

	if (visInfo.GetVisTask() == FA || (visInfo.GetVisTask() == FA_VALUE && mIgnoreBoxVis)){
		if (visInfo.GetShape() == TUBE){
			if (visInfo.GetEncoding() == COLOR)
				Switch_FA_TUBE_COLOR();
			else if (visInfo.GetEncoding() == VALUE)
				Switch_FA_TUBE_VALUE();
			else if (visInfo.GetEncoding() == ESIZE)
				Switch_FA_TUBE_SIZE();
			else if (visInfo.GetEncoding() == TEXTURE)
				Switch_FA_TUBE_TEXTURE();
			else
				cerr << "Unresolved encoding for FA, TUBE: " 
				<< toString(visInfo.GetEncoding()) << endl;
		}
		else if (visInfo.GetShape() == SUPERQUADRIC){
			if (visInfo.GetEncoding() == COLOR)
				Switch_FA_SUPERQUADRICS_COLOR();
			else if (visInfo.GetEncoding() == VALUE)
				Switch_FA_SUPERQUADRICS_VALUE();
			else
				cerr << "Unresolved encoding for FA, SUPERQUADRICS: "
				<< toString(visInfo.GetEncoding()) << endl;
		}
		else
			cerr << "Unresolved shape for FA: "
			<< toString(visInfo.GetShape()) << endl;
	}
	else if (visInfo.GetVisTask() == TRACE){
		if (visInfo.GetShape() == LINE){
			if (visInfo.GetVisCue() == DEPTH)
				Switch_TRACE_LINE_DEPTH();
			else if (visInfo.GetVisCue() == AMBIENT_OCCULUSION)
				Switch_TRACE_LINE_AO();
			else if (visInfo.GetVisCue() == ENCODING)
				Switch_TRACE_LINE_ENCODING();
			else if (visInfo.GetVisCue() == BASIC)
				Switch_TRACE_LINE_BASIC();
			else
				cerr << "Unresolved cue for TRACE, LINE: "
				<< toString(visInfo.GetVisCue()) << endl;
		}
		else if (visInfo.GetShape() == TUBE){
			if (visInfo.GetVisCue() == DEPTH)
				Switch_TRACE_TUBE_DEPTH();
			else if (visInfo.GetVisCue() == AMBIENT_OCCULUSION)
				Switch_TRACE_TUBE_AO();
			else if (visInfo.GetVisCue() == ENCODING)
				Switch_TRACE_TUBE_ENCODING();
			else if (visInfo.GetVisCue() == BASIC)
				Switch_TRACE_TUBE_BASIC();
			else
				cerr << "Unresolved cue for TRACE, TUBE: "
				<< toString(visInfo.GetVisCue()) << endl;
		}
		else
			cerr << "Unresolved shape for TRACE: "
			<< toString(visInfo.GetShape()) << endl;
	}
	else if (visInfo.GetVisTask() == TUMOR){
		if (visInfo.GetShape() == LINE){
			if (visInfo.GetVisCue() == DEPTH)
				Switch_TUMOR_LINE_DEPTH();
			else if (visInfo.GetVisCue() == AMBIENT_OCCULUSION)
				Switch_TUMOR_LINE_AO();
			else if (visInfo.GetVisCue() == ENCODING)
				Switch_TUMOR_LINE_ENCODING();
			else if (visInfo.GetVisCue() == BASIC)
				Switch_TUMOR_LINE_BASIC();
			else
				cerr << "Unresolved cue for TUMOR, LINE: "
				<< toString(visInfo.GetVisCue()) << endl;
		}
		else if (visInfo.GetShape() == TUBE){
			if (visInfo.GetVisCue() == DEPTH)
				Switch_TUMOR_TUBE_DEPTH();
			else if (visInfo.GetVisCue() == AMBIENT_OCCULUSION)
				Switch_TUMOR_TUBE_AO();
			else if (visInfo.GetVisCue() == ENCODING)
				Switch_TUMOR_TUBE_ENCODING();
			else if (visInfo.GetVisCue() == BASIC)
				Switch_TUMOR_TUBE_BASIC();
			else
				cerr << "Unresolved cue for TUMOR, TUBE: "
				<< toString(visInfo.GetVisCue()) << endl;
		}
		else
			cerr << "Unresolved shape for TUMOR: "
			<< toString(visInfo.GetShape()) << endl;
	}
	else if (visInfo.GetVisTask() == FA_VALUE){
		Switch_FA_VALUE();
	}
	else if (!visInfo.IsEmpty())
		cerr << "Unresolved task: "  << toString(visInfo.GetVisTask()) << endl;

	if (visInfo.GetVisTask() != FA_VALUE){
		// update shown indices
		if (mTractVis) mTractVis->SetTractsShown(mVisData->GetTractIndices());
		if (mTractVis_Aux) mTractVis_Aux->SetTractsShown(mVisData->GetTractIndices());
	}
}

void MyVisTract::Switch_FA_TUBE_COLOR(){
	if (mTubeVis == NULL){
		mTubeVis = new MyTractVisBase;
		mTubeVis->SetTracts(mVisData->GetTracts());
		mTubeVis->SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
		mTubeVis->ComputeGeometry();
		mTubeVis->LoadShader();
		mTubeVis->LoadGeometry();
	}
	mTubeVis->ResetRenderingParameters();
	mTubeVis->SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
	mTubeVis->SetTexture(mColorTextures[mVisData->GetVisInfo().GetMappingMethod()]);
	mTubeVis->ClearInfluences();
	mTubeVis->SetValueToTextureInfluence(1);
	mTubeVis->SetTractsShown(mVisData->GetTractIndices());
	mTractVis = mTubeVis;
	mTractVis_Aux = NULL;
}

void MyVisTract::Switch_FA_TUBE_VALUE(){
	Switch_FA_TUBE_COLOR();
	mTubeVis->SetTexture(mValueTextures[mVisData->GetVisInfo().GetMappingMethod()]);
}

void MyVisTract::Switch_FA_TUBE_SIZE(){
	Switch_FA_TUBE_COLOR();
	mTubeVis->ClearInfluences();
	mTubeVis->SetValueToSizeInfluence(2 * mTubeVis->mTrackRadius);
}

void MyVisTract::Switch_FA_TUBE_TEXTURE(){
	Switch_FA_TUBE_COLOR();
	mTubeVis->ClearInfluences();
	mTubeVis->SetValueToTextureRatioInfluence(2);
}

void MyVisTract::Switch_FA_SUPERQUADRICS_COLOR(){
	if (mSuperquadricVis == NULL){
		mSuperquadricVis = new MyTractVisBase;
		mSuperquadricVis->SetTracts(mVisData->GetTracts());
		mSuperquadricVis->SetShape(MyTractVisBase::TRACK_SHAPE_SUPERQUADRIC);
		mSuperquadricVis->ComputeGeometry();
		mSuperquadricVis->LoadShader();
		mSuperquadricVis->LoadGeometry();
	}
	mSuperquadricVis->ResetRenderingParameters();
	mSuperquadricVis->SetShape(MyTractVisBase::TRACK_SHAPE_SUPERQUADRIC);
	mSuperquadricVis->SetTexture(mColorTextures[mVisData->GetVisInfo().GetMappingMethod()]);
	mSuperquadricVis->ClearInfluences();
	mSuperquadricVis->SetValueToTextureInfluence(1);
	mSuperquadricVis->SetTractsShown(mVisData->GetTractIndices());
	mSuperquadricVis->UpdateBoundingBox();
	mTractVis = mSuperquadricVis;
	mTractVis_Aux = NULL;
}

void MyVisTract::Switch_FA_SUPERQUADRICS_VALUE(){
	Switch_FA_SUPERQUADRICS_COLOR();
	mSuperquadricVis->SetTexture(mValueTextures[mVisData->GetVisInfo().GetMappingMethod()]);
}

void MyVisTract::Switch_TRACE_LINE_DEPTH(){
	if (mLineDDHVis == NULL){
		mLineDDHVis = new MyTrackDDH;
		mLineDDHVis->SetTracts(mVisData->GetTracts());
		mLineDDHVis->SetShape(MyTractVisBase::TRACK_SHAPE_LINE);
		mLineDDHVis->ComputeGeometry();
		mLineDDHVis->LoadShader();
		mLineDDHVis->LoadGeometry();
	}
	mLineDDHVis->ResetRenderingParameters();
	mLineDDHVis->SetShape(MyTractVisBase::TRACK_SHAPE_LINE);
	mLineDDHVis->SetLightIntensity(SCALE_LINE_DEPTH);
	mLineDDHVis->ClearInfluences();
	static_cast<MyTrackDDH*>(mLineDDHVis)->mStrokeWidth = mLineDDHVis->mTrackRadius * 2;
	static_cast<MyTrackDDH*>(mLineDDHVis)->mStripWidth = mLineDDHVis->mTrackRadius * 8;
	mLineDDHVis->SetTractsShown(mVisData->GetTractIndices());
	mLineDDHVis->UpdateBoundingBox();

	mTractVis = mLineDDHVis;
	mTractVis_Aux = NULL;
}

void MyVisTract::Switch_TRACE_TUBE_DEPTH(){
	Switch_TRACE_LINE_DEPTH();
	if (mTubeDDHVis == NULL){
		mTubeDDHVis = new MyTubeDDH;
		mTubeDDHVis->SetTracts(mVisData->GetTracts());
		mTubeDDHVis->SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
		mTubeDDHVis->SetCapType(MyTractVisBase::CAP_TYPE_FLAT);
		mTubeDDHVis->ComputeGeometry();
		mTubeDDHVis->LoadShader();
		mTubeDDHVis->LoadGeometry();
	}
	mTubeDDHVis->ResetRenderingParameters();
	mTubeDDHVis->SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
	mTubeDDHVis->SetLightIntensity(SCALE_TUBE_DEPTH);
	mTubeDDHVis->ClearInfluences();
	//mTubeDDHVis->mTrackRadius = 
	//	static_cast<MyTrackDDH*>(mLineDDHVis)->mStrokeWidth / 2;
	mTubeDDHVis->SetTractsShown(mVisData->GetTractIndices());
	mTubeDDHVis->UpdateBoundingBox();

	// do not add sphere twice!

	mTractVis_Aux = mTubeDDHVis;
}

void MyVisTract::Switch_TRACE_LINE_AO(){
	if (mLineAOVis == NULL){
		mLineAOVis = new MyLineAO;
		mLineAOVis->SetTracts(mVisData->GetTracts());
		mLineAOVis->SetShape(MyTractVisBase::TRACK_SHAPE_LINE);
		mLineAOVis->ComputeGeometry();
		mLineAOVis->LoadShader();
		mLineAOVis->LoadGeometry();
		static_cast<MyLineAO*>(mLineAOVis)->Resize(mCanvasWidth, mCanvasHeight);
	}
	mLineAOVis->ResetRenderingParameters();
	mLineAOVis->SetShape(MyTractVisBase::TRACK_SHAPE_LINE);
	mLineAOVis->SetLightIntensity(SCALE_LINE_AO);
	mLineAOVis->ClearInfluences();
	static_cast<MyLineAO*>(mLineAOVis)->Resize(mCanvasWidth, mCanvasHeight);
	static_cast<MyLineAO*>(mLineAOVis)->mLineAOTotalStrength = 1;
	mLineAOVis->SetTractsShown(mVisData->GetTractIndices());
	mLineAOVis->UpdateBoundingBox();

	mTractVis = mLineAOVis;
	mTractVis_Aux = NULL;
}

void MyVisTract::Switch_TRACE_TUBE_AO(){
	if (mTubeAOVis == NULL){
		mTubeAOVis = new MyLineAO;
		mTubeAOVis->SetTracts(mVisData->GetTracts());
		mTubeAOVis->SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
		mTubeAOVis->SetCapType(MyTractVisBase::CAP_TYPE_FLAT);
		mTubeAOVis->ComputeGeometry();
		mTubeAOVis->LoadShader();
		mTubeAOVis->LoadGeometry();
		static_cast<MyLineAO*>(mTubeAOVis)->Resize(mCanvasWidth, mCanvasHeight);
	}
	mTubeAOVis->ResetRenderingParameters();
	mTubeAOVis->SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
	mTubeAOVis->SetLightIntensity(SCALE_TUBE_AO);
	mTubeAOVis->ClearInfluences();
	static_cast<MyLineAO*>(mTubeAOVis)->Resize(mCanvasWidth, mCanvasHeight);
	static_cast<MyLineAO*>(mTubeAOVis)->mLineAOTotalStrength = 1;
	mTubeAOVis->SetTractsShown(mVisData->GetTractIndices());
	mTubeAOVis->UpdateBoundingBox();

	mTractVis = mTubeAOVis;
	mTractVis_Aux = NULL;
}

void MyVisTract::Switch_TRACE_LINE_ENCODING(){
	if (mLineVis == NULL){
		mLineVis = new MyTractVisBase;
		mLineVis->SetTracts(mVisData->GetTracts());
		mLineVis->SetShape(MyTractVisBase::TRACK_SHAPE_LINE);
		mLineVis->ComputeGeometry();
		mLineVis->LoadShader();
		mLineVis->LoadGeometry();
	}
	mLineVis->ResetRenderingParameters();
	mLineVis->SetShape(MyTractVisBase::TRACK_SHAPE_LINE);
	mLineVis->SetLightIntensity(SCALE_LINE_ENCODING);
	mLineVis->ClearInfluences();
	mLineVis->SetColorInfluence(1);
	mLineVis->SetTractsShown(mVisData->GetTractIndices());
	mLineVis->UpdateBoundingBox();

	if (mVisData->GetVisInfo().GetMappingMethod() != 0){
		mLineVis->ClearInfluences();
		mLineVis->SetPerTractColor(&mVisData->GetTractColors());
	}
	else {
		mLineVis->SetPerTractColor(NULL);
	}

	mTractVis = mLineVis;
	mTractVis_Aux = NULL;
}

void MyVisTract::Switch_TRACE_TUBE_ENCODING(){
	if (mTubeVisFlatCap == NULL){
		mTubeVisFlatCap = new MyTractVisBase;
		mTubeVisFlatCap->SetTracts(mVisData->GetTracts());
		mTubeVisFlatCap->SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
		mTubeVisFlatCap->SetCapType(MyTractVisBase::CAP_TYPE_FLAT);
		mTubeVisFlatCap->ComputeGeometry();
		mTubeVisFlatCap->LoadShader();
		mTubeVisFlatCap->LoadGeometry();
	}
	mTubeVisFlatCap->ResetRenderingParameters();
	mTubeVisFlatCap->SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
	mTubeVisFlatCap->SetLightIntensity(SCALE_TUBE_ENCODING);
	mTubeVisFlatCap->ClearInfluences();
	mTubeVisFlatCap->SetColorInfluence(1);
	mTubeVisFlatCap->SetTractsShown(mVisData->GetTractIndices());
	mTubeVisFlatCap->UpdateBoundingBox();

	if (mVisData->GetVisInfo().GetMappingMethod() != 0){
		mTubeVisFlatCap->ClearInfluences();
		mTubeVisFlatCap->SetPerTractColor(&mVisData->GetTractColors());
	}
	else {
		mTubeVisFlatCap->SetPerTractColor(NULL);
	}

	mTractVis = mTubeVisFlatCap;
	mTractVis_Aux = NULL;
}

void MyVisTract::Switch_TRACE_LINE_BASIC(){
	if (mLineVis == NULL){
		mLineVis = new MyTractVisBase;
		mLineVis->SetTracts(mVisData->GetTracts());
		mLineVis->SetShape(MyTractVisBase::TRACK_SHAPE_LINE);
		mLineVis->ComputeGeometry();
		mLineVis->LoadShader();
		mLineVis->LoadGeometry();
	}
	mLineVis->ResetRenderingParameters();
	mLineVis->SetShape(MyTractVisBase::TRACK_SHAPE_LINE);
	mLineVis->SetLightIntensity(SCALE_LINE_BASIC);
	mLineVis->SetPixelHaloWidth(4);
	mLineVis->ClearInfluences();
	mLineVis->SetTractsShown(mVisData->GetTractIndices());
	mLineVis->UpdateBoundingBox();

	mTractVis = mLineVis;
}

void MyVisTract::Switch_TRACE_TUBE_BASIC(){
	if (mTubeVisFlatCap == NULL){
		mTubeVisFlatCap = new MyTractVisBase;
		mTubeVisFlatCap->SetTracts(mVisData->GetTracts());
		mTubeVisFlatCap->SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
		mTubeVisFlatCap->SetCapType(MyTractVisBase::CAP_TYPE_FLAT);
		mTubeVisFlatCap->SetLightIntensity(SCALE_TUBE_BASIC);
		mTubeVisFlatCap->ComputeGeometry();
		mTubeVisFlatCap->LoadShader();
		mTubeVisFlatCap->LoadGeometry();
	}
	mTubeVisFlatCap->ResetRenderingParameters();
	mTubeVisFlatCap->SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
	mTubeVisFlatCap->SetPixelHaloWidth(4);
	mTubeVisFlatCap->ClearInfluences();
	mTubeVisFlatCap->SetTractsShown(mVisData->GetTractIndices());
	mTubeVisFlatCap->UpdateBoundingBox();

	mTractVis = mTubeVisFlatCap;
}


void MyVisTract::Switch_TUMOR_LINE_DEPTH(){
	Switch_TRACE_LINE_DEPTH();
	AddTumorSphere();
}

void MyVisTract::Switch_TUMOR_TUBE_DEPTH(){
	Switch_TRACE_TUBE_DEPTH();
	AddTumorSphere();
}

void MyVisTract::Switch_TUMOR_LINE_AO(){
	Switch_TRACE_LINE_AO();

	AddTumorSphere();
}

void MyVisTract::Switch_TUMOR_TUBE_AO(){
	Switch_TRACE_TUBE_AO();

	AddTumorSphere();
}

void MyVisTract::Switch_TUMOR_LINE_ENCODING(){
	Switch_TRACE_LINE_ENCODING();

	AddTumorSphere();
}

void MyVisTract::Switch_TUMOR_TUBE_ENCODING(){
	Switch_TRACE_TUBE_ENCODING();

	AddTumorSphere();
}

void MyVisTract::Switch_TUMOR_LINE_BASIC(){
	Switch_TRACE_LINE_BASIC();

	AddTumorSphere();
}

void MyVisTract::Switch_TUMOR_TUBE_BASIC(){
	Switch_TRACE_TUBE_BASIC();

	AddTumorSphere();
}

void MyVisTract::AddTumorSphere(){
	MySphereGeometry* sphereGeometry = mTractVis->GetSphereGeometry();
	if (sphereGeometry) delete sphereGeometry;
	mTractVis->SetSphereGeometry(NULL);
	if (mVisData->GetSpheres().size() > 0){
		if (mVisData->GetVisInfo().GetVisCue() == DEPTH)
			sphereGeometry = new MySphereDDHGeometry;
		else if (mVisData->GetVisInfo().GetVisCue() == AMBIENT_OCCULUSION)
			sphereGeometry = new MySphereAOGeometry;
		else sphereGeometry = new MySphereGeometry;
		sphereGeometry->SetSphere(&(mVisData->GetSphere(0)));
		sphereGeometry->SetShaderProgram(mTractVis->GetShaderProgram());
		sphereGeometry->GenerateGeometry();
		mTractVis->SetSphereGeometry(sphereGeometry);
	}
}

void MyVisTract::Switch_FA_VALUE(){
	if (mVisData->GetVisInfo().IsEmpty()) return;
	if (mBoxTubeVis != NULL) delete mBoxTubeVis;
	if ( mBoxTracts == NULL) delete mBoxTracts;
	mBoxTracts = new MyTracks;
	mBoxTracts->BoxSubsetFrom(*mVisData->GetTracts(), 
		mVisData->GetBoxes()[0], mVisData->GetTractIndices());
	mBoxTubeVis = new MyTractVisBase();
	mBoxTubeVis->SetTracts(mBoxTracts);
	mBoxTubeVis->SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
	mBoxTubeVis->SetCapType(MyTractVisBase::CAP_TYPE_FLAT);
	mBoxTubeVis->ComputeGeometry();
	mBoxTubeVis->LoadShader();
	mBoxTubeVis->LoadGeometry();
	mBoxTubeVis->ResetRenderingParameters();
	mBoxTubeVis->SetTexture(mColorTextures[mVisData->GetVisInfo().GetMappingMethod()]);
	mBoxTubeVis->ClearInfluences();
	mBoxTubeVis->SetValueToTextureInfluence(1);

	mTractVis = mBoxTubeVis;
	mTractVis_Aux = NULL;
}