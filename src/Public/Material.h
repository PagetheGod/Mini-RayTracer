#pragma once


#include "Hittable.h"
#include "Color.h"

class Material
{
public:
	virtual ~Material() = default;
	
	virtual bool Scatter(const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered) = 0;
};