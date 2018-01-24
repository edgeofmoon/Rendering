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
	// determine the glyph colors
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
	tractVises.back()->SetColorInfluence(1);
	centers << pos;
}

void anisotripToEigenValue(float eigens[3], float cl, float cp, float cs){
	float lmd1, lmd2, lmd3;
	lmd3 = 1.0f;
	lmd1 = (3 * cp + 2 * cs + 6 * cl) / (2 * cs) * lmd3;
	lmd2 = (1 + 3 * cp / (2 * cs)) * lmd3;
	MyVec3f v(lmd1, lmd2, lmd3);
	v.normalize();
	eigens[0] = v[0];
	eigens[1] = v[1];
	eigens[2] = v[2];
}

bool within(double x){
	return 0 <= x && x <= 1;
}

bool baryCentric(const MyVec2f &A, const MyVec2f &B, const MyVec2f &C, const MyVec2f &P,
	float& alpha, float& beta, float& gamma){
	double det = (B[1] - C[1])*(A[0] - C[0]) + (C[0] - B[0])*(A[1] - C[1]);
	double factor_alpha = (B[1] - C[1])*(P[0] - C[0]) + (C[0] - B[0])*(P[1] - C[1]);
	double factor_beta = (C[1] - A[1])*(P[0] - C[0]) + (A[0] - C[0])*(P[1] - C[1]);
	alpha = factor_alpha / det;
	beta = factor_beta / det;
	gamma = 1.0 - alpha - beta;

	return P == A || P == B || P == C || (within(alpha) && within(beta) && within(gamma));
}

void dataInit(){
	MySuperquadric::SetMeshDetail(100, 100);
	box.SetNull();
	MyVec3f eigenvectors[3] = { { -3, 1, 1 }, { 1, 3, 0 } };
	eigenvectors[0].normalize();
	eigenvectors[1].normalize();
	eigenvectors[2] = eigenvectors[0] ^ eigenvectors[1];
	float w = 10;
	float h = 7;
	float egs[3];
	float cl, cp, cs;
	float offset = 0.2;
	MyVec2f pl(-w / 2 - offset, 0 - offset), pp(w / 2 + offset, 0 - offset), ps(0, h);
	for (int i = 0; i < 6; i++){
		for (int j = 0; j < 6 - i; j++){
			float y = h / 5 * i;
			float dw = w - i*w / 5;
			float x = (i == 5 ? 0 : -dw / 2 + dw / (5 - i)*j);
			baryCentric(pl, pp, ps, MyVec2f(x, y), cl, cp, cs);
			anisotripToEigenValue(egs, cl, cp, cs);
			MyTensor3f tensor;
			tensor.SetEigenVectors(eigenvectors);
			tensor.SetEigenValues(egs);
			addTractVis(tensor, MyVec3f(x + 5,y,0));
			box.Engulf(MyVec3f(x + 5, y, 0));
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
