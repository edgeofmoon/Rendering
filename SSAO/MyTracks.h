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
#include "MyBoundingObject.h"
#include "MyTensor.h"

class MySingleTrackData
{
public:
	MyArray3f mPoints;
	MyArray<MyArrayf> mPointScalars;
	MyArrayf mTrackProperties;

	const MyVec3f& operator[](unsigned int index) const{
		return mPoints[index];
	}

	void CopyFrom(const MySingleTrackData& traj, int st, int ed){
		int n = ed - st + 1;
		mPoints.resize(n);
		mPointScalars.resize(n);
		mTrackProperties = traj.mTrackProperties;
		for (int i = 0; i < n; i++){
			mPoints[i] = traj.mPoints[st + i];
			mPointScalars[i] = traj.mPointScalars[st + i];
		}
	}

	int Size() const { return mPoints.size(); };
};

typedef struct MyTrackHeader_Trk
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
} MyTrackHeader;

class MyTracks
{
public:

	MyTracks();
	MyTracks(const std::string& filename);
	
	// get functions
	inline MyArray<MySingleTrackData>& GetTracts() { return mTracks; };
	inline const MyArray<MySingleTrackData>& GetTracts() const { return mTracks; };
	inline const MySingleTrackData& operator[](unsigned int index) const{ return mTracks[index];}
	inline const MySingleTrackData& At(unsigned int index) const{ return mTracks[index]; }
	inline const MyVec3f& At(unsigned int ix, unsigned int iy) const{ return mTracks[ix][iy]; }
	inline const MyTrackHeader& GetHeader() const { return mHeader; };
	inline MyTrackHeader& GetHeader() { return mHeader; };

	int Read(const std::string& filename);
	int AppendTrackColor(const std::string& filename);
	int Save(const std::string& filename) const;
	int SavePartial(const std::string& filename, const std::vector<int>& saveTrackIdx) const;

	void SetTracts(const MyArray<MySingleTrackData>& t){ mTracks = t; };
	void SetHeader(const MyTrackHeader& header){ mHeader = header; };
	MyTracks Subset(const std::vector<int>& trackIndices) const;
	void AddTracks(const MyTracks& tracks);

	void CopyTracksFrom(const MyTracks& track);
	void BoxSubsetFrom(const MyTracks& tract, const MyBoundingObject& bobj, 
		const MyArrayi& indices, int minSeg = 2);

	int GetNumTracks() const;
	int GetNumVertex(int trackIdx) const;
	MyBoundingBox ComputeBoundingBox(const MyArrayi tractIndices) const;
	MyBoundingBox GetBoundingBox() const { return mBoundingBox; };
	bool IsTractIntersected(const MyBoundingObject& bobj, int tractIndex) const;
	int CountTractsIntersected(const MyBoundingObject& bobj) const;
	int CountTractsIntersected(const MyBoundingObject& bobj, const MyArrayi& indices) const;
	MyArrayi GetTractsIntersected(const MyBoundingObject& bobj) const;
	MyArrayi GetTractsIntersected(const MyBoundingObject& bobj, const MyArrayi& indices) const;
	int CountVertexInBox(const MyBoundingObject& bobj) const;
	int CountVertexInBox(const MyBoundingObject& bobj, const MyArrayi& indices) const;
	MyArray2i GetVertexInBox(const MyBoundingObject& bobj) const;
	MyArray2i GetVertexInBox(const MyBoundingObject& bobj, const MyArrayi& indices) const;
	void GetSampleValueInfo(const MyBoundingObject& bobj, int& nSample, float& valueSum) const;
	void GetSampleValueInfo(const MyBoundingObject& bobj, const MyArrayi& indices, int& nSample, float& valueSum) const;
	void GetSampleClampedValueInfo(const MyBoundingObject& bobj, float minv, float maxv, int& nSample, float& valueSum) const;
	void GetSampleClampedValueInfo(const MyBoundingObject& bobj, float minv, float maxv, const MyArrayi& indices, int& nSample, float& valueSum) const;
	void GetSampleClampedValues(const MyBoundingObject& bobj, float minv, float maxv, MyArrayf& values) const;
	void GetSampleClampedValues(const MyBoundingObject& bobj, float minv, float maxv, const MyArrayi& indices, MyArrayf& values) const;

	void FilterByTrackLength(const std::vector<int>& inset, float threshold[2], std::vector<int>& outset) const;
	void FilterByIndexSkip(const std::vector<int>& inset, float skipRatio, std::vector<int>& outset) const;

#ifdef RIC
	Point GetPoint(int trackIdx, int pointIdx) const;
#endif
	MyVec3f GetCoord(int trackIdx, int pointIdx) const;
	MyVec3f GetCoord(const MyVec2i& idx) const;
	MyColor4f GetTrackColor(int trackIdx) const;
	MyColor4f GetPointColor(int trackIdx, int vIdx) const;
	float GetValue(int trackIdx, int pointIdx) const;
	MyTensor3f GetTensor(int trackIdx, int pointIdx) const;
	float ComputeTrackLength(int trackIdx) const;

protected:

	MyArray<MySingleTrackData> mTracks;
	MyBoundingBox mBoundingBox;
	MyTrackHeader mHeader;
};
	

#endif
