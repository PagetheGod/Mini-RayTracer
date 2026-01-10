#pragma once

#include "Vector3D.h"


class Ray
{
public:
	Ray(const Point3D& Origin, const Vector3D Direction);
	const Point3D& Origin() const { return m_Origin; }
	const Vector3D& Direction() const { return m_Direction; }
	Point3D At(float T) const;

private:
	Point3D m_Origin;
	Vector3D m_Direction;
};