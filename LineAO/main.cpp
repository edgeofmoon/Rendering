

//#define MESH
#define TRACK

#include <iostream>
#include <iomanip>
using namespace std;

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "GL/glui.h"

#ifdef TRACK
#include "MyTracks.h"
#include "MyTrackRings.h"
#include "MyLineAO.h"
#endif

#ifdef MESH
#include "MyMesh.h"
#endif

#include "MyTrackBall.h"
#include "MyFrameBuffer.h"
#include "MyGraphicsTool.h"
#include "MyBlackBodyColor.h"
#include "MyPrimitiveDrawer.h"
#include "MyBitmap.h"

#include "MySsaoPass.h"
#include "MyBlurPass.h"
#include "MyLightingPass.h"

int windowWidth = 1000;
int windowHeight = 800;
float dsr_factor;
int gl_error;


#ifdef TRACK
MyLineAO track;
float lineWidth = 1;
#endif

#ifdef MESH
MyMesh mesh;
#endif
unsigned int gTex;

MyTrackBall trackBall;
MyFrameBuffer geomFb, ssaoFb, blurFb, lightingFb;
MySsaoPass ssaoPass;
MyBlurPass blurPass;
MyLightingPass lightingPass;

#define MAX_RENDER_MODE 4
int renderIdx = 0;
int bdrawTracks = 1;
int bdrawAxes = 0;
MyVec3f boxOffset[2];
int bdrawBoxes = 0;
int activeBox = 0;
int boxOpacityIndex = 0;
// glui code
GLUI_Panel* panel[MAX_RENDER_MODE + 1] = { 0 };

void RenderTexture(int texture, int x, int y, int width, int height){
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
}

unsigned int MakeTexture(MyString fn){
	MyBitmap bitmap;
	bitmap.Open(fn);
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmap.GetWidth(),
		bitmap.GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap.GetPixelBufferRGB());
	return texture;
}

#ifdef TRACK

void drawBox(MyVec3f low, MyVec3f high, int id = 0){
	if (id == activeBox) glColor3f(1, 0, 0);
	else glColor3f(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex3f(low[0], low[1], low[2]);
	glVertex3f(low[0], high[1], low[2]);
	glVertex3f(high[0], high[1], low[2]);
	glVertex3f(high[0], low[1], low[2]);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(low[0], low[1], high[2]);
	glVertex3f(low[0], high[1], high[2]);
	glVertex3f(high[0], high[1], high[2]);
	glVertex3f(high[0], low[1], high[2]);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(low[0], low[1], low[2]);
	glVertex3f(low[0], low[1], high[2]);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(low[0], high[1], low[2]);
	glVertex3f(low[0], high[1], high[2]);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(high[0], high[1], low[2]);
	glVertex3f(high[0], high[1], high[2]);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(high[0], low[1], low[2]);
	glVertex3f(high[0], low[1], high[2]);
	glEnd();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	float opacity = (float)boxOpacityIndex / 2;
	glColor4f(1, 1, 1, opacity);
	glBegin(GL_QUADS);
	// font
	glVertex3f(low[0], high[1], low[2]);
	glVertex3f(high[0], high[1], low[2]);
	glVertex3f(high[0], low[1], low[2]);
	glVertex3f(low[0], low[1], low[2]);
	// back
	glVertex3f(high[0], high[1], high[2]);
	glVertex3f(low[0], high[1], high[2]);
	glVertex3f(low[0], low[1], high[2]);
	glVertex3f(high[0], low[1], high[2]);
	//left
	glVertex3f(low[0], high[1], high[2]);
	glVertex3f(low[0], high[1], low[2]);
	glVertex3f(low[0], low[1], low[2]);
	glVertex3f(low[0], low[1], high[2]);
	//right
	glVertex3f(high[0], high[1], low[2]);
	glVertex3f(high[0], high[1], high[2]);
	glVertex3f(high[0], low[1], high[2]);
	glVertex3f(high[0], low[1], low[2]);
	// top
	glVertex3f(low[0], high[1], high[2]);
	glVertex3f(high[0], high[1], high[2]);
	glVertex3f(high[0], high[1], low[2]);
	glVertex3f(low[0], high[1], low[2]);
	// bottom
	glVertex3f(low[0], low[1], low[2]);
	glVertex3f(high[0], low[1], low[2]);
	glVertex3f(high[0], low[1], high[2]);
	glVertex3f(low[0], low[1], high[2]);
	glEnd();
	glPopAttrib();
}


void drawTracks(int x, int y, int width, int height){
	MyGraphicsTool::SetViewport(MyVec4i(x, y, width, height));
	glPushMatrix(); {
		MyGraphicsTool::LoadTrackBall(&trackBall);
		MyGraphicsTool::Rotate(180, MyVec3f(0, 1, 0));
		MyBoundingBox box = track.GetTracts()->GetBoundingBox();
		MyGraphicsTool::Translate(-box.GetCenter());
		if (bdrawTracks){
			glLineWidth(lineWidth);
			track.Show();
		}
		else{
			//track.ShowCapsOnly();
		}
	}glPopMatrix();
}

#endif
#ifdef MESH
void drawMesh(int x, int y, int width, int height){
	MyGraphicsTool::SetViewport(MyVec4i(x, y, width, height));
	glPushMatrix(); {
		MyGraphicsTool::LoadTrackBall(&trackBall);
		MyGraphicsTool::Rotate(180, MyVec3f(0, 1, 0));
		MyBoundingBox box = mesh.GetBoundingBox();
		MyGraphicsTool::Translate(-box.GetCenter());
		mesh.Render();
	}glPopMatrix();
}
#endif
void drawAxes(){
	if (!bdrawAxes) return;
	glPushMatrix(); {
		MyGraphicsTool::LoadTrackBall(&trackBall);
		glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(100, 0, 0);
		glColor3f(0, 1, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 100, 0);
		glColor3f(0, 0, 1);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 100);
		glEnd();
	}glPopMatrix();
}
void drawLegend(){
	glViewport(0, 0, windowWidth, windowHeight);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_1D, MyBlackBodyColor::GetTexture());

	int nStep = 100;
	float xOffset = 0.7;
	float yOffset = 0.1;
	float width = 0.2;
	float height = 0.04;
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= nStep; i++){
		float idxf = i / (float)nStep;
		float x = xOffset + idxf*width;
		glTexCoord1f(idxf);
		glColor3f(1, 1, 1);
		glVertex3f(x, yOffset, 0);
		glVertex3f(x, yOffset + height, 0);
	}
	glEnd();

	glColor3f(0, 0, 0);
	MyPrimitiveDrawer::DrawBitMapTextLarge(MyVec3f(xOffset, yOffset + height, 0),
		MyString(0.2));
	MyPrimitiveDrawer::DrawBitMapTextLarge(MyVec3f(xOffset + width, yOffset + height, 0),
		MyString(1.0));

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glPopAttrib();
}

/**************************************** GLUI Callback ********************/

GLUI* glui;
#define MAX_LIGHT_COMPONENTS 3
float meshPrecision = 0.01;
GLUI_Scrollbar* lightComponentSlider[MAX_LIGHT_COMPONENTS] = { 0 };
int lightComponentRatioControl = 1;
int dsrIndex = 2;
int cullface = 0;

void reRender(int mode){
	glutPostRedisplay();
}

void changeDsr(int mode){
	dsr_factor = dsrIndex / 2.f;

	int tw = windowWidth * dsr_factor;
	int th = windowHeight * dsr_factor;
	geomFb.SetSize(tw, th);
	geomFb.Build();

	ssaoFb.SetSize(tw, th);
	ssaoFb.Build();

	blurFb.SetSize(tw, th);
	blurFb.Build();

	lightingFb.SetSize(tw, th);
	lightingFb.Build();
	glutPostRedisplay();
}

void switchRenderMode(int mode){
	for (int i = 1; i <= MAX_RENDER_MODE; i++){
		if (i - 1 <= renderIdx) panel[i]->enable();
		else panel[i]->disable();
	}
	glutPostRedisplay();
}

#ifdef TRACK
int trackShape = 0;
int trackFaces = 20;
GLUI_Panel* tubeParameterPanel;
GLUI_Panel* lineParameterPanel;
void changeTrackShape(int id){
	MyTractVisBase::TrackShape oldShape = track.GetShape();
	int oldNumFaces = track.GetNumberFaces();
	switch (trackShape)
	{
	case 0:
		track.SetShape(MyTractVisBase::TrackShape::TRACK_SHAPE_TUBE);
		tubeParameterPanel->enable();
		lineParameterPanel->disable();
		break;
	case 1:
		track.SetShape(MyTractVisBase::TrackShape::TRACK_SHAPE_LINE);
		tubeParameterPanel->disable();
		lineParameterPanel->enable();
		break;
	default:
		break;
	}
	track.SetNumberFaces(trackFaces);
	if (oldShape != track.GetShape() || oldNumFaces != track.GetNumberFaces()){
		track.ComputeGeometry();
		track.LoadGeometry();
	}
	glutPostRedisplay();
}

void resetTrackShape(){
	MyTractVisBase::TrackShape oldShape = track.GetShape();
	int oldNumFaces = track.GetNumberFaces();
	track.ResetRenderingParameters();
	if (oldShape != track.GetShape() || oldNumFaces != track.GetNumberFaces()){
		track.ComputeGeometry();
		track.LoadGeometry();
	}
	switch (track.GetShape())
	{
	case MyTractVisBase::TRACK_SHAPE_TUBE:
		tubeParameterPanel->enable();
		trackShape = 0;
		break;
	case MyTractVisBase::TRACK_SHAPE_LINE:
		tubeParameterPanel->disable();
		trackShape = 1;
		break;
	default:
		break;
	}
	trackFaces = track.GetNumberFaces();
	glutPostRedisplay();
}
#endif

void changeLightComponent(int component){
	if (lightComponentRatioControl){
		float rest = 1.0;
		if (component >= 0 && component < MAX_LIGHT_COMPONENTS){
			rest = 1 - lightComponentSlider[component]->get_float_val();
		}
		// must have this since the rest sum could
		// be larger than 1 when normailzation is
		// disabled.
		float restReal = 0;
		for (int i = 0; i < MAX_LIGHT_COMPONENTS; i++){
			if (i != component){
				restReal += lightComponentSlider[i]->get_float_val();
			}
		}
		for (int i = 0; i < MAX_LIGHT_COMPONENTS; i++){
			if (i != component){
				float oldValue = lightComponentSlider[i]->get_float_val();
				float newValue;
				if (rest <= 0){
					if (restReal <= 0){
						newValue = 1.f / MAX_LIGHT_COMPONENTS;
					}
					else{
						newValue = 0;
					}
				}
				else if (restReal <= 0){
					if (component >= 0 && component < MAX_LIGHT_COMPONENTS){
						newValue = rest / (MAX_LIGHT_COMPONENTS - 1);
					}
					else newValue = rest / MAX_LIGHT_COMPONENTS;
				}
				else{
					newValue = oldValue / restReal*rest;
				}
				lightComponentSlider[i]->set_float_val(newValue);
			}
		}
	}
	glutPostRedisplay();
}

#ifdef MESH
void changeMeshPrecision(int mode){
	mesh.Read("data\\lh.pial.obj");
	mesh.MergeVertices(meshPrecision);
	mesh.GenPerVertexNormal();
	mesh.Build();
}
#endif

void resetShaders(int mode){
#ifdef TRACK
	track.LoadShader();
	glutPostRedisplay();
#endif
#ifdef MESH
	mesh.CompileShader();
	glutPostRedisplay();
#endif
}

void resetRenderingParameters(int mode){
	switch (mode)
	{
	case 0:
#ifdef TRACK
		resetTrackShape();
#endif
		break;
	case 1:
		ssaoPass.ResetRenderingParameters();
		break;
	case 2:
		blurPass.ResetRenderingParameters();
		break;
	case 3:
		lightingPass.ResetRenderingParameters();
		lightComponentRatioControl = 1;
		break;
	default:
#ifdef TRACK
		resetTrackShape();
#endif
		ssaoPass.ResetRenderingParameters();
		blurPass.ResetRenderingParameters();
		lightingPass.ResetRenderingParameters();
		lightComponentRatioControl = 1;
		break;
	}
	if (cullface == 1){
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	else{
		glDisable(GL_CULL_FACE);
	}
	glui->sync_live();
	glutPostRedisplay();
}

/**************************************** GLUT Callback ********************/
void myGlutDisplay(){
	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth, windowHeight));
	glPushMatrix(); {
		MyGraphicsTool::LoadTrackBall(&trackBall);
		MyGraphicsTool::Rotate(180, MyVec3f(0, 1, 0));
		MyBoundingBox box = track.GetTracts()->GetBoundingBox();
		MyGraphicsTool::Translate(-box.GetCenter());
		if (bdrawTracks){
			track.Show();
		}
	}glPopMatrix();
	glutSwapBuffers();
}
void myGlutKeyboard(unsigned char Key, int x, int y)
{
	switch (Key)
	{
	case 27:
		exit(0);
		break;
	case 'r':
	case 'R':
#ifdef TRACK
		track.LoadShader();
#endif
		ssaoPass.CompileShader();
		blurPass.CompileShader();
		lightingPass.CompileShader();
		break;
	case 'i':
		renderIdx = (renderIdx + 1) % MAX_RENDER_MODE;
		cout << renderIdx << endl;
		break;
	case 'I':
		renderIdx = (renderIdx + MAX_RENDER_MODE - 1) % MAX_RENDER_MODE;
		cout << renderIdx << endl;
		break;
	};

	glutPostRedisplay();
}

void myGlutMenu(int value)
{
	myGlutKeyboard(value, 0, 0);
}

void myGlutMouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0){
		trackBall.ScaleMultiply(1.05);
	}
	else{
		trackBall.ScaleMultiply(1 / 1.05);
	}
	glutPostRedisplay();
}

void myGlutMouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN){
		trackBall.StartMotion(x, y);
	}
	else if (state == GLUT_UP){
		trackBall.EndMotion(x, y);
	}
	glutPostRedisplay();
}

void myGlutMotion(int x, int y)
{
	trackBall.RotateMotion(x, y);
	glutPostRedisplay();
}

void myGlutPassiveMotion(int x, int y){
}

void myGlutReshape(int x, int y)
{
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
	glViewport(tx, ty, tw, th);
	windowWidth = tw;
	windowHeight = th;
	trackBall.Reshape(tw, th);
	MyMatrixf projectionMatrix = MyMatrixf::PerspectiveMatrix(60, windowWidth / (float)windowHeight, 1, 100);
	//MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-100,100,-100,100, 1, 200);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	gluLookAt(0, 0, 50, 0, 0, 0, 0, 1, 0);

	track.Resize(tw, th);
	changeDsr(-1);

	glutPostRedisplay();
}


/**************************************** main() ********************/

int main(int argc, char* argv[])
{

	/****************************************/
	/*   Initialize GLUT and create window  */
	/****************************************/

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize(windowWidth, windowHeight);
	int main_window = glutCreateWindow("Rendering Test");
	glewInit();
	glutDisplayFunc(myGlutDisplay);
	glutMotionFunc(myGlutMotion);
	glutReshapeFunc(myGlutReshape);
	glutPassiveMotionFunc(myGlutPassiveMotion);
	glutMouseWheelFunc(myGlutMouseWheel);
	glutMouseFunc(myGlutMouse);
	glutKeyboardFunc(myGlutKeyboard);

	glEnable(GL_DEPTH_TEST);

	/****************************************/
	/*         Here's the GLSL code         */
	/****************************************/
	geomFb.AddExtraDrawTexture(GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_EDGE);
	geomFb.AddExtraDrawTexture(GL_RGBA, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_EDGE);
	glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
	glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
	glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);

	/****************************************/
	/*               Loading Data           */
	/****************************************/

	ssaoPass.Build();
	blurPass.Build();
	lightingPass.Build();

#ifdef TRACK
	trackBall.SetRotationMatrix(MyMatrixf::RotateMatrix(90, 1, 0, 0));
	trackBall.ScaleMultiply(1.3);
	MyTracks tractData;
	//tractData.Read("..\\SSAO\\data\\normal_s3.data");
	//tractData.Read("C:\\Users\\GuohaoZhang\\Dropbox\\data\\normal_s3_tensorboy.trk");
	//tractData.Read("data\\normal_s5.tensorinfo");
	//tractData.Read("data\\cFile.tensorinfo");
	//tractData.Read("C:\\Users\\GuohaoZhang\\Desktop\\tmpdata\\dti.trk");
	tractData.Read("C:\\Users\\GuohaoZhang\\Desktop\\tmpdata\\ACR.trk");
	//tractData.Read("dti_20_0995.data");
	track.SetTracts(&tractData);
	track.SetShape(MyTractVisBase::TRACK_SHAPE_LINE);
	//track.SetShape(MyTractVisBase::TRACK_SHAPE_TUBE);
	track.ComputeGeometry();
	track.LoadShader();
	track.LoadGeometry();

	/*
	ring.CopyTracksFrom(track);
	ring.SetShape(MyTracks::TRACK_SHAPE_TUBE);
	ring.ComputeGeometry();
	ring.LoadShader();
	ring.LoadGeometry();


	trackLine.CopyTracksFrom(track);
	trackLine.SetShape(MyTracks::TRACK_SHAPE_LINE);
	trackLine.ComputeGeometry();
	trackLine.LoadShader();
	trackLine.LoadGeometry();
	*/

#endif

#ifdef MESH

	gTex = MakeTexture("2dir_128.bmp");

	meshPrecision = 0.01;
	mesh.Read("data\\lh.trans.pial.obj");
	//mesh.Read("lh.pial.obj");
	//mesh.Read("data\\lh.trans.pial.obj");
	MyMesh mesh2;
	mesh2.Read("data\\rh.trans.pial.obj");
	mesh.Merge(mesh2);
	mesh.MergeVertices(meshPrecision);
	mesh.GenPerVertexNormal();
	//mesh.Write("data\\rh.trans_wnormal.pial.obj");
	mesh.CompileShader();
	mesh.mTexture = gTex;
	mesh.Build();
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#endif
	/****************************************/
	/*         Here's the GLUI code         */
	/****************************************/
	printf("GLUI version: %3.2f\n", GLUI_Master.get_version());

	GLUI_Master.set_glutDisplayFunc(myGlutDisplay);
	GLUI_Master.set_glutReshapeFunc(myGlutReshape);
	GLUI_Master.set_glutKeyboardFunc(myGlutKeyboard);
	GLUI_Master.set_glutSpecialFunc(NULL);
	GLUI_Master.set_glutMouseFunc(myGlutMouse);

	glui = GLUI_Master.create_glui_subwindow(main_window,
		GLUI_SUBWINDOW_RIGHT);

	glui->set_main_gfx_window(main_window);

	// add panels for parameter tunning


	panel[0] = new GLUI_Panel(glui, "Rendering Mode");
	GLUI_Spinner* dsrSpinner = new GLUI_Spinner(
		panel[0], "DSR Index", GLUI_SPINNER_INT,
		&dsrIndex, -1, changeDsr);
	dsrSpinner->set_int_limits(1, 4);
	new GLUI_Button(panel[0], "Reset All", -1, resetRenderingParameters);
	new GLUI_Button(panel[0], "Reset Shaders", -1, resetShaders);
	new GLUI_StaticText(panel[0], "AO Strength");
	GLUI_Scrollbar* lineAOtotalStrengthSlider = new GLUI_Scrollbar
		(panel[0], "AO Strength", GLUI_SCROLL_HORIZONTAL,
		&(track.mLineAOTotalStrength), -1, reRender);
	lineAOtotalStrengthSlider->set_float_limits(0, 10);
	GLUI_Spinner* blurRadiusSpinner = new GLUI_Spinner(
		panel[0], "Blur Radius", GLUI_SPINNER_INT,
		&track.mBlurRadius, -1, reRender);
	blurRadiusSpinner->set_int_limits(0, 4);

	panel[1] = new GLUI_Panel(glui, "Geometry Pass");
	GLUI_RadioGroup* shapeRadioGroup = new GLUI_RadioGroup(panel[1],
		&trackShape, 0, changeTrackShape);
	new GLUI_RadioButton(shapeRadioGroup, "Tube");
	new GLUI_RadioButton(shapeRadioGroup, "Line");
	tubeParameterPanel = new GLUI_Panel(panel[1], "Tube Parameters");
	new GLUI_StaticText(tubeParameterPanel, "Tube Radius");
	GLUI_Scrollbar* tubeRadiusSlider = new GLUI_Scrollbar
		(tubeParameterPanel, "Tube Radius", GLUI_SCROLL_HORIZONTAL,
		&(track.mTrackRadius), -1, reRender);
	tubeRadiusSlider->set_float_limits(0, 1);
	GLUI_Spinner* trackFaceSpinner = new GLUI_Spinner(
		tubeParameterPanel, "Number Faces", GLUI_SPINNER_INT,
		&trackFaces, -1, changeTrackShape);
	trackFaceSpinner->set_int_limits(2, 20);
	trackFaceSpinner->set_int_val(track.GetNumberFaces());
	lineParameterPanel = new GLUI_Panel(panel[1], "Line Parameters");
	new GLUI_StaticText(tubeParameterPanel, "Line Width");
	GLUI_Scrollbar* lineWidthSlider = new GLUI_Scrollbar
		(lineParameterPanel, "Line Width", GLUI_SCROLL_HORIZONTAL,
		&(lineWidth), -1, reRender);
	lineWidthSlider->set_float_limits(0, 4);

	glutMainLoop();

	return EXIT_SUCCESS;
}

