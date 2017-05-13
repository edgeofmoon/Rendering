#pragma once
#include "MyTextArea.h"

#include <unordered_set>

class MyNumberArea :
	public MyTextArea
{
public:
	MyNumberArea();
	~MyNumberArea();

	bool GetInt(int &x) const;
	bool GetFloat(float &x) const;

protected:
	static const std::unordered_set<unsigned char> ValideKeys;
	virtual bool IsKeyValid(unsigned char key);
};

