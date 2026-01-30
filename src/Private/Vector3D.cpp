#include "../Public/Vector3D.h"
#include <cmath>


Vector3D::Vector3D(float InX, float InY, float InZ) : X(InX), Y(InY), Z(InZ)
{
	
}

Vector3D Vector3D::operator-() const
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

//Check if a vector is near 0 to avoid explosion later
bool Vector3D::NearZero() const
{
	return (std::abs(X) <= 1e-8 && std::abs(Y) <= 1e-8 && std::abs(Z) <= 1e-8);
}

Vector3D Vector3D::RandomVector()
{
	return Vector3D(Utility::RandomFloat(), Utility::RandomFloat(), Utility::RandomFloat());
}

Vector3D Vector3D::RandomVector(float Min, float Max)
{
	return Vector3D(Utility::RandomFloat(Min, Max), Utility::RandomFloat(Min, Max), Utility::RandomFloat(Min, Max));
}

//Generate a random unit vector within the unit sphere using rejection
Vector3D Vector3D::RandomUnitVector()
{
	while (true)
	{
		Vector3D Candidate = Vector3D::RandomVector(-1.f, 1.f);
		float LengthSq = Candidate.LengthSquared();
		//Bounded below to avoid overflow due to float imprecisions
		if (LengthSq > 1e-38 && LengthSq <= 1.f)
		{
			return Candidate / std::sqrt(LengthSq);
		}
	}
}
//Generate a random unit vector within the unit hemisphere given by a normal using rejection
Vector3D Vector3D::RandomUnitOnHemiSphere(const Vector3D& Normal)
{
	Vector3D Candidate = Vector3D::RandomUnitVector();
	if (Candidate.Dot(Normal) > 0.f)
	{
		return Candidate;
	}
	return -Candidate;
}

//Generate a random point on a unit disk. Used to implement defocus blur
Vector3D Vector3D::RandomOnUnitDisk()
{
	while (true)
	{
		Vector3D Point = Vector3D(Utility::RandomFloat(-1.f, 1.f), Utility::RandomFloat(-1.f, 1.f), 0.f);
		if (Point.LengthSquared() <= 1.f)
		{
			return Point;
		}
	}
}

//Generate a reflected vector according to an incoming vector V, and a hit point surface noraml Normal
//It's basically V + 2b, where b = V's projection onto Normal x Normal(unit). But since V is pointing towards the surface we need to negate b
Vector3D Vector3D::Reflect(const Vector3D& V, const Vector3D& Normal)
{
	return V - 2.f * V.Dot(Normal) * Normal;
}

//Generate a refracted vector according to the incoming vector InVector, surface normal, and the RelativeRI
//The relative RI is the relative refraction index, obtained by RI(In)/RI(Out)
Vector3D Vector3D::Refract(const Vector3D& InVector, const Vector3D& Normal, const float RelativeRI)
{
	float CosTheta = std::min(-InVector.Dot(Normal), 1.f);
	
	const Vector3D OutPerp = RelativeRI * (InVector + CosTheta * Normal);
	const Vector3D OutPara = -std::sqrt(std::abs(1.f - OutPerp.LengthSquared())) * Normal;
	return OutPerp + OutPara;
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
