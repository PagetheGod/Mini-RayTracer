#pragma once

#include <string>
#include <fstream>
#include <Windows.h>
#include "Ray.h"
#include "Color.h"

#define WIN32_LEAN_AND_MEAN
class SoftwareRenderer
{
public:
	SoftwareRenderer(int Width = 1920);
	bool Initialize(const char* OutFileName);
	void RenderPPM();


	~SoftwareRenderer() = default;

private:
	int m_Width;
	int m_Height;
	float m_AspectRatio;
	float m_ViewportWidth;
	float m_ViewportHeight;
	Vector3D m_ViewportU;
	Vector3D m_ViewportV;
	Vector3D m_DeltaU;
	Vector3D m_DeltaV;
	float m_FocalLength;
	Point3D m_CameraCenter;
	std::ofstream m_OutFileStream;
};