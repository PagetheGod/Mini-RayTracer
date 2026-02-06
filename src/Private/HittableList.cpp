#include "../Public/HittableList.h"
#include "../Public/Material.h"
#include "../Public/VMaterial.h"
#include "../Public/ComputeShaderManager.h"


HittableList::HittableList() : m_SphereTransforms(SphereTransformComponent{}), m_VSphereMatComponent(VSphereMatComponent{}), m_CSTransformBuffer(nullptr),
m_CSMaterialBuffer(nullptr)
{

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
		if (Object->Hit(R, Interval(HitInterval.Min, ClosestSoFar), OutHitRecord))
		{
			HasHit = true;
			ClosestSoFar = OutHitRecord.t;
		}
	}

	return HasHit;
}

//These two functions are just here temporarily. Obviously this is not a good architecture but we go with it FOR NOW
bool HittableList::VBulkHit(const Ray& R, Interval HitInterval, HitRecord& OutHitRecord, MaterialScatterData& OutScatterData)
{
	bool HasHit = false;
	float ClosestSoFar = HitInterval.Max;
	HitRecord ClosestHitRecord;

	for (size_t i = 0; i < m_NumObjects; i++)
	{
		if (VSphereHit(R, Interval(HitInterval.Min, ClosestSoFar), m_SphereTransforms.TransformData[i].SphereCenter, m_SphereTransforms.TransformData[i].SphereRadius, OutHitRecord))
		{
			HasHit = true;
			ClosestSoFar = OutHitRecord.t;
			OutScatterData = m_VSphereMatComponent.MaterialData[i];
			OutHitRecord.VHitMaterial = m_VSphereMatComponent.MaterialTypes[i];
			
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

void HittableList::VAddSphere(const SphereObjectData& Data, const MaterialScatterData& MatData, MaterialType MatType)
{
	m_SphereTransforms.TransformData.emplace_back(Data.Center, Data.Radius);
	m_VSphereMatComponent.MaterialData.emplace_back(MatData.FuzzOrRI, MatData.Albedo);
	m_VSphereMatComponent.MaterialTypes.push_back(MatType);
	m_NumObjects++;
}

SphereTransformBufferType* HittableList::GetCSTransformBuffer()
{
	m_CSTransformBuffer = new SphereTransformBufferType[m_NumObjects];
	for (size_t i = 0; i < m_NumObjects; i++)
	{
		SphereTransformData& Data = m_SphereTransforms.TransformData[i];
		m_CSTransformBuffer[i].SphereCenterPos = XMFLOAT3(Data.SphereCenter.X, Data.SphereCenter.Y, Data.SphereCenter.Z);
		m_CSTransformBuffer[i].Radius = Data.SphereRadius;
	}
	return m_CSTransformBuffer;
}

SphereMaterialBufferType* HittableList::GetCSMaterialBuffer()
{
	m_CSMaterialBuffer = new SphereMaterialBufferType[m_NumObjects];
	for (size_t i = 0; i < m_NumObjects; i++)
	{
		auto& [MaterialType, MaterialData] = m_VSphereMatComponent;
		m_CSMaterialBuffer[i].Albedo = XMFLOAT3(MaterialData[i].Albedo.X, MaterialData[i].Albedo.Y, MaterialData[i].Albedo.Z);
		m_CSMaterialBuffer[i].FuzzOrRI = MaterialData[i].FuzzOrRI;
		m_CSMaterialBuffer[i].Type = MaterialType[i];
	}
	return m_CSMaterialBuffer;
}

