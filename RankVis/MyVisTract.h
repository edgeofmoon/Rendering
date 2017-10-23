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
	//void SetSaturationTexture(unsigned int satTex){ mValueTexture = satTex; };

	void SetColorTextures(const MyArrayui& c){ mColorTextures = c; };
	void SetValueTextures(const MyArrayui& c){ mValueTextures = c; };

	const MyArrayui& GetColorTextures() const { return mColorTextures; };
	const MyArrayui& GetValueTextures() const { return mValueTextures; };

	const MyTractVisBase* GetTractVis() const { return mTractVis; };
	MyTractVisBase* GetTractVis() { return mTractVis; };
	const MyTractVisBase* GetTractVisAux() const { return mTractVis_Aux; };
	MyTractVisBase* GetTractVisAux() { return mTractVis_Aux; };

	void SetIgnoreBoxVis(bool b){ mIgnoreBoxVis = b; };
	bool GetIgnoreBoxVis() const { return mIgnoreBoxVis; };

	static void UseNormalizedLighting(bool b);

protected:
	// preset data
	const MyVisData* mVisData;
	//unsigned int mColorTexture;
	//unsigned int mValueTexture;
	MyArrayui mColorTextures;
	MyArrayui mValueTextures;
	int mCanvasWidth, mCanvasHeight;

	// derived vis
	MyTractVisBase* mTractVis;
	// for tube+DDH
	MyTractVisBase* mTractVis_Aux;

	// temporal geometry
	bool mIgnoreBoxVis;
	MyTractVisBase* mBoxTubeVis;
	MyTracks* mBoxTracts;

	// stored geometry
	MyTractVisBase* mTubeVis;
	MyTractVisBase* mLineVis;
	MyTractVisBase* mTubeVisFlatCap;
	MyTractVisBase* mSuperquadricVis;
	MyTractVisBase* mLineDDHVis;
	MyTractVisBase* mTubeDDHVis;
	MyTractVisBase* mLineAOVis;
	MyTractVisBase* mTubeAOVis;

	// light intensity
	static float SCALE_LINE_AO;
	static float SCALE_LINE_BASIC;
	static float SCALE_LINE_DEPTH;
	static float SCALE_LINE_ENCODING;
	static float SCALE_TUBE_AO;
	static float SCALE_TUBE_BASIC;
	static float SCALE_TUBE_DEPTH;
	static float SCALE_TUBE_ENCODING;

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
	// FA_VALUE task
	void Switch_FA_VALUE();

};

