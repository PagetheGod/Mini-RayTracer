#pragma once

#include "Hittable.h"
#include "Color.h"
#include <vector>

class Material;

//A(hopefully) well-aligned struct that packs all the transform data of a single sphere
struct SphereTransformData
{
	Vector3D SphereCenter;
	float SphereRadius;
};

struct SphereTransformComponent
{
	SphereTransformComponent()
	{
		TransformData.reserve(10);
	}
	std::vector<SphereTransformData> TransformData;
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
	void VAddSphere(const SphereObjectData& Data);


private:
	std::vector<std::shared_ptr<Hittable>> m_Objects;
	SphereTransformComponent m_SphereTransforms;
	SphereMaterialComponent m_SphereMaterials;
};