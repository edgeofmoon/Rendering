
#include <iostream>
#include <iomanip>
using namespace std;

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "GL/glui.h"
#include "MyTracks.h"
#include "MyTrackBall.h"
#include "MyFrameBuffer.h"
#include "MyGraphicsTool.h"

#include "MySsaoPass.h"
#include "MyBlurPass.h"
#include "MyLightingPass.h"

#define DSR_FACTOR_X 2
#define DSR_FACTOR_Y 2
int windowWidth = 1000;
int windowHeight = 800;
int gl_error;

MyTracks track;
MyTrackBall trackBall;
MyFrameBuffer geomFb, ssaoFb, blurFb;
MySsaoPass ssaoPass;
MyBlurPass blurPass;
MyLightingPass lightingPass;

#define MAX_RENDER_MODE 4
int renderIdx = 3;

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

void drawTracks(int x, int y, int width, int height){
	MyGraphicsTool::SetViewport(MyVec4i(x, y, width, height));
	glPushMatrix(); {
		MyGraphicsTool::LoadTrackBall(&trackBall);
		MyGraphicsTool::Rotate(180, MyVec3f(0, 1, 0));
		MyBoundingBox box = track.GetBoundingBox();
		MyGraphicsTool::Translate(-box.GetCenter());
		track.Show();
	}glPopMatrix();
}

void drawAxes(){
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

/**************************************** GLUI Callback ********************/

GLUI* glui;
#define MAX_LIGHT_COMPONENTS 3
GLUI_Scrollbar* lightComponentSlider[MAX_LIGHT_COMPONENTS] = { 0 };
int lightComponentRatioControl = 1;
int trackShape = 0;
int trackFaces = 6;
GLUI_Panel* tubeParameterPanel;

void reRender(int mode){
	glutPostRedisplay();
}

void switchRenderMode(int mode){
	for (int i = 1; i <= MAX_RENDER_MODE; i++){
		if (i - 1 <= renderIdx) panel[i]->enable();
		else panel[i]->disable();
	}
	glutPostRedisplay();
}

void changeTrackShape(int id){
	MyTracks::TrackShape oldShape = track.GetShape();
	int oldNumFaces = track.GetNumberFaces();
	switch (trackShape)
	{
	case 0:
		track.SetShape(MyTracks::TrackShape::TRACK_SHAPE_TUBE);
		tubeParameterPanel->enable();
		break;
	case 1:
		track.SetShape(MyTracks::TrackShape::TRACK_SHAPE_LINE);
		tubeParameterPanel->disable();
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
	MyTracks::TrackShape oldShape = track.GetShape();
	int oldNumFaces = track.GetNumberFaces();
	track.ResetRenderingParameters();
	if (oldShape != track.GetShape() || oldNumFaces != track.GetNumberFaces()){
		track.ComputeGeometry();
		track.LoadGeometry();
	}
	switch (track.GetShape())
	{
	case MyTracks::TrackShape::TRACK_SHAPE_TUBE:
		tubeParameterPanel->enable();
		trackShape = 0;
		break;
	case MyTracks::TrackShape::TRACK_SHAPE_LINE:
		tubeParameterPanel->disable();
		trackShape = 1;
		break;
	default:
		break;
	}
	trackFaces = track.GetNumberFaces();
}

void changeLightComponent(int component){
	if (lightComponentRatioControl){
		float rest = 1.0;
		if (component >= 0 && component < MAX_LIGHT_COMPONENTS){
			rest = 1 - lightComponentSlider[component]->get_float_val();
		}
		float restReal = 0;
		for (int i = 0; i < MAX_LIGHT_COMPONENTS; i++){
			if (i != component){
				restReal += lightComponentSlider[i]->get_float_val();
			}
		}
		for (int i = 0; i < MAX_LIGHT_COMPONENTS; i++){
			if (i != component){
				float oldValue = lightComponentSlider[i]->get_float_val();
				float newValue = oldValue / restReal*rest;
				lightComponentSlider[i]->set_float_val(newValue);
			}
		}
	}
	cout << "lighting: " << setprecision(2)
		<< lightingPass.mLightItensity << ", "
		<< lightingPass.mAmbient << ", "
		<< lightingPass.mDiffuse << ", "
		<< lightingPass.mSpecular << ", "
		<< lightingPass.mShininess << endl;
	glutPostRedisplay();
}

void resetRenderingParameters(int mode){
	switch (mode)
	{
	case 0:
		resetTrackShape();
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
		resetTrackShape();
		ssaoPass.ResetRenderingParameters();
		blurPass.ResetRenderingParameters();
		lightingPass.ResetRenderingParameters();
		lightComponentRatioControl = 1;
		break;
	}
	glui->sync_live();
	glutPostRedisplay();
}

/**************************************** GLUT Callback ********************/
void myGlutDisplay(){
	glClearColor(0.5, 0.5, 0.5, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// geometry pass
	glBindFramebuffer(GL_FRAMEBUFFER, geomFb.GetFrameBuffer());
	geomFb.Clear();
	drawTracks(0, 0, windowWidth, windowHeight);
	drawAxes();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (renderIdx == 0){
		RenderTexture(geomFb.GetColorTexture(), 0, 0, windowWidth, windowHeight);
		glutSwapBuffers();
		return;
	}

	if (renderIdx >= 1){
		// ssao pass
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFb.GetFrameBuffer());
		ssaoFb.Clear();
		ssaoPass.SetColorTexture(geomFb.GetColorTexture());
		ssaoPass.SetPositionTexture(geomFb.GetExtraDrawTexture(0));
		ssaoPass.SetNormalTexture(geomFb.GetExtraDrawTexture(1));
		ssaoPass.Render();
		drawAxes();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	if (renderIdx == 1){
		RenderTexture(ssaoFb.GetColorTexture(), 0, 0, windowWidth, windowHeight);
		glutSwapBuffers();
		return;
	}

	if (renderIdx >= 2){
		// blur pass
		glBindFramebuffer(GL_FRAMEBUFFER, blurFb.GetFrameBuffer());
		ssaoFb.Clear();
		blurPass.SetInputTexture(ssaoFb.GetColorTexture());
		blurPass.Render();
		drawAxes();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	if (renderIdx == 2){
		RenderTexture(blurFb.GetColorTexture(), 0, 0, windowWidth, windowHeight);
		glutSwapBuffers();
		return;
	}

	if (renderIdx >= 3){
		// lighting pass
		lightingPass.SetColorTexture(geomFb.GetColorTexture());
		lightingPass.SetPositionTexture(geomFb.GetExtraDrawTexture(0));
		lightingPass.SetNormalTexture(geomFb.GetExtraDrawTexture(1));
		lightingPass.SetSsaoTexture(blurFb.GetColorTexture());
		lightingPass.Render();
		drawAxes();
		glutSwapBuffers();
	}

}
void myGlutKeyboard(unsigned char Key, int x, int y)
{
	switch (Key)
	{
	case 27:
	case 'q':
		exit(0);
		break;
	case 'r':
	case 'R':
		track.LoadShader();
		ssaoPass.CompileShader();
		blurPass.CompileShader();
		lightingPass.CompileShader();
		break;
	case 'a':
		renderIdx = (renderIdx + 1) % MAX_RENDER_MODE;
		cout << renderIdx << endl;
		break;
	case 'A':
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
		trackBall.ScaleMultiply(1/1.05);
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
	MyMatrixf projectionMatrix = MyMatrixf::PerspectiveMatrix(60, windowWidth / (float)windowHeight, 1, 200);
	//MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-100,100,-100,100, 1, 200);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	gluLookAt(0, 0, 150, 0, 0, 0, 0, 1, 0);

	geomFb.SetSize(tw, th);
	geomFb.Build();

	ssaoFb.SetSize(tw, th);
	ssaoFb.Build();

	blurFb.SetSize(tw, th);
	blurFb.Build();

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

	//track.Read("C:\\Users\\GuohaoZhang\\Desktop\\tmpdata\\ACR.trk");
	track.Read("C:\\Users\\GuohaoZhang\\Dropbox\\data\\normal_s3.data");
	//track.Read("ACR.trk");
	//track.SetShape(MyTracks::TRACK_SHAPE_LINE);
	track.SetShape(MyTracks::TRACK_SHAPE_TUBE);
	track.ComputeGeometry();
	track.LoadShader();
	track.LoadGeometry();

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
	GLUI_RadioGroup* radioGroup = new GLUI_RadioGroup(panel[0],
		&renderIdx, 0, switchRenderMode);
	new GLUI_RadioButton(radioGroup, "Geometry Pass");
	new GLUI_RadioButton(radioGroup, "SSAO Pass");
	new GLUI_RadioButton(radioGroup, "Blur Pass");
	new GLUI_RadioButton(radioGroup, "Lighting Pass");
	new GLUI_Button(panel[0], "Reset All", -1, resetRenderingParameters);

	// geometry pass
	panel[1] = new GLUI_Panel(glui, "Geometry Pass");
	new GLUI_Button(panel[1], "Reset", 0, resetRenderingParameters);
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
	GLUI_Spinner* trackFaceSpinner = new GLUI_Spinner(tubeParameterPanel, "Number Faces",
		&trackFaces, -1, changeTrackShape);
	trackFaceSpinner->set_int_limits(2, 12);
	trackFaceSpinner->set_int_val(track.GetNumberFaces());

	// ssao pass
	panel[2] = new GLUI_Panel(glui, "SSAO Pass");
	new GLUI_Button(panel[2], "Reset", 1, resetRenderingParameters);
	new GLUI_StaticText(panel[2], "Sample Radius");
	GLUI_Scrollbar* sampleRadiusSlider = new GLUI_Scrollbar
		(panel[2], "Sample Radius", GLUI_SCROLL_HORIZONTAL,
		&(ssaoPass.mSampleRadius), -1, reRender);
	sampleRadiusSlider->set_float_limits(0, 100);

	// blur pass
	panel[3] = new GLUI_Panel(glui, "Blur Pass");
	new GLUI_Button(panel[3], "Reset", 2, resetRenderingParameters);
	GLUI_Spinner* blurRadiusSpinner = new GLUI_Spinner
		(panel[3], "Blur Radius (Pixel)", GLUI_SPINNER_INT,
		&(blurPass.mBlurRadius), -1, reRender);
	blurRadiusSpinner->set_int_limits(0, 20);

	// lighting pass
	panel[4] = new GLUI_Panel(glui, "Lighting Pass");
	new GLUI_Button(panel[4], "Reset", 3, resetRenderingParameters);
	new GLUI_Checkbox(panel[4], "Normalize Itensity",
		&lightComponentRatioControl, -1, changeLightComponent);
	new GLUI_Checkbox(panel[4], "Use SSAO",
		&lightingPass.mUseSsao, -1, changeLightComponent);
	new GLUI_StaticText(panel[4], "Light Intensity");
	GLUI_Scrollbar* lightIntensitySlider = new GLUI_Scrollbar
		(panel[4], "Light Intensity", GLUI_SCROLL_HORIZONTAL,
		&(lightingPass.mLightItensity), -1, reRender);
	lightIntensitySlider->set_float_limits(0, 10);
	new GLUI_StaticText(panel[4], "Ambient");
	lightComponentSlider[0] = new GLUI_Scrollbar
		(panel[4], "Ambient", GLUI_SCROLL_HORIZONTAL,
		&(lightingPass.mAmbient), 0, changeLightComponent);
	lightComponentSlider[0]->set_float_limits(0, 1);
	new GLUI_StaticText(panel[4], "Diffuse");
	lightComponentSlider[1] = new GLUI_Scrollbar
		(panel[4], "Diffuse", GLUI_SCROLL_HORIZONTAL,
		&(lightingPass.mDiffuse), 1, changeLightComponent);
	lightComponentSlider[1]->set_float_limits(0, 1);
	new GLUI_StaticText(panel[4], "Specular");
	lightComponentSlider[2] = new GLUI_Scrollbar
		(panel[4], "Specular", GLUI_SCROLL_HORIZONTAL,
		&(lightingPass.mSpecular), 2, changeLightComponent);
	lightComponentSlider[2]->set_float_limits(0, 1);
	new GLUI_StaticText(panel[4], "Shininess");
	GLUI_Scrollbar* shininessSlider = new GLUI_Scrollbar
		(panel[4], "Shininess", GLUI_SCROLL_HORIZONTAL,
		&(lightingPass.mShininess), -1, changeLightComponent);
	shininessSlider->set_float_limits(0, 128);


	// set init state right
	switchRenderMode(renderIdx);

	glutMainLoop();

	return EXIT_SUCCESS;
}

