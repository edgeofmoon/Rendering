#include "MyMesh.h"
#include "Shader.h"
#include "MyMathHelper.h"

#include <fstream>
#include <map>
#include <iostream>
#include <cassert>
#include <sstream>
using namespace std;


#include "GL\glew.h"
#include <GL/freeglut.h>

MyMesh::MyMesh()
{
}


MyMesh::~MyMesh()
{
}

int MyMesh::Read(const MyString& fileName){
	ifstream infile(fileName);
	if (!infile.is_open()){
		return -1;
	}
	char line[1000];
	char id;
	MyVec3f p;
	MyVec3i t;
	mVertices.clear();
	mNormals.clear();
	mTriangles.clear();
	while (!infile.eof())
	{
		infile >> id;
		switch (id)
		{
		case 'v':
		case 'V':
			infile >> p[0] >> p[1] >> p[2];
			mVertices << p;
			mBox.Engulf(p);
			break;
		case 'n':
		case 'N':
			infile >> p[0] >> p[1] >> p[2];
			mNormals << p;
			break;
		case 'f':
		case 'F':
			infile >> t[0] >> t[1] >> t[2];
			t -= MyVec3i(1, 1, 1);
			mTriangles << t;
			break;
		case '#':
		default:
			infile.getline(line, 1000);
			break;
		}
	}
	infile.close();
	return 1;
}

int MyMesh::Write(const MyString& fileName){
	ofstream outfile(fileName);
	if (!outfile.is_open()){
		return -1;
	}
	char space = ' ';
	for (int i = 0; i < mVertices.size(); i++){
		outfile << 'v' << space << mVertices[i][0]
			<< space << mVertices[i][1]
			<< space << mVertices[i][2]
			<< endl;
	}
	for (int i = 0; i < mNormals.size(); i++){
		outfile << "vn" << space << mNormals[i][0]
			<< space << mNormals[i][1]
			<< space << mNormals[i][2]
			<< endl;
	}
	if (mNormals.size() == mVertices.size()){
		for (int i = 0; i < mTriangles.size(); i++){
			outfile << 'f' << space;
			for (int j = 0; j < 3; j++){
				int idx = mTriangles[i][j] + 1;
				outfile << idx << "//" << idx << space;
			}
			outfile << endl;
		}
	}
	else{
		for (int i = 0; i < mTriangles.size(); i++){
			outfile << 'f' << space;
			for (int j = 0; j < 3; j++){
				int idx = mTriangles[i][j] + 1;
				outfile << idx << space;
			}
			outfile << endl;
		}
	}
	outfile.close();
	return 1;
}


int MyMesh::ReadAsc(const MyString& fileName){
	ifstream infile(fileName);
	if (!infile.is_open()){
		return -1;
	}
	mVertices.clear();
	mNormals.clear();
	mTriangles.clear();
	int numVertices = -1;
	int numTriangles = -1;
	while (!infile.eof()){
		char line[1000];
		infile.getline(line, 1000);
		int startIdx = 0;
		while (line[startIdx] == ' ') startIdx++;
		if (line[startIdx] == '#') continue;
		stringstream ss(line + startIdx);
		if (numVertices < 0){
			ss >> numVertices >> numTriangles;
			mVertices.reserve(numVertices);
			mTriangles.reserve(numTriangles);
		}
		else if (numVertices > 0){
			MyVec3f p;
			ss >> p[0] >> p[1] >> p[2];
			mVertices.push_back(p);
			numVertices--;
		}
		else if (numTriangles > 0){
			MyVec3i t;
			ss >> t[0] >> t[1] >> t[2];
			mTriangles.push_back(t);
			numTriangles--;
		}
		else{
			break;
		}
	}
	infile.close();
	return 1;
}

int MyMesh::WriteAsc(const MyString& fileName){
	ofstream outfile(fileName);
	if (!outfile.is_open()){
		return -1;
	}
	char space = ' ';
	outfile << mVertices.size() << space
		<< mTriangles.size() << space
		<< endl;
	for (int i = 0; i < mVertices.size(); i++){
		outfile << mVertices[i][0]
			<< space << mVertices[i][1]
			<< space << mVertices[i][2]
			<< space << '0'
			<< endl;
	}
	for (int i = 0; i < mTriangles.size(); i++){
		outfile
			<< mTriangles[i][0] << space
			<< mTriangles[i][1] << space
			<< mTriangles[i][2]
			<< space << '0'
			<< endl;
	}
	outfile.close();
	return 1;
}

int MyMesh::WriteOFF(const MyString& fileName){
	ofstream outfile(fileName);
	if (!outfile.is_open()){
		return -1;
	}
	char space = ' ';
	outfile << "OFF" << endl;
	outfile << mVertices.size() << space
		<< mTriangles.size() << space
		<< '0' << endl << endl;
	for (int i = 0; i < mVertices.size(); i++){
		outfile << mVertices[i][0]
			<< space << mVertices[i][1]
			<< space << mVertices[i][2]
			<< endl;
	}
	for (int i = 0; i < mTriangles.size(); i++){
		outfile << '3' << space
			<< mTriangles[i][0] << space
			<< mTriangles[i][1] << space
			<< mTriangles[i][2]
			<< endl;
	}
	outfile.close();
	return 1;
}

int MyMesh::WriteNOFF(const MyString& fileName){
	ofstream outfile(fileName);
	if (!outfile.is_open()){
		return -1;
	}
	char space = ' ';
	outfile << "NOFF" << endl;
	outfile << mVertices.size() << space
		<< mTriangles.size() << space
		<< '0' << endl;
	for (int i = 0; i < mVertices.size(); i++){
		outfile << mVertices[i][0]
			<< space << mVertices[i][1]
			<< space << mVertices[i][2]
			<< space << mNormals[i][0]
			<< space << mNormals[i][1]
			<< space << mNormals[i][2]
			<< endl;
	}
	for (int i = 0; i < mTriangles.size(); i++){
		outfile << '3' << space
			<< mTriangles[i][0] << space
			<< mTriangles[i][1] << space
			<< mTriangles[i][2]
			<< endl;
	}
	outfile.close();
	return 1;
}

int MyMesh::WriteSMFD(const MyString& fileName){
	ofstream outfile(fileName);
	if (!outfile.is_open()){
		return -1;
	}
	char space = ' ';
	MyVec3f up(0, 0, 1);
	for (int i = 0; i < mVertices.size(); i++){
		outfile << 'v' << space
			<< mVertices[i][0] << space
			<< mVertices[i][1] << space
			<< mVertices[i][2] << endl;
		const MyVec3f& normal = mNormals[i];
		MyVec3f d2 = up^normal;
		MyVec3f d1 = normal ^ d2;
		float f1 = up*d1;
		if (f1 < 0.001 && f1>=0) f1 = 0.001;
		if (f1 > -0.001 && f1<=0) f1 = -0.001;
		float f2 = 1/f1;
		outfile << 'D' << space
			<< f1 << space
			<< d1[0] << space
			<< d1[1] << space
			<< d1[2] << endl;
		outfile << 'd' << space
			<< f2 << space
			<< d2[0] << space
			<< d2[1] << space
			<< d2[2] << endl;
	}
	outfile << endl;
	for (int i = 0; i < mTriangles.size(); i++){
		outfile << "t" << space
			<< mTriangles[i][0] + 1 << space
			<< mTriangles[i][1] + 1 << space
			<< mTriangles[i][2] + 1 << endl;
	}
	outfile.close();
	return 1;
}

int MyMesh::WritePLY(const MyString& fileName){
	ofstream outfile(fileName);
	if (!outfile.is_open()){
		return -1;
	}
	char space = ' ';
	// write header
	outfile << "ply" << endl
		<< "format ascii 1.0" << endl
		<< "element vertex " << mVertices.size() << endl
		<< "property float x" << endl
		<< "property float y" << endl
		<< "property float z" << endl
		<< "element face " << mTriangles.size() << endl
		<< "property list uchar int vertex_indices" << endl
		<< "end_header" << endl;
	for (int i = 0; i < mVertices.size(); i++){
		outfile << mVertices[i][0] << space
			<< mVertices[i][1] << space
			<< mVertices[i][2] << endl;
	}
	for (int i = 0; i < mTriangles.size(); i++){
		outfile << "3" << space
			<< mTriangles[i][0] << space
			<< mTriangles[i][1] << space
			<< mTriangles[i][2] << endl;
	}
	outfile.close();
	return 1;
}

int MyMesh::WriteBIN(const MyString& fileName){
	ofstream outfile(fileName, ios::binary);
	if (!outfile.is_open()){
		return -1;
	}
	int sizes[2] = { mVertices.size(), mTriangles.size() };
	outfile.write((char*)sizes, sizeof(int) * 2);
	outfile.write((char*)(&mVertices[0][0]), mVertices.size()*sizeof(MyVec3f));
	unsigned long* tris = new unsigned long[mTriangles.size() * 3];
	for (int i = 0; i < mTriangles.size(); i++){
		tris[i * 3 + 0] = mTriangles[i][0];
		tris[i * 3 + 1] = mTriangles[i][1];
		tris[i * 3 + 2] = mTriangles[i][2];
	}
	outfile.write((char*)tris, mTriangles.size() * 3 * sizeof(unsigned long));
	delete[] tris;
	outfile.close();
	return 1;
}

void MyMesh::CompileShader(){
	if (glIsShader(mShaderProgram)){
		glDeleteProgram(mShaderProgram);
	}

	// check if there is color
	if (mColors.size() < mVertices.size()){
		mColors.resize(mVertices.size(), MyColor4f(1, 1, 1, 1));
	}
	mShaderProgram = InitShader("Shaders\\geom.vert", "Shaders\\geom.frag", "fragColour", "position");
	//mShaderProgram = InitShader("Shaders\\tracks.vert", "Shaders\\tracks.frag", "fragColour");

	mNormalAttribute = glGetAttribLocation(mShaderProgram, "normal");
	if (mNormalAttribute < 0) {
		cerr << "Shader did not contain the 'normal' attribute." << endl;
	}
	mPositionAttribute = glGetAttribLocation(mShaderProgram, "position");
	if (mPositionAttribute < 0) {
		cerr << "Shader did not contain the 'position' attribute." << endl;
	}
	mColorAttribute = glGetAttribLocation(mShaderProgram, "color");
	if (mColorAttribute < 0) {
		cerr << "Shader did not contain the 'color' attribute." << endl;
	}
}

void MyMesh::Build(){
	if (!glIsShader(mShaderProgram)){
		CompileShader();
	}

	if (glIsVertexArray(mVertexArray)){
		glDeleteVertexArrays(1, &mVertexArray);
	}
	glGenVertexArrays(1, &mVertexArray);
	glBindVertexArray(mVertexArray);
	// vertex
	if (glIsBuffer(mVertexBuffer)){
		glDeleteBuffers(1, &mVertexBuffer);
	}
	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(MyVec3f), &mVertices[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mPositionAttribute);
	glVertexAttribPointer(mPositionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// normal
	if (glIsBuffer(mNormalBuffer)){
		glDeleteBuffers(1, &mNormalBuffer);
	}
	glGenBuffers(1, &mNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(MyVec3f), &mNormals[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mNormalAttribute);
	glVertexAttribPointer(mNormalAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// color
	if (glIsBuffer(mColorBuffer)){
		glDeleteBuffers(1, &mColorBuffer);
	}
	glGenBuffers(1, &mColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, mColors.size() * sizeof(MyColor4f), &mColors[0].r, GL_STATIC_DRAW);
	glEnableVertexAttribArray(mColorAttribute);
	glVertexAttribPointer(mColorAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
	if (glIsBuffer(mIndexBuffer)){
		glDeleteBuffers(1, &mIndexBuffer);
	}
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mTriangles.size() * sizeof(MyVec3i), &mTriangles[0][0], GL_STATIC_DRAW);

	// unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}
void MyMesh::Render(){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(mVertexArray);
	glUseProgram(mShaderProgram);

	int mvmatLocation = glGetUniformLocation(mShaderProgram, "mvMat");
	float modelViewMat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMat);
	glUniformMatrix4fv(mvmatLocation, 1, GL_FALSE, modelViewMat);

	int normatLocation = glGetUniformLocation(mShaderProgram, "normalMat");
	float normalMat[16];
	bool bInv = MyMathHelper::InvertMatrix4x4ColMaj(modelViewMat, normalMat);
	assert(bInv);
	MyMathHelper::TransposeMatrix4x4ColMaj(normalMat, normalMat);
	glUniformMatrix4fv(normatLocation, 1, GL_FALSE, normalMat);

	int projmatLocation = glGetUniformLocation(mShaderProgram, "projMat");
	float projMat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
	glUniformMatrix4fv(projmatLocation, 1, GL_FALSE, projMat);

	int colorLocation = glGetUniformLocation(mShaderProgram, "color");
	glUniform3f(colorLocation, 1, 1, 1);

	int radiusLocation = glGetUniformLocation(mShaderProgram, "radius");
	glUniform1f(radiusLocation, 0);

	int textureLocation = glGetUniformLocation(mShaderProgram, "colorTex");
	glUniform1i(textureLocation, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, mTexture);

	glDrawElements(GL_TRIANGLES, mTriangles.size()*3, GL_UNSIGNED_INT, 0);

	glUseProgram(0);
	glBindVertexArray(0);
	glPopAttrib();
}
void MyMesh::Destory(){
}

void MyMesh::ApplyTransform(const MyMatrixf& mat){
	mBox.Reset();
	for (int i = 0; i < mVertices.size(); i++){
		mVertices[i] = MyMathHelper::MatMulVec(mat, mVertices[i]);
		mBox.Engulf(mVertices[i]);
	}
	MyMatrixf inverse = MyMathHelper::InverseMatrix4x4ColMaj(mat);
	for (int i = 0; i < mNormals.size(); i++){
		mNormals[i] = MyMathHelper::MatMulVec(inverse, mNormals[i]);
	}
}

int MyMesh::GenPerVertexNormal(){
	MyArray4f sumNormal(this->GetNumVertex(), MyVec4f(0, 0, 0, 0));
	for (int i = 0; i < this->GetNumTriangle(); i++){
		MyVec3i triangle = this->GetTriangle(i);
		MyVec3f normal = this->ComputeTriangleNormal(triangle);
		MyVec4f normal2add = MyVec4f(normal[0], normal[1], normal[2], 1);
		float triangleArea = this->ComputeTriangleArea(triangle);
		normal2add[3] = triangleArea;
		sumNormal[triangle[0]] += normal2add;
		sumNormal[triangle[1]] += normal2add;
		sumNormal[triangle[2]] += normal2add;
	}
	mNormals.clear();
	mNormals.resize(this->GetNumVertex());
	for (int i = 0; i < this->GetNumVertex(); i++){
		MyVec4f normal = sumNormal[i];
		if (normal[3] != 0){
			normal /= normal[3];
		}
		mNormals[i] = MyVec3f(normal[0], normal[1], normal[2]).normalized();
	}
	return 1;
}

int MyMesh::Merge(const MyMesh& mesh){
	int vertexSize = mVertices.size();
	mVertices += mesh.mVertices;
	mNormals += mesh.mNormals;
	mTriangles.reserve(mTriangles.size() + mesh.mTriangles.size());
	MyVec3i offset(vertexSize, vertexSize, vertexSize);
	for (int i = 0; i < mesh.mTriangles.size(); i++){
		mTriangles << mesh.mTriangles[i] + offset;
	}
	mBox.Engulf(mesh.mBox);
	return 1;
}

int MyMesh::MergeVertices(float prec){
	// locate unique vertices
	cout << "Before merge: #v = " << mVertices.size()
		<< ", #f = " << mTriangles.size() << endl;
	map<string, int> vertexMap;
	map<int, int> vertexRedirect;
	MyArray3f uniqueVertices;
	for (int i = 0; i < mVertices.size(); i++){
		string key = vertex2string(mVertices[i], prec);
		auto itr = vertexMap.find(key);
		if (itr == vertexMap.end()){
			vertexMap[key] = i;
			vertexRedirect[i] = uniqueVertices.size();
			uniqueVertices.push_back(mVertices[i]);
		}
		else{
			vertexRedirect[i] = vertexRedirect[itr->second];
		}
	}

	// flag degenerated triangles
	vector<int> trianlgesToRemove;
	for (int i = 0; i < mTriangles.size(); i++){
		MyVec3i triangle = mTriangles[i];
		MyVec3i triangleRedirected;
		for (int iv = 0; iv < 3; iv++){
			triangleRedirected[iv] = vertexRedirect[triangle[iv]];
		}
		if (triangleRedirected[0] == triangleRedirected[1]
			|| triangleRedirected[1] == triangleRedirected[2]
			|| triangleRedirected[2] == triangleRedirected[0]){
			trianlgesToRemove.push_back(i);
		}
	}

	// construct cleaned triangles
	mTriangles.EraseBySortedIndices(&trianlgesToRemove);
	for (int i = 0; i < mTriangles.size(); i++){
		MyVec3i& triangle = mTriangles[i];
		for (int iv = 0; iv < 3; iv++){
			triangle[iv] = vertexRedirect[triangle[iv]];
		}
	}

	int diff = mVertices.size() - uniqueVertices.size();
	mVertices = uniqueVertices;

	cout << "After merge: #v = " << mVertices.size()
		<< ", #f = " << mTriangles.size() << endl;

	return diff;
}

int MyMesh::RemoveVertex(const MyArrayi& vertices){

	cout << "Before merge: #v = " << mVertices.size()
		<< ", #f = " << mTriangles.size() << endl;

	MyArrayi vs = vertices;
	sort(vs.begin(), vs.end());
	MyArrayi vertexLeft = MyArrayi::GetSequence(0, mVertices.size() - 1);
	vertexLeft.EraseBySortedIndices(&vs);
	MyArrayi vertexRedirect(mVertices.size(), -1);
	for (int i = 0; i < vertexLeft.size();i++){
		int newIdx = i;
		int oldIdx = vertexLeft[i];
		vertexRedirect[oldIdx] = newIdx;
	}

	// flag bad triangles
	vector<int> trianlgesToRemove;
	for (int i = 0; i < mTriangles.size(); i++){
		MyVec3i triangle = mTriangles[i];
		MyVec3i triangleRedirected;
		for (int iv = 0; iv < 3; iv++){
			triangleRedirected[iv] = vertexRedirect[triangle[iv]];
		}
		if (triangleRedirected[0] == -1
			|| triangleRedirected[1] == -1
			|| triangleRedirected[2] == -1){
			trianlgesToRemove.push_back(i);
		}
	}

	mVertices.EraseBySortedIndices(&vs);
	mTriangles.EraseBySortedIndices(&trianlgesToRemove);
	for (int i = 0; i < mTriangles.size(); i++){
		MyVec3i& triangle = mTriangles[i];
		for (int iv = 0; iv < 3; iv++){
			triangle[iv] = vertexRedirect[triangle[iv]];
		}
	}

	cout << "After remove: #v = " << mVertices.size()
		<< ", #f = " << mTriangles.size() << endl;

	return 1;
}

MyVec3f MyMesh::GetVertex(int idx) const{
	return mVertices[idx];
}

MyVec3f MyMesh::GetNormal(int idx) const{
	return mNormals[idx];
}

MyVec3i MyMesh::GetTriangle(int idx) const{
	return mTriangles[idx];
}

MyBoundingBox MyMesh::GetBoundingBox() const{
	return mBox;
}

int MyMesh::GetNumVertex() const{
	return mVertices.size();
}

int MyMesh::GetNumNormal() const{
	return mNormals.size();
}
int MyMesh::GetNumTriangle() const{
	return mTriangles.size();
}


const float* MyMesh::GetVertexData() const{
	return &mVertices[0][0];
}

const float* MyMesh::GetNormalData() const{
	return &mNormals[0][0];
}

const int* MyMesh::GetTriangleData() const{
	return &mTriangles[0][0];
}

MyVec3f MyMesh::ComputeTriangleNormal(const MyVec3i& triangle) const{
	MyVec3f p0 = this->GetVertex(triangle[0]);
	MyVec3f p1 = this->GetVertex(triangle[1]);
	MyVec3f p2 = this->GetVertex(triangle[2]);
	return (p1 - p0) ^ (p2 - p1);
}

float MyMesh::ComputeTriangleArea(const MyVec3i& triangle) const{
	MyVec3f p0 = this->GetVertex(triangle[0]);
	MyVec3f p1 = this->GetVertex(triangle[1]);
	MyVec3f p2 = this->GetVertex(triangle[2]);
	return abs((p1 - p0).dotMultiply(p2 - p0));
}

void makeEdge(MyVec2i& edge){
	if (edge[0] > edge[1]){
		int tmp = edge[0];
		edge[0] = edge[1];
		edge[1] = tmp;
	}
}


void increaseEdgeTriangleCount(map<MyVec2i, int, bool(*)(MyVec2i, MyVec2i)>& edgeNumTriangle, MyVec2i edge){
	map<MyVec2i, int>::iterator itr = edgeNumTriangle.find(edge);
	if (itr != edgeNumTriangle.end())
		itr->second++;
	else
		edgeNumTriangle[edge] = 1;
}

MyVec2i MyMesh::makeEdge(int i, int j){
	MyVec2i edge(i, j);
	if (i > j){
		edge[0] = j;
		edge[1] = i;
	}
	return edge;
}

bool MyMesh::compareEdge(MyVec2i edge0, MyVec2i edge1){
	if (edge0[0] < edge1[0]) return true;
	if (edge0[0] > edge1[0]) return false;
	if (edge0[1] < edge1[1]) return true;
	//if (edge0[1] > edge1[1]) return false;
	return false;
}

int MyMesh::ClearNonRegularFaces(){
	int numTriangles = mTriangles.size();
	MyArrayb removeTag(numTriangles, false);

	bool(*fn_pt)(MyVec2i, MyVec2i) = compareEdge;
	map<MyVec2i, int, bool(*)(MyVec2i, MyVec2i)> edgeNumTriangle(compareEdge);
	for (int i = 0; i < numTriangles; i++){
		if (!removeTag[i]){
			MyVec3i triangle = mTriangles[i];
			MyVec2i edge0 = makeEdge(triangle[0], triangle[1]);
			MyVec2i edge1 = makeEdge(triangle[1], triangle[2]);
			MyVec2i edge2 = makeEdge(triangle[2], triangle[0]);
			increaseEdgeTriangleCount(edgeNumTriangle, edge0);
			increaseEdgeTriangleCount(edgeNumTriangle, edge1);
			increaseEdgeTriangleCount(edgeNumTriangle, edge2);
		}
	}

	int numBadEdge = 0;
	int evilEdge = 0;
	for (map<MyVec2i, int, bool(*)(MyVec2i, MyVec2i)>::iterator itr = edgeNumTriangle.begin();
		itr != edgeNumTriangle.end(); itr++){
		if (!(itr->second > 1)){
			numBadEdge++;
		}
		if (itr->second > 2){
			evilEdge++;
		}
	}
	cout << "number bad edge: " << numBadEdge << endl;
	cout << "number evil edge: " << evilEdge << endl;

	int numRemoval = 0;
	int numItr = 0;
	do{
		numRemoval = 0;
		for (int i = 0; i < numTriangles; i++){
			if (!removeTag[i]){
				MyVec3i triangle = mTriangles[i];
				MyVec2i edge0 = makeEdge(triangle[0], triangle[1]);
				MyVec2i edge1 = makeEdge(triangle[1], triangle[2]);
				MyVec2i edge2 = makeEdge(triangle[2], triangle[0]);
				if (edgeNumTriangle[edge0] == 1){
					removeTag[i] = true;
					edgeNumTriangle[edge0] --;
					edgeNumTriangle[edge1] --;
					edgeNumTriangle[edge2] --;
					numRemoval++;
					cout << "remove triange: " << i << endl;
					continue;
				}
				if (edgeNumTriangle[edge1] == 1){
					removeTag[i] = true;
					edgeNumTriangle[edge0] --;
					edgeNumTriangle[edge1] --;
					edgeNumTriangle[edge2] --;
					numRemoval++;
					cout << "remove triange: " << i << endl;
					continue;
				}
				if (edgeNumTriangle[edge2] == 1){
					removeTag[i] = true;
					edgeNumTriangle[edge0] --;
					edgeNumTriangle[edge1] --;
					edgeNumTriangle[edge2] --;
					numRemoval++;
					cout << "remove triange: " << i << endl;
					continue;
				}
			}
		}
		cout << numRemoval << " removed in iteration " << numItr << endl;
		numItr++;
	} while (numRemoval>0);

	numBadEdge = 0;
	evilEdge = 0;
	for (map<MyVec2i, int, bool(*)(MyVec2i, MyVec2i)>::iterator itr = edgeNumTriangle.begin();
		itr != edgeNumTriangle.end(); itr++){
		if (!(itr->second > 1)){
			numBadEdge++;
		}
		if (itr->second > 2){
			evilEdge++;
		}
	}
	cout << "number bad edge: " << numBadEdge << endl;
	cout << "number evil edge: " << evilEdge << endl;

	int health = 0;
	for (int i = 0; i < numTriangles; i++){
		if (!removeTag[i]){
			mTriangles[health++] = mTriangles[i];
		}
	}
	mTriangles.resize(health);

	return 1;
}

string MyMesh::vertex2string(const MyVec3f& vec, float precision){
	string rst = to_string((int)round(vec[0] / precision)) +"_"
		+ to_string((int)round(vec[1] / precision)) + "_"
		+ to_string((int)round(vec[2] / precision));
	return rst;
}