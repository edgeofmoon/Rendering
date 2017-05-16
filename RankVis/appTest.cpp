#include "MyVisRankingApp.h"

#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
using namespace std;

int windowWidth = 800;
int windowHeight = 800;

MyVisRankingApp* app = 0;

/**************************************** GLUT Callback ********************/
void myGlutDisplay(){
	app->HandleGlutDisplay();

}
void myGlutKeyboard(unsigned char key, int x, int y){
	app->HandleGlutKeyboard(key, x, y);
}

void myGlutSpecialInput(int key, int x, int y)
{
	app->HandleGlutSpecialKeyboard(key, x, y);
}

void myGlutMenu(int value){
	
}

void myGlutMouseWheel(int button, int dir, int x, int y){
	app->HandleGlutMouseWheel(button, dir, x, y);
}

void myGlutMouse(int button, int state, int x, int y){
	app->HandleGlutMouse(button, state, x, y);
}

void myGlutMotion(int x, int y){
	app->HandleGlutMotion(x, y);
}

void myGlutPassiveMotion(int x, int y){
	app->HandleGlutPassiveMotion(x, y);
}

void myGlutReshape(int x, int y){
	app->HandleGlutReshape(x, y);
}

int main(int argc, char* argv[]){
	if (argc < 4){
		cout << "Useage: " << argv[0] << " " << "[UserIndex] [TrialIndex] [Mode]" << endl
			<< "Mode: Training: 1, Formal: 2, Lighting profile: 4, Occlusion profile: 8," << endl
			<< "Debug mode mask: 256" << endl;
		char tmp[256];
		std::cin.getline(tmp, 256);
		return 0;
	}
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

	int uidx = 0, tidx = -1, mode = 2;
	if (argc > 1) uidx = atoi(argv[1]);
	if (argc > 2) tidx = atoi(argv[2]);
	if (argc > 3) mode = atoi(argv[3]);
	app = new MyVisRankingApp;
	app->Init(uidx, tidx, mode);
	app->Next();

	glutMainLoop();
	return EXIT_SUCCESS;
}