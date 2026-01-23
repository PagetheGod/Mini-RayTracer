#pragma once

#include "HittableList.h"


#define USEBULKHIT 0

class Camera
{
public:
	Camera(Point3D InCameraCenter = Point3D(0.f, 0.f, 0.f), float InFocalLength = 1.f, int InSamplePerPixel = 10);
	Color CalculateHitColor(HittableList& World, Point3D PixelLocation, Vector3D PixelDeltaU, Vector3D PixelDeltaV) const;
	void SetSampleCount(int InSampleCount)
	{
		m_SamplesPerPixel = InSampleCount;
	}
	void SetMaxDepth(int InMaxDepth)
	{
		m_MaxDepth = InMaxDepth;
	}
public:
	Point3D CameraCenter;
	float FocalLength;
private:
	//Construct a ray going from origin to a random sample point around a particular pixel
	Ray SendRayToSample(Point3D PixelLocation, Vector3D PixelDeltaU, Vector3D PixelDeltaV) const;
	//Generate the vector to a random sample inside a unit square(-0.5 to 0.5), the return result is meant to be used as an offset
	Vector3D SampleSquare() const;
	//Perform recursive path tracing for all the rays
	Color PerformPathTrace(const Ray& R, HittableList& World) const;

private:;
	int m_SamplesPerPixel = 10;
	int m_MaxDepth = 10;
};