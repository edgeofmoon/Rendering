#pragma once

#include "MyColor4.h"
#include "MyArray.h"

class MyColorLegend
{
public:
	MyColorLegend();
	~MyColorLegend();

	enum InterpolationMethod{
		RGB_LINEAR = 1,
		LAB_LINEAR = 2,
		HSV_LINEAR = 3,
	};

	void SetColorsFromData(const float* data, int comp, int w, int h, bool horizontal = true);
	void SetColorsFromData(const unsigned char* data, int comp, int w, int h, bool horizontal = true);
	void SetColors(const MyArray<MyColor4f>& c){ mColors = c; };
	void SetInterpolationMethod(InterpolationMethod m){ mInterpolationMethod = m; };
	const MyArray<MyColor4f>& GetColors() const{ return mColors; };
	MyColor4f GetColorByValue(float v) const;
	MyArray<MyColor4f> Resample(int nSample) const;
	void PrintToFileLAB(const MyString& fileName) const;
	void PrintToFileRGB(const MyString& fileName) const;

	float ComputeArcLength() const;
	float ComputeArcLengthInterpolated(int nSample) const;
	void CutFromCenterByArcLength(float arcLength);
	void CutByLuminance(float minLu, float maxLu);
	MyArray<MyColor4f> ComputeCutFromCenterByArcLength(float arcLength);

	static MyColor4f GetColorBlending(const MyColor4f& c1, float a1, 
		const MyColor4f& c2, float a2, InterpolationMethod m = RGB_LINEAR);
	static float ComputeArcLength(const MyArray<MyColor4f>& colors);
	static float GetDeltaE00(const MyColor4f& c1, const MyColor4f& c2);
	static MyArrayf ComputeDeltaE00(const MyArray<MyColor4f>& colors);
	static MyArrayf ComputeDeltaE00(const MyArray<MyColor4f>& colors, const MyColor4f& ref);

protected:
	MyArray<MyColor4f> mColors;
	InterpolationMethod mInterpolationMethod;
};

