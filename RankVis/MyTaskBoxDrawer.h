#pragma once

#include "MyBoundingBox.h"

class MyTaskBoxDrawer
{
public:
	MyTaskBoxDrawer(const MyBoundingBox* box = NULL);
	~MyTaskBoxDrawer();

	void SetBox(const MyBoundingBox* box){ mBox = box; };
	void Draw();
	void SetHighlighted(bool h){ mHighlighted = h; };

protected:
	const MyBoundingBox* mBox;
	bool mHighlighted;
};

