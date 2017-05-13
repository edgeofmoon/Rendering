#pragma once
#include "MySphereGeometry.h"

class MySphereAOGeometry :
	public MySphereGeometry
{
public:
	MySphereAOGeometry();
	~MySphereAOGeometry();

	virtual void GenerateGeometry();
	//virtual void DrawGeometry();
};

