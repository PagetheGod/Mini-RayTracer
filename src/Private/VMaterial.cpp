#include "Public/VMaterial.h"
#include "Public/HittableList.h"

bool VMaterial::DispatchScatter(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered, const MaterialScatterData& Data, MaterialType Type)
{
	bool Result = false;
	switch (Type)
	{
		case MaterialType::Lambertian:
		{
			Result = Lambertian(R, InHitRecord, OutAttenuation, OutScattered, Data);
			break;
		}
		case MaterialType::Metal:
		{
			Result = Metalic(R, InHitRecord, OutAttenuation, OutScattered, Data);
			break;
		}
		case MaterialType::Dielectric:
		{
			Result = Dielectric(R, InHitRecord, OutAttenuation, OutScattered, Data);
			break;
		}
		default:
		{
			return false;
		}
	}
	return Result;
}

bool VMaterial::Lambertian(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered, const MaterialScatterData& Data)
{
	//There's a chance that the random unit vector is pointing opposite to the normal, so we need to check for 0
	Vector3D ScatterDirection = Vector3D::RandomUnitVector() + InHitRecord.HitNormal;
	if (ScatterDirection.NearZero())
	{
		ScatterDirection = InHitRecord.HitNormal;
	}
	OutScattered = Ray(InHitRecord.HitPoint, ScatterDirection);
	OutAttenuation = Data.Albedo;

	return true;
}

bool VMaterial::Metalic(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered, const MaterialScatterData& Data)
{
	Vector3D ScatterDirection = Vector3D::Reflect(R.Direction(), InHitRecord.HitNormal);
	ScatterDirection = ScatterDirection.Normalize() + Data.FuzzOrRI * Vector3D::RandomUnitVector();
	OutScattered = Ray(InHitRecord.HitPoint, ScatterDirection);
	OutAttenuation = Data.Albedo;
	return OutScattered.Direction().Dot(InHitRecord.HitNormal) > 0.f;
}

bool VMaterial::Dielectric(const Ray& R, const HitRecord& InHitRecord, Color& OutAttenuation, Ray& OutScattered, const MaterialScatterData& Data)
{
	//We do not attenuate(absorb) refracted lights
	OutAttenuation = Color(1.f, 1.f, 1.f);

	const Vector3D UnitDirection = R.Direction().Normalize();

	//Get the relative RI that we need to calculate the refraction. Note that we use 1.f as the other RI here because everything else in the scene is air(1.f)
	float RelativeRI = InHitRecord.IsFrontFace ? 1.f / Data.FuzzOrRI : Data.FuzzOrRI;

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

float VMaterial::Reflectance(float Cosine, float RelativeRI)
{
	float R0 = (1.f - RelativeRI) / (1.f + RelativeRI);
	R0 = R0 * R0;

	float t = 1.f - Cosine;
	float t2 = t * t;
	return R0 + (1.f - R0) * t2 * t2 * t;
}
