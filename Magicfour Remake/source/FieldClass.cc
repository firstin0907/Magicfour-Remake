#include "../include/FieldClass.hh"

#include <fstream>

#include "../include/GameException.hh"

using namespace std;

FieldClass::FieldClass(const char* filename)
{
	ifstream fin(filename);

	if(fin.fail()) throw filenotfound_error(filename, WFILE, __LINE__);

	while (!fin.eof())
	{
		int left, bottom, right, top;
		fin >> left >> bottom >> right >> top;
		grounds_.emplace_back(rect_t{ left, bottom, right, top });
	}
}
