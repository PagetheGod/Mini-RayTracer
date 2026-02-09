#pragma once

#include "Hittable.h"
#include "Color.h"
#include <vector>

class Material;
class VMaterial;
//We probably should put the definitions of these into some interface class to avoid all the forward decls
//And the pseudo circular references
struct SphereTransformBufferType;
struct SphereMaterialBufferType;

enum MaterialType : uint8_t
{
	Lambertian,
	Metal,
	Dielectric
};

//A(hopefully) well-aligned struct that packs all the transform data of a single sphere
struct SphereTransformData
{
	Vector3D SphereCenter;
	float SphereRadius;
};

//Struct to pack the two(or one, there's a waste of space going on here) floats we need for material scatter calculations
struct MaterialScatterData
{
	float FuzzOrRI = 0.f;
	Color Albedo = (0.f, 0.f, 0.f);
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
	std::vector<std::shared_ptr<Material>> SphereMaterials;
};

struct VSphereMatComponent
{
	VSphereMatComponent()
	{
		MaterialData.reserve(10);
		MaterialTypes.reserve(10);
	}
	std::vector<MaterialType> MaterialTypes;
	std::vector<MaterialScatterData> MaterialData;
};

struct SphereObjectData
{
	Vector3D Center;
	float Radius;
	std::shared_ptr<Material> Material;
};

class HittableList : public Hittable
{
public:
	HittableList();
	HittableList(std::shared_ptr<Hittable> Object);
	~HittableList();
	virtual bool Hit(const Ray& R, Interval HitInterval, HitRecord& OutHitRecord) override;
	//This functions uses the sphere data arrays in the hittablelist class to perform hit detection. Potentially bad name
	bool VBulkHit(const Ray& R, Interval HitInterval, HitRecord& OutHitRecord, MaterialScatterData& OutScatterData);
	bool VSphereHit(const Ray& R, Interval HitInterval, const Vector3D& Center, const float Radius, HitRecord& OutHitRecord);
	void Clear();
	void Add(std::shared_ptr<Hittable> Object);
	void VAddSphere(const SphereObjectData& Data, const MaterialScatterData& MatData, MaterialType MatType);
	SphereTransformBufferType* GetCSTransformBuffer();
	SphereMaterialBufferType* GetCSMaterialBuffer();
	unsigned int GetNumObjects() const { return m_NumObjects; }
public:
	
private:
	std::vector<std::shared_ptr<Hittable>> m_Objects;
	SphereTransformComponent m_SphereTransforms;
	SphereMaterialComponent m_SphereMaterials;
	SphereTransformBufferType* m_CSTransformBuffer;
	SphereMaterialBufferType* m_CSMaterialBuffer;
	VSphereMatComponent m_VSphereMatComponent;
	unsigned int m_NumObjects = 0;
};