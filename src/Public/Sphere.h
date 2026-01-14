#pragma once

#include "Hittable.h"

class Sphere : public Hittable
{
public:
	Sphere(const Point3D& InCenter, const float InRadius);
	virtual bool Hit(const Ray& R, float Ray_tMin, float Ray_tMax, HitRecord& OutHitRecord) override;

private:
	Point3D m_Center;
	float m_Radius;
};