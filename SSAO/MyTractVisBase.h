#pragma once

#include "MyTracks.h"

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
	void SetTracts(const MyTracks* tracts);
	const MyTracks* GetTracts() const { return mTracts; };

	virtual void ComputeGeometry();
	virtual void LoadGeometry();
	virtual void LoadShader();
	virtual void Show();

	// volitile rendering parameters
	float mTrackRadius;
	void ResetRenderingParameters();


	int GetNumberFaces() const { return mFaces; };
	void SetNumberFaces(int f) { mFaces = f; };

	TrackShape GetShape() const { return mShape; };
	void SetShape(TrackShape shape){ mShape = shape; };

	const MyArrayi& GetTractsShown() { return mFiberToDraw; };
	void SetTractsShown(const MyArrayi& arr){ mFiberToDraw = arr; };

	void ClearGeometry();
	void ShowCapsOnly();

protected:
	// preset data
	const MyTracks* mTracts;
	TrackShape mShape;
	int mFaces;
	int mSuperquadricSkip;

	// for geometry
	MyArray3f mVertices;
	MyArray3f mNormals;
	MyArray2f mTexCoords;
	MyArray<MyColor4f> mColors;
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
	unsigned int mIndexBuffer;
	bool mbScreenSpace;

	int mTexUniform;
	int mNormalAttribute;
	int mPositionAttribute;
	int mTexCoordAttribute;
	int mColorAttribute;

	void ComputeTubeGeometry();
	void ComputeLineGeometry();
	void ComputeSuperquadricGeometry();

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

