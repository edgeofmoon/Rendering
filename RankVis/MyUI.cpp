#include "MyUI.h"
#include "MyGraphicsTool.h"

using namespace MyUI;

MyVec4i MyUI::mUIViewport = MyVec4i(0, 0, 0, 0);

void MyUI::UIBeginDrawer(){
	MyGraphicsTool::PushAllAttributes();
	MyGraphicsTool::PushProjectionMatrix();
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::SetViewport(mUIViewport);
	MyGraphicsTool::LoadProjectionMatrix(
		//&MyMatrixf::OrthographicMatrix(0, 1, 0, 1, 0, 1));
		&MyMatrixf::OrthographicMatrix(0, mUIViewport[2], 0, mUIViewport[3], 0, 1));
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	MyGraphicsTool::SetLineWidth(2);
}

void MyUI::UIEndDrawer(){
	MyGraphicsTool::PopMatrix();
	MyGraphicsTool::PopProjectionMatrix();
	MyGraphicsTool::PopAttributes();
}
