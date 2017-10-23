#include "MyColorTextureMaker.h"
#include "MyColorLegend.h"
#include "MyConstants.h"
#include "MyBlackBodyColor.h"
#include "MyBitmap.h"
#include "MyColorConverter.h"
#include "MyTexture.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

MyColorTextureMaker::MyColorTextureMaker()
{
}

MyColorTextureMaker::~MyColorTextureMaker()
{
}

MyArrayui MyColorTextureMaker::MakeColorTextures(){
	MyArrayui textures;
	MyBitmap bitmap;
	MyColorLegend legend;
	//MyTexture::SetInterpolateMethod(GL_NEAREST);
	float lowLu = 38;
	float highLu = 88;
	// isoluminant
	bitmap.Open("..\\SSAO\\data\\isoluminant.bmp");
	legend.SetColorsFromData(bitmap.GetPixelBufferRGB(), 3, bitmap.GetWidth(), bitmap.GetHeight());
	MyArray<MyColor4f> colors = legend.GetColors();
	for (int i = 0; i < colors.size(); i++){
		MyColorConverter::Lab lab = MyColorConverter::rgb2lab(colors[i]);
		lab.l = 50;
		colors[i] = MyColorConverter::lab2rgb(lab);
	}
	legend.SetColors(colors);
	legend.CutFromCenterByArcLength(160.f);
	//legend.CutByLuminance(lowLu, highLu);
	textures << MyTexture::MakeGLTexture(legend.GetColors(), legend.GetColors().size(), 1);

	// gray scale
	colors.clear();
	for (int i = 0; i <= 1000; i++){
		MyColorConverter::Lab lab(i / 10.f, 0, 0);
		colors << MyColorConverter::lab2rgb(lab);
	}
	legend.SetColors(colors);
	//legend.CutByLuminance(lowLu, highLu);
	textures << MyTexture::MakeGLTexture(legend.GetColors(), legend.GetColors().size(), 1);

	// heated body
	legend.SetColorsFromData(&MyBlackBodyColor::mData_1024_3[0][0], 3, 1024, 1);
	legend.CutFromCenterByArcLength(122.f);
	//legend.CutByLuminance(lowLu, highLu);
	textures << MyTexture::MakeGLTexture(legend.GetColors(), legend.GetColors().size(), 1);

	// extended black body
	legend.SetColorsFromData(&MyConstants::BlackBodyExtended[0][0], 3, 1024, 1);
	legend.CutFromCenterByArcLength(200.f);
	//legend.CutByLuminance(lowLu, highLu);
	textures << MyTexture::MakeGLTexture(legend.GetColors(), legend.GetColors().size(), 1);

	// diverging smooth
	MyColorLegend divergingLegend;
	divergingLegend.SetColorsFromData(&MyConstants::DivergingSmooth[0][0], 3, 1024, 1);
	divergingLegend.CutFromCenterByArcLength(122.f);
	//legend.CutByLuminance(lowLu, highLu);
	textures << MyTexture::MakeGLTexture(divergingLegend.GetColors(), divergingLegend.GetColors().size(), 1);

	// cool-warm smooth
	colors = { MyColor4f::blue(), MyColor4f::cyan(), MyColor4f::white(), MyColor4f::yellow(), MyColor4f::red() };
	legend.SetColors(colors);
	colors = legend.Resample(1000);
	for (int i = 0; i < colors.size(); i++){
		MyColorConverter::Lab lab = MyColorConverter::rgb2lab(colors[i]);
		float li = float(i) / (colors.size() - 1);
		MyColor4f c = divergingLegend.GetColorByValue(li);
		MyColorConverter::Lab lab2 = MyColorConverter::rgb2lab(c);
		lab.l = lab2.l;
		colors[i] = MyColorConverter::lab2rgb(lab);
	}
	legend.SetColors(colors);
	legend.CutFromCenterByArcLength(200.f);
	//legend.CutByLuminance(lowLu, highLu);
	textures << MyTexture::MakeGLTexture(legend.GetColors(), legend.GetColors().size(), 1);

	return textures;
}