#include "MyVisEnum.h"
#include "MyVisEnum.h"

using namespace MyVisEnum;

string MyVisEnum::toString(RetinalChannel rc){
	switch (rc){
	case VALUE:
		return "VALUE";
		break;
	case TEXTURE:
		return "TEXTURE";
		break;
	case SHAPE:
		return "SHAPE";
		break;
	case ESIZE:
		return "SIZE";
		break;
	case COLOR:
		return "COLOR";
		break;
	case ORIENTATION:
		return "ORIENTATION";
		break;
	default:
		return "INVALID";
		break;
	}
}

string MyVisEnum::toString(VisTask task){
	switch (task){
	case FA:
		return "FA";
		break;
	case TRACE:
		return "TRACE";
		break;
	case SAME:
		return "SAME";
		break;
	case TUMOR:
		return "TUMOR";
		break;
	case START:
		return "START";
		break;
	case END:
		return "END";
		break;
	default:
		return "INVALID";
		break;
	}
}

string MyVisEnum::toString(Bundle bundle){
	switch (bundle){
	case CC:
		return "CC";
		break;
	case CST:
		return "CST";
		break;
	case IFO:
		return "IFO";
		break;
	case ILF:
		return "ILF";
		break;
	case CG:
		return "CG";
		break;
	default:
		return "INVALID";
		break;
	}
}

string MyVisEnum::toString(Shape shape){
	switch (shape){
	case LINE:
		return "LINE";
		break;
	case TUBE:
		return "TUBE";
		break;
	case SUPERQUADRIC:
		return "SUPERQUADRIC";
		break;
	case SUPERELLIPSOID:
		return "SUPERELLIPSOID";
		break;
	case RIBBON:
		return "RIBBON";
		break;
	default:
		return "INVALID";
		break;
	}
}

string MyVisEnum::toString(FiberCover cv){
	switch (cv){
	case BUNDLE:
		return "BUNDLE";
		break;
	case WHOLE:
		return "WHOLE";
		break;
	case PARTIAL:
		return "PARTIAL";
		break;
	default:
		return "INVALID";
		break;
	}
}

string MyVisEnum::toString(VisCue cue){
	
	switch (cue){
	case BASIC:
		return "BASIC";
		break;
	case CURV_MIN:
		return "CURV_MIN";
		break;
	case CURV_MAX:
		return "CURV_MAX";
		break;
	case DEPTH:
		return "DEPTH";
		break;
	case AMBIENT_OCCULUSION:
		return "AMB_OCCT";
		break;
	case ENCODING:
		return "ENCODING";
		break;
	default:
		return "INVALID";
		break;
	}
}

string MyVisEnum::toString(CollisionStatus cs){
	switch (cs){
	case AWAY:
		return "AWAY";
		break;
	case TOUCH:
		return "TANGENTIAL";
		break;
	case INTERSECT:
		return "INTERSECT";
		break;
	default:
		return "INVALID";
		break;
	}
}