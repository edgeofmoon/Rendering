#include "MyNumberArea.h"

#include "GL/glew.h"
#include <GL/freeglut.h>

const std::unordered_set<unsigned char> MyNumberArea::ValideKeys({
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'-', '+', 'e', '.', '\b', 127 });


MyNumberArea::MyNumberArea()
{
}


MyNumberArea::~MyNumberArea()
{
}

bool MyNumberArea::GetInt(int &x) const{
	try {
		size_t idx;
		int n = stoi(mText, &idx);
		if (idx == mText.size()){
			x = n;
			return true;
		}
		return false;
	}
	catch (std::invalid_argument& e){
		// if no conversion could be performed
		return false;
	}
	catch (std::out_of_range& e){
		// if the converted value would fall out of the range of the result type 
		// or if the underlying function (std::strtol or std::strtoull) sets errno 
		// to ERANGE.
		return false;
	}
	catch (...) {
		// everything else
		return false;
	}
}

bool MyNumberArea::GetFloat(float &x) const{
	try {
		size_t idx;
		float n = stof(mText, &idx);
		if (idx == mText.size()){
			x = n;
			return true;
		}
		return false;
	}
	catch (std::invalid_argument& e){
		// if no conversion could be performed
		return false;
	}
	catch (std::out_of_range& e){
		// if the converted value would fall out of the range of the result type 
		// or if the underlying function (std::strtol or std::strtoull) sets errno 
		// to ERANGE.
		return false;
	}
	catch (...) {
		// everything else
		return false;
	}
}

bool MyNumberArea::IsKeyValid(unsigned char key){
	if (ValideKeys.count(key) == 0) return false;
	return true;
}
