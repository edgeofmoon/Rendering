#include "MyColorLegend.h"
#include "MyColorConverter.h"
#include "CIEDE2000.h"

#include <algorithm>
using namespace std;
using namespace MyColorConverter;

MyColorLegend::MyColorLegend()
{
	mInterpolationMethod = RGB_LINEAR;
}


MyColorLegend::~MyColorLegend()
{
}

void MyColorLegend::SetColorsFromData(
	const unsigned char* data, int comp, int w, int h, bool horizontal){
	int len = (horizontal ? w : h);
	int dx = (horizontal ? 1 : 0);
	int dy = (horizontal ? 0 : 1);
	mColors.resize(len);
	int x = 0, y = 0;
	for (int i = 0; i < len; i++){
		mColors[i].a = 1;
		for (int c = 0; c < comp; c++){
			*((&mColors[i].r) + c) = float(data[(y*w + x)*comp + c]) / 255;
		}
		x += dx; y += dy;
	}
}

void MyColorLegend::SetColorsFromData(
	const float* data, int comp, int w, int h, bool horizontal){
	int len = (horizontal ? w : h);
	int dx = (horizontal ? 1 : 0);
	int dy = (horizontal ? 0 : 1);
	mColors.resize(len);
	int x = 0, y = 0;
	for (int i = 0; i < len; i++){
		memcpy(&(mColors[i].r), &(data[(y*w + x)*comp]), comp*sizeof(float));
		x += dx; y += dy;
	}
}

MyColor4f MyColorLegend::GetColorByValue(float v) const{
	float nv = max(min(1.f, v), 0.f);
	int w = mColors.size()-1;
	int w1 = nv*w, w2 = min(nv*w + 1.f, float(w));
	float a = w2 - nv*w, b = 1 - a;
	MyColor4f c1 = mColors[w1];
	MyColor4f c2 = mColors[w2];
	return GetColorBlending(c1, a, c2, b, mInterpolationMethod);
}

MyArray<MyColor4f> MyColorLegend::Resample(int nSample) const{
	MyArray<MyColor4f> rst(nSample);
	for (int i = 0; i < nSample; i++){
		float v = i / float(nSample - 1);
		if (nSample == mColors.size() && i == nSample - 1){
			int debug = 1;
		}
		rst[i] = GetColorByValue(v);
	}
	return rst;
}

MyColor4f MyColorLegend::GetColorBlending(
	const MyColor4f& c1, float a1, const MyColor4f& c2, float a2, InterpolationMethod m){
	if (m == RGB_LINEAR){
		return c1*a1 + c2*a2;
	}
	else if (m == LAB_LINEAR){
		MyColorConverter::Lab lab1 = MyColorConverter::rgb2lab(c1);
		MyColorConverter::Lab lab2 = MyColorConverter::rgb2lab(c2);
		MyColorConverter::Lab lab;
		lab.l = lab1.l*a1 + lab2.l*a2;
		lab.a = lab1.a*a1 + lab2.a*a2;
		lab.b = lab1.b*a1 + lab2.b*a2;
		MyColor4f c = MyColorConverter::lab2rgb(lab.l, lab.a, lab.b);
		c.a = c1.a*a1 + c2.a*a2;
		return c;
	}
	else if (m == HSV_LINEAR){
		/*
		float hsv1[] = { c1.getH(), c1.getSaturation(), c1.getV() };
		float hsv2[] = { c2.getH(), c2.getSaturation(), c2.getV() };
		float hsv[] = {
			hsv1[0] * a1 + hsv2[0] * a2,
			hsv1[1] * a1 + hsv2[1] * a2,
			hsv1[2] * a1 + hsv2[2] * a2
		};
		MyColor4f c;
		c.fromHSV(hsv[0], hsv[1], hsv[2]);
		c.a = c1.a*a1 + c2.a*a2;
		return c;
		*/
		MyColorConverter::Hsv hsv1 = MyColorConverter::rgb2hsv(c1);
		MyColorConverter::Hsv hsv2 = MyColorConverter::rgb2hsv(c2);
		MyColorConverter::Hsv hsv = {
			hsv1.h * a1 + hsv2.h * a2,
			hsv1.s * a1 + hsv2.s * a2,
			hsv1.v * a1 + hsv2.v * a2
		};
		MyColor4f c = MyColorConverter::hsv2rgb(hsv);
		c.a = c1.a*a1 + c2.a*a2;
		return c;
	}
	else return MyColor4f();
}

float MyColorLegend::ComputeArcLength() const{
	return ComputeArcLength(mColors);
}

float MyColorLegend::ComputeArcLengthInterpolated(int nSample) const{
	if (nSample < 2) return 0;
	MyArray<MyColor4f> samples = Resample(nSample);
	return ComputeArcLength(samples);
}

float MyColorLegend::ComputeArcLength(const MyArray<MyColor4f>& colors){
	if (colors.size() < 2) return 0;
	float rst = 0;
	for (int i = 1; i < colors.size(); i++){
		rst += GetDeltaE00(colors[i - 1], colors[i]);
	}
	return rst;
}

float MyColorLegend::GetDeltaE00(const MyColor4f& c1, const MyColor4f& c2){
	MyColorConverter::Lab lab1 = MyColorConverter::rgb2lab(rgba(c1.r, c1.g, c1.b));
	MyColorConverter::Lab lab2 = MyColorConverter::rgb2lab(rgba(c2.r, c2.g, c2.b));
	CIEDE2000::LAB lab001 = { lab1.l, lab1.a, lab1.b };
	CIEDE2000::LAB lab002 = { lab2.l, lab2.a, lab2.b };
	float dist = CIEDE2000::CIEDE2000(lab001, lab002);
	return dist;
}

MyArrayf MyColorLegend::ComputeDeltaE00(const MyArray<MyColor4f>& colors){
	MyArrayf rst;
	for (int i = 1; i < colors.size(); i++){
		rst << GetDeltaE00(colors[i - 1], colors[i]);
	}
	return rst;
}