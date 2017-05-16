#include "MyColorLegend.h"
#include "MyConstants.h"
#include "MyBlackBodyColor.h"
#include "MyArray.h"
#include "MyBitmap.h"
#include "MyGraphicsTool.h"
#include "MyTexture.h"
#include "MyColorConverter.h"
#include "MyTracks.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
using namespace std;

int windowWidth = 800;
int windowHeight = 800;

unsigned int colorTex;
MyArrayf colorDists;
MyArrayf luminances;

const int maxMethodIdx = 2;
const int maxSampleIdx = 9;

int colorIdx = 0;
int methodIdx = 1;
int sampleIdx = 0;

MyArray<MyColorLegend> legends;
MyArrayStr names = { "Black Body Color", "Extened Black Body", "Isoluminant Map", "True Isoluminant Map",
"Monoluminant Map", "Monoluminant Extended", "Linearluminant Map", "Diverging Smooth",
"Diverging Bent", "Saturation_HSV" };
MyArrayi nSamples = { 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600 };
MyArrayStr methodNames = { "RGB", "LAB", "HSV" };
MyArray<MyColorLegend::InterpolationMethod> methods = {
	MyColorLegend::RGB_LINEAR, MyColorLegend::LAB_LINEAR,
	MyColorLegend::HSV_LINEAR
};

void reload();

void print(char c, int n){
	while (n--){
		cout << c;
	}
}

void RenderTexture(int texture, int x, int y, int width, int height){
	MyGraphicsTool::PushAllAttributes();
	MyGraphicsTool::SetViewport(MyVec4i(x, y, width, height));
	MyGraphicsTool::PushProjectionMatrix();
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::LoadProjectionMatrix(&MyMatrixf::OrthographicMatrix(-1, 1, -1, 1, 1, 10));
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	MyGraphicsTool::EnableTexture2D();
	MyGraphicsTool::BindTexture2D(texture);
	MyGraphicsTool::BeginTriangleFan();
	MyGraphicsTool::TextureCoordinate(MyVec2f(0, 0));
	MyGraphicsTool::Color(MyColor4f(1, 1, 1));
	MyGraphicsTool::Vertex(MyVec3f(-1, -1, -5));
	MyGraphicsTool::TextureCoordinate(MyVec2f(1, 0));
	//MyGraphicsTool::Color(MyColor4f(1, 0, 0));
	MyGraphicsTool::Vertex(MyVec3f(1, -1, -5));
	MyGraphicsTool::TextureCoordinate(MyVec2f(1, 1));
	//MyGraphicsTool::Color(MyColor4f(1, 1, 0));
	MyGraphicsTool::Vertex(MyVec3f(1, 1, -5));
	MyGraphicsTool::TextureCoordinate(MyVec2f(0, 1));
	//MyGraphicsTool::Color(MyColor4f(0, 1, 0));
	MyGraphicsTool::Vertex(MyVec3f(-1, 1, -5));
	MyGraphicsTool::EndPrimitive();
	MyGraphicsTool::BindTexture2D(0);
	MyGraphicsTool::DisableTexture2D();
	MyGraphicsTool::PopMatrix();
	MyGraphicsTool::PopProjectionMatrix();
	MyGraphicsTool::PopAttributes();
}

void Update(){
	system("cls");
	legends[colorIdx].SetInterpolationMethod(methods[methodIdx]);
	print('*', 10);
	cout << colorIdx + 1 << "-" << names[colorIdx];
	print('*', 10);
	cout << endl;
	cout << "Interpolation: " << methodNames[methodIdx] << endl;
	MyArray<MyColor4f> colors;
	if (sampleIdx > 1){
		colors = legends[colorIdx].Resample(nSamples[sampleIdx - 2]);
		cout << "Sample: " << nSamples[sampleIdx - 2] << endl;
	}
	else if (sampleIdx > 0){
		int nSample = legends[colorIdx].GetColors().size();
		colors = legends[colorIdx].Resample(nSample);
		cout << "Sample: " << nSample << endl;
	}
	else {
		colors = legends[colorIdx].GetColors();
		cout << "Raw: " << legends[colorIdx].GetColors().size() << endl;
	}
	cout << "Arc length (CIEDE00): " << MyColorLegend::ComputeArcLength(colors) << endl;
	colorDists = MyColorLegend::ComputeDeltaE00(colors);
	luminances.resize(colors.size());
	for (int i = 0; i < colors.size(); i++) 
		luminances[i] = MyColorConverter::rgb2lab(colors[i]).l;
	if (glIsTexture(colorTex)) glDeleteTextures(1, &colorTex);
	colorTex = MyTexture::MakeGLTexture(colors, colors.size(), 1);
}

/**************************************** GLUT Callback ********************/
void myGlutDisplay(){
	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	if (glIsTexture(colorTex)){
		RenderTexture(colorTex, 0, 0, windowWidth, windowHeight/2);

		// DELTAE00
		glColor4f(0, 0, 0, 1);
		for (int i = 0; i < colorDists.size(); i++){
			float h = colorDists[i];
			glBegin(GL_LINES);
			glVertex2f(float(i + 0.5f) / colorDists.size() * windowWidth, windowHeight / 2);
			glVertex2f(float(i + 0.5f) / colorDists.size() * windowWidth, windowHeight / 2 + h * 200);
			glEnd();
		}

		// luminance
		glColor4f(1, 0, 0, 1);
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < luminances.size(); i++){
			glVertex2f(float(i) / (luminances.size() - 1) * windowWidth, (luminances[i] / 100 + 1) * windowHeight / 2);
		}
		glEnd();
	}
	glPopAttrib();
	glutSwapBuffers();
}

void myGlutSpecialInput(int key, int x, int y)
{
}

void myGlutMenu(int value){

}

void myGlutMouseWheel(int button, int dir, int x, int y){
}

void myGlutMouse(int button, int state, int x, int y){
}

void myGlutMotion(int x, int y){
}

void myGlutPassiveMotion(int x, int y){
}

void myGlutReshape(int x, int y){
	windowWidth = x;
	windowHeight = y;
	glViewport(0, 0, x, y);
	MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(0,x,0,y, 0, 1);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
}


void myGlutKeyboard(unsigned char key, int x, int y){
	if (key >= '0' && key <= '9'){
		sampleIdx = key - '0';
		Update();
	}
	else{
		switch (key){
			case 'a':
			case 'A':
				colorIdx--;
				if (colorIdx < 0) colorIdx = legends.size() - 1;
				Update();
				break;
			case 'd':
			case 'D':
				colorIdx++;
				if (colorIdx > legends.size() - 1) colorIdx = 0;
				Update();
				break;
			case 'w':
			case 'W':
				methodIdx--;
				if (methodIdx < 0) methodIdx = maxMethodIdx;
				Update();
				break;
			case 's':
			case 'S':
				methodIdx++;
				if (methodIdx > maxMethodIdx) methodIdx = 0;
				Update();
				break;
			case 'q':
			case 'Q':
				sampleIdx--;
				if (sampleIdx < 0) sampleIdx = 0;
				Update();
				break;
			case 'e':
			case 'E':
				sampleIdx++;
				if (sampleIdx > maxSampleIdx) sampleIdx = maxSampleIdx;
				Update();
				break;
			case 'f':
			case 'F':
				legends[colorIdx].CutFromCenterByArcLength(160.f);
				sampleIdx = 0;
				Update();
				break;
			case 'r':
			case 'R':
				reload();
				Update();
				break;
		}
	}
	glutPostRedisplay();
}

int init(int argc, char* argv[]){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize(windowWidth, windowHeight);
	int main_window = glutCreateWindow("ColorTest");
	glewInit();
	glutDisplayFunc(myGlutDisplay);
	glutMotionFunc(myGlutMotion);
	glutReshapeFunc(myGlutReshape);
	glutPassiveMotionFunc(myGlutPassiveMotion);
	glutMouseWheelFunc(myGlutMouseWheel);
	glutMouseFunc(myGlutMouse);
	glutKeyboardFunc(myGlutKeyboard);
	glutSpecialFunc(myGlutSpecialInput);
	return 1;
}

void PrintColorInfo(){
	for (int i = 0; i < methods.size(); i++){
		print('*', 10);
		cout << methodNames[i];
		print('*', 10);
		cout << endl;
		for (int leg = 0; leg < legends.size(); leg++){
			legends[leg].SetInterpolationMethod(methods[i]);
			cout << names[leg] << ": " << endl
				<< legends[leg].GetColors().size() << " Points: "
				<< legends[leg].ComputeArcLength() << endl;

			cout << legends[leg].GetColors().size() << " Samples: "
				<< legends[leg].ComputeArcLengthInterpolated(legends[leg].GetColors().size()) << endl;

			for (int j = 0; j < nSamples.size(); j++){
				cout << nSamples[j] << " Samples: "
					<< legends[leg].ComputeArcLengthInterpolated(nSamples[j]) << endl;
			}
			cout << endl;
		}
	}
}

void tractStats(){
	MyTracks tracts;
	tracts.Read("C:\\Users\\GuohaoZhang\\Dropbox\\data\\normal_s3_tensorboy_RevZ.trk");
	MyString tab("\t");
	ofstream ofile;
	/*
	ofile.open("stats.txt");
	ofile << "FA_R" << tab
		<< "FA_T" << tab
		<< "EVAL1" << tab
		<< "EVAL2" << tab
		<< "EVAL3" << endl;
	for (int i = 0; i < tracts.GetNumTracks(); i++){
		for (int j = 0; j < tracts.GetNumVertex(i); j++){
			MyTensor3f tensor = tracts.GetTensor(i, j);
			ofile << tracts.GetValue(i, j) << tab
				<< tensor.GetFA() << tab
				<< tensor.GetEigenValue(0) << tab
				<< tensor.GetEigenValue(1) << tab
				<< tensor.GetEigenValue(2) << endl;
		}
	}
	ofile.close();
	*/
	ofile.open("segLen.txt");
	ofile << "SegLength" + tab + "LinearAni" + tab + "PlanAni" + tab + "SphereAni" << endl;
	for (int i = 0; i < tracts.GetNumTracks(); i++){
		for (int j = 1; j < tracts.GetNumVertex(i); j++){
			MyTensor3f tensor = tracts.GetTensor(i, j);
			ofile << (tracts.GetCoord(i, j) - tracts.GetCoord(i, j - 1)).norm() << tab
				<< tensor.GetLinearAnisotropy() << tab
				<< tensor.GetPlanarAnisotropy() << tab
				<< tensor.GetSphericalAnisotropy() << endl;
		}
	}
	ofile.close();
}

void reload(){
	MyBitmap bitmap;
	int idx = 0;
	legends[idx++].SetColorsFromData(&MyBlackBodyColor::mData_1024_3[0][0], 3, 1024, 1);
	legends[idx++].SetColorsFromData(&MyConstants::BlackBodyExtended[0][0], 3, 1024, 1);
	bitmap.Open("..\\SSAO\\data\\isoluminant.bmp");
	legends[idx++].SetColorsFromData(bitmap.GetPixelBufferRGB(), 3, bitmap.GetWidth(), bitmap.GetHeight());
	//legends[1].SetColorsFromData(&MyConstants::IsoluminanceMap[0][0], 3, 33, 1);
	MyArray<MyColor4f> colors = legends[idx - 1].GetColors();
	for (int i = 0; i < colors.size(); i++){
		MyColorConverter::Lab lab = MyColorConverter::rgb2lab(colors[i]);
		lab.l = 50;
		colors[i] = MyColorConverter::lab2rgb(lab);
	}
	legends[idx++].SetColors(colors);
	//bitmap.Open("..\\SSAO\\data\\monoLuminance.bmp");
	//legends[3].SetColorsFromData(bitmap.GetPixelBufferRGB(), 3, bitmap.GetWidth(), bitmap.GetHeight());
	legends[idx++].SetColorsFromData(&MyConstants::MonoluminanceMap[0][0], 3, 1024, 1);
	legends[idx++].SetColorsFromData(&MyConstants::MonoluminanceExtended[0][0], 3, 1024, 1);
	colors = legends[idx - 2].GetColors();
	for (int i = 0; i < colors.size(); i++){
		MyColorConverter::Lab lab = MyColorConverter::rgb2lab(colors[i]);
		lab.l = float(i) / (colors.size() - 1) * 100;
		colors[i] = MyColorConverter::lab2rgb(lab);
	}
	legends[idx++].SetColors(colors);
	legends[idx++].SetColorsFromData(&MyConstants::DivergingSmooth[0][0], 3, 1024, 1);
	legends[idx++].SetColorsFromData(&MyConstants::DivergingBent[0][0], 3, 1024, 1);

	int nhsv = colors.size();
	MyArrayf hsvs;
	for (int i = 0; i < nhsv; i++){
		MyColorConverter::Hsv hsv(10, i / float(nhsv - 1), 1.f);
		MyColor4f c = MyColorConverter::hsv2rgb(hsv);
		hsvs << c.r << c.g << c.b;
	}
	legends[idx++].SetColorsFromData(&hsvs[0], 3, 1024, 1);
}

int main(int argc, char* argv[]){
	//tractStats();
	//return 1;
	init(argc, argv);
	MyTexture::SetInterpolateMethod(GL_NEAREST);
	legends.resize(10);
	reload();
	Update();
	glutMainLoop();
}
