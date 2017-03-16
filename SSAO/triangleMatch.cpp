#include "MyMesh.h"
#include <vector>
#include <fstream>
#include <string>
using namespace std;

void readMeshNames(string filename, string dirname, vector<string>& rst){
	ifstream namefile(filename);
	while (!namefile.eof()){
		string fn;
		namefile >> fn;
		if (!fn.empty()){
			rst.push_back(dirname + fn);
		}
	}
}

MyVec3f computeTriangleCenter(const MyMesh& mesh, int triangle){
	MyVec3i tri = mesh.GetTriangle(triangle);
	MyVec3f v(0, 0, 0);
	for (int i = 0; i < 3; i++){
		v += mesh.GetVertex(tri[i]);
	}
	return v / 3;
}

int main(int argc, char* argv[]){
	vector<string> objFiles;
	vector<MyMesh*> meshes;
	MyMesh theMesh;
	vector<vector<MyVec3f>> meshTriangleCenters;
	vector<int> triangleMatch;
	if (argc < 4){
		cout << "Useage: " << argv[0] << " meshfile patchnamefile patchdir [outputfile]" << endl;
		return 0;
	}
	theMesh.Read(argv[1]);
	theMesh.ClearNonRegularFaces();
	readMeshNames(argv[2], argv[3], objFiles);

	for (int i = 0; i < objFiles.size();i++){
		string objFile = objFiles[i];
		MyMesh* mesh = new MyMesh;
		mesh->Read(objFile);
		meshTriangleCenters.push_back(vector<MyVec3f>());
		for (int i = 0; i < mesh->GetNumTriangle(); i++){
			MyVec3f center = computeTriangleCenter(*mesh, i);
			meshTriangleCenters.back().push_back(center);
		}
		meshes.push_back(mesh);
	}
	int curProg = 0;
	for (int i = 0; i < theMesh.GetNumTriangle(); i++){
		MyVec3f center = computeTriangleCenter(theMesh, i);
		float minDistSqd = FLT_MAX;
		int minMesh = -1;
		for (int j = 0; j < meshes.size(); j++){
			if (meshes[j]->GetBoundingBox().IsIn(center)){
				for (MyVec3f pc : meshTriangleCenters[j]){
					float dist = (pc - center).squared();
					if (dist < minDistSqd){
						minDistSqd = dist;
						minMesh = j;
					}
				}
			}
		}
		triangleMatch.push_back(minMesh);
		int progress = i / (float)(theMesh.GetNumTriangle() - 1) * 1000;
		if (progress > curProg){
			curProg = progress;
			cout << "Processed: " << i << "/" << theMesh.GetNumTriangle()
				<< " (" << progress / 10 << "%)\r";
		}
	}
	string outName = "triangleMatch.txt";
	if (argc > 4){
		outName = argv[4];
	}
	ofstream outFile(outName);
	for (int i : triangleMatch){
		outFile << i << endl;
	}
	outFile.close();
}