#pragma once

/*
* This file strays pretty far from the book because I really do not like how the book introduces tons of circular dependencies in the Common header
*/


//Standards
#include <cmath>
#include <iostream>
#include <memory>
#include <limits>
#include <random>

//Custom headers


namespace Constants
{
	inline constexpr float g_PI = 3.14159265358979f;
	inline constexpr float g_Infinity = std::numeric_limits<float>::infinity();
}

namespace Utility
{
	inline static float DegreeToRadian(float Degree)
	{
		return Degree * Constants::g_PI / 180.f;
	}
	inline static float RandomFloat()
	{
		//thread_local makes the variable "static" to the thread that creates it. So other threads do not interfere.
		//If we had just marked this as static, we basically get a chaotic race. Not only is the random generator not safe now, we are also slowing things down
		thread_local std::uniform_real_distribution<float> UniformDist;
		thread_local std::mt19937 RandomGenerator;
		return UniformDist(RandomGenerator);
	}
	//Return a random float in the range [Min, Max), by default uses [0, 1)
	inline static float RandomFloat(float Min, float Max)
	{
		return Min + Utility::RandomFloat() * (Max - Min);
	}
}

