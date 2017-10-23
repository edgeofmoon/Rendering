
#include "MyTracks.h"
#include "MyVisData.h"
#include "MyMatrix.h"
#include "MyMathHelper.h"
#include "MyTrackBall.h"
#include "MyPrimitiveDrawer.h"
#include "MyTractVisBase.h"
#include "MyColorLegend.h"
#include "MyConstants.h"

#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GL/freeglut.h>

using namespace std;
using namespace MyVisEnum;

MyColorLegend legend;
MyTracks tracts;
MyTractVisBase tractVis;
MyTrackBall trackBall;
MyArrayi tractIndices;
MyArray<MyColor4f> colors;
float SIGMA = 64.f;
int tractIdx = 0;
int methodIdx = 0;

int windowWidth = 800;
int windowHeight = 800;

float endPointPositionSimilarity(const MyTracks& tracts, int i, int j, float sigma){
	if (i == j) return 0;
	MyVec3f pi[2] = { tracts[i].mPoints.front(), tracts[i].mPoints.back() };
	MyVec3f pj[2] = { tracts[j].mPoints.front(), tracts[j].mPoints.back() };
	float w1 = (pi[0] - pj[0]).squared() + (pi[1] - pj[1]).squared();
	w1 = exp(-w1 / (2 * sigma*sigma));
	float w2 = (pi[0] - pj[1]).squared() + (pi[1] - pj[0]).squared();
	w2 = exp(-w2 / (2 * sigma*sigma));
	return w1 + w2;
}
float getDist(const MyVec3f& p, const MyTracks& tracts, int i){
	float rst = FLT_MAX;
	for (int j = 1; j < tracts.GetNumVertex(i); j++){
		float d = MyMathHelper::PointToLineSegmentDistance(p, 
			tracts.GetCoord(i, j - 1), tracts.GetCoord(i, j));
		rst = min(rst, d);
	}
	return rst;
}

float getAlpha(int k, int m, float sigma){
	float a = (k - (m + 1) / 2);
	a = a*a;
	a = a / (sigma*sigma);
	a = exp(a);
	return a;
}

float tractTrajectoryDistanceOneSide(const MyTracks& tracts, int i, int j, float lambda = 0.5f){
	float leni = tracts.ComputeTrackLength(i);
	int m = tracts.GetNumVertex(i);
	MyArrayf alphas(m);
	for (int k = 0; k < m; k++){
		float alpha = getAlpha(k + 1, m, leni*lambda);
		alphas[k] = alpha;
	}
	float Z = 0;
	for (float f : alphas) Z += f;
	float rst = 0;
	for (int k = 0; k < m; k++){
		float dist = getDist(tracts.GetCoord(i, k), tracts, j);
		rst += dist * alphas[k] / Z;
	}
	rst /= m;
	return rst;
}

float tractTrajectoryDistance(const MyTracks& tracts, int i, int j, float lambda = 0.5f){
	float dij = tractTrajectoryDistanceOneSide(tracts, i, j, lambda);
	float dji = tractTrajectoryDistanceOneSide(tracts, j, i, lambda);
	return max(dij, dji);
}

void computeDistMatrix(MyMatrixf& matrix, const MyTracks& tracts, const MyArrayi& indices, int method = 0){
	int n = indices.size();
	matrix = MyMatrixf(n, n);
#pragma omp parallel for
	for (int i = 0; i < n; i++){
		for (int j = i + 1; j < n; j++){
			float dist;
			if (method == 0){
				dist = endPointPositionSimilarity(tracts, indices[i], indices[j], SIGMA);
			}
			else {
				dist = tractTrajectoryDistance(tracts, indices[i], indices[j]);
			}
			matrix.At(i, j) = matrix.At(j, i) = dist;
		}
	}

	float minD = FLT_MAX;
	float maxD = -FLT_MAX;
	for (int i = 0; i < n; i++){
		for (int j = i + 1; j < n; j++){
			maxD = max(maxD, matrix.At(i, j));
			minD = min(minD, matrix.At(i, j));
		}
	}

	for (int i = 0; i < n; i++){
		for (int j = i + 1; j < n; j++){

		//	matrix.At(i, j) = matrix.At(j, i) = (matrix.At(i, j)-minD)/(maxD-minD);
		}
	}
}

MyArray<MyColor4f> getColorToTract(const MyTracks& tracts, int i, const MyArrayi& indices, int method = 0){
	MyArray<MyColor4f> rst;
	MyArrayf dists;
	float maxDist = -FLT_MAX;
	float minDist = FLT_MAX;
	for (int j = 0; j < indices.size(); j++){
		float dist;
		if (i == indices[j]) dist = 0;
		else if (method == 0){
			dist = endPointPositionSimilarity(tracts, i, indices[j], SIGMA);
		}
		else {
			dist = tractTrajectoryDistance(tracts, i, indices[j]);
		}
		dists << dist;
		maxDist = max(maxDist, dist);
		minDist = min(minDist, dist);
	}
	for (int j = 0; j < indices.size(); j++){
		float v = (dists[j] - minDist) / (maxDist - minDist);
		//MyColor4f c = legend.GetColorByValue(v);
		MyColor4f c(1,v,v,1);
		rst << c;
	}
	return rst;
}

void update(){
	colors = getColorToTract(tracts, tractIndices[tractIdx], 
		tractIndices, methodIdx);
	tractVis.SetPerTractColor(&colors);
}

void saveMatrix(const MyMatrixf& mat, const MyString& fileName, char decimer = ' '){
	ofstream ofile(fileName);
	if (!ofile.is_open()){
		cerr << "Cannot open output file: " << fileName << endl;
		return;
	}
	//ofile << mat.GetNumRows() << decimer << mat.GetNumCols() << endl;
	for (int i = 0; i < mat.GetNumRows(); i++){
		for (int j = 0; j < mat.GetNumCols(); j++){
			ofile << mat.At(i, j);
			if (j != mat.GetNumCols()-1) ofile << decimer;
		}
		ofile << endl;
	}
	ofile.close();
}

void saveVec(const MyTracks& tracts, const MyArrayi& indices, const MyString& fileName, char decimer = ' '){
	ofstream ofile(fileName);
	if (!ofile.is_open()){
		cerr << "Cannot open output file: " << fileName << endl;
		return;
	}
	//ofile << mat.GetNumRows() << decimer << mat.GetNumCols() << endl;
	for (int i = 0; i < indices.size(); i++){
		int n = tracts.GetNumVertex(indices[i]);
		MyVec3f c = tracts.GetCoord(indices[i], n / 2);
		ofile << c[0] << decimer << c[1] << decimer << c[2] << endl;
	}
	ofile.close();
}

void DrawHighlightedTracts(MyTractVisBase* tractVis,
	const MyArrayi& tractIndices, const MyColor4f& color){
	if (!tractVis) return;
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDepthFunc(GL_LEQUAL);
	MyColor4f baseColor;
	MyArrayi tractShow;
	MyArrayf influences;
	float ambient;
	const MyArray<MyColor4f>* colors = tractVis->GetPerTractColor();
	//MyColor4f baseSphereColor;
	baseColor = tractVis->GetBaseColor();
	tractShow = tractVis->GetTractsShown();
	ambient = tractVis->GetAmbient();
	influences = tractVis->GetInfluences();
	//if(sg) baseSphereColor = sg->GetColor();
	tractVis->SetTractsShown(tractIndices);
	tractVis->SetBaseColor(color);
	tractVis->SetAmbient(1);
	tractVis->ClearInfluences();
	tractVis->mTrackRadius = 1;
	tractVis->SetPerTractColor(0);
	//if (sg) sg->SetColor(color);
	tractVis->Show();
	tractVis->SetBaseColor(baseColor);
	tractVis->SetTractsShown(tractShow);
	tractVis->SetAmbient(ambient);
	tractVis->SetInfluences(influences);
	tractVis->mTrackRadius = 0.206;
	tractVis->SetPerTractColor(colors);
	//if (sg) sg->SetColor(baseSphereColor);
	glPopAttrib();
}

/////////////////////////GLUT functions//////////////////

/**************************************** GLUT Callback ********************/
void myGlutDisplay(){
	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	MyGraphicsTool::LoadTrackBall(&trackBall);
	MyBoundingBox box = tracts.GetBoundingBox();
	MyGraphicsTool::Translate(-box.GetCenter());
	tractVis.Show();
	DrawHighlightedTracts(&tractVis, { tractIndices[tractIdx] }, MyColor4f::red());
	glPopMatrix();
	glutSwapBuffers();

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
	case '+':
		tractIdx = (tractIdx + 1) % tractIndices.size();
	case '=':
		tractIdx = (tractIdx + 9) % tractIndices.size();
		update();
		break;
	case '_':
		tractIdx = (tractIdx - 1) % tractIndices.size();
	case '-':
		tractIdx = (tractIdx - 9) % tractIndices.size();
		update();
		break;
	case '0':
	case ')':
		methodIdx = 1-methodIdx;
		update();
		break;
	case '}':
		SIGMA *= 1.1;
	case ']':
		SIGMA *= 1.1;
		update();
		break;
	case '{':
		SIGMA /= 1.1;
	case '[':
		SIGMA /= 1.1;
		update();
		break;
	}
	glutPostRedisplay();

	cout << "Method: " << (methodIdx == 0 ? "EndPoint" : "Trajectory") << endl;
	cout << "Sigma: " << SIGMA << endl;
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
		trackBall.StartRotation(x, y);
	}
	else if (state == GLUT_UP){
		trackBall.EndRotation(x, y);
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
	glViewport(0, 0, x, y);
	MyMatrixf projectionMatrix = MyMatrixf::PerspectiveMatrix(60, x / (float)y, 1, 300);
	//MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-100,100,-100,100, 1, 200);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	gluLookAt(0, 0, 150, 0, 0, 0, 0, 1, 0);
	trackBall.Reshape(x, y);
	trackBall.SetScaleRange(pow(1.05f, -5), pow(1.05f, 20));
	trackBall.ResetRotate();
	trackBall.ResetScale();
	glutPostRedisplay();
}

int main(int argc, char* argv[]){
	tracts.Read("C:\\Users\\GuohaoZhang\\Dropbox\\data\\normal_s3_tensorboy_RevZ.trk");
	MyVisData visData(MyVisInfo(false, false, TRACE, VALUE, 0, SUPERQUADRIC, BASIC, CC, WHOLE, 0, 3));
	visData.SetTracts(&tracts);
	visData.LoadFromDirectory("C:\\Users\\GuohaoZhang\\Dropbox\\data\\traces");
	tractIndices = visData.GetTractIndices();
	//tractIndices.resize(100);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize(windowWidth, windowHeight);
	int main_window = glutCreateWindow("DistViewer");
	glewInit();
	glutDisplayFunc(myGlutDisplay);
	glutMotionFunc(myGlutMotion);
	glutReshapeFunc(myGlutReshape);
	glutPassiveMotionFunc(myGlutPassiveMotion);
	glutMouseWheelFunc(myGlutMouseWheel);
	glutMouseFunc(myGlutMouse);
	glutKeyboardFunc(myGlutKeyboard);
	glutSpecialFunc(myGlutSpecialInput);

	tractVis.SetTracts(&tracts);
	tractVis.LoadShader();
	tractVis.ComputeGeometry();
	tractVis.LoadGeometry();
	tractVis.ClearInfluences();
	tractVis.SetAmbient(1);
	tractVis.SetDiffuse(0);
	tractVis.SetTractsShown(tractIndices);
	legend.SetColorsFromData(&MyConstants::BlackBodyExtended[0][0], 3, 1024, 1);
	update();

	/*
	//saveVec(tracts, visData.GetTractIndices(), "s3_cc_whole.vec");
	//return 1;
	MyMatrixf distMat;
	computeDistMatrix(distMat, tracts, tractIndices, 0);
	saveMatrix(distMat, "s3_endPointPos.sim");
	return 1;

	computeDistMatrix(distMat, tracts, tractIndices, 1);
	saveMatrix(distMat, "s3_tractTraj.sim");
	return 1;
	*/

	glutMainLoop();
	return EXIT_SUCCESS;
}