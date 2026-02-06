#pragma once

#include "D3D11Class.h"
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Sphere.h"
#include "Camera.h"
#include <strsafe.h>

class HardwareRenderer
{
public:
	HardwareRenderer(int Width, int Height, float AspectRatio);
	~HardwareRenderer();
	bool Intialize(HWND hWnd);
	void GetShaderBuffers();

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
	Camera m_Camera;
	std::unique_ptr<D3D11Class> m_D3D11;
	std::unique_ptr<HittableList> m_World;
	HWND m_hWnd;
	SphereTransformBufferType* m_CSTransformBuffer;
	SphereMaterialBufferType* m_CSMaterialBuffer;
};