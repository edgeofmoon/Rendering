#include "MyPrimitiveDrawer.h"
#include "MyLine.h"
#include "MyPolyLine.h"
#include "MyQuarternion.h"

MyPrimitiveDrawer::MyPrimitiveDrawer(void)
{
}


MyPrimitiveDrawer::~MyPrimitiveDrawer(void)
{
}

void MyPrimitiveDrawer::DrawQuadAt(const MyVec3f& pos0,
		const MyVec3f& pos1,
		const MyVec3f& pos2,
		const MyVec3f& pos3){
	MyGraphicsTool::BeginQuads();
	MyGraphicsTool::Vertex(pos0);
	MyGraphicsTool::Vertex(pos1);
	MyGraphicsTool::Vertex(pos2);
	MyGraphicsTool::Vertex(pos3);
	MyGraphicsTool::EndPrimitive();
}

void MyPrimitiveDrawer::DrawQuadsAt(const std::vector<MyVec3f>& vecs){
	MyGraphicsTool::BeginQuads();
	for(int i = 0;i<vecs.size();i++){
		MyGraphicsTool::Vertex(vecs[i]);
	}
	MyGraphicsTool::EndPrimitive();
}

void MyPrimitiveDrawer::DrawTextureOnViewport(unsigned int texture){
	MyGraphicsTool::PushAllAttributes();
	MyGraphicsTool::PushProjectionMatrix();
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::LoadProjectionMatrix(&MyMatrixf::OrthographicMatrix(0, 1, 0, 1, 0, 1));
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	MyGraphicsTool::EnableTexture2D();
	MyGraphicsTool::BindTexture2D(texture);
	MyGraphicsTool::BeginTriangleFan();
	MyVec3f pos[] = { MyVec3f(0, 0, -0.5), MyVec3f(1, 0, -0.5), MyVec3f(1, 1, -0.5), MyVec3f(0, 1, -0.5) };
	MyVec2f tex[] = { MyVec2f(0, 0), MyVec2f(1, 0), MyVec2f(1, 1), MyVec2f(0, 1) };
	for (int i = 0; i < 4; i++){
		MyGraphicsTool::Color(MyColor4f::white());
		MyGraphicsTool::TextureCoordinate(tex[i]);
		MyGraphicsTool::Vertex(pos[i]);
	}
	MyGraphicsTool::EndPrimitive();
	MyGraphicsTool::UnbindTexture2D(texture);
	MyGraphicsTool::PopMatrix();
	MyGraphicsTool::PopProjectionMatrix();
	MyGraphicsTool::PopAttributes();
}

int MyPrimitiveDrawer::GetBitMapTextWidth(const MyString& str){
	int length = 0;
	for(int i = 0;i<str.size();i++){
		length += MyGraphicsTool::GetBitmapWidth(str[i]);
	}
	return length;
}

int MyPrimitiveDrawer::GetBitMapTextLargeWidth(const MyString& str){
	int length = 0;
	for(int i = 0;i<str.size();i++){
		length += MyGraphicsTool::GetBitmapLargeWidth(str[i]);
	}
	return length;
}

int MyPrimitiveDrawer::GetStrokeWidth(const MyString& str){
	int length = 0;
	for(int i = 0;i<str.size();i++){
		length += MyGraphicsTool::GetStrokeWidth(str[i]);
	}
	return length;
}

void MyPrimitiveDrawer::DrawLineAt(const MyVec3f& s, const MyVec3f& e){
	MyGraphicsTool::BeginLines();
	MyGraphicsTool::Vertex(s);
	MyGraphicsTool::Vertex(e);
	MyGraphicsTool::EndPrimitive();
}

void MyPrimitiveDrawer::DrawLine(const MyLine3f& line){
	MyPrimitiveDrawer::DrawLineAt(line.GetStart(), line.GetEnd());
}

void MyPrimitiveDrawer::DrawLineAt(const MyVec2f& s, const MyVec2f& e){
	MyGraphicsTool::BeginLines();
	MyGraphicsTool::Vertex(s);
	MyGraphicsTool::Vertex(e);
	MyGraphicsTool::EndPrimitive();
}

void MyPrimitiveDrawer::DrawLine(const MyLine2f& line){
	MyPrimitiveDrawer::DrawLineAt(line.GetStart(), line.GetEnd());
}

void MyPrimitiveDrawer::DrawSphereAt(const MyVec3f& n, float r){
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::Translate(n);
	MyGraphicsTool::Sphere(r);
	MyGraphicsTool::PopMatrix();
}

void MyPrimitiveDrawer::DrawCircle(const MyVec3f& n, float r, int segs){
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::Translate(n);
	MyGraphicsTool::BeginLineLoop();
	for(int i = 0;i<segs;i++){
		float angle = 2*MY_PI/segs*i;
		MyGraphicsTool::Vertex(MyVec2f(r*cos(angle), r*sin(angle)));
	}
	MyGraphicsTool::EndPrimitive();
	MyGraphicsTool::PopMatrix();
}

void MyPrimitiveDrawer::DrawBitMapText(const MyVec3f& pos, const std::string& text, int alignment){
	if(alignment == 0){
		MyGraphicsTool::RasterPos(pos);
		for (int i = 0;i<text.length();i++){
			MyGraphicsTool::BitmapChar(text.at(i));
		}
	}
	else if(alignment == 1){
		MyBoundingBox box = MyPrimitiveDrawer::GetBitMapTextBox(text,pos);
		MyGraphicsTool::RasterPos(pos-MyVec3f(box.GetWidth()/2,0,0));
		for (int i = 0;i<text.length();i++){
			MyGraphicsTool::BitmapChar(text.at(i));
		}
	}
	else if(alignment == 2){
		MyBoundingBox box = MyPrimitiveDrawer::GetBitMapTextBox(text,pos);
		MyGraphicsTool::RasterPos(pos-MyVec3f(box.GetWidth(),0,0));
		for (int i = 0;i<text.length();i++){
			MyGraphicsTool::BitmapChar(text.at(i));
		}
	}
}

void MyPrimitiveDrawer::DrawBitMapTextLarge(const MyVec3f& pos, const std::string& text, int alignment){
	if(alignment == 0){
		MyGraphicsTool::RasterPos(pos);
		for (int i = 0;i<text.length();i++){
			MyGraphicsTool::BitmapCharLarge(text.at(i));
		}
	}
	else if(alignment == 1){
		MyBoundingBox box = MyPrimitiveDrawer::GetBitMapLargeTextBox(text,pos);
		MyGraphicsTool::RasterPos(pos-MyVec3f(box.GetWidth()/2,0,0));
		for (int i = 0;i<text.length();i++){
			MyGraphicsTool::BitmapCharLarge(text.at(i));
		}
	}
	else if(alignment == 2){
		MyBoundingBox box = MyPrimitiveDrawer::GetBitMapLargeTextBox(text,pos);
		MyGraphicsTool::RasterPos(pos-MyVec3f(box.GetWidth(),0,0));
		for (int i = 0;i<text.length();i++){
			MyGraphicsTool::BitmapCharLarge(text.at(i));
		}
	}
}

void MyPrimitiveDrawer::DrawStrokeText(
	const MyVec3f& pos, const std::string& text, const MyVec3f& scale){
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::Translate(pos);
	MyGraphicsTool::Scale(scale);
	for (int i = 0;i<text.length();i++){
		MyGraphicsTool::StrokeChar(text.at(i));
	}
	MyGraphicsTool::PopMatrix();
}

void MyPrimitiveDrawer::DrawStrokeTextOrtho(
	const MyVec3f& pos, const std::string& text, const MyVec3f& scale){
	MyMatrixd projMat = MyGraphicsTool::GetProjectionMatrix();
	MyMatrixd mvMat = MyGraphicsTool::GetModelViewMatrix();
	MyVec4i viewport = MyGraphicsTool::GetViewport();
	MyGraphicsTool::PushAllAttributes();
	MyGraphicsTool::PushProjectionMatrix();
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::LoadProjectionMatrix(&
		MyMatrixf::OrthographicMatrix(0, viewport[2], 0, viewport[3], 0, 1));
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	MyVec3f p = MyGraphicsTool::GetProjection(pos, mvMat, projMat, viewport);
	p[2] = 0;
	MyPrimitiveDrawer::DrawStrokeText(p, text, scale);
	MyGraphicsTool::PopMatrix();
	MyGraphicsTool::PopProjectionMatrix();
	MyGraphicsTool::PopAttributes();
}

void MyPrimitiveDrawer::DrawStrokeTextUpDowm(const MyVec3f& pos, const std::string& text, const MyVec3f& scale){
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::Translate(pos);
	MyGraphicsTool::Scale(scale);
	MyQuarternion textOrientation(MY_PI,0,0,1);
	int offset = 0;
	for (int i = 0;i<text.length();i++){
		int width = MyGraphicsTool::GetStrokeWidth(text.at(i));
		MyGraphicsTool::PushMatrix();
		MyGraphicsTool::Translate(MyVec3f(offset,0,0));
		//MyGraphicsTool::Rotate(MY_PI/2,MyVec3f(0,0,1));
		MyGraphicsTool::Translate(MyVec3f(width/2.f,0,0));
		MyGraphicsTool::MultiplyMatrix(&(textOrientation.GetMatrix().Transpose()));
		MyGraphicsTool::Translate(MyVec3f(-width/2.f,0,0));
		MyGraphicsTool::StrokeChar(text.at(i));
		MyGraphicsTool::PopMatrix();
		offset += width;
	}
	MyGraphicsTool::PopMatrix();
}

void MyPrimitiveDrawer::Draw(const MyPolyline2f& polyline){
	if(polyline.GetLoop()){
		MyGraphicsTool::BeginLineLoop();
	}
	else{
		MyGraphicsTool::BeginLineStrip();
	}
	for(int i = 0;i<polyline.GetNumPoints();i++){
		MyGraphicsTool::Vertex(polyline.GetPoint(i));
	}
	MyGraphicsTool::EndPrimitive();
}

void MyPrimitiveDrawer::Draw(const MyPolyline3f& polyline){
	if(polyline.GetLoop()){
		MyGraphicsTool::BeginLineLoop();
	}
	else{
		MyGraphicsTool::BeginLineStrip();
	}
	for(int i = 0;i<polyline.GetNumPoints();i++){
		MyGraphicsTool::Vertex(polyline.GetPoint(i));
	}
	MyGraphicsTool::EndPrimitive();
}

MyBoundingBox MyPrimitiveDrawer::GetBitMapTextBox(const MyString& text, MyVec3f offset){
	int width = MyPrimitiveDrawer::GetBitMapTextWidth(text);
	int height = 12;
	MyVec3f winPos = MyPrimitiveDrawer::GetWindowPosFromWorldPos(offset);
	MyVec3f trWorldPos = MyPrimitiveDrawer::GetWorldPosFromWindowPos(winPos+MyVec3f(width,height,0));
	MyBoundingBox box(offset, trWorldPos);
	//box.Translate(offset);
	return box;
}

MyBoundingBox MyPrimitiveDrawer::GetBitMapLargeTextBox(const MyString& text, MyVec3f offset){
	int width = MyPrimitiveDrawer::GetBitMapTextLargeWidth(text);
	int height = 12;
	MyVec3f winPos = MyPrimitiveDrawer::GetWindowPosFromWorldPos(offset);
	MyVec3f trWorldPos = MyPrimitiveDrawer::GetWorldPosFromWindowPos(winPos+MyVec3f(width,height,0));
	MyBoundingBox box(offset, trWorldPos);
	//box.Translate(offset);
	return box;
}

float MyPrimitiveDrawer::GetStrokeTextWidth(const MyString& text){
	float width = 0;
	for (int i = 0; i < text.size(); i++){
		width += GetStrokeWidth(text[i]);
	}
	return width;
}