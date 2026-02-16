#pragma once

#include <random>

class RandomClass
{
public:
	// return random number in range of [s, e)
	template <typename T>
	static inline T rand(T s, T e)
	{
		if constexpr (std::is_integral_v<T>)
		{
			return std::uniform_int_distribution<T>(s, e - 1)(generator_);
		}
		else
		{
			return std::uniform_real_distribution<T>(s, e)(generator_);
		}
	}

	template <typename T>
	static inline T rand(T e)
	{
		if constexpr (std::is_integral_v<T>)
		{
			return std::uniform_int_distribution<T>(0, e - 1)(generator_);
		}
		else
		{
			return std::uniform_real_distribution<T>(0, e)(generator_);
		}
	}

private:
	static std::mt19937 generator_;
};

inline std::mt19937 RandomClass::generator_ = std::mt19937(std::random_device()());