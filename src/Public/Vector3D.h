#pragma once



using Point3D = Vector3D;

class Vector3D
{
public:
	Vector3D(float InX = 0.0, float InY = 0.0, float InZ = 0.0);

	//Getters
	float GetX() const { return X; }
	float GetY() const { return Y; }
	float GetZ() const { return Z; }

	//Class-scope operator overloads
	Vector3D operator-();
	Vector3D& operator+=(const Vector3D& Other);
	Vector3D& operator*=(const float Scalar);
	Vector3D& operator/=(const float Scalar);

	//Vector functions
	float Length() const;
	float LengthSquared() const;
	float Dot(const Vector3D& Other) const;
	Vector3D Cross(const Vector3D& Other) const;
	Vector3D Normalize() const;
	
public:
	//The components do not follow the m_ convention because accessing them through component names is more natural
	float X;
	float Y;
	float Z;
};

//Global-scope vector operator overloads
Vector3D operator+(const Vector3D& Lhs, const Vector3D& Rhs);
Vector3D operator-(const Vector3D& Lhs, const Vector3D& Rhs);
Vector3D operator*(const Vector3D& Lhs, const Vector3D& Rhs);
Vector3D operator*(float Scalar, const Vector3D& Vector);
Vector3D operator*(const Vector3D& Vector, float Scalar);