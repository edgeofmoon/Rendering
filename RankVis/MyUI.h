#pragma once

#include "MyVec.h";
namespace MyUI{
	typedef enum MyObjectStatusBit_t{
		// status
		ENABLE_BIT = (1 << 0),
		HIDE_BIT = (1 << 1),
		DOWN_BIT = (1 << 2),
		INSIDE_BIT = (1 << 3),
		FOCUS_BIT = (1 << 4),

		// event
		PUSHED_BIT = (1 << 5),
		POPED_BIT = (1 << 6),
		ENTERED_BIT = (1 << 7),
		EXITED_BIT = (1 << 8),
		ALL_EVENT_BITS = (PUSHED_BIT | POPED_BIT
		| ENTERED_BIT | EXITED_BIT),
	}MyObjectStatusBit;

	extern MyVec4i mUIViewport;

	void UIBeginDrawer();
	void UIEndDrawer();

	inline void SetViewport(const MyVec4i& viewport){ mUIViewport = viewport; }

}