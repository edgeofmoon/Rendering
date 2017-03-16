#include "MyMesh.h"
#include "MyMatrix.h"
#include "MyMathHelper.h"
#include "OSCB.h"
#include <iostream>
#include <iomanip>
using namespace std;

int main(int argc, char* argv[])
{
	/*
	MyMesh mesh;
	mesh.Read("data\\rh.trans.pial.obj");
	mesh.MergeVertices();
	mesh.GenPerVertexNormal();
	std::cout << setprecision(2) << fixed;
	mesh.Write("data\\rh.trans.normal.pial.obj");
	return 1;
	*/
	string folderName("C:\\Users\\GuohaoZhang\\Dropbox\\Programs\\webViewer\\simpleMC\\data\\pial_DK_trans\\");
	auto files = OSCB::get_all_files_names_within_folder(folderName);
	for each (auto file in files){
		MyMesh mesh;
		//mesh.Read("C:\\Users\\GuohaoZhang\\Desktop\\tmpp\\all_obj\\pial_Full\\lh.pial.obj");
		mesh.Read(folderName + file);
		mesh.MergeVertices();
		mesh.GenPerVertexNormal();
		int dotPlace = file.find_last_of('.');
		auto basename = file.substr(0, dotPlace);
		auto outname = "C:\\Users\\GuohaoZhang\\Dropbox\\Programs\\webViewer\\simpleMC\\data\\pial_DK_trans_normal\\" + basename + "_normal.obj";
		mesh.Write(outname);
	}
	return 1;
	//MNI152
	float transform[16] = {
		1.031964032,  0.02738066751,  0.008192314853, -7.222488849,
		-0.0090080873,  1.062903577, -0.2377809608,  7.510265682,
		-0.007620772714,  0.1712549451,  1.167785654, -27.81433516,
		0,  0,  0,  1
	};
	//MNI152 header changed
	float transform1[16] = {
		0.8250255381,  0.02204908986,  0.006356894267, -7.158828415,
		-0.007378002228,  0.8499971843, -0.1917515541,  7.672315232,
		-0.006173159605,  0.1374441124,  0.9339806001, -27.83962385,
		0,  0,  0,  1
	};
	//target
	float transform2[16] = {
		0.8156025996,  0.02399433406,  0.01313809059,  12.37115942,
		0.0005876789452,  0.8443615623, -0.2346520965,  31.49131751,
		-0.02380553532,  0.1963749961,  0.826347665,  1.555147246,
		0,  0,  0,  1
	};
	MyMatrixf mat(transform1, 4, 4);
	mat = mat*MyMatrixf::ScaleMatrix(1.25, 1.25, 1.25);
	mat = mat*MyMatrixf::TranslateMatrix(7, 8, 18);

	/*
	MyMesh meshLeft;
	MyMesh meshRight;
	meshLeft.Read("C:\\Users\\GuohaoZhang\\Desktop\\tmpp\\all_obj\\pial_Full\\lh.pial.obj");
	meshRight.Read("C:\\Users\\GuohaoZhang\\Desktop\\tmpp\\all_obj\\pial_Full\\rh.pial.obj");
	meshLeft.ApplyTransform(mat);
	meshRight.ApplyTransform(mat);
	meshLeft.Write("lh.trans.pial.obj");
	meshRight.Write("rh.trans.pial.obj");
	*/

	/*
	//string folderName("C:\\Users\\GuohaoZhang\\Dropbox\\Programs\\webViewer\\simpleMC\\data\\pial_DK\\");
	string folderName("C:\\Users\\GuohaoZhang\\Desktop\\tmpp\\all_obj\\subcortical\\");
	auto files = OSCB::get_all_files_names_within_folder(folderName);
	for each (auto file in files){
		MyMesh mesh;
		//mesh.Read("C:\\Users\\GuohaoZhang\\Desktop\\tmpp\\all_obj\\pial_Full\\lh.pial.obj");
		mesh.Read(folderName+file);
		mesh.ApplyTransform(mat);
		int dotPlace = file.find_last_of('.');
		auto basename = file.substr(0, dotPlace);
		auto outname = "subcortical_trans\\"+basename + "_trans.obj";
		mesh.Write(outname);
	}
	*/
}