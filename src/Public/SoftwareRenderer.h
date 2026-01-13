#pragma once


#include <string>
#include <fstream>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Ray.h"
#include "Color.h"
#include <strsafe.h>

class D2D1Class;


class SoftwareRenderer
{
public:
	SoftwareRenderer(int Width, int Height, float AspectRatio);
	bool Initialize(const char* OutFileName, HWND hWnd);
	void ClearWindow();
	void RenderPPM();
	void RenderFrameBuffer();
	void RenderToWindow();
	void Shutdown();

	~SoftwareRenderer() = default;

private:
	float TestHitSphere(const Point3D& Center, float Radius, const Ray& R);

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
	HWND m_hWnd;
	unsigned char* m_FrameBuffer;
	D2D1Class* m_D2D1;
};