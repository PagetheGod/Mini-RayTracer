#pragma once

#include "HittableList.h"

//This class is not used by the project as of right now due to design decisions I amde
class Sphere : public Hittable
{
public:
	Sphere(const Point3D& InCenter, const float InRadius, std::shared_ptr<Material> InMaterial);
	virtual bool Hit(const Ray& R, Interval HitInterval, HitRecord& OutHitRecord) override;

private:
	Point3D m_Center;
	float m_Radius;
	std::shared_ptr<Material> m_Material;
};