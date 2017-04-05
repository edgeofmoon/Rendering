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

	// volitile rendering parameters
	float mTrackRadius;
	void ResetRenderingParameters();

protected:
	// preset data
	const MyTracks* mTracts;


	TrackShape mShape;
	// for geometry
	int mFaces;
	MyArray3f mVertices;
	MyArray3f mNormals;
	//MyArray2f mTexCoords;
	//MyArrayf mRadius;
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
	//unsigned int mTexCoordBuffer;
	//unsigned int mRadiusBuffer;
	unsigned int mColorBuffer;
	unsigned int mIndexBuffer;
	bool mbScreenSpace;

	int mTexUniform;
	int mNormalAttribute;
	int mPositionAttribute;
	//int mTexCoordAttribute;
	//int mRadiusAttribute;
	int mColorAttribute;

	void ComputeTubeGeometry();
	virtual void ComputeLineGeometry();

public:
	int GetNumberFaces() const { return mFaces; };
	void SetNumberFaces(int f);
	void ClearGeometry();
	virtual void ComputeGeometry();
	virtual void LoadGeometry();
	virtual void LoadShader();
	virtual void Show();
	void ShowCapsOnly();
	void SetShape(TrackShape shape){ mShape = shape; };
	TrackShape GetShape() const { return mShape; };

	// for filtering
protected:
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

