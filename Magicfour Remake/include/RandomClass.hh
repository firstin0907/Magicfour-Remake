#pragma once

#include <random>

class RandomClass
{
public:
	template <typename T>
	static inline T rand(T cases)
	{
		return std::uniform_int_distribution<T>(0, cases - 1)(generator);
	}

	// return random number in range of [s, e)
	template <typename T>
	static inline T rand(T s, T e)
	{
		return std::uniform_int_distribution<T>(s, e)(generator);
	}

private:
	static std::mt19937 generator;
};