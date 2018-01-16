#ifndef MYTRACK_H
#define MYTRACK_H

#include <string>
#include <vector>

#ifdef RIC
#include "RicPoint.h"
#include "RicVolume.h"
#include "Array3D.h"
#include <atomic>
#endif

#include "MyArray.h"
#include "MyColor4.h"
#include "MyVec.h"
#include "MyBoundingBox.h"

class MySingleTrackData
{
public:
	int mSize;
	std::vector<MyVec3f> mPoints;
	std::vector<std::vector<float> > mPointScalars;
	std::vector<float> mTrackProperties;
};

class MyTracks
{
public:

	MyTracks();

	MyTracks(const std::string& filename);
	
	int Read(const std::string& filename);
	
	int Save(const std::string& filename) const;
	int SavePartial(const std::string& filename, const std::vector<int>& saveTrackIdx) const;

	MyTracks Subset(const std::vector<int>& trackIndices) const;
	void AddTracks(const MyTracks& tracks);

	void CopyTracksFrom(const MyTracks& track);

	int GetNumTracks() const;
	int GetNumVertex(int trackIdx) const;
	MyBoundingBox ComputeBoundingBox(const MyArrayi tractIndices) const;
	MyBoundingBox GetBoundingBox() const { return mBoundingBox; };
	void GetSampleValueInfo(MyBoundingBox& box, int& nSample, float& valueSum) const;
	void GetSampleValueInfo(MyBoundingBox& box, const MyArrayi& indices, int& nSample, float& valueSum) const;

	void FilterByTrackLength(const std::vector<int>& inset, float threshold[2], std::vector<int>& outset) const;
	void FilterByIndexSkip(const std::vector<int>& inset, float skipRatio, std::vector<int>& outset) const;

#ifdef RIC
	Point GetPoint(int trackIdx, int pointIdx) const;
#endif
	MyVec3f GetCoord(int trackIdx, int pointIdx) const;
	float GetValue(int trackIdx, int pointIdx) const;
	float ComputeTrackLength(int trackIdx) const;

	// rendering parameters
	float mTrackRadius;
	void ResetRenderingParameters();

	MyVec3f mBoxOffset0, mBoxOffset1;
protected:
	struct MyTrackHeader
	{
		char id_string[6];
		short dim[3];
		float voxel_size[3];
		float origin[3];
		short n_scalars;
		char scalar_name[10][20];
		short n_properties; 
		char property_name[10][20];
		float vox_to_ras[4][4];
		char reserved[444];
		char voxel_order[4];
		char pad2[4];
		float image_orientation_patient[6];
		char pad1[2];
		unsigned char invert_x;
		unsigned char invert_y;
		unsigned char invert_z;
		unsigned char swap_xy;
		unsigned char swap_yz;
		unsigned char swap_zx;
		int n_count;
		int version;
		int hdr_size; 
	} mHeader;
	
	std::vector<MySingleTrackData> mTracks;

public:
	enum TrackShape{
		TRACK_SHAPE_LINE = 1,
		TRACK_SHAPE_TUBE = 2
	};

protected:
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
	

#endif
