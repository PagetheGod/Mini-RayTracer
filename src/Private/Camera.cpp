#include "../Public/Camera.h"
#include "../Public/Material.h"
#include <stack>

//Initialize camera parameters and delta U,V
//The camera center is also the origin of our coordinate system
Camera::Camera(Point3D InCameraCenter, float InFocalLength, int InSamplePerPixel, float InVerticalFOV) : CameraCenter(InCameraCenter), FocalLength(InFocalLength),
VerticalFOV(InVerticalFOV), m_SamplesPerPixel(InSamplePerPixel)
{
	FocalLength = (LookAt - CameraCenter).Length();

	CameraW = (CameraCenter - LookAt).Normalize();
	CameraU = Up.Cross(CameraW).Normalize();
	CameraV = CameraW.Cross(CameraU);

	float DefocusRadius = FocusDistance * std::tan(Utility::DegreeToRadian(DefocusAngle / 2.f));
	DefocusDiskU = DefocusRadius * CameraU;
	DefocusDiskV = DefocusRadius * CameraV;

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
	Point3D RayOrigin = DefocusAngle <= 0.f ? CameraCenter : SampleDefocusDisk();
	Vector3D RayDirection = PixelSample - RayOrigin;

	return Ray(RayOrigin, RayDirection);
}

Vector3D Camera::SampleSquare() const
{
	return Vector3D(Utility::RandomFloat(-0.5f, 0.5f), Utility::RandomFloat(-0.5f, 0.5f), 0.f);
}

/*
* This function uses stack for now. It's mostly used for getting some practices with stack
* It's not optimal, since we would only ever have one Ray on the stack
* It had since been changed to using a for-loop. However, the stack implementations are kept for reference and possible future uses
*/
Color Camera::PerformPathTrace(const Ray& R, HittableList& World) const
{
	Color PixelColor = Color{ 0.f, 0.f, 0.f };
	HitRecord TempHitRecord;
	Ray CurrentRay = R;
	Color TotalAttenuation = Color{1.f, 1.f, 1.f};
	for (int i = 0; i < m_MaxDepth; i++)
	{

		if (!USEBULKHIT)
		{
			if (World.Hit(CurrentRay, Interval(0.001f, Constants::g_Infinity), TempHitRecord))
			{
				/*
				* Lambertian reflection - light is more likely to bounce in directions close to normal
				* So we add a random unit vector to the normal. To get a random bounce on the unit tangent sphere
				*/
				Ray ScatteredRay;
				Color Attenuation;
				if (TempHitRecord.HitMaterial->Scatter(CurrentRay, TempHitRecord, Attenuation, ScatteredRay))
				{
					CurrentRay = ScatteredRay;
					TotalAttenuation = TotalAttenuation * Attenuation;
				}
				else
				{
					return Color(0.f, 0.f, 0.f);
				}
			}
			else
			{
				Vector3D UnitDirection = CurrentRay.Direction().Normalize();
				float t = 0.5f * (UnitDirection.X + 1.f);//We are working with a unit vector with X in [-1,1] so we have to map X from [-1,1] to [0,1] first
				PixelColor += ((1.f - t) * Color(0.9f, 0.9f, 0.9f) + t * Color(0.5f, 0.7f, 1.f));
				return TotalAttenuation * PixelColor;
			}
		}
		else
		{
			if (World.VBulkHit(CurrentRay, Interval(0.001f, Constants::g_Infinity), TempHitRecord))
			{
				/*
				* Lambertian reflection - light is more likely to bounce in directions close to normal
				* So we add a random unit vector to the normal. To get a random bounce on the unit tangent sphere
				*/
				Ray ScatteredRay;
				Color Attenuation;
				if (TempHitRecord.HitMaterial->Scatter(CurrentRay, TempHitRecord, Attenuation, ScatteredRay))
				{
					CurrentRay = ScatteredRay;
					TotalAttenuation = TotalAttenuation * Attenuation;
				}
				else
				{
					return Color(0.f, 0.f, 0.f);
				}
			}
			else
			{
				Vector3D UnitDirection = CurrentRay.Direction().Normalize();
				float t = 0.5f * (UnitDirection.X + 1.f);//We are working with a unit vector with X in [-1,1] so we have to map X from [-1,1] to [0,1] first
				PixelColor += ((1.f - t) * Color(0.9f, 0.9f, 0.9f) + t * Color(0.5f, 0.7f, 1.f));
				return TotalAttenuation * PixelColor;
			}
		}
		
	}
	/*
	std::vector<Ray> Cont;
	Cont.reserve(m_MaxDepth);
	std::stack<Ray, std::vector<Ray>> TraceStack(std::move(Cont));
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
	}*/

	return Color{ 0.f, 0.f, 0.f };
}

Point3D Camera::SampleDefocusDisk() const
{
	Point3D Point = Vector3D::RandomOnUnitDisk();
	return CameraCenter + Point.X * DefocusDiskU + Point.Y * DefocusDiskV;
}
