#include "MyVisInfo.h"

#include <sstream>
#include <iomanip>

using namespace MyVisEnum;

MyArray2f MyVisInfo::FAAnswerOptionRanges = {
	MyVec2f(-0.2, -0.15),
	MyVec2f(-0.15, -0.1),
	MyVec2f(-0.1, -0.05),
	MyVec2f(-0.05, 0.05),
	MyVec2f(0.05, 0.1),
	MyVec2f(0.1, 0.15),
	MyVec2f(0.15, 0.2),
};

MyVisInfo::MyVisInfo(VisTask task, bool isTraining) :
	mIsEmpty(true), mIsTraining(isTraining), mTask(task), mMappingMethod(0), mQuest(-1){
	mNeedMoreInput = false;
	mDoShowLegend = false;
}

MyVisInfo::MyVisInfo(bool isEmpty, bool isTraining, VisTask task, 
	RetinalChannel encoding,  int method, Shape sp, VisCue visCue, 
	Bundle bundle, FiberCover cover, int quest, int resolution, bool needMoreInput) :
	mIsEmpty(isEmpty), mIsTraining(isTraining), mTask(task), 
	mEncoding(encoding), mMappingMethod(method),
	mShape(sp), mVisCue(visCue), mBundle(bundle),
	mCover(cover), mQuest(quest), mResolution(resolution), mNeedMoreInput(needMoreInput){
	mDoShowLegend = true;
}

MyVisInfo::~MyVisInfo()
{
}

MyString MyVisInfo::GetLegendString() const{
	if (mTask == FA_VALUE){
		switch (mMappingMethod){
		case 0:
			return "iso-luminant color scale";
		case 1:
			return "gray scale";
		case 2:
			return "black-body color scale";
		case 3:
			return "extended black-body color scale";
		case 4:
			return "diverging color scale";
		case 5:
			return "cool-warm color scale";
		}
	}
	else if (mTask == TRACE){
		switch (mMappingMethod){
		case 0:
			return "Boy's surface color";
		case 1:
			return "eigen map embedding";
		case 2:
			return "spectral distance embedding";
		case 3:
			return "absolute color";
		case 4:
			return "gray color";
		}
	}
	return "";
}

MyString MyVisInfo::GetTaskTransitionString() const{
	if (mDoShowLegend){
		return "Color map now changes to " + GetLegendString() +".";
	}
	else if (IsEmpty()){
		switch (mTask){
		case START:
			if (IsTraining()) return "Welcome to the training session! Please press the Next button to start.";
			else return "Welcome to the formal study! Please press the Next button to start.";
			break;
		case END:
			if (IsTraining()) return "Your training is finished. Thank you!";
			else return "The formal study is completed. Thank you for your participation!";
			break;
		case FA:
			return "Task type 1 " + GetTaskHintString(mTask);
		case TRACE:
			return "Task type 2 " + GetTaskHintString(mTask);
		case TUMOR:
			return "Task type 3 " + GetTaskHintString(mTask);
			break;
		case FA_VALUE:
			return "Task type 1 " + GetTaskHintString(mTask);
		default:
			return "Unknow transition!";
			break;
		}
	}
	else{
		return "";
	}
}

MyString MyVisInfo::GetTaskHintString() const{
	return GetTaskHintString(mTask);
}

int MyVisInfo::GetNumberAnswerOption() const{
	switch (mTask){
	case FA:
		return 7;
		break;
	case TRACE:
		return 3;
		break;
	case TUMOR:
		return 3;
		break;
	case FA_VALUE:
		return 0;
		break;
	default:
		return 0;
		break;
	}
}

MyString MyVisInfo::GetAnswerOptionString(int idx) const{
	return MyVisInfo::GetTaskAnswerOptionString(mTask, idx);
}

MyString MyVisInfo::GetAnswerHintString() const{
	return MyVisInfo::GetTaskAnswerHintString(mTask);
}

int MyVisInfo::GetTaskRawIndex() const{
	// based on task folder names
	switch (mTask){
	case FA:
		return 1;
		break;
	case TRACE:
		return 2;
		break;
	case SAME:
		return 5;
		break;
	default:
		return -1;
		break;
	}
}

MyString MyVisInfo::GetTaskFolderName() const{
	switch (mTask){
	case FA:
		return string("region_for_task1");
		break;
	case TRACE:
		return string("region_for_task2");
		break;
	case SAME:
		return string("region_for_task5");
		break;
	case TUMOR:
		return string("region_for_task6");
		break;
	case FA_VALUE:
		return string("region_for_task7");
		break;
	default:
		return string("[WRONG TASK NAME!!!]");
		break;
	}
}

MyString MyVisInfo::GetCoverFolderName() const{
	switch (mCover){
	case BUNDLE:
		return string("normal_allfb");
		break;
	case PARTIAL:
		return string("normal_partial");
		break;
	case WHOLE:
		return string("normal_whole");
		break;
	default:
		return string("[WRONG FIBER COVER!!!]");
		break;
	}
}

MyString MyVisInfo::GetResolutionFolderName() const{
	return "s" + MyString(mResolution);
}
MyString MyVisInfo::GetBundleFolderName() const{
	switch (mBundle){
	case CC:
		return string("cc");
		break;
	case CST:
		return string("cst");
		break;
	case IFO:
		return string("ifo");
		break;
	case ILF:
		return string("ilf");
		break;
	case CG:
		return string("cg");
		break;
	default:
		return string("[WRONG BUNDLE NAME!!!]");
		break;
	}
}
MyString MyVisInfo::GetQuestFolderName() const{
	/*
	switch (mQuest){
	case 1:
		return string(".");
		break;
	case 2:
		return string("pos1");
		break;
	case 3:
		return string("pos2");
		break;
	default:
		return string("[WRONG QUEST NAME!!!]");
		break;
	}
	*/
	if (mQuest == 0) return ".";
	else if (mQuest > 0) return "pos" + to_string(mQuest);
	else return "[WRONG QUEST NAME!!!]";
}

MyString MyVisInfo::GetTractIndexFileName() const{
	return "match.data";
}

MyString MyVisInfo::GetTractSelectFileName() const{
	if (mTask == SAME){
		if (mQuest == 1){
			// let 1 be yes case
			return MyString("yes/fiberidx_200000") + MyString(GetTaskRawIndex())
				+ MyString("_region_") + GetResolutionFolderName() + ".data";
		}
		else{
			// and 2 be no case
			return MyString("no/fiberidx_compound_region_") + GetResolutionFolderName() + ".data";
		}
	}
	else{
		// trace task
		return MyString("fiberidx_200000") + MyString(GetTaskRawIndex()) + MyString("_region_")
			+ GetResolutionFolderName() + ".data";
	}
}

MyString MyVisInfo::GetTractColorFileName(int idx) const{
	return string("tractcolor_") + MyString(idx) + ".color";
}

MyString MyVisInfo::GetBoxFileName(int idx) const{
	return string("tumorbox_") + MyString(idx) + string("_region_") 
		+ GetResolutionFolderName() + ".data";
}

MyString MyVisInfo::GetSphereFileName(int idx) const{
	return string("tumorsphere_") + MyString(idx) + string("_region_")
		+ GetResolutionFolderName() + ".data";
}

MyString MyVisInfo::GetString(const MyString& decimer) const{
	return toString(mTask) + decimer
		+ toString(mCover) + decimer
		+ toString(mBundle) + decimer
		+ to_string(mQuest) + decimer
		+ toString(mShape) + decimer
		+ toString(mEncoding) + decimer
		+ to_string(mMappingMethod) + decimer
		+ toString(mVisCue);
}

MyString MyVisInfo::GetStringHeader(const MyString& decimer){
	return "TASK" + decimer
		+ "COVER" + decimer
		+ "BUNDLE" + decimer
		+ "QUEST" + decimer
		+ "SHAPE" + decimer
		+ "ENCODING" + decimer
		+ "METHOD" + decimer
		+ "CUE";
}

MyString MyVisInfo::GetTaskHintString(MyVisEnum::VisTask task){
	switch (task){
	case FA:
		return "(FA difference): What is the difference between average FA value of box 1 and that of box 2?";
		break;
	case TRACE:
		return "(Tract tracing): Do the tracts originated from the red spheres end in box 1, 2, or 3?";
		break;
	case TUMOR:
		return "(Tumor contact): Is the sphere intersecting, tangential or away from the tracts?";
		break;
	case FA_VALUE:
		return "(Average FA value): What is the average FA value of the tracts?";
		break;
	default:
		return "Invalid task!";
		break;
	}
}

MyString MyVisInfo::GetFAAnswerOptionRangeString(int idx){
	const MyVec2f& range = FAAnswerOptionRanges[idx];
	stringstream lss, hss;
	lss << std::setprecision(2);
	hss << std::setprecision(2);
	MyString low, high;
	lss << range[0];
	lss >> low;
	hss << range[1];
	hss >> high;
	if (idx == 6) return "[" + low + ", " + high + "]";
	else return "["+low + ", " + high+")";
}

MyString MyVisInfo::GetTaskAnswerOptionString(VisTask task, int idx){
	if (task == FA){
		return GetFAAnswerOptionRangeString(idx);
	}
	else if (task == TRACE){
		return MyString(idx + 1);
	}
	else if (task == TUMOR){
		switch (idx){
		case 0: return "Intersect";
		case 1: return "Tangential";
		case 2: return "Away";
		default: return "Invalid";
		}
	}
	else if (task == FA_VALUE){
		return "";
	}
	return "Invalid";
}

MyString MyVisInfo::GetTaskAnswerHintString(VisTask task){
	if (task == FA){
		return MyString("Select a range representing averge FA of box 1 minus that of box 2\n\n")+
			MyString("<---------Box 1 Lower------------------Similar------------------Box 1 Higher--------->\n");
	}
	else if (task == TRACE){
		return "Select a box that the mark tracts end in";
	}
	else if (task == TUMOR){
		return "Select the collision state between the tumor sphere and the tracts";
	}
	return "Invalid";
}
const MyArray2f& MyVisInfo::GetFAAnswerOptionRanges(){
	return FAAnswerOptionRanges;
}

int MyVisInfo::GetDataIndex() const{
	if (mTask == FA_VALUE){
		// temp solotion for FA_VALUE task
		// that only uses WHOLE(CC,CST,IFO,ILF)
		// and BUNDLE(CC,CST)
		int idx = 0;
		if (mCover == WHOLE){
			switch (mBundle){
			case CC:
				idx = 0;
				break;
			case CST:
				idx = 2;
				break;
			case IFO:
				idx = 4;
				break;
			case ILF:
				idx = 5;
				break;
			default:
				return -1;
				break;
			}
		}
		else if (mCover == BUNDLE){
			switch (mBundle){
			case CC:
				idx = 1;
				break;
			case CST:
				idx = 3;
				break;
			case IFO:
				return -1;
				break;
			case ILF:
				return -1;
				break;
			default:
				return -1;
				break;
			}
		}
		return idx * 12 + mQuest;
	}
	else if (mTask == TRACE){
		int coverIdxOffset = 0;
		int bundleIdxOffset = 0;
		switch (mCover){
		case WHOLE:
			coverIdxOffset = 0;
			break;
		case BUNDLE:
			coverIdxOffset = 1;
			break;
		default:
			coverIdxOffset = -1;
			break;
		}
		switch (mBundle){
		case CC:
			bundleIdxOffset = 0;
			break;
		case CST:
			bundleIdxOffset = 1;
			break;
		case IFO:
			bundleIdxOffset = 2;
			break;
		case ILF:
			bundleIdxOffset = 3;
			break;
		default:
			bundleIdxOffset = -1;
			break;
		}
		int id = coverIdxOffset * 4 * 4 + bundleIdxOffset * 4 + mQuest;
		if (id < 0) return -1;
		return id;
	}
	return -1;
}