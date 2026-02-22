#include "Public/SubMaterials.h"

bool Lambertian::Scatter(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered)
{
	//There's a chance that the random unit vector is pointing opposite to the normal, so we need to check for 0
	Vector3D ScatterDirection = Vector3D::RandomUnitVector() + InHitRecord.HitNormal;
	if (ScatterDirection.NearZero())
	{
		ScatterDirection = InHitRecord.HitNormal;
	}
	OutScattered = Ray(InHitRecord.HitPoint, ScatterDirection);
	OutAttenuation = m_Albedo;

	return true;
}

bool Metal::Scatter(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered)
{
	Vector3D ScatterDirection = Vector3D::Reflect(R.Direction(), InHitRecord.HitNormal);
	ScatterDirection = ScatterDirection.Normalize() + m_Fuzz * Vector3D::RandomUnitVector();
	OutScattered = Ray(InHitRecord.HitPoint, ScatterDirection);
	OutAttenuation = m_Albedo;
	return OutScattered.Direction().Dot(InHitRecord.HitNormal) > 0.f;
}

bool Dielectric::Scatter(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered)
{
	//We do not attenuate(absorb) refracted lights
	OutAttenuation = Color(1.f, 1.f, 1.f);

	const Vector3D UnitDirection = R.Direction().Normalize();

	//Get the relative RI that we need to calculate the refraction. Note that we use 1.f as the other RI here because everything else in the scene is air(1.f)
	float RelativeRI = InHitRecord.IsFrontFace ? 1.f / m_RefractionIndex : m_RefractionIndex;

	//We need to also account for total internal reflection, case where we travel from a material with hight RI to one with low RI
	//And we hit it at a grazing angle, this causes the light to get reflected instead of refracted(basically Snell's Law can't find a solution for sin(theta') here
	float CosTheta = std::min(-UnitDirection.Dot(InHitRecord.HitNormal), 1.f);
	float SinTheta = std::sqrt(1 - CosTheta * CosTheta);
	bool CanRefract = RelativeRI * SinTheta <= 1.f;
	Vector3D ScatteredDirection; 
	if (CanRefract && Reflectance(CosTheta, RelativeRI) <= Utility::RandomFloat())
	{
		ScatteredDirection = Vector3D::Refract(UnitDirection, InHitRecord.HitNormal, RelativeRI);
	}
	else
	{
		ScatteredDirection = Vector3D::Reflect(UnitDirection, InHitRecord.HitNormal);
	}
	
	OutScattered = Ray(InHitRecord.HitPoint, ScatteredDirection);

	return true;
}

float Dielectric::Reflectance(float Cosine, float RelativeRI)
{
	float R0 = (1.f - RelativeRI) / (1.f + RelativeRI);
	R0 = R0 * R0;

	return R0 + (1.f - R0) * std::pow(1.f - Cosine, 5);
}
