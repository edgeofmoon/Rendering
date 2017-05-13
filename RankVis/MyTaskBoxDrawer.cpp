#include "MyTaskBoxDrawer.h"


#include "GL\glew.h"
#include <GL/freeglut.h>

MyTaskBoxDrawer::MyTaskBoxDrawer(const MyBoundingBox* box){
	SetBox(box);
	mHighlighted = false;
}


MyTaskBoxDrawer::~MyTaskBoxDrawer(){

}

void MyTaskBoxDrawer::Draw(){
	MyVec3f lowerEnd = mBox->GetLowPos();
	MyVec3f higherEnd = mBox->GetHighPos();

	MyVec3f vertices[8];
	vertices[0] = MyVec3f(higherEnd[0], lowerEnd[1], lowerEnd[2]);
	vertices[1] = MyVec3f(higherEnd[0], higherEnd[1], lowerEnd[2]);
	vertices[2] = MyVec3f(higherEnd[0], higherEnd[1], higherEnd[2]);
	vertices[3] = MyVec3f(higherEnd[0], lowerEnd[1], higherEnd[2]);
	vertices[4] = MyVec3f(lowerEnd[0], lowerEnd[1], lowerEnd[2]);
	vertices[5] = MyVec3f(lowerEnd[0], higherEnd[1], lowerEnd[2]);
	vertices[6] = MyVec3f(lowerEnd[0], higherEnd[1], higherEnd[2]);
	vertices[7] = MyVec3f(lowerEnd[0], lowerEnd[1], higherEnd[2]);

	MyVec4i faces[6];
	faces[0] = MyVec4i(0, 1, 2, 3); // front
	faces[1] = MyVec4i(4, 7, 6, 5); // back
	faces[2] = MyVec4i(0, 3, 7, 4); // left
	faces[3] = MyVec4i(5, 6, 2, 1); // right
	faces[4] = MyVec4i(2, 6, 7, 3); // top
	faces[5] = MyVec4i(0, 4, 5, 1); // bottom

	int edges[12][2];
	edges[0][0] = 0;  edges[0][1] = 1;
	edges[1][0] = 1;  edges[1][1] = 2;
	edges[2][0] = 2;  edges[2][1] = 3;
	edges[3][0] = 3;  edges[3][1] = 0;
	edges[4][0] = 4;  edges[4][1] = 5;
	edges[5][0] = 5;  edges[5][1] = 6;
	edges[6][0] = 6;  edges[6][1] = 7;
	edges[7][0] = 7;  edges[7][1] = 4;
	edges[8][0] = 0;  edges[8][1] = 4;
	edges[9][0] = 1;  edges[9][1] = 5;
	edges[10][0] = 2; edges[10][1] = 6;
	edges[11][0] = 3; edges[11][1] = 7;

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	if (mHighlighted) glColor4f(0.8, 0, 0, 1.0);
	else glColor4f(0.8, 0.8, 0.8, 1.0);
	for (int iface = 0; iface<6; iface++){
		glBegin(GL_POLYGON); {
			MyVec4i vidx = faces[iface];
			glVertex3f(vertices[vidx[0]][0], vertices[vidx[0]][1], vertices[vidx[0]][2]);
			glVertex3f(vertices[vidx[1]][0], vertices[vidx[1]][1], vertices[vidx[1]][2]);
			glVertex3f(vertices[vidx[2]][0], vertices[vidx[2]][1], vertices[vidx[2]][2]);
			glVertex3f(vertices[vidx[3]][0], vertices[vidx[3]][1], vertices[vidx[3]][2]);
		}glEnd();
	}
	glDisable(GL_CULL_FACE);

	glLineWidth(2);
	glColor3f(1, 1, 0);
	for (int iedge = 0; iedge<12; iedge++){
		glBegin(GL_LINES); {
			int vidx1 = edges[iedge][0];
			int vidx2 = edges[iedge][1];
			glVertex3f(vertices[vidx1][0], vertices[vidx1][1], vertices[vidx1][2]);
			glVertex3f(vertices[vidx2][0], vertices[vidx2][1], vertices[vidx2][2]);
		}glEnd();
	}
	glPopAttrib();
}