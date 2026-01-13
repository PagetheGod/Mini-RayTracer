#pragma once


#include "Ray.h"



//Abstract class representing a hittable object in the scene. I do not like the idea of this abstract class, maybe switch to something else later
class Hittable
{
public:

	virtual ~Hittable() = default;
	virtual bool Hit(const Ray& R, float Ray_tMin, float Ray_tMax, HitRecord& OutHitRecord) = 0;
};


struct HitRecord
{
	Point3D HitPoint;
	Vector3D HitNormal;
	float t;
};