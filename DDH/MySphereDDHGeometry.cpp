#include "MySphereDDHGeometry.h"
#include "MyArray.h"
#include "Shader.h"

#include "GL\glew.h"
#include <GL/freeglut.h>

#include <iostream>
using namespace std;

#define PI 3.141592653589793238462643


MySphereDDHGeometry::MySphereDDHGeometry():
	MySphereGeometry(),mHaloColor(1, 1, 1, 1){
}


MySphereDDHGeometry::~MySphereDDHGeometry()
{
	if (glIsProgram(mBasicShader)) glDeleteProgram(mBasicShader);
	if (glIsProgram(mDDHShader)) glDeleteProgram(mDDHShader);
	if (glIsVertexArray(mDDHVertexArray)) glDeleteVertexArrays(1, &mDDHVertexArray);
	if (glIsBuffer(mDDHVertexBuffer)) glDeleteBuffers(1, &mDDHVertexBuffer);
	if (glIsBuffer(mDDHAngleBuffer)) glDeleteBuffers(1, &mDDHAngleBuffer);
	if (glIsBuffer(mDDHIndexBuffer)) glDeleteBuffers(1, &mDDHIndexBuffer);
}

void MySphereDDHGeometry::GenerateGeometry(){
	CompileShaders();
	MyArray3f vertices;
	MyArray3f normals;
	MyArray3i triangles;
	int idxOffset = 0;
	int tIdxOffset = 0;
	int vSize = idxOffset + (mMeshDetailX + 1)*(mMeshDetailY + 1);
	int tSize = tIdxOffset + mMeshDetailX*(mMeshDetailY - 1) * 2;
	if (vertices.size() < vSize) vertices.resize(vSize);
	if (normals.size() < vSize) normals.resize(vSize);
	if (triangles.size() < tSize) triangles.resize(tSize);

	for (int h = 0, idx = idxOffset; h < mMeshDetailX; h++){
		float angleX = h * PI * 2.f / mMeshDetailX;
		for (int v = 0; v <= mMeshDetailY; v++, idx++){
			float angleY = v * PI / mMeshDetailY;
			float x = sin(angleX)*sin(angleY);
			float y = cos(angleX)*sin(angleY);
			float z = cos(angleY);
			MyVec3f p(x, y, z);
			vertices[idx] = p*mSphere->GetRadius() + mSphere->GetCenter();
			normals[idx] = p;
		}
	}
	memcpy(&vertices[mMeshDetailX*(mMeshDetailY + 1) + idxOffset][0],
		&vertices[idxOffset][0], (mMeshDetailY + 1)*sizeof(MyVec3f));
	memcpy(&normals[mMeshDetailX*(mMeshDetailY + 1) + idxOffset][0],
		&normals[idxOffset][0], (mMeshDetailY + 1)*sizeof(MyVec3f));

	// indices
	for (int h = 0, tidx = tIdxOffset; h < mMeshDetailX; h++){
		for (int v = 0; v < mMeshDetailY; v++){
			int idx = h*(mMeshDetailY + 1) + v + idxOffset;
			if (v != 0) triangles[tidx++] = MyVec3i(idx, idx + mMeshDetailY + 1, idx + mMeshDetailY + 2);
			if (v != mMeshDetailY - 1) triangles[tidx++] = MyVec3i(idx, idx + mMeshDetailY + 2, idx + 1);
		}
	}

	// load to GPU
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
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MyVec3f), &vertices[0][0], GL_STATIC_DRAW);
	int attribute = glGetAttribLocation(mBasicShader, "position");
	glEnableVertexAttribArray(attribute);
	glVertexAttribPointer(attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// normal
	if (glIsBuffer(mNormalBuffer)){
		glDeleteBuffers(1, &mNormalBuffer);
	}
	glGenBuffers(1, &mNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(MyVec3f), &normals[0][0], GL_STATIC_DRAW);
	attribute = glGetAttribLocation(mBasicShader, "normal");
	glEnableVertexAttribArray(attribute);
	glVertexAttribPointer(attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// index
	if (glIsBuffer(mIndexBuffer)){
		glDeleteBuffers(1, &mIndexBuffer);
	}
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(MyVec3i), &triangles[0][0], GL_STATIC_DRAW);
	// unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// ddh geometries
	vertices.clear();
	vertices.resize(1 + mMeshDetailX, mSphere->GetCenter());
	MyArray2f texCoords(1 + mMeshDetailX, MyVec2f(1, mSphere->GetRadius()));
	MyArrayf angles(1 + mMeshDetailX);
	triangles.resize(mMeshDetailX);
	angles[0] = 0;
	texCoords[0] = MyVec2f(0, mSphere->GetRadius());
	for (int i = 1; i <= mMeshDetailX; i++){
		angles[i] = (i-1) / float(mMeshDetailX) * 2 * PI;
		triangles[i-1] = MyVec3i(0, i, i % mMeshDetailX + 1);
	}

	// load to GPU
	if (glIsVertexArray(mDDHVertexArray)){
		glDeleteVertexArrays(1, &mDDHVertexArray);
	}
	glGenVertexArrays(1, &mDDHVertexArray);
	glBindVertexArray(mDDHVertexArray);
	// vertex
	if (glIsBuffer(mDDHVertexBuffer)){
		glDeleteBuffers(1, &mDDHVertexBuffer);
	}
	glGenBuffers(1, &mDDHVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mDDHVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MyVec3f), &vertices[0][0], GL_STATIC_DRAW);
	attribute = glGetAttribLocation(mDDHShader, "position");
	glEnableVertexAttribArray(attribute);
	glVertexAttribPointer(attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// texCoord
	if (glIsBuffer(mTexCoordBuffer)){
		glDeleteBuffers(1, &mTexCoordBuffer);
	}
	glGenBuffers(1, &mTexCoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mTexCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(MyVec2f), &texCoords[0][0], GL_STATIC_DRAW);
	attribute = glGetAttribLocation(mDDHShader, "texCoord");
	glEnableVertexAttribArray(attribute);
	glVertexAttribPointer(attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	// angle
	if (glIsBuffer(mDDHAngleBuffer)){
		glDeleteBuffers(1, &mDDHAngleBuffer);
	}
	glGenBuffers(1, &mDDHAngleBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mDDHAngleBuffer);
	glBufferData(GL_ARRAY_BUFFER, angles.size() * sizeof(float), &angles[0], GL_STATIC_DRAW);
	attribute = glGetAttribLocation(mDDHShader, "angle");
	glEnableVertexAttribArray(attribute);
	glVertexAttribPointer(attribute, 1, GL_FLOAT, GL_FALSE, 0, 0);
	// index
	if (glIsBuffer(mDDHIndexBuffer)){
		glDeleteBuffers(1, &mDDHIndexBuffer);
	}
	glGenBuffers(1, &mDDHIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mDDHIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(MyVec3i), &triangles[0][0], GL_STATIC_DRAW);
	// unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void MySphereDDHGeometry::DrawGeometry(){
	float mvMat[16], mvMatInv[16], projMat[16];
	float stripWidth, stripDepth;
	glGetUniformfv(mShaderProgram, glGetUniformLocation(mShaderProgram, "mvMat"), mvMat);
	glGetUniformfv(mShaderProgram, glGetUniformLocation(mShaderProgram, "mvMatInv"), mvMatInv);
	glGetUniformfv(mShaderProgram, glGetUniformLocation(mShaderProgram, "projMat"), projMat);
	glGetUniformfv(mShaderProgram, glGetUniformLocation(mShaderProgram, "stripWidth"), &stripWidth);
	glGetUniformfv(mShaderProgram, glGetUniformLocation(mShaderProgram, "stripDepth"), &stripDepth);

	glUseProgram(mBasicShader);
	glBindVertexArray(mVertexArray);
	glUniformMatrix4fv(glGetUniformLocation(mBasicShader, "mvMat"), 1, GL_FALSE, mvMat);
	glUniformMatrix4fv(glGetUniformLocation(mBasicShader, "mvMatInv"), 1, GL_FALSE, mvMatInv);
	glUniformMatrix4fv(glGetUniformLocation(mBasicShader, "projMat"), 1, GL_FALSE, projMat);
	glUniform4fv(glGetUniformLocation(mBasicShader, "baseColor"), 1, &(mColor.r));
	glDrawElements(GL_TRIANGLES, mMeshDetailX*(mMeshDetailY - 1) * 2 * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glUseProgram(mDDHShader);
	glBindVertexArray(mDDHVertexArray);
	glUniformMatrix4fv(glGetUniformLocation(mDDHShader, "mvMat"), 1, GL_FALSE, mvMat);
	glUniformMatrix4fv(glGetUniformLocation(mDDHShader, "mvMatInv"), 1, GL_FALSE, mvMatInv);
	glUniformMatrix4fv(glGetUniformLocation(mDDHShader, "projMat"), 1, GL_FALSE, projMat);
	glUniform4fv(glGetUniformLocation(mDDHShader, "baseColor"), 1, &(mHaloColor.r));
	glUniform1f(glGetUniformLocation(mDDHShader, "stripWidth"), stripWidth);
	glUniform1f(glGetUniformLocation(mDDHShader, "stripDepth"), stripDepth);
	glUniform1f(glGetUniformLocation(mDDHShader, "radius"), mSphere->GetRadius());
	glDrawElements(GL_TRIANGLES, mMeshDetailX * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void MySphereDDHGeometry::CompileShaders(){
	if (glIsProgram(mBasicShader)) glDeleteProgram(mBasicShader);
	mBasicShader = InitShader("..\\DDH\\shaders\\basic.vert", "..\\DDH\\shaders\\basic.frag", "fragColour");

	if (glIsProgram(mDDHShader)) glDeleteProgram(mDDHShader);
	mDDHShader = InitShader("..\\DDH\\shaders\\sphereDDH.vert", "..\\DDH\\shaders\\sphereDDH.frag", "fragColour");
}