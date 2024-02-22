#pragma once

#include <vector>

#include "GroundClass.hh"

class FieldClass
{
public:
	FieldClass(const char* filename);
	~FieldClass() = default;

	inline const std::vector<GroundClass>& GetGrounds()
	{
		return grounds_;
	}

private:
	std::vector<GroundClass> grounds_;
};