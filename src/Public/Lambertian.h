#pragma once

#include "Material.h"


class Lambertian : Material
{
public:
	Lambertian(const Color& Albedo) : m_Albedo(Albedo) {}
	virtual bool Scatter(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered) override;
private:
	Color m_Albedo;
};