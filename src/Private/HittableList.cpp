#include "../Public/HittableList.h"

HittableList::HittableList(std::shared_ptr<Hittable> Object)
{
	m_Objects.reserve(10);
	m_Objects.push_back(Object);
}

bool HittableList::Hit(const Ray& R, float Ray_tMin, float Ray_tMax, HitRecord& OutHitRecord)
{
	bool HasHit = false;
	float ClosestSoFar = Ray_tMax;
	HitRecord ClosestHitRecord;

	for (const std::shared_ptr<Hittable>& Object : m_Objects)
	{
		if (Object->Hit(R, Ray_tMin, ClosestSoFar, ClosestHitRecord))
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
