#pragma once

//Standards
#include <cmath>
#include <iostream>
#include <memory>
#include <limits>

//Custom headers. 
#include "Vector3D.h"
#include "Ray.h"
#include "Color.h"


namespace Constants
{
	inline constexpr float g_PI = 3.1415926535897f;
	inline constexpr float g_Infinity = std::numeric_limits<float>::infinity();
}

namespace Utility
{
	inline static float DegreeToRadian(float Degree)
	{
		return Degree * Constants::g_PI / 180.f;
	}
}

