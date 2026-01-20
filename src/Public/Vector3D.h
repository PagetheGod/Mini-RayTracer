#pragma once
#include <iostream>
#include "Commons.h"

class Vector3D
{
public:
	Vector3D(float InX = 0.0, float InY = 0.0, float InZ = 0.0);

	//Getters - for point
	float GetX() const { return X; }
	float GetY() const { return Y; }
	float GetZ() const { return Z; }
	//Getters - for color
	float R() const { return X; }
	float G() const { return Y; }
	float B() const { return Z; }




	//Class-scope operator overloads
	Vector3D operator-() const;
	Vector3D& operator+=(const Vector3D& Other);
	Vector3D& operator*=(const float Scalar);
	Vector3D& operator/=(const float Scalar);

	//Vector functions
	float Length() const;
	float LengthSquared() const;
	float Dot(const Vector3D& Other) const;
	Vector3D Cross(const Vector3D& Other) const;
	Vector3D Normalize() const;
	static Vector3D RandomVector();
	static Vector3D RandomVector(float Min, float Max);
	static Vector3D RandomUnitVector();
	static Vector3D RandomUnitOnHemiSphere(const Vector3D& Normal);

public:
	//The components do not follow the m_ convention because accessing them through component names is more natural
	//Also they are not really member private so there's no reason to use m_ anyway
	float X;
	float Y;
	float Z;
};

using Point3D = Vector3D;

//Global-scope vector operator overloads
std::ostream& operator<<(std::ostream& OutFileStream, const Vector3D& Vector);
Vector3D operator+(const Vector3D& Lhs, const Vector3D& Rhs);
Vector3D operator-(const Vector3D& Lhs, const Vector3D& Rhs);
Vector3D operator*(const Vector3D& Lhs, const Vector3D& Rhs);
Vector3D operator*(float Scalar, const Vector3D& Vector);
Vector3D operator*(const Vector3D& Vector, float Scalar);
Vector3D operator/(const Vector3D& Vector, float Scalar);