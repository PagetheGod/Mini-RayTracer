#pragma once

#include "HittableList.h"


#define USEBULKHIT 1

class Camera
{
public:
	Camera(Point3D InCameraCenter = Point3D(13.f, 2.f, 3.f), float InFocalLength = 1.f, int InSamplePerPixel = 10, float InVerticalFOV = 20.f);
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
	//These variables can be set in the constructor, I just don't want to crowd the constructor with tons of parameters
	//A simple way to solve it would be packing all these camera info into a struct so we can pass it around
	Point3D CameraCenter;
	Point3D LookAt = Point3D(0.f, 0.f, 0.f);
	Vector3D Up = Vector3D(0.f, 1.f, 0.f);
	float FocalLength;
	float VerticalFOV;
	//For depth of field
	float FocusDistance = 10.0f;
	float DefocusAngle = 0.6f;//Variation angle of rays through pixel samples
	Vector3D DefocusDiskU;
	Vector3D DefocusDiskV;


	//Calculated u, v, w that can't be set directly
	Vector3D CameraU;
	Vector3D CameraV;
	Vector3D CameraW;

private:
	//Construct a ray going from origin to a random sample point around a particular pixel
	Ray SendRayToSample(Point3D PixelLocation, Vector3D PixelDeltaU, Vector3D PixelDeltaV) const;
	//Generate the vector to a random sample inside a unit square(-0.5 to 0.5), the return result is meant to be used as an offset
	Vector3D SampleSquare() const;
	//Perform recursive path tracing for all the rays
	Color PerformPathTrace(const Ray& R, HittableList& World) const;
	//Sample a random point in the camera defocus disk
	Point3D SampleDefocusDisk() const;
private:;
	int m_SamplesPerPixel = 10;
	int m_MaxDepth = 10;
};