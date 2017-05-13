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
	std::vector<MyVec3f> mPoints;
	std::vector<std::vector<float> > mPointScalars;
	std::vector<float> mTrackProperties;

	const MyVec3f& operator[](unsigned int index) const{
		return mPoints[index];
	}

	int Size() const { return mPoints.size(); };
};

struct MyTrackHeader_Trk
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
};

class MyTracks
{
public:

	MyTracks();
	MyTracks(const std::string& filename);
	
	// get functions
	inline const MySingleTrackData& operator[](unsigned int index) const{ return mTracks[index];}
	inline const MySingleTrackData& At(unsigned int index) const{ return mTracks[index]; }
	inline const MyVec3f& At(unsigned int ix, unsigned int iy) const{ return mTracks[ix][iy]; }
	inline const MyTrackHeader_Trk& GetHeader() const { return mHeader; };

	int Read(const std::string& filename);
	int AppendTrackColor(const std::string& filename);
	int Save(const std::string& filename) const;
	int SavePartial(const std::string& filename, const std::vector<int>& saveTrackIdx) const;

	MyTracks Subset(const std::vector<int>& trackIndices) const;
	void AddTracks(const MyTracks& tracks);

	void CopyTracksFrom(const MyTracks& track);

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

	void FilterByTrackLength(const std::vector<int>& inset, float threshold[2], std::vector<int>& outset) const;
	void FilterByIndexSkip(const std::vector<int>& inset, float skipRatio, std::vector<int>& outset) const;

#ifdef RIC
	Point GetPoint(int trackIdx, int pointIdx) const;
#endif
	MyVec3f GetCoord(int trackIdx, int pointIdx) const;
	MyVec3f GetCoord(const MyVec2i& idx) const;
	MyColor4f GetTrackColor(int trackIdx) const;
	float GetValue(int trackIdx, int pointIdx) const;
	MyTensor3f GetTensor(int trackIdx, int pointIdx) const;
	float ComputeTrackLength(int trackIdx) const;

protected:

	std::vector<MySingleTrackData> mTracks;
	MyBoundingBox mBoundingBox;
	MyTrackHeader_Trk mHeader;
};
	

#endif
