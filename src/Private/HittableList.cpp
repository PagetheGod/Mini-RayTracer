#include "../Public/HittableList.h"

HittableList::HittableList(std::shared_ptr<Hittable> Object)
{
	m_Objects.reserve(10);
	m_Objects.push_back(Object);
	m_SphereTransforms = SphereTransformComponent{};
	m_SphereMaterials = SphereMaterialComponent{};
}

bool HittableList::Hit(const Ray& R, Interval HitInterval, HitRecord& OutHitRecord)
{
	bool HasHit = false;
	float ClosestSoFar = HitInterval.Max;
	HitRecord ClosestHitRecord;

	for (const std::shared_ptr<Hittable>& Object : m_Objects)
	{
		if (Object->Hit(R, Interval(HitInterval.Min, ClosestSoFar), ClosestHitRecord))
		{
			HasHit = true;
			ClosestSoFar = ClosestHitRecord.t;
			OutHitRecord = ClosestHitRecord;
		}
	}

	return HasHit;
}

void HittableList::Clear()
{
	m_Objects.clear();
}

void HittableList::Add(std::shared_ptr<Hittable> Object)
{
	m_Objects.push_back(Object);
}

void HittableList::ECSAdd(const SphereObjectData& Data)
{
	m_SphereTransforms.SphereCenters.push_back(Data.Center);
	m_SphereTransforms.SphereRadius.push_back(Data.Radius);
	m_SphereMaterials.SphereMaterials.push_back(Data.Material);
}
