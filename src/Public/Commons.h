#pragma once

//Standards
#include <cmath>
#include <iostream>
#include <memory>
#include <limits>
#include <random>

//Custom headers. 
#include "Vector3D.h"
#include "Ray.h"
#include "Color.h"


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
		static std::uniform_real_distribution<float> UniformDist;
		static std::mt19937 RandomGenerator;
		return UniformDist(RandomGenerator);
	}
	//Return a random float in the range [Min, Max), by default uses [0, 1)
	inline static float RandomFloat(float Min, float Max)
	{
		return Min + Utility::RandomFloat() * (Max - Min);
	}
}

