#include "../Public/Sphere.h"

Sphere::Sphere(const Point3D& InCenter, const float InRadius) : m_Center(InCenter), m_Radius(std::fmax(0.f, InRadius))
{

}

bool Sphere::Hit(const Ray& R, Interval HitInterval, HitRecord& OutHitRecord)
{
	//A simple test function to do ray sphere intersection
	Vector3D RayDir = R.Direction();
	Point3D RayOrigin = R.Origin();
	Vector3D RayOriToCenter = m_Center - RayOrigin;

	/*
	* 1.Now let's do some simplifications, first notice that b has a factor of -2 in it
	* 2.Let's assume for some number h, b = -2h, this will allows to simply b to just h, and the discriminant to h square minus ac, the four got factor out
	* 3.We have h = -2 * RayDir dot RayOriToCenter/ -2, solve this equation and we get h = RayDir Dot RayOriToCenter
	* 4.Also we can simply tho self dot products to length squared
	*/

	float a = RayDir.LengthSquared();
	float h = (RayDir.Dot(RayOriToCenter));
	float c = RayOriToCenter.LengthSquared() - m_Radius * m_Radius;


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
	Vector3D OutwardNormal = (OutHitRecord.HitPoint - m_Center) / m_Radius;
	Hittable::SetFaceNormal(R, OutwardNormal, OutHitRecord);



	return true;
}
