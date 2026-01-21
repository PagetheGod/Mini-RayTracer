#pragma once

#include "Hittable.h"
#include "Color.h"
#include <vector>

class Material;

struct SphereTransformComponent
{
	SphereTransformComponent()
	{
		SphereCenters.reserve(10);
		SphereRadius.reserve(10);
	}
	std::vector<Vector3D> SphereCenters;
	std::vector<float> SphereRadius;
};

struct SphereMaterialComponent
{
	SphereMaterialComponent()
	{
		SphereMaterials.reserve(10);
	}
	std::vector<std::weak_ptr<Material>> SphereMaterials;
};

struct SphereObjectData
{
	Vector3D Center;
	float Radius;
	std::weak_ptr<Material> Material;
};

class HittableList : public Hittable
{
public:
	HittableList() = default;
	HittableList(std::shared_ptr<Hittable> Object);
	virtual bool Hit(const Ray& R, Interval HitInterval, HitRecord& OutHitRecord) override;

	void Clear();
	void Add(std::shared_ptr<Hittable> Object);
	void ECSAdd(const SphereObjectData& Data);


private:
	std::vector<std::shared_ptr<Hittable>> m_Objects;
	SphereTransformComponent m_SphereTransforms;
	SphereMaterialComponent m_SphereMaterials;
};