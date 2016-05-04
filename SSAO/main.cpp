
#include <iostream>
using namespace std;

#include <GL/glew.h>
#include <GL/freeglut.h>
//#include "GL/glui.h"
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
int renderIdx = 0;

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
	//GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
	tx = ty = 0;
	tw = x, th = y;
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
	/*
	printf("GLUI version: %3.2f\n", GLUI_Master.get_version());

	GLUI_Master.set_glutDisplayFunc(myGlutDisplay);
	GLUI_Master.set_glutReshapeFunc(myGlutReshape);
	GLUI_Master.set_glutKeyboardFunc(myGlutKeyboard);
	GLUI_Master.set_glutSpecialFunc(NULL);
	GLUI_Master.set_glutMouseFunc(myGlutMouse);

	GLUI* glui = GLUI_Master.create_glui_subwindow(main_window,
		GLUI_SUBWINDOW_RIGHT);

	glui->set_main_gfx_window(main_window);
	*/

	glutMainLoop();

	return EXIT_SUCCESS;
}

