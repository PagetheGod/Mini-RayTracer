#pragma once


#include "Ray.h"
#include "Interval.h"

/*
* About the IsFrontFace variable:
* 1. Eventually we will have to deal with situations where we need to tell whether a ray hits the front face or the back face(inside) of an object
* 2. We have two ways to do this: 1. have the normals always point out, and use the dot product to determine which side the light hits at time of coloring
* 2.2 -> have the normals always point against the ray, and store whether we hit the object from inside/outside at the time of geometric calculations
* 2. cont, in this project, we have more materials than complex geometries, so opt for storing the information at geometry
*/

class Material;
enum MaterialType : uint8_t;


struct HitRecord
{
	Point3D HitPoint;
	Vector3D HitNormal;
	float t;
	bool IsFrontFace;
	std::shared_ptr<Material> HitMaterial;
	MaterialType VHitMaterial;
};

//Abstract class representing a hittable object in the scene. I do not like the idea of this abstract class, maybe switch to something else later
class Hittable
{
public:
	virtual ~Hittable() = default;
	virtual bool Hit(const Ray& R, Interval HitInterval, HitRecord& OutHitRecord) = 0;
	
	static void SetFaceNormal(const Ray& R, const Vector3D& OutwardNormal, HitRecord& OutHitRecord);
};


