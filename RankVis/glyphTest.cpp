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
#include "MyTensor.h"
#include "MyTrackBall.h"
#include "MySuperquadric.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
using namespace std;

int windowWidth = 800;
int windowHeight = 800;

MyArray<MyTracks *> tracts;
MyArray<MyTractVisBase *> tractVises;
MyTrackBall trackBall;
MyArray3f centers;
MyBoundingBox box;
/**************************************** GLUT Callback ********************/
void myGlutDisplay(){
	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < tractVises.size();i++){
		glPushMatrix();
		MyGraphicsTool::Translate(centers[i]);
		MyGraphicsTool::LoadTrackBall(&trackBall);
		tractVises[i]->Show();
		glPopMatrix();
	}

	glColor3f(0, 0, 0);
	for (int i = 0; i <= 9; i++){
		float x = i / 10.f;
		MyVec3f pos(x, -0.07, 0);
		MyPrimitiveDrawer::DrawBitMapTextLarge(pos*10, MyString(x), 1);
	}
	MyVec3f pos(0.45, -0.11, 0);
	MyPrimitiveDrawer::DrawBitMapTextLarge(pos * 10, "-------FA value increases----->", 1);

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
	else if (button == GLUT_RIGHT_BUTTON){
		trackBall.ResetRotate();
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
	float width = 10;
	float height = 10;
	float xCenter = 4.5;
	float yCenter = 3.5;
	float xdy = 1;
	float ydx = y / float(x);
	if (ydx < 1){
		xdy = 1 / ydx;
		ydx = 1;
	}
	MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(
		xCenter - width/2*xdy, xCenter + width/2*xdy, yCenter - height/2*ydx, yCenter + height/2*ydx, -100, 100);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	trackBall.Reshape(x, y);
}


void myGlutKeyboard(unsigned char key, int x, int y){
	switch (key){
	case 27:
		exit(0);
		break;
	case 'h':
	case 'H':
		trackBall.ResetRotate();
		trackBall.ResetScale();
		break;
	}
	glutPostRedisplay();
}
/**************************************** GLUT Callback ********************/

bool compute(float fa, float a, float b, float &c1, float &c2){
	float t = 1 - fa*fa;
	float ac4 = (a + b)*(a + b) - 4 * t * (t * (a * a + b * b) - a * b);
	if (ac4 < 0) return false;
	if (fa == 1){
		if (b > 0) return false;
		c1 = c2 = 0;
		return true;
	}
	c1 = (a + b) + sqrtf(ac4);
	c2 = (a + b) - sqrtf(ac4);
	c1 /= (2 * t);
	c2 /= (2 * t);
	return true;
	//cout << a << "," << b << c1 << "," << c2 << endl;
}

void addTractVis(const MyTensor3f& tensor, const MyVec3f pos){
	tracts.push_back(new MyTracks());
	tracts.back()->GetTracts().push_back(MySingleTrackData());
	tracts.back()->GetTracts().back().mPoints << MyVec3f(0, 0, 0);
	tracts.back()->GetTracts().back().mPointScalars << MyArrayf();
	tracts.back()->GetTracts().back().mPointScalars.back() << tensor.GetFA()
		<< tensor.GetEigenValue(0) << tensor.GetEigenValue(1) << tensor.GetEigenValue(2)
		<< tensor.GetEigenVector(0)[0] << tensor.GetEigenVector(0)[1] << tensor.GetEigenVector(0)[2]
		<< tensor.GetEigenVector(1)[0] << tensor.GetEigenVector(1)[1] << tensor.GetEigenVector(1)[2]
		<< tensor.GetEigenVector(2)[0] << tensor.GetEigenVector(2)[1] << tensor.GetEigenVector(2)[2];
	tracts.back()->GetTracts().back().mTrackProperties = { 0.5, 0.5, 0.5 };
	tracts.back()->GetHeader().n_count = 1;
	tracts.back()->GetHeader().n_scalars = 13;
	tracts.back()->GetHeader().n_properties = 3;
	tractVises.push_back(new MyTractVisBase());
	tractVises.back()->SetTracts(tracts.back());
	tractVises.back()->SetShape(MyTractVisBase::TRACK_SHAPE_SUPERQUADRIC);
	tractVises.back()->LoadShader();
	tractVises.back()->ComputeGeometry();
	tractVises.back()->LoadGeometry();
	tractVises.back()->ClearInfluences();
	centers << pos;
}

void dataInit(){
	MySuperquadric::SetMeshDetail(100, 100);
	box.SetNull();
	MyVec3f eigenvectors[3] = { { 1, 1, 0 }, { 1, -1, 1 } };
	eigenvectors[0].normalize();
	eigenvectors[1].normalize();
	eigenvectors[2] = eigenvectors[0] ^ eigenvectors[1];
	for (int i = 0; i <= 10; i++){
		float fa = i / 10.f;
		float a = 1;
		MyVec3f pos(fa, 0, 0);
		for (int j = 0; j <= 10; j++){
			float b = j / 10.f;
			float c1, c2;
			if (compute(fa, a, b, c1, c2)){
				MyTensor3f tensor;
				tensor.SetEigenVectors(eigenvectors);
				if (c1 <= b && c1 >= 0){
					tensor.SetEigenValues(a, b, c1);
					addTractVis(tensor, pos * 10);
					pos[1] += 0.1;
					//cout << a << "," << b << "," << c1 << endl;
					//cout << fa << "=" << tensor.GetFA() << endl;
					box.Engulf(pos * 10);
				}
				if (c2 <= b && c2 != c1 && c2 >= 0){
					tensor.SetEigenValues(a, b, c2);
					addTractVis(tensor, pos * 10);
					pos[1] += 0.1;
					//cout << a << "," << b << "," << c2 << endl;
					//cout << fa << "=" << tensor.GetFA() << endl;
					box.Engulf(pos * 10);
				}
			}
		}
	}
}

int init(int argc, char* argv[]){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize(windowWidth, windowHeight);
	int main_window = glutCreateWindow("GlyphTest");
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
	return 1;
}
int main(int argc, char* argv[]){
	//tractStats();
	//return 1;
	init(argc, argv);
	dataInit();
	glutMainLoop();
}
