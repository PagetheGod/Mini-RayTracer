#pragma once

#include "Commons.h"
#ifndef _MSC_VER
#include<algorithm>
#endif

class Interval
{
public:
	Interval() = default;
	Interval(float InMin, float InMax): Min(InMin), Max(InMax) {} 

	//Whether a number num is contained in the interval, including END POINTS
	inline bool Contains(float Num) const
	{
		return (Num >= Min && Num <= Max);
	}
	//Whether a number num is contained in the interval, excluding END POINTS
	inline bool Surrounds(float Num) const
	{
		return (Num > Min && Num < Max);
	}
	inline float Size() const
	{
		return Max - Min;
	}
	inline float Clamp(float ToClamp) const
	{
		return std::clamp(ToClamp, Min, Max);
	}
public:
	float Min = Constants::g_Infinity;
	float Max = -Constants::g_Infinity;

	static const Interval g_Empty;
	static const Interval g_Universe;
};

