#include "../Public/Camera.h"
#include <stack>

//Initialize camera parameters and delta U,V
//The camera center is also the origin of our coordinate system
Camera::Camera(Point3D InCameraCenter, float InFocalLength, int InSamplePerPixel) : CameraCenter(InCameraCenter), FocalLength(InFocalLength), m_SamplesPerPixel(InSamplePerPixel)
{

}
Color Camera::CalculateHitColor(HittableList& World, Point3D PixelLocation, Vector3D PixelDeltaU, Vector3D PixelDeltaV) const
{
	//Later we need to multiply the calculation from multiple samples with this to average them
	float SampleScaleFactor = 1.f / (float)m_SamplesPerPixel;

	Color PixelColor = Color(0.f, 0.f, 0.f);
	for (int i = 0; i < m_SamplesPerPixel; i++)
	{
		Ray CurrentRay = SendRayToSample(PixelLocation, PixelDeltaU, PixelDeltaV);
		PixelColor += PerformPathTrace(CurrentRay, World);
	}
	PixelColor *= SampleScaleFactor;
	PixelColor = NormalizeColor(PixelColor);
	return PixelColor;
}

Ray Camera::SendRayToSample(Point3D PixelLocation, Vector3D PixelDeltaU, Vector3D PixelDeltaV) const
{
	Vector3D Offset = SampleSquare();


	Point3D PixelSample = PixelLocation + Offset.X * PixelDeltaU + Offset.Y * PixelDeltaV;

	return Ray(CameraCenter, PixelSample - CameraCenter);
}

Vector3D Camera::SampleSquare() const
{
	return Vector3D(Utility::RandomFloat(-0.5f, 0.5f), Utility::RandomFloat(-0.5f, 0.5f), 0.f);
}

Color Camera::PerformPathTrace(const Ray& R, HittableList& World) const
{
	Color PixelColor = Color{ 0.f, 0.f, 0.f };
	HitRecord TempHitRecord;
	std::stack<Ray> TraceStack;
	int CurrentDepth = 0;
	TraceStack.push(R);
	while (!TraceStack.empty() && CurrentDepth < m_MaxDepth)
	{
		Ray CurrentRay = TraceStack.top();
		TraceStack.pop();
		if (World.Hit(CurrentRay, Interval(0.001f, Constants::g_Infinity), TempHitRecord))
		{
			Vector3D DiffuseDirection = Vector3D::RandomUnitOnHemiSphere(TempHitRecord.HitNormal);
			Ray NewRay = Ray(TempHitRecord.HitPoint, DiffuseDirection);
			CurrentDepth++;
			TraceStack.push(NewRay);
		}
		else
		{
			Vector3D UnitDirection = CurrentRay.Direction().Normalize();
			float t = 0.5f * (UnitDirection.X + 1.f);//We are working with a unit vector with X in [-1,1] so we have to map X from [-1,1] to [0,1] first
			PixelColor += ((1.f - t) * Color(0.9f, 0.9f, 0.9f) + t * Color(0.5f, 0.7f, 1.f));
			return std::pow(0.5f, CurrentDepth) * PixelColor;
		}
	}

	return Color{ 0.f, 0.f, 0.f };
}
