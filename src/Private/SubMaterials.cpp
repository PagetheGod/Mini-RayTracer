#include "../Public/SubMaterials.h"

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
