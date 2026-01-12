#include "../Public/Vector3D.h"
#include <cmath>


Vector3D::Vector3D(float InX, float InY, float InZ) : X(InX), Y(InY), Z(InZ)
{
	
}

Vector3D Vector3D::operator-()
{
	return Vector3D(-X, -Y, -Z);
}

Vector3D& Vector3D::operator+=(const Vector3D& Other)
{
	X += Other.X;
	Y += Other.Y;
	Z += Other.Z;
	return *this;
}

Vector3D& Vector3D::operator*=(const float Scalar)
{
	X *= Scalar;
	Y *= Scalar;
	Z *= Scalar;
	return *this;
}

Vector3D& Vector3D::operator/=(const float Scalar)
{
	if (Scalar > 0.0001f)
	{
		return *this *= (1.f / Scalar);
	}
	return *this;
}

float Vector3D::Length() const
{
	return sqrtf(LengthSquared());
}

float Vector3D::LengthSquared() const
{
	return X * X + Y * Y + Z * Z;
}

float Vector3D::Dot(const Vector3D& Other) const
{
	return X * Other.X + Y * Other.Y + Z * Other.Z;
}

Vector3D Vector3D::Cross(const Vector3D& Other) const
{
	return Vector3D(Y * Other.Z -  Z *Other.Y, Z * Other.X - X * Other.Z, X * Other.Y - Y * Other.X);
}

Vector3D Vector3D::Normalize() const
{
	//Get safe normalized vector by default
	const float Length = this->Length();
	if (Length <= 0.0001f)
	{
		return Vector3D(0.0f, 0.0f, 0.0f);
	}
	return Vector3D(X / Length, Y / Length, Z / Length);
}

std::ostream& operator<<(std::ostream& OutFileStream, const Vector3D& Vector)
{
	return OutFileStream << Vector.X << ' ' << Vector.Y << ' ' << Vector.Z;
}

//Global-scope, non-member definitions
Vector3D operator+(const Vector3D& Lhs, const Vector3D& Rhs)
{
	return Vector3D(Lhs.X + Rhs.X, Lhs.Y + Rhs.Y, Lhs.Z + Rhs.Z);
}

Vector3D operator-(const Vector3D& Lhs, const Vector3D& Rhs)
{
	return Vector3D(Lhs.X - Rhs.X, Lhs.Y - Rhs.Y, Lhs.Z - Rhs.Z);
}

Vector3D operator*(const Vector3D& Lhs, const Vector3D& Rhs)
{
	//Not sure what multiplying two vectors means in this context, not yet
	return Vector3D(Lhs.X * Rhs.X, Lhs.Y * Rhs.Y, Lhs.Z * Rhs.Z);
}

Vector3D operator*(float Scalar, const Vector3D& Vector)
{
	return Vector3D(Vector.X * Scalar, Vector.Y * Scalar, Vector.Z * Scalar);
}

Vector3D operator*(const Vector3D& Vector, float Scalar)
{
	return Vector3D(Vector.X * Scalar, Vector.Y * Scalar, Vector.Z * Scalar);
}

Vector3D operator/(const Vector3D& Vector, float Scalar)
{
	return Vector3D(Vector.X / Scalar, Vector.Y / Scalar, Vector.Z / Scalar);
}
