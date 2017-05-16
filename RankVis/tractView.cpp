#include "MyTractVisLabelled.h"
#include "MyTracks.h"
#include "MyTrackBall.h"
#include "MyPrimitiveDrawer.h"

#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
using namespace std;

int windowWidth = 800;
int windowHeight = 800;

MyTracks tracts;
MyTractVisLabelled tractVis;
MyTrackBall trackBall;
MyArray<MyColor4f> textColors;
MyArray3f textPos;
MyArrayStr texts;

/**************************************** GLUT Callback ********************/
void myGlutDisplay(){
	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	MyGraphicsTool::LoadTrackBall(&trackBall);
	MyBoundingBox box = tracts.GetBoundingBox();
	MyGraphicsTool::Translate(-box.GetCenter());
	tractVis.Show();

	glLineWidth(4);
	for (int i = 0; i < 5; i++){
		MyGraphicsTool::Color(textColors[i]);
		MyPrimitiveDrawer::DrawStrokeTextOrtho(textPos[i], texts[i], MyVec3f(0.3,0.3,0.3));
	}
	glPopMatrix();
	glutSwapBuffers();

}
void myGlutKeyboard(unsigned char key, int x, int y){
	switch (key){
	case 27:
		exit(0);
		break;
	}
}

void myGlutSpecialInput(int key, int x, int y){
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
	if (state == GLUT_DOWN){
		trackBall.StartMotion(x, y);
	}
	else if (state == GLUT_UP){
		trackBall.EndMotion(x, y);
	}
	glutPostRedisplay();
}

void myGlutMotion(int x, int y){
	trackBall.RotateMotion(x, y);
	glutPostRedisplay();
}

void myGlutPassiveMotion(int x, int y){
}

void myGlutReshape(int x, int y){
	glViewport(0, 0, x, y);
	MyMatrixf projectionMatrix = MyMatrixf::PerspectiveMatrix(60, x / (float)y, 1, 300);
	//MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-100,100,-100,100, 1, 200);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	gluLookAt(0, 0, 150, 0, 0, 0, 0, 1, 0);
	trackBall.Reshape(x, y);
	trackBall.ResetRotate();
	trackBall.ResetScale();
	glutPostRedisplay();
}

int main(int argc, char* argv[]){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize(windowWidth, windowHeight);
	int main_window = glutCreateWindow("RankVis");
	glewInit();
	glutDisplayFunc(myGlutDisplay);
	glutMotionFunc(myGlutMotion);
	glutReshapeFunc(myGlutReshape);
	glutPassiveMotionFunc(myGlutPassiveMotion);
	glutMouseWheelFunc(myGlutMouseWheel);
	glutMouseFunc(myGlutMouse);
	glutKeyboardFunc(myGlutKeyboard);
	glutSpecialFunc(myGlutSpecialInput);
	glutFullScreen();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	tracts.Read("C:\\Users\\GuohaoZhang\\Dropbox\\data\\forallfb.trk");
	tractVis.SetTracts(&tracts);
	tractVis.LoadShader();
	tractVis.ComputeGeometry();
	tractVis.LoadGeometry();
	tractVis.ClearInfluences();
	tractVis.SetColorInfluence(1);

	
	//m_textColors.push_back(rgba(1,1,1));
	textColors.push_back(MyColor4f(1, 0, 0));
	textColors.push_back(MyColor4f(1, 1, 0));
	textColors.push_back(MyColor4f(0, 1, 0));
	textColors.push_back(MyColor4f(0, 0, 1));
	textColors.push_back(MyColor4f(1, 0, 1));

	texts.resize(5);
	texts[0] = "CG";
	texts[1] = "CC";
	texts[2] = "ILF";
	texts[3] = "CST";
	texts[4] = "IFO";

	for (int j = 0; j < 5; j++){
		for (int i = 0; i < tracts.GetNumTracks(); i++){
			if (tracts.GetPointColor(i, 0) == textColors[j]){
				textPos << tracts.GetCoord(i, 0);
				break;
			}
		}
	}
	glutMainLoop();
	return EXIT_SUCCESS;
}