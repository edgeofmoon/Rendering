#include "MySphereGeometry.h"
#include "MyArray.h"

#include "GL\glew.h"
#include <GL/freeglut.h>

#define PI 3.141592653589793238462643

MySphereGeometry::MySphereGeometry()
{
	mMeshDetailX = 100;
	mMeshDetailY = 100;
	mColor = MyColor4f(0.5, 0.5, 0.5, 1);
}


MySphereGeometry::~MySphereGeometry()
{
	if (glIsVertexArray(mVertexArray)) glDeleteVertexArrays(1, &mVertexArray);
	if (glIsBuffer(mVertexBuffer)) glDeleteBuffers(1, &mVertexBuffer);
	if (glIsBuffer(mNormalBuffer)) glDeleteBuffers(1, &mNormalBuffer);
	if (glIsBuffer(mTexCoordBuffer)) glDeleteBuffers(1, &mTexCoordBuffer);
	if (glIsBuffer(mColorBuffer)) glDeleteBuffers(1, &mColorBuffer);
	if (glIsBuffer(mValueBuffer)) glDeleteBuffers(1, &mValueBuffer);
	if (glIsBuffer(mIndexBuffer)) glDeleteBuffers(1, &mIndexBuffer);
}

void MySphereGeometry::GenerateGeometry(){
	MyArray3f vertices;
	MyArray3f normals;
	MyArray2f texCoords;
	MyArray3i triangles;
	MyArray<MyColor4f> colors;
	MyArrayf values;
	int idxOffset = 0;
	int tIdxOffset = 0;
	int vSize = idxOffset + GetVertexCount();
	int tSize = tIdxOffset + GetTriangleCount();
	if (vertices.size() < vSize) vertices.resize(vSize);
	if (normals.size() < vSize) normals.resize(vSize);
	if (texCoords.size() < vSize) texCoords.resize(vSize);
	if (triangles.size() < tSize) triangles.resize(tSize);
	colors.resize(tSize, mColor);
	values.resize(tSize, 0);

	for (int h = 0, idx = idxOffset; h < mMeshDetailX; h++){
		float angleX = h * PI * 2.f / mMeshDetailX;
		for (int v = 0; v <= mMeshDetailY; v++, idx++){
			float angleY = v * PI / mMeshDetailY;
			float x = sin(angleX)*sin(angleY);
			float y = cos(angleX)*sin(angleY);
			float z = cos(angleY);
			MyVec3f p(x,y,z);
			vertices[idx] = p*mSphere->GetRadius() + mSphere->GetCenter();
			texCoords[idx] = MyVec2f((float)h / mMeshDetailX, (float)v / mMeshDetailY);
			normals[idx] = p;
		}
	}
	memcpy(&vertices[mMeshDetailX*(mMeshDetailY + 1) + idxOffset][0],
		&vertices[idxOffset][0], (mMeshDetailY + 1)*sizeof(MyVec3f));
	memcpy(&normals[mMeshDetailX*(mMeshDetailY + 1) + idxOffset][0],
		&normals[idxOffset][0], (mMeshDetailY + 1)*sizeof(MyVec3f));
	for (int v = 0, idx = mMeshDetailX*(mMeshDetailY + 1) + idxOffset; v <= mMeshDetailY; v++, idx++){
		//vertices[idx] = vertices[idxOffset + v];
		texCoords[idx] = MyVec2f(1.f, (float)v / mMeshDetailY);
	}

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
	int attribute = glGetAttribLocation(mShaderProgram, "position");
	glEnableVertexAttribArray(attribute);
	glVertexAttribPointer(attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// normal
	if (glIsBuffer(mNormalBuffer)){
		glDeleteBuffers(1, &mNormalBuffer);
	}
	glGenBuffers(1, &mNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(MyVec3f), &normals[0][0], GL_STATIC_DRAW);
	attribute = glGetAttribLocation(mShaderProgram, "normal");
	glEnableVertexAttribArray(attribute);
	glVertexAttribPointer(attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// color
	if (glIsBuffer(mColorBuffer)){
		glDeleteBuffers(1, &mColorBuffer);
	}
	glGenBuffers(1, &mColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(MyColor4f), &colors[0].r, GL_STATIC_DRAW);
	attribute = glGetAttribLocation(mShaderProgram, "color");
	glEnableVertexAttribArray(attribute);
	glVertexAttribPointer(attribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
	// texCoord
	if (texCoords.size() > 0){
		if (glIsBuffer(mTexCoordBuffer)){
			glDeleteBuffers(1, &mTexCoordBuffer);
		}
		glGenBuffers(1, &mTexCoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, mTexCoordBuffer);
		glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(MyVec2f), &texCoords[0][0], GL_STATIC_DRAW);
		attribute = glGetAttribLocation(mShaderProgram, "texCoord");
		glEnableVertexAttribArray(attribute);
		glVertexAttribPointer(attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}
	// value
	if (values.size() > 0){
		if (glIsBuffer(mValueBuffer)){
			glDeleteBuffers(1, &mValueBuffer);
		}
		glGenBuffers(1, &mValueBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, mValueBuffer);
		glBufferData(GL_ARRAY_BUFFER, values.size() * sizeof(float), &values[0], GL_STATIC_DRAW);
		attribute = glGetAttribLocation(mShaderProgram, "value");
		glEnableVertexAttribArray(attribute);
		glVertexAttribPointer(attribute, 1, GL_FLOAT, GL_FALSE, 0, 0);
	}
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
}

void MySphereGeometry::DrawGeometry(){
	glBindVertexArray(mVertexArray);
	glUniform4fv(glGetUniformLocation(mShaderProgram, "baseColor"), 1, &mColor.r);
	glDrawElements(GL_TRIANGLES, GetTriangleCount()*3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}