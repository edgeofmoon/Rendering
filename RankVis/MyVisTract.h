#pragma once

#include "MyTractVisBase.h"
#include "MyVisData.h"

#include "MySphereGeometry.h"

class MyVisTract
{
public:
	MyVisTract();
	~MyVisTract();

	void Show();
	void Update();
	void Resize(int w, int h);

	void SetVisData(const MyVisData* visData){ mVisData = visData; };
	//void SetColorTexture(unsigned int colorTex){ mColorTexture = colorTex; };
	//void SetSaturationTexture(unsigned int satTex){ mSaturationTexture = satTex; };

	void SetColorTextures(const MyArrayui& c){ mColorTextures = c; };
	void SetValueTextures(const MyArrayui& c){ mValueTextures = c; };

	const MyTractVisBase* GetTractVis() const { return mTractVis; };
	MyTractVisBase* GetTractVis() { return mTractVis; };
	const MyTractVisBase* GetTractVisAux() const { return mTractVis_Aux; };
	MyTractVisBase* GetTractVisAux() { return mTractVis_Aux; };

protected:
	// preset data
	const MyVisData* mVisData;
	//unsigned int mColorTexture;
	//unsigned int mSaturationTexture;
	MyArrayui mColorTextures;
	MyArrayui mValueTextures;
	int mCanvasWidth, mCanvasHeight;

	// derived vis
	MyTractVisBase* mTractVis;
	// for tube+DDH
	MyTractVisBase* mTractVis_Aux;

	// stored geometry
	MyTractVisBase* mTubeVis;
	MyTractVisBase* mLineVis;
	MyTractVisBase* mSuperquadricVis;
	MyTractVisBase* mLineDDHVis;
	MyTractVisBase* mTubeDDHVis;
	MyTractVisBase* mLineAOVis;
	MyTractVisBase* mTubeAOVis;

	// spheres
	MySphereGeometry* mSphereGeometry;

	// switch based on cue / encoding, initialize if not yet
	// FA task
	void Switch_FA_TUBE_COLOR();
	void Switch_FA_TUBE_VALUE();
	void Switch_FA_TUBE_SIZE();
	void Switch_FA_TUBE_TEXTURE();
	void Switch_FA_SUPERQUADRICS_COLOR();
	void Switch_FA_SUPERQUADRICS_VALUE();
	// TRACE task
	void Switch_TRACE_LINE_DEPTH();
	void Switch_TRACE_TUBE_DEPTH();
	void Switch_TRACE_LINE_AO();
	void Switch_TRACE_TUBE_AO();
	void Switch_TRACE_LINE_ENCODING();
	void Switch_TRACE_TUBE_ENCODING();
	void Switch_TRACE_LINE_BASIC();
	void Switch_TRACE_TUBE_BASIC();
	// TUMOR task
	void Switch_TUMOR_LINE_DEPTH();
	void Switch_TUMOR_TUBE_DEPTH();
	void Switch_TUMOR_LINE_AO();
	void Switch_TUMOR_TUBE_AO();
	void Switch_TUMOR_LINE_ENCODING();
	void Switch_TUMOR_TUBE_ENCODING();
	void Switch_TUMOR_LINE_BASIC();
	void Switch_TUMOR_TUBE_BASIC();
	void AddTumorSphere();
};

