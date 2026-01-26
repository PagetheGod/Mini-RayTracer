#pragma once

#include "Material.h"


class Lambertian : public Material
{
public:
	Lambertian(const Color& Albedo) : m_Albedo(Albedo) {}
	virtual bool Scatter(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered) override;
private:
	//Kinda bad name here. Albedo in this case is used to calculate how much we attenuate(absorb) each color component
	//In other words, it's the "opposite" of color, so it's basically color, just used in the reverse way
	Color m_Albedo;
};

class Metal : public Material
{
public:
	//Fuzz is a paramter that gives us a fuzzy reflections for metallic materials
	//It represents the radius of a sphere centered at the original endpoint of a reflected ray(it scales a random vector on the unit sphere)
	//Use 0 for no fuzziness
	Metal(const Color& Albedo, const float Fuzz) : m_Albedo(Albedo), m_Fuzz(Fuzz < 1.f ? Fuzz : 1.f) {}
	virtual bool Scatter(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered) override;
private:
	//Kinda bad name here. Albedo in this case is used to calculate how much we attenuate(absorb) each color component
	//In other words, it's the "opposite" of color, so it's basically color, just used in the reverse way
	Color m_Albedo;
	float m_Fuzz;
};