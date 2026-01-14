#include "../Public/Hittable.h"

void Hittable::SetFaceNormal(const Ray& R, const Vector3D& OutwardNormal, HitRecord& OutHitRecord)
{
	//OutwardNormal is assumed to be normal length
	OutHitRecord.IsFrontFace = OutwardNormal.Dot(R.Direction()) < 0.f;
	OutHitRecord.HitNormal = OutHitRecord.IsFrontFace ? OutwardNormal : -OutwardNormal;
}
