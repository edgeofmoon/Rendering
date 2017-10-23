#pragma once

#include "MyTracks.h"
#include "MySphereGeometry.h"

class MyTractVisBase
{
public:
	MyTractVisBase();
	~MyTractVisBase();

	enum TrackShape{
		TRACK_SHAPE_LINE = 1,
		TRACK_SHAPE_TUBE = 2,
		TRACK_SHAPE_SUPERQUADRIC = 3
	};

	enum CapType{
		CAP_TYPE_ROUND = 1,
		CAP_TYPE_FLAT = 2,
	};

	typedef struct RenderingParameters_t{
		TrackShape Shape;
		CapType CapType;
		int Faces;
		int SuperquadricSkip;
		unsigned int Texture;
		MyColor4f PixelHaloColor;
		float PixelHaloWidth;

		// material
		MyColor4f BaseColor;
		float LightIntensity;
		float Ambient;
		float Diffuse;
		float Specular;
		float Shininess;

		// encoding
		float ColorInfluence;
		float ValueToTextureInfluence;
		float ValueToSizeInfluence;
		float ValueToTextureRatioInfluence;
	}RenderingParameters;

	void SetTracts(const MyTracks* tracts);
	const MyTracks* GetTracts() const { return mTracts; };

	virtual void ComputeGeometry();
	virtual void LoadGeometry();
	virtual void LoadShader();
	virtual void Show();

	static float DefaultTrackRadius;
	// volitile rendering parameters
	float mTrackRadius;
	const RenderingParameters& GetRenderingParamters() const { return mRenderingParameters; };
	void SetRenderingParamters(const RenderingParameters& r){ mRenderingParameters = r; };
	virtual void ResetRenderingParameters();

	int GetNumberFaces() const { return mRenderingParameters.Faces; };
	void SetNumberFaces(int f) { mRenderingParameters.Faces = f; };

	TrackShape GetShape() const { return mRenderingParameters.Shape; };
	void SetShape(TrackShape shape){ mRenderingParameters.Shape = shape; };

	CapType GetCapType() const { return mRenderingParameters.CapType; };
	void SetCapType(CapType ct){ mRenderingParameters.CapType = ct; };

	const MyArrayi& GetTractsShown() { return mFiberToDraw; };
	void SetTractsShown(const MyArrayi& arr){ mFiberToDraw = arr; };

	void SetTexture(unsigned int texture){ mRenderingParameters.Texture = texture; };
	unsigned int GetTexture() const { return mRenderingParameters.Texture; }

	void SetPixelHaloColor(const MyColor4f& c){ mRenderingParameters.PixelHaloColor = c; };
	MyColor4f GetPixelHaloColor() const { return mRenderingParameters.PixelHaloColor; };

	void SetPixelHaloWidth(float w){ mRenderingParameters.PixelHaloWidth = w; };
	float GetPixelHaloWidth() const{ return mRenderingParameters.PixelHaloWidth; };

	void SetSphereGeometry(MySphereGeometry* sg){ mSphereGeometry = sg; };
	MySphereGeometry* GetSphereGeometry() const { return mSphereGeometry; };

	void SetPerTractColor(const MyArray<MyColor4f>* c){ mPerTractColor = c; };
	const MyArray<MyColor4f>* GetPerTractColor() const { return mPerTractColor; };

	void SetBaseColor(const MyColor4f& c){ mRenderingParameters.BaseColor = c; };
	MyColor4f GetBaseColor() const { return mRenderingParameters.BaseColor; };
	void SetLightIntensity(float li){ mRenderingParameters.LightIntensity = li; };
	float GetLightIntensity() const{ return mRenderingParameters.LightIntensity; };
	void SetAmbient(float ab) { mRenderingParameters.Ambient = ab; };
	float GetAmbient() const { return mRenderingParameters.Ambient; };
	void SetDiffuse(float ab) { mRenderingParameters.Diffuse = ab; };
	float GetDiffuse() const { return mRenderingParameters.Diffuse; };

	void SetColorInfluence(float ci){ mRenderingParameters.ColorInfluence = ci; };
	void SetValueToTextureInfluence(float vt){ mRenderingParameters.ValueToTextureInfluence = vt; };
	void SetValueToSizeInfluence(float vs){ mRenderingParameters.ValueToSizeInfluence = vs; };
	void SetValueToTextureRatioInfluence(float vtr){ mRenderingParameters.ValueToTextureRatioInfluence = vtr; };
	void ClearInfluences() {
		mRenderingParameters.ColorInfluence
			= mRenderingParameters.ValueToTextureInfluence
			= mRenderingParameters.ValueToSizeInfluence
			= mRenderingParameters.ValueToTextureRatioInfluence
			= 0;
	};
	void SetToInfluence(int idx);
	MyArrayf GetInfluences() const;
	void SetInfluences(const MyArrayf& influences);

	void UpdateBoundingBox();
	MyBoundingBox GetBoundingBox() const{ return mBoundingBox; };

	virtual inline unsigned int GetShaderProgram() const { return mShaderProgram; };

	void ClearGeometry();
	void DrawGeometry();
	void ShowCapsOnly();

	static RenderingParameters DefaultRenderingParameters;

protected:
	// preset data
	const MyTracks* mTracts;
	RenderingParameters mRenderingParameters;
	MySphereGeometry* mSphereGeometry;
	const MyArray<MyColor4f>* mPerTractColor;
	
	// for geometry
	MyArray3f mVertices;
	MyArray3f mNormals;
	MyArray2f mTexCoords;
	MyArray<MyColor4f> mColors;
	MyArrayf mValues;
	MyArray3i mIndices;
	MyArrayi mLineIndices;
	MyArrayi mIdxOffset;
	MyBoundingBox mBoundingBox;

	// for shader
	unsigned int mShaderProgram;
	unsigned int mVertexArray;
	unsigned int mVertexBuffer;
	unsigned int mNormalBuffer;
	unsigned int mTexCoordBuffer;
	unsigned int mColorBuffer;
	unsigned int mValueBuffer;
	unsigned int mIndexBuffer;
	bool mbScreenSpace;

	int mTexUniform;
	int mNormalAttribute;
	int mPositionAttribute;
	int mTexCoordAttribute;
	int mColorAttribute;
	int mValueAttribute;

	void ComputeTubeGeometry();
	void ComputeLineGeometry();
	void ComputeSuperquadricGeometry();

	void PrintProgress(float current, float all, float step);

	// for filtering
	MyArrayi mFiberToDraw;
	MyArrayb mFiberDraw;

#ifdef RIC
	static void MaskFiber(MyTracks* tracks,
		Array3D<float>* mask, int startIdx, int endIdx);
	static void FiberVolumeDensity(MyTracks* tracks,
		Array3D<atomic<int>>* density, const MyArrayi* indices, int startIdx, int endIdx);

	RicVolume* mFilterVolume;
	unsigned int mFilterVolumeTexture;

public:
	void GetVoxelIndex(const MyVec3f vertex, long &x, long &y, long &z) const;
	void FilterByVolumeMask(Array3D<float>& mask);
	void SetFiberToDraw(const MyArrayi* fiberToDraw);
	void AddVolumeFilter(RicVolume& vol);
	void ToDensityVolume(float* densityVol, int x, int y, int z);
#endif
};

