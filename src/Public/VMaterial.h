#pragma once

#include "Hittable.h"
#include "Color.h"


struct MaterialScatterData;
enum MaterialType : uint8_t;

class VMaterial
{
public:
	VMaterial() = default;
	
	static bool DispatchScatter(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered, const MaterialScatterData& Data, MaterialType Type);

private:
	static bool Lambertian(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered, const MaterialScatterData&);
	static bool Metalic(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered, const MaterialScatterData&);
	static bool Dielectric(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered, const MaterialScatterData&);
	//Schlick Approximation for reflectance, this is used to approximate materials like glass, which can have varying degree of reflectance based on viewing angle
	static float Reflectance(float Cosine, float RelativeRI);

};