#pragma once

#include "Hittable.h"
#include <vector>


class HittableList : public Hittable
{
public:
	HittableList() = default;
	HittableList(std::shared_ptr<Hittable> Object);
	virtual bool Hit(const Ray& R, Interval HitInterval, HitRecord& OutHitRecord) override;

	void Clear();
	void Add(std::shared_ptr<Hittable> Object);


private:
	std::vector<std::shared_ptr<Hittable>> m_Objects;
};