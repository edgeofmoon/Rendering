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

/*
#define SCALE_LINE_AO		1
#define SCALE_LINE_BASIC	1
#define SCALE_LINE_DEPTH	1
#define SCALE_LINE_ENCODING	1
#define SCALE_TUBE_AO		1
#define SCALE_TUBE_BASIC	1
#define SCALE_TUBE_DEPTH	1
#define SCALE_TUBE_ENCODING	1
*/

#define SCALE_LINE_AO		1.942544853
#define SCALE_LINE_BASIC	1
#define SCALE_LINE_DEPTH	1
#define SCALE_LINE_ENCODING	1.008202336
#define SCALE_TUBE_AO		1.7908629
#define SCALE_TUBE_BASIC	1
#define SCALE_TUBE_DEPTH	1.005692041
#define SCALE_TUBE_ENCODING	1.000280219

MyVisTract::MyVisTract()
	:mVisData(NULL), mTractVis(NULL), mTractVis_Aux(NULL){
	mTubeVis = NULL;
	mLineVis = NULL;
	mSuperquadricVis = NULL;
	mLineDDHVis = NULL;
	mTubeDDHVis = NULL;
	mLineAOVis = NULL;
	mTubeAOVis = NULL;
	mCanvasWidth = 0;
	mCanvasHeight = 0;
	mSphereGeometry = 0;
}


MyVisTract::~MyVisTract(){
	if (mTubeVis) delete mTubeVis;
	if (mLineVis) delete mLineVis;
	if (mSuperquadricVis) delete mSuperquadricVis;
	if (mLineDDHVis) delete mLineDDHVis;
	if (mTubeDDHVis) delete mTubeDDHVis;
	if (mLineAOVis) delete mLineAOVis;
	if (mTubeAOVis) delete mTubeAOVis;
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

	if (visInfo.GetVisTask() == FA){
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
	else if (!visInfo.IsEmpty())
		cerr << "Unresolved task: "  << toString(visInfo.GetVisTask()) << endl;

	// update shown indices
	if(mTractVis) mTractVis->SetTractsShown(mVisData->GetTractIndices());
	if (mTractVis_Aux) mTractVis_Aux->SetTractsShown(mVisData->GetTractIndices());
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
	mTubeVis->SetValueToTextureRatioInfluence(1);
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
		mTubeDDHVis->ComputeGeometry();
		mTubeDDHVis->LoadShader();
		mTubeDDHVis->LoadGeometry();
	}
	mTubeDDHVis->ResetRenderingParameters();
	mTubeDDHVis->SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
	mTubeDDHVis->SetLightIntensity(SCALE_TUBE_DEPTH);
	mTubeDDHVis->ClearInfluences();
	mTubeDDHVis->mTrackRadius = 
		static_cast<MyTrackDDH*>(mLineDDHVis)->mStrokeWidth / 2;
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

	mTractVis = mLineVis;
	mTractVis_Aux = NULL;
}

void MyVisTract::Switch_TRACE_TUBE_ENCODING(){
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
	mTubeVis->SetLightIntensity(SCALE_TUBE_ENCODING);
	mTubeVis->ClearInfluences();
	mTubeVis->SetColorInfluence(1);
	mTubeVis->SetTractsShown(mVisData->GetTractIndices());
	mTubeVis->UpdateBoundingBox();

	mTractVis = mTubeVis;
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
	if (mTubeVis == NULL){
		mTubeVis = new MyTractVisBase;
		mTubeVis->SetTracts(mVisData->GetTracts());
		mTubeVis->SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
		mTubeVis->SetLightIntensity(SCALE_TUBE_BASIC);
		mTubeVis->ComputeGeometry();
		mTubeVis->LoadShader();
		mTubeVis->LoadGeometry();
	}
	mTubeVis->ResetRenderingParameters();
	mTubeVis->SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
	mTubeVis->SetPixelHaloWidth(4);
	mTubeVis->ClearInfluences();
	mTubeVis->SetTractsShown(mVisData->GetTractIndices());
	mTubeVis->UpdateBoundingBox();

	mTractVis = mTubeVis;
}


void MyVisTract::Switch_TUMOR_LINE_DEPTH(){
	Switch_TRACE_LINE_DEPTH();

	AddTumorSphere();
}

void MyVisTract::Switch_TUMOR_TUBE_DEPTH(){
	Switch_TRACE_LINE_DEPTH();

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
	if (mSphereGeometry) delete mSphereGeometry;
	mSphereGeometry = NULL;
	mTractVis->SetSphereGeometry(NULL);
	if (mVisData->GetSpheres().size() > 0){
		if (mVisData->GetVisInfo().GetVisCue() == DEPTH)
			mSphereGeometry = new MySphereDDHGeometry;
		else if (mVisData->GetVisInfo().GetVisCue() == AMBIENT_OCCULUSION)
			mSphereGeometry = new MySphereAOGeometry;
		else mSphereGeometry = new MySphereGeometry;
		mSphereGeometry->SetSphere(&(mVisData->GetSphere(0)));
		mSphereGeometry->SetShaderProgram(mTractVis->GetShaderProgram());
		mSphereGeometry->GenerateGeometry();
		mTractVis->SetSphereGeometry(mSphereGeometry);
	}
}
