#include "MyVisInfo.h"

using namespace MyVisEnum;

MyVisInfo::MyVisInfo(VisTask task, bool isTraining) :
	mIsEmpty(true), mIsTraining(isTraining), mTask(task), mMappingMethod(0), mQuest(-1){
}

MyVisInfo::MyVisInfo(bool isEmpty, bool isTraining, VisTask task, RetinalChannel encoding, 
	int method, Shape sp, VisCue visCue, Bundle bundle, FiberCover cover, int quest, int resolution):
	mIsEmpty(isEmpty), mIsTraining(isTraining), mTask(task), 
	mEncoding(encoding), mMappingMethod(method),
	mShape(sp), mVisCue(visCue), mBundle(bundle),
	mCover(cover), mQuest(quest), mResolution(resolution){
}

MyVisInfo::~MyVisInfo()
{
}

MyString MyVisInfo::GetTaskTransitionString() const{
	if (IsEmpty()){
		switch (mTask){
		case START:
			if (IsTraining()) return "Welcome to the training session! Press Next button to start.";
			else return "Welcome to the formal study! Press Next button to start.";
			break;
		case END:
			if (IsTraining()) return "You have finished all the training.";
			else return "You have finished all the test. Thank you for your participation!";
			break;
		case FA:
		case TRACE:
		case TUMOR:
			if (IsTraining()) return "You are now heading to the task training: " + GetTaskHintString(mTask);
			else return "You are now heading to the next task: " + GetTaskHintString(mTask);
			break;
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

MyString MyVisInfo::GetTaskHintString(MyVisEnum::VisTask task){
	switch (task){
	case FA:
		return "What is the difference of average FA values between box 1 and box 2?";
		break;
	case TRACE:
		return "Do the fibers originated from yellow spheres end in box 1, 2, or 3?";
		break;
	case TUMOR:
		return "Is the sphere intersecting, touching or away from the tracts?";
		break;
	default:
		return "Invalid task!";
		break;
	}
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