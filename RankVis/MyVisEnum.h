#pragma once

#include <string>

namespace MyVisEnum{

	using namespace std;

	typedef enum RetinalChannel_t{
		VALUE = 1,
		TEXTURE = 2,
		SHAPE = 3,
		ESIZE = 4,
		COLOR = 5,
		ORIENTATION = 6,
		POSITION = 7
	} RetinalChannel;

	typedef enum VisTask_t{
		FA = 1,
		TRACE = 2,
		SAME = 3,
		TUMOR = 5,
		START = 6,
		END = 7
	} VisTask;

	typedef enum Bundle_t{
		CC = 1,
		CST = 2,
		IFO = 3,
		ILF = 4,
		CG = 5
	} Bundle;

	typedef enum Shape_t{
		LINE = 1,
		TUBE = 2,
		SUPERQUADRIC = 3,
		SUPERELLIPSOID = 4,
		RIBBON = 5
	} Shape;

	typedef enum FiberCover_t{
		BUNDLE = 1,
		WHOLE = 2,
		PARTIAL = 3
	} FiberCover;

	typedef enum VisCue_t{
		BASIC = 1,
		CURV_MIN = 2,
		CURV_MAX = 3,
		DEPTH = 4,
		AMBIENT_OCCULUSION = 5,
		ENCODING = 6,
	} VisCue;

	typedef enum CollisionStatus_t{
		INTERSECT = 1,
		TOUCH = 2,
		AWAY = 3
	} CollisionStatus;

	string toString(RetinalChannel rc);
	string toString(VisTask task);
	string toString(Bundle bundle);
	string toString(Shape shape);
	string toString(FiberCover cv);
	string toString(VisCue cue);
	string toString(CollisionStatus cs);
}