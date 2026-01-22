#pragma once


#include "Hittable.h"
#include "Color.h"

class Material
{
public:
	virtual ~Material() = default;
	
	virtual bool Scatter(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered) = 0;
};