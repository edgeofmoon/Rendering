#include "MyColorLegend.h"
#include "MyConstants.h"
#include "MyBlackBodyColor.h"
#include "MyArray.h"
#include "MyBitmap.h"
#include "MyPrimitiveDrawer.h"
#include "MyTexture.h"
#include "MyColorConverter.h"
#include "MyTracks.h"
#include "MyTractVisBase.h"
#include "MyTrackBall.h"
#include "MyVisData.h"
#include "MyFrameBuffer.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
using namespace std;
using namespace MyVisEnum;

int windowWidth = 650;
int windowHeight = 800;
float tractSplit = 0.8;
float dsr = 2;

unsigned int colorTex;
MyArrayf colorDists;
MyArrayf luminances;


int colorIdx = 0;
int methodIdx = 1;
int sampleIdx = 0;

MyTracks* tracts = NULL;
MyTractVisBase* tractVis = NULL;
MyTrackBall trackBall;
MyArrayi tractIndices;
MyFrameBuffer fb;

MyArray<MyColorLegend> legends;
MyArrayStr names;
MyArrayi nSamples = { 8, 16, 128, 1000 };
MyArrayStr methodNames = { "RGB", "LAB", "HSV" };
MyArray<MyColorLegend::InterpolationMethod> methods = {
	MyColorLegend::RGB_LINEAR, MyColorLegend::LAB_LINEAR,
	MyColorLegend::HSV_LINEAR
};
const int maxMethodIdx = 2;
const int maxSampleIdx = nSamples.size() + 1;

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
	colorDists = MyColorLegend::ComputeDeltaE00(colors, colors[0]);
	luminances.resize(colors.size());
	for (int i = 0; i < colors.size(); i++)
		luminances[i] = MyColorConverter::rgb2lab(colors[i]).l;
	if (glIsTexture(colorTex)) glDeleteTextures(1, &colorTex);
	colorTex = MyTexture::MakeGLTexture(colors, colors.size(), 1);
	tractVis->SetTexture(colorTex);
}

void drawTracts(){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	MyVec4i viewport = MyGraphicsTool::GetViewport();

	glBindFramebuffer(GL_FRAMEBUFFER, fb.GetFrameBuffer());
	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	int w = fb.GetWidth();
	int h = fb.GetHeight();
	glViewport(0, 0, w, h);
	//MyMatrixf projectionMatrix = MyMatrixf::PerspectiveMatrix(60, w / (float)h, 1, 300);
	MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-w / 10, w / 10, -h / 10, h / 10, -500, 500);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	gluLookAt(0, 0, 150, 0, 0, 0, 0, 1, 0);
	glPushMatrix();
	MyGraphicsTool::LoadTrackBall(&trackBall);
	MyBoundingBox box = tractVis->GetTracts()->GetBoundingBox();
	MyGraphicsTool::Translate(-box.GetCenter());
	tractVis->Show();
	glPopMatrix();
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	MyGraphicsTool::SetViewport(viewport);
	MyPrimitiveDrawer::DrawTextureOnViewport(fb.GetColorTexture());
	glPopAttrib();
}

void drawColorMaps(){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	MyGraphicsTool::PushProjectionMatrix();
	MyGraphicsTool::PushMatrix();
	if (!glIsTexture(colorTex)) return;
	MyVec4i viewport = MyGraphicsTool::GetViewport();
	int h = viewport[3] / 4;
	int hh = viewport[3] - h;
	RenderTexture(colorTex, viewport[0], viewport[1], viewport[2], h);

	MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(viewport[0], viewport[2], viewport[1], viewport[3], 0, 1);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	// DELTAE00
	glColor4f(0, 0, 1, 1);
	float maxH = 0;
	for (int i = 0; i < colorDists.size(); i++){
		maxH = max(maxH, colorDists[i]);
	}
	for (int i = 0; i < colorDists.size(); i++){
		float d = colorDists[i] / maxH;
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(float(i) / colorDists.size() * viewport[2], h);
		glVertex2f(float(i + 1) / colorDists.size() * viewport[2], h);
		glVertex2f(float(i + 1) / colorDists.size() * viewport[2], h + hh*d);
		glVertex2f(float(i) / colorDists.size() * viewport[2], h + hh*d);
		glEnd();
	}

	MyGraphicsTool::PopMatrix();
	MyGraphicsTool::PopProjectionMatrix();
	glPopAttrib();
}

void drawLabTraj(){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glClear(GL_DEPTH_BUFFER_BIT);
	MyVec4i viewport = MyGraphicsTool::GetViewport();
	MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-150, 150, -10, 110, -1100, 1100);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	glPushMatrix();
	glLoadIdentity();
	gluLookAt(60, 60, 25, 0, 0, 0, 0, 0, 1);
	MyGraphicsTool::LoadTrackBall(&trackBall);
	MyArray<MyColor4f> rgbs = legends[colorIdx].GetColors();
	MyArray3f labs;
	MyVec3f minLab(FLT_MAX, FLT_MAX, FLT_MAX);
	MyVec3f maxLab = -minLab;
	for (int i = 0; i < rgbs.size(); i++){
		MyColorConverter::Lab lab = MyColorConverter::rgb2lab(rgbs[i]);
		labs << MyVec3f(lab.l, lab.a, lab.b);
		minLab = MyVec3f::min(minLab, labs.back());
		maxLab = MyVec3f::max(maxLab, labs.back());
	}


	glLineWidth(3);
	glBegin(GL_LINE_STRIP);
	for (int i = -100; i <= 100; i++){
		MyColorConverter::Lab lab(50, i, 0);
		MyGraphicsTool::Color(MyColorConverter::lab2rgb(lab));
		glVertex3f(i, 0, 50);
	}
	glEnd();
	glColor3f(0, 0, 0);
	//MyPrimitiveDrawer::DrawBitMapTextLarge(MyVec3f(100, 0, 50), "a", 2);
	glBegin(GL_LINE_STRIP);
	for (int i = -100; i <= 100; i++){
		MyColorConverter::Lab lab(50, 0, i);
		MyGraphicsTool::Color(MyColorConverter::lab2rgb(lab));
		glVertex3f(0, i, 50);
	}
	glEnd();
	glColor3f(0, 0, 0);
	//MyPrimitiveDrawer::DrawBitMapTextLarge(MyVec3f(0, 100, 50), "b", 0);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i <= 100; i++){
		MyColorConverter::Lab lab(i, 0, 0);
		MyGraphicsTool::Color(MyColorConverter::lab2rgb(lab));
		glVertex3f(0, 0, i);
	}
	glEnd();
	glColor3f(0, 0, 0);
	//MyPrimitiveDrawer::DrawBitMapTextLarge(MyVec3f(0, 0, 100), "L", 1);

	glLineWidth(5);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < labs.size(); i++){
		MyPrimitiveDrawer::Color(rgbs[i]);
		glVertex3f(labs[i][1], labs[i][2], labs[i][0]);
	}
	glEnd();
	glPopAttrib();
}

/**************************************** GLUT Callback ********************/
void myGlutDisplay(){
	int wheight_tract = windowHeight*tractSplit;
	int wheight_color = windowHeight - wheight_tract;
	int wheight_lab = windowHeight*0.15;
	int wwidth_lab = wheight_lab;

	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, wheight_color, windowWidth, wheight_tract);
	//drawTracts();

	//glViewport(windowWidth - wwidth_lab, wheight_color, wwidth_lab, wheight_lab);
	drawLabTraj();

	glViewport(0, 0, windowWidth, wheight_color);
	drawColorMaps();

	glutSwapBuffers();
}

void myGlutSpecialInput(int key, int x, int y)
{
}

void myGlutMenu(int value){

}

void myGlutMouseWheel(int button, int dir, int x, int y){
	if (dir > 0){
		trackBall.ScaleMultiply(1.05);
	}
	else{
		trackBall.ScaleMultiply(1 / 1.05);
	}
	glutPostRedisplay();
}

void myGlutMouse(int button, int state, int x, int y){
	if (button == GLUT_LEFT_BUTTON){
		if (state == GLUT_DOWN){
			trackBall.StartRotation(x, y);
		}
		else if (state == GLUT_UP){
			trackBall.EndRotation(x, y);
		}
	}
	else{
		trackBall.SetTranslateScale(0.1, 0.1);
		if (state == GLUT_DOWN){
			trackBall.StartTranslation(x, y);
		}
		else if (state == GLUT_UP){
			trackBall.EndTranslation(x, y);
		}
	}
	glutPostRedisplay();
}

void myGlutMotion(int x, int y){
	trackBall.Motion(x, y);
	glutPostRedisplay();
}

void myGlutPassiveMotion(int x, int y){
}

void myGlutReshape(int x, int y){
	windowWidth = x;
	windowHeight = y;
	glViewport(0, 0, x, y);
	MyMatrixf projectionMatrix = MyMatrixf::PerspectiveMatrix(60, x / (float)y, 1, 300);
	//MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-100,100,-100,100, 1, 200);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	gluLookAt(0, 0, 150, 0, 0, 0, 0, 1, 0);
	trackBall.Reshape(x, y);
	//trackBall.ResetRotate();
	//trackBall.ResetScale();

	int w = x*dsr;
	int h = y*dsr*tractSplit;
	if (fb.GetWidth() != w || fb.GetHeight() != h){
		fb.SetSize(w, h);
		fb.Build();
	}

	glutPostRedisplay();
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
		case 'h':
		case 'H':
			trackBall.ResetRotate();
			trackBall.ResetScale();
			trackBall.ResetTranslate();
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
	glEnable(GL_DEPTH_TEST);
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


void reload(){
	MyBitmap bitmap;
	int idx = 0;
	names = {"True Isoluminant Map", "Extended Black body"};
	legends.resize(names.size());
	bitmap.Open("..\\SSAO\\data\\isoluminant.bmp");
	legends[0].SetColorsFromData(bitmap.GetPixelBufferRGB(), 3, bitmap.GetWidth(), bitmap.GetHeight());
	MyArray<MyColor4f> colors = legends[0].GetColors();
	for (int i = 0; i < colors.size(); i++){
		MyColorConverter::Lab lab = MyColorConverter::rgb2lab(colors[i]);
		lab.l = 50;
		colors[i] = MyColorConverter::lab2rgb(lab);
	}
	legends[0].SetColors(colors);
	legends[1].SetColorsFromData(&MyConstants::BlackBodyExtended[0][0], 3, 1024, 1);

	tracts = new MyTracks();
	tracts->Read("C:\\Users\\GuohaoZhang\\Dropbox\\data\\normal_s3_tensorboy_RevZ.trk");
	MyVisData visData(MyVisInfo(false, false, TRACE, VALUE, 0, TUBE, BASIC, CC, BUNDLE, 0, 3));
	visData.SetTracts(tracts);
	visData.LoadFromDirectory("C:\\Users\\GuohaoZhang\\Dropbox\\data\\traces");
	tractIndices = visData.GetTractIndices();

	tractVis = new MyTractVisBase();
	tractVis->SetTracts(tracts);
	tractVis->LoadShader();
	tractVis->ComputeGeometry();
	tractVis->LoadGeometry();
	tractVis->ClearInfluences();
	tractVis->SetValueToTextureInfluence(1);
	tractVis->SetTractsShown(tractIndices);
}

int main(int argc, char* argv[]){
	//tractStats();
	//return 1;
	init(argc, argv);
	MyTexture::SetInterpolateMethod(GL_NEAREST);
	reload();
	Update();
	glutMainLoop();
}
