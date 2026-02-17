#pragma once

#include "D3D11Class.h"
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Sphere.h"
#include "Camera.h"
#include <strsafe.h>


class ComputeShaderManager;


class HardwareRenderer
{
public:
	HardwareRenderer(unsigned int Width, unsigned int Height, float AspectRatio);
	~HardwareRenderer();
	bool Intialize(HWND hWnd, unsigned int SampleCount, unsigned int MaxDepth);
	void GetShaderBuffers();
	bool RenderScene();
	void FillBackBuffer();
	void ClearBackground();

private:
	void CreateWorld();

private:
	unsigned int m_Width;
	unsigned int m_Height;
	float m_AspectRatio;
	float m_ViewportWidth = 0.f;
	float m_ViewportHeight = 0.f;
	Vector3D m_ViewportU;
	Vector3D m_ViewportV;
	Vector3D m_DeltaU;
	Vector3D m_DeltaV;
	Camera m_Camera;
	std::unique_ptr<D3D11Class> m_D3D11;
	std::unique_ptr<HittableList> m_World;
	std::unique_ptr<ComputeShaderManager> m_ComputeShaderManager;
	HWND m_hWnd;
	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_DeviceContext;
	SphereTransformBufferType* m_CSTransformBuffer;
	SphereMaterialBufferType* m_CSMaterialBuffer;
};