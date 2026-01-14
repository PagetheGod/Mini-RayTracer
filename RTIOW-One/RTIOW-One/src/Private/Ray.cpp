#include "../Public/Ray.h"

Ray::Ray(const Point3D& InOrigin, const Vector3D InDirection) : m_Origin(InOrigin), m_Direction(InDirection)
{
}

Point3D Ray::At(float T) const
{
	return m_Origin + T * m_Direction;
}
