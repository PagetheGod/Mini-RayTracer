#include "../Public/HittableList.h"
#include "../Public/Material.h"


HittableList::HittableList()
{
	m_SphereTransforms = SphereTransformComponent{};
	m_SphereMaterials = SphereMaterialComponent{};
}

HittableList::HittableList(std::shared_ptr<Hittable> Object)
{
	m_Objects.reserve(10);
	m_Objects.push_back(Object);
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

//These two functions are just here temporarily. Obviously this is not a good architecture but we go with it FOR NOW
bool HittableList::VBulkHit(const Ray& R, Interval HitInterval, HitRecord& OutHitRecord)
{
	bool HasHit = false;
	float ClosestSoFar = HitInterval.Max;
	HitRecord ClosestHitRecord;

	for (size_t i = 0; i < m_NumObjects; i++)
	{
		if (VSphereHit(R, HitInterval, m_SphereTransforms.TransformData[i].SphereCenter, m_SphereTransforms.TransformData[i].SphereRadius, OutHitRecord))
		{
			HasHit = true;
			ClosestSoFar = ClosestHitRecord.t;
			ClosestHitRecord.HitMaterial = m_SphereMaterials.SphereMaterials[i];
			OutHitRecord = ClosestHitRecord;
		}
	}


	return HasHit;
}

bool HittableList::VSphereHit(const Ray& R, Interval HitInterval, const Vector3D& Center, const float Radius, HitRecord& OutHitRecord)
{
	//A simple function to do ray sphere intersection
	Vector3D RayDir = R.Direction();
	Point3D RayOrigin = R.Origin();
	Vector3D RayOriToCenter = Center - RayOrigin;

	/*
	* 1.Now let's do some simplifications, first notice that b has a factor of -2 in it
	* 2.Let's assume for some number h, b = -2h, this will allows to simply b to just h, and the discriminant to h square minus ac, the four got factor out
	* 3.We have h = -2 * RayDir dot RayOriToCenter/ -2, solve this equation and we get h = RayDir Dot RayOriToCenter
	* 4.Also we can simply tho self dot products to length squared
	*/

	float a = RayDir.LengthSquared();
	float h = (RayDir.Dot(RayOriToCenter));
	float c = RayOriToCenter.LengthSquared() - Radius * Radius;


	float Discriminant = h * h - a * c;

	if (Discriminant < 0.f)
	{
		return false;
	}

	float SqrtDis = std::sqrtf(Discriminant);

	float Root = (h - SqrtDis) / a;

	if (!HitInterval.Surrounds(Root))
	{
		Root = (h + SqrtDis) / a;
		if (!HitInterval.Surrounds(Root))
		{
			return false;
		}
	}

	OutHitRecord.HitPoint = R.At(Root);
	OutHitRecord.t = Root;
	Vector3D OutwardNormal = (OutHitRecord.HitPoint - Center) / Radius;
	Hittable::SetFaceNormal(R, OutwardNormal, OutHitRecord);

	return true;
}

void HittableList::Clear()
{
	m_Objects.clear();
}

void HittableList::Add(std::shared_ptr<Hittable> Object)
{
	m_Objects.push_back(Object);
}

void HittableList::VAddSphere(const SphereObjectData& Data)
{
	m_SphereTransforms.TransformData.emplace_back(Data.Center, Data.Radius);
	m_SphereMaterials.SphereMaterials.push_back(Data.Material);
}
