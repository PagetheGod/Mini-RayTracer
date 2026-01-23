#pragma once

#include "Material.h"


class Lambertian : Material
{
public:
	Lambertian(const Color& Albedo) : m_Albedo(Albedo) {}
	virtual bool Scatter(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered) override;
private:
	//Kinda bad name here. Albedo in this case is used to calculate how much we attenuate(absorb) each color component
	//In other words, it's the "opposite" of color, so it's basically color, just used in the reverse way
	Color m_Albedo;
};

class Metal : Material
{
public:
	Metal(const Color& Albedo) : m_Albedo(Albedo) {}
	virtual bool Scatter(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered) override;
private:
	//Kinda bad name here. Albedo in this case is used to calculate how much we attenuate(absorb) each color component
	//In other words, it's the "opposite" of color, so it's basically color, just used in the reverse way
	Color m_Albedo;
};