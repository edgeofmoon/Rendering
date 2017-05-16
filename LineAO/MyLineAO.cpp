#include "MyLineAO.h"
#include "MyVec.h"
#include "Shader.h"
#include "MyGraphicsTool.h"
#include "MyMathHelper.h"
#include "MyBlackBodyColor.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <cassert>
using namespace std;

MyLineAO::MyLineAO()
	:MyTractVisBase()
{
	mLineAOTotalStrength = 1.0f;
	mBlurRadius = 0;
	m_width = m_height = 0;
}

MyLineAO::~MyLineAO() {
	if (glIsProgram(mLineShader)) glDeleteProgram(mLineShader);
	if (glIsProgram(mTextureShader)) glDeleteProgram(mTextureShader);
	if (glIsProgram(mAOShader)) glDeleteProgram(mAOShader);

	glDeleteVertexArrays( 1, &mVertexArray );
	glDeleteBuffers( 1, &mVertexBuffer );
	glDeleteBuffers( 1, &mTangentBuffer );
	glDeleteBuffers( 1, &mIndexBuffer );

	glDeleteVertexArrays( 1, &quadVAO );
	glDeleteBuffers( 1, &quadVBO );
	glDeleteBuffers( 1, &quadIBO );
	glDeleteBuffers( 1, &quadTex );

	glDeleteFramebuffers( 1, &gBuffer );
	glDeleteTextures( 1, &gColor );
	glDeleteTextures( 1, &gNDMap );
	glDeleteTextures( 1, &gZoomMap );

	glDeleteRenderbuffers( 1, &depthRBO );

	glDeleteTextures( 1, &noise );
}

void MyLineAO::Resize(int w, int h){
	if (m_width != w || m_height != h){
		m_width = w;
		m_height = h;
		initGBuffer();
		initAOBuffer();
		genNoiseTexture();
	}
}

void MyLineAO::LoadShader(){

	glDeleteProgram(mLineShader);
	mLineShader = InitShader("..\\LineAO\\shaders\\Line.vert", "..\\LineAO\\shaders\\Line.frag", "fragColour");
	mPositionAttribute = glGetAttribLocation(mLineShader, "position");
	if (mPositionAttribute < 0) {
		cerr << "Shader did not contain the 'position' attribute." << endl;
	}
	mTangentAttribute = glGetAttribLocation(mLineShader, "tangent");
	if (mTangentAttribute < 0) {
		cerr << "Shader did not contain the 'tangent' attribute." << endl;
	}
	mNormalAttribute = glGetAttribLocation(mLineShader, "normal");
	if (mNormalAttribute < 0) {
		cerr << "Shader did not contain the 'normal' attribute." << endl;
	}

	glDeleteProgram(mTextureShader);
	mTextureShader = InitShader("..\\LineAO\\shaders\\Lightning.vert", "..\\LineAO\\shaders\\Lightning.frag", "fragColour");

	glDeleteProgram(mAOShader);
	mAOShader = InitShader("..\\LineAO\\shaders\\LineAO.vert", "..\\LineAO\\shaders\\LineAO.frag", "fragColour");
}


void MyLineAO::ComputeGeometry(){
	ClearGeometry();
	if (mRenderingParameters.Shape == TRACK_SHAPE_TUBE){
		this->ComputeTubeGeometry();
	}
	else{
		this->ComputeLineGeometry();
	}
}


void MyLineAO::ComputeTubeGeometry(){
	int nFaces = mRenderingParameters.Faces;
	int currentIdx = 0;
	mIdxOffset.clear();
	int totalPoints = 0;
	for (int it = 0; it < mTracts->GetNumTracks(); it++){
		totalPoints += mTracts->At(it).Size();
	}
	totalPoints *= (nFaces + 1);

	// for caps
	totalPoints += mTracts->GetNumTracks()*(1 + nFaces) * 2;

	cout << "Allocating Storage for Geometry...\r";

	mVertices.resize(totalPoints);
	mNormals.resize(totalPoints);
	mTangents.resize(totalPoints);

	for (int it = 0; it < mTracts->GetNumTracks(); it++){
		if ((int)((it + 1) * 100 / (float)mTracts->GetNumTracks())
			- (int)(it * 100 / (float)mTracts->GetNumTracks()) >= 1){
			cout << "Computing: " << it*100.f / mTracts->GetNumTracks() << "%.          \r";
		}
		int npoints = mTracts->At(it).Size();

		const float myPI = 3.1415926f;
		float dangle = 2 * myPI / nFaces;
		MyVec3f pole(0.6, 0.8, 0);

		MyArray3f candicates;
		candicates << MyVec3f(0, 0, 1)
			<< MyVec3f(0, 1, 1)
			<< MyVec3f(0, 1, 0)
			<< MyVec3f(1, 1, 0)
			<< MyVec3f(1, 0, 0)
			<< MyVec3f(1, 0, 1);
		float max = -1;
		int maxIdx;
		MyVec3f genDir = mTracts->GetCoord(it, 0) - mTracts->GetCoord(it, npoints - 1);
		genDir.normalize();
		for (int i = 0; i<candicates.size(); i++){
			float cp = (candicates[i].normalized() ^ genDir).norm();
			if (cp>max){
				max = cp;
				maxIdx = i;
			}
		}
		pole = candicates[maxIdx].normalized();

		for (int i = 0; i<npoints; i++){
			MyVec3f p = mTracts->GetCoord(it, i);
			MyVec3f d;
			if (i == npoints - 1) d = p - mTracts->GetCoord(it, i - 1);
			else if (i == 0) d = mTracts->GetCoord(it, i + 1) - p;
			else d = mTracts->GetCoord(it, i + 1) - mTracts->GetCoord(it, i - 1);

			MyVec3f perpend1 = (pole^d).normalized();
			MyVec3f perpend2 = (perpend1^d).normalized();
			//if ((perpend1^perpend2)*d < 0) dangle = -dangle;
			for (int is = 0; is<nFaces; is++){
				float angle = dangle*is;
				MyVec3f pt = sin(angle)*perpend1 + cos(angle)*perpend2;
				mVertices[currentIdx + i*(nFaces + 1) + is] = pt * 0 + p;
				mNormals[currentIdx + i*(nFaces + 1) + is] = pt;
				mTangents[currentIdx + i*(nFaces + 1) + is] = -d.normalized();
			}
			mVertices[currentIdx + i*(nFaces + 1) + nFaces] = mVertices[currentIdx + i*(nFaces + 1)];
			mNormals[currentIdx + i*(nFaces + 1) + nFaces] = mNormals[currentIdx + i*(nFaces + 1)];
			mTangents[currentIdx + i*(nFaces + 1) + nFaces] = mTangents[currentIdx + i*(nFaces + 1)];
		}

		mIdxOffset << currentIdx;
		currentIdx += npoints*(nFaces + 1);

		// add front cap
		{
			MyVec3f p = mTracts->GetCoord(it, 0);
			MyVec3f d = mTracts->GetCoord(it, 1) - p;
			d.normalize();
			MyVec3f perpend1 = (pole^d).normalized();
			MyVec3f perpend2 = (perpend1^d).normalized();
			mVertices[currentIdx] = p;
			mNormals[currentIdx] = -d;
			mTangents[currentIdx] = -d;
			for (int is = 0; is < nFaces; is++){
				float angle = dangle*is;
				MyVec3f pt = sin(angle)*perpend1 + cos(angle)*perpend2;
				MyVec3f pe = pt*0 + p;
				mVertices[currentIdx + is + 1] = pe;
				mNormals[currentIdx + is + 1] = pt;
				mTangents[currentIdx + is + 1] = -d;
			}
		}

		// add back cap
		currentIdx += nFaces + 1;
		{
			MyVec3f p = mTracts->GetCoord(it, npoints - 1);
			MyVec3f d = mTracts->GetCoord(it, npoints - 2) - p;
			d.normalize();
			MyVec3f perpend1 = (pole^d).normalized();
			MyVec3f perpend2 = (perpend1^d).normalized();
			mVertices[currentIdx] = p;
			mNormals[currentIdx] = -d;
			mTangents[currentIdx] = -d;
			for (int is = 0; is < nFaces; is++){
				float angle = dangle*is;
				MyVec3f pt = sin(angle)*perpend1 + cos(angle)*perpend2;
				MyVec3f pe = pt*0 + p;
				mVertices[currentIdx + is + 1] = pe;
				mNormals[currentIdx + is + 1] = pt;
				mTangents[currentIdx + is + 1] = -d;
			}
		}
		currentIdx += nFaces + 1;
	}
	// index
	mIndices.clear();
	for (int it = 0; it<mTracts->GetNumTracks(); it++){
		int offset = mIdxOffset[it];
		for (int i = 1; i<mTracts->GetNumVertex(it); i++){
			for (int j = 0; j < nFaces; j++){
				/*
				// wrong direction!
				mIndices << MyVec3i((i - 1)*(nFaces + 1) + j + offset,
				(i)*(nFaces + 1) + j + offset,
				(i)*(nFaces + 1) + (j + 1) + offset);
				mIndices << MyVec3i((i - 1)*(nFaces + 1) + j + offset,
				(i)*(nFaces + 1) + (j + 1) + offset,
				(i - 1)*(nFaces + 1) + (j + 1) + offset);
				*/
				// fixed direction
				mIndices << MyVec3i((i - 1)*(nFaces + 1) + j + offset,
					(i)*(nFaces + 1) + (j + 1) + offset,
					(i)*(nFaces + 1) + j + offset);
				mIndices << MyVec3i((i - 1)*(nFaces + 1) + j + offset,
					(i - 1)*(nFaces + 1) + (j + 1) + offset,
					(i)*(nFaces + 1) + (j + 1) + offset);
			}
		}
		// add caps
		offset += mTracts->GetNumVertex(it)*(nFaces + 1);
		for (int j = 0; j < nFaces; j++){
			mIndices << MyVec3i(offset, offset + (j + 1) % (nFaces)+1, offset + j + 1);
		}
		offset += 1 + nFaces;
		for (int j = 0; j < nFaces; j++){
			mIndices << MyVec3i(offset, offset + (j + 1) % (nFaces)+1, offset + j + 1);
		}
	}
	cout << "Computing completed.\n";
}

void MyLineAO::ComputeLineGeometry(){
	int currentIdx = 0;
	mIdxOffset.clear();
	int totalPoints = 0;
	for (int it = 0; it < mTracts->GetNumTracks(); it++){
		totalPoints += mTracts->At(it).Size();
	}

	cout << "Allocating Storage for Geometry...\r";

	mVertices.clear();
	mTangents.clear();
	mVertices.reserve(totalPoints);
	mTangents.reserve(totalPoints);


	mIdxOffset.clear();
	mIdxOffset.reserve(mTracts->GetNumTracks());
	for (int it = 0; it < mTracts->GetNumTracks(); it++){
		int npoints = mTracts->At(it).Size();
		for (int i = 0; i<npoints; i++){
			MyVec3f p = mTracts->GetCoord(it, i);
			MyVec3f tangent;

			if (i == 0) {
				MyVec3f pointNext = mTracts->GetCoord(it, i + 1);
				tangent = p - pointNext;
			}
			else if (i == npoints - 1) {
				MyVec3f pointBefore = mTracts->GetCoord(it, i - 1);
				tangent = pointBefore - p;
			}
			else {
				MyVec3f pointNext = mTracts->GetCoord(it, i + 1);
				MyVec3f pointBefore = mTracts->GetCoord(it, i - 1);
				tangent = pointBefore - pointNext;
			}

			mVertices.push_back(p);
			mTangents.push_back(tangent.normalized());

		}

		mIdxOffset << currentIdx;
		currentIdx += npoints;
	}

	mLineIndices.clear();
	mLineIndices.reserve(totalPoints);
	// index
	for (int it = 0; it < mVertices.size(); it++){
		mLineIndices << it;
	}
	cout << "Computing completed.\n";
}

void MyLineAO::ClearGeometry(){
	MyTractVisBase::ClearGeometry();
	mTangents.clear();
}

void MyLineAO::LoadGeometry(){

	initQuad();
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
	// tangent
	if (glIsBuffer(mTangentBuffer)){
		glDeleteBuffers(1, &mTangentBuffer);
	}
	glGenBuffers(1, &mTangentBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mTangentBuffer);
	glBufferData(GL_ARRAY_BUFFER, mTangents.size() * sizeof(MyVec3f), &mTangents[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mTangentAttribute);
	glVertexAttribPointer(mTangentAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	if (mRenderingParameters.Shape == TRACK_SHAPE_TUBE){
		// normal
		if (glIsBuffer(mNormalBuffer)){
			glDeleteBuffers(1, &mNormalBuffer);
		}
		glGenBuffers(1, &mNormalBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
		glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(MyVec3f), &mNormals[0][0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(mNormalAttribute);
		glVertexAttribPointer(mNormalAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//index
		if (glIsBuffer(mIndexBuffer)){
			glDeleteBuffers(1, &mIndexBuffer);
		}
		glGenBuffers(1, &mIndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(MyVec3i), &mIndices[0][0], GL_STATIC_DRAW);
	}
	else{
		//index
		if (glIsBuffer(mIndexBuffer)){
			glDeleteBuffers(1, &mIndexBuffer);
		}
		glGenBuffers(1, &mIndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mLineIndices.size() * sizeof(int), &mLineIndices[0], GL_STATIC_DRAW);
	}
	// unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// now free everything
	ClearGeometry();
}

void MyLineAO::initQuad() {
	// create quad
	if (glIsVertexArray(quadVAO)) glDeleteVertexArrays(1, &quadVAO);
	glGenVertexArrays( 1, &quadVAO );

	if (glIsBuffer(quadVBO)) glDeleteBuffers(1, &quadVBO);
	glGenBuffers( 1, &quadVBO );

	if (glIsBuffer(quadIBO)) glDeleteBuffers(1, &quadIBO);
	glGenBuffers( 1, &quadIBO );

	if (glIsBuffer(quadTex)) glDeleteBuffers(1, &quadTex);
	glGenBuffers( 1, &quadTex );

	// data
	const GLfloat quadVertices[] = {
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f
	};

	const GLuint quadIndices[] = {
		0, 1, 2,
		0, 2, 3
	};

	const GLfloat texCoords[] = {
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

	glBindVertexArray( quadVAO );

	glBindBuffer( GL_ARRAY_BUFFER, quadVBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( quadVertices ), quadVertices, GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( GLfloat ), (GLvoid*)0 );
	glEnableVertexAttribArray( 0 );

	glBindBuffer( GL_ARRAY_BUFFER, quadTex );
	glBufferData( GL_ARRAY_BUFFER, sizeof( texCoords ), texCoords, GL_STATIC_DRAW );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), (GLvoid*)0 );
	glEnableVertexAttribArray( 1 );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, quadIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( quadIndices ), quadIndices, GL_STATIC_DRAW );

	glBindVertexArray( 0 );
}

void MyLineAO::initGBuffer() {
	if (glIsFramebuffer(gBuffer)) glDeleteFramebuffers(1, &gBuffer);
	glGenFramebuffers( 1, &gBuffer );
	glBindFramebuffer( GL_FRAMEBUFFER, gBuffer );

	if (glIsTexture(gColor)) glDeleteTextures(1, &gColor);
	glGenTextures( 1, &gColor );
	glBindTexture( GL_TEXTURE_2D, gColor );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gColor, 0 );

	if (glIsTexture(gNDMap)) glDeleteTextures(1, &gNDMap);
	glGenTextures( 1, &gNDMap );
	glBindTexture( GL_TEXTURE_2D, gNDMap );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//glGenerateMipmap( GL_TEXTURE_2D );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNDMap, 0 );

	if (glIsTexture(gZoomMap)) glDeleteTextures(1, &gZoomMap);
	glGenTextures( 1, &gZoomMap );
	glBindTexture( GL_TEXTURE_2D, gZoomMap );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gZoomMap, 0 );

	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers( 3, attachments );

	// add renderbuffer
	if (glIsRenderbuffer(depthRBO)) glDeleteRenderbuffers(1, &depthRBO);
	glGenRenderbuffers( 1, &depthRBO );
	glBindRenderbuffer( GL_RENDERBUFFER, depthRBO );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);

	if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE ) 
	std::cout << "ERROR: Framebuffer incomplete! " << std::endl;

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void MyLineAO::initAOBuffer() {
	if (glIsFramebuffer(aoBuffer)) glDeleteFramebuffers(1, &aoBuffer);
	glGenFramebuffers(1, &aoBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, aoBuffer);

	if (glIsTexture(aoColor)) glDeleteTextures(1, &aoColor);
	glGenTextures(1, &aoColor);
	glBindTexture(GL_TEXTURE_2D, aoColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, aoColor, 0);

	GLuint attachments[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR: Framebuffer incomplete! " << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MyLineAO::genNoiseTexture() {
	// TODO: change to texture repeating by modifying texture coordinates

	int width = m_width;
	int height = m_height;

	std::vector< float > randVectors( width * height * 3 );

	for( int y=0; y<height; y++ ) {
		for( int x=0; x<width; x++ ) {
			randVectors[ y * width * 3 + x * 3 + 0 ] = static_cast< float >( rand() ) / static_cast< float >( RAND_MAX );		
			randVectors[ y * width * 3 + x * 3 + 1 ] = static_cast< float >( rand() ) / static_cast< float >( RAND_MAX );
			randVectors[ y * width * 3 + x * 3 + 2 ] = static_cast< float >( rand() ) / static_cast< float >( RAND_MAX );
		}
	}

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	if (glIsTexture(noise)) glDeleteTextures(1, &noise);
	glGenTextures( 1, &noise );
	glBindTexture( GL_TEXTURE_2D, noise );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, &randVectors[0] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
}

// ---------------------     rendering passes     -------------------------

void MyLineAO::lineShadingPass() const {
	glEnable( GL_DEPTH_TEST );

	// select framebuffer as render target
	int frameBuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &frameBuffer);
	glBindFramebuffer( GL_FRAMEBUFFER, gBuffer );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// w component is depth, so it is 1 for background
	float ndClearColor[] = { 0, 0, 0, 1 };
	glClearBufferfv(GL_COLOR, 1, ndClearColor);
	glUseProgram(mLineShader);
	glBindVertexArray( mVertexArray );

	int mvmatLocation = glGetUniformLocation(mLineShader, "mvMat");
	float modelViewMat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMat);
	glUniformMatrix4fv(mvmatLocation, 1, GL_FALSE, modelViewMat);

	int projmatLocation = glGetUniformLocation(mLineShader, "projMat");
	float projMat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
	glUniformMatrix4fv(projmatLocation, 1, GL_FALSE, projMat);

	int normatLocation = glGetUniformLocation(mLineShader, "normalMat");
	float normalMat[16];
	bool bInv = MyMathHelper::InvertMatrix4x4ColMaj(modelViewMat, normalMat);
	assert(bInv);
	MyMathHelper::TransposeMatrix4x4ColMaj(normalMat, normalMat);
	glUniformMatrix4fv(normatLocation, 1, GL_FALSE, normalMat);

	int radiusLocation = glGetUniformLocation(mLineShader, "radius");
	glUniform1f(radiusLocation, mTrackRadius);

	glUniform4fv(glGetUniformLocation(mLineShader, "baseColor"), 1, &mRenderingParameters.BaseColor.r);
	glUniform1f(glGetUniformLocation(mLineShader, "lightIntensity"), mRenderingParameters.LightIntensity);
	glUniform1f(glGetUniformLocation(mLineShader, "ambient"), mRenderingParameters.Ambient);
	glUniform1f(glGetUniformLocation(mLineShader, "diffuse"), mRenderingParameters.Diffuse);
	glUniform1f(glGetUniformLocation(mLineShader, "specular"), mRenderingParameters.Specular);
	glUniform1f(glGetUniformLocation(mLineShader, "shininess"), mRenderingParameters.Shininess);

	int shapeLoc = glGetUniformLocation(mLineShader, "hasNormal");

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);

	if (mRenderingParameters.Shape == TRACK_SHAPE_TUBE){
		glUniform1i(shapeLoc, 1);
		//this has no cap considered
		int nFaces = mRenderingParameters.Faces;
		for (int i = 0; i < mFiberToDraw.size(); i++){
			int fiberIdx = mFiberToDraw[i];
			//int offset = (mIdxOffset[fiberIdx] / (nFaces + 1) - fiberIdx)*nFaces * 6;
			//int numVertex = (mTracts->GetNumVertex(fiberIdx) - 1)*(nFaces + 0) * 6;
			// add cap offset
			int offset = (mIdxOffset[fiberIdx] / (nFaces + 1) - fiberIdx * 2)*nFaces * 6;
			int numVertex = (mTracts->GetNumVertex(fiberIdx) - 1)*(nFaces + 0) * 6 + nFaces * 6;
			glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
		}
	}
	else{
		glUniform1i(shapeLoc, 0);
		for (int i = 0; i < mFiberToDraw.size(); i++){
			int fiberIdx = mFiberToDraw[i];
			int offset = mIdxOffset[fiberIdx];
			int numVertex = mTracts->GetNumVertex(fiberIdx);
			glDrawElements(GL_LINE_STRIP, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
		}
	}

	if (mSphereGeometry){
		glUniform1i(shapeLoc, 1);
		glUniform1f(radiusLocation, 0);
		mSphereGeometry->DrawGeometry();
	}

	glBindVertexArray( 0 );
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	glUseProgram(0);
}

void MyLineAO::lineAOPass() const {
	//glDisable(GL_DEPTH_TEST);

	int frameBuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &frameBuffer);
	// select framebuffer as render target
	glBindFramebuffer(GL_FRAMEBUFFER, aoBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(mAOShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gColor);
	glUniform1i(glGetUniformLocation(mAOShader, "gColor"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNDMap);
	glUniform1i(glGetUniformLocation(mAOShader, "gNDMap"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gZoomMap);
	glUniform1i(glGetUniformLocation(mAOShader, "gZoomMap"), 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, noise);
	glUniform1i(glGetUniformLocation(mAOShader, "noise"), 3);

	glUniform1f(glGetUniformLocation(mAOShader, "u_colorSizeX"), m_width);
	glUniform1f(glGetUniformLocation(mAOShader, "u_colorSizeY"), m_height);


	glUniform1f(glGetUniformLocation(mAOShader, "u_lineAOTotalStrength"), mLineAOTotalStrength);

	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);


	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	glUseProgram(0);
}

void MyLineAO::lightningPass() const {
	//glDisable( GL_DEPTH_TEST );
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(mTextureShader);

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, gColor );
	glUniform1i(glGetUniformLocation(mTextureShader, "gColor"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, aoColor);
	glUniform1i(glGetUniformLocation(mTextureShader, "aoColor"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gNDMap);
	glUniform1i(glGetUniformLocation(mTextureShader, "gNDMap"), 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gZoomMap);
	glUniform1i(glGetUniformLocation(mTextureShader, "gZoomMap"), 3);

	glUniform1i(glGetUniformLocation(mTextureShader, "blurX"), mBlurRadius);
	glUniform1i(glGetUniformLocation(mTextureShader, "blurY"), mBlurRadius);

	glBindVertexArray( quadVAO );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, quadIBO );
	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
	glBindVertexArray( 0 );

	glUseProgram(0);
}

void MyLineAO::Show() {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	lineShadingPass();
	lineAOPass();
	lightningPass();
	glPopAttrib();

	// check window resize
	/*GLint value[4];
	glGetIntegerv( GL_VIEWPORT, value );

	if( value[2] != m_width || value[3] != m_height ) const_cast< MyLineAO* >(this)->initGBuffer();*/

}