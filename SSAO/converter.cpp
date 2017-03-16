#include "MyMesh.h"
#include "MyTracks.h"
#include <vector>
using namespace std;
int main(int argc, char* argv[])
{
	if (argc < 2){
		return 0;
	}
	float meshPrecision = 0.01;
	MyMesh mesh;
	//mesh.Read(argv[1]);
	//mesh.MergeVertices(meshPrecision);
	//mesh.ClearNonRegularFaces();
	//mesh.GenPerVertexNormal();
	mesh.Read("data\\lh.pial.DK.fusiform_trans.obj");
	//mesh.ReadAsc("data\\lh.pial.asc");
	mesh.MergeVertices(meshPrecision);
	//mesh.RemoveVertex(MyArrayi(1, 40969));
	mesh.GenPerVertexNormal();
	mesh.Write("data\\lh.pial.DK.fusiform_trans_normal.obj");
	//mesh.WriteNOFF("data\\lh.pial.off");
	//mesh.WriteAsc("data\\lh.sphere_v163841.asc");
	//mesh.WriteBIN("data\\lh.pial_v163841.bin");
	//mesh.WriteNOFF("data\\lh_trans_pial_noff.off");
	//mesh.WritePLY("data\\lh.pial.ply");
	//mesh.WriteSMFD("data\\lh.trans.pial.smfd");

	//MyTracks track;
	//track.Read("C:\\Users\\GuohaoZhang\\Desktop\\tmpdata\\dti.trk");
	//track.Save("C:\\Users\\GuohaoZhang\\Desktop\\tmpdata\\dti.data");

	/*
	MyTracks track;
	track.Read("C:\\Users\\GuohaoZhang\\Desktop\\tmpdata\\dti.trk");
	vector<int> idx;
	for (int i = 0; i < track.GetNumTracks(); i++){
		idx.push_back(i);
	}
	vector<int> filter1, filter2;
	float lengthLimit[] = { 20, FLT_MAX };
	track.FilterByIndexSkip(idx, 0.99, filter1);
	cout << filter1.size() << endl;
	track.FilterByTrackLength(filter1, lengthLimit, filter2);
	cout << filter2.size() << endl;
	track.SavePartial("dti_20_099.data", filter2);
	*/
}