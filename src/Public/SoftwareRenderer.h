#pragma once


#include <string>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Sphere.h"
#include "Camera.h"
#include "ThreadPool.h"
#include <atomic>
#include <strsafe.h>

class D2D1Class;

class SoftwareRenderer
{
public:
	SoftwareRenderer(unsigned int Width, unsigned int Height, float AspectRatio);
	bool Initialize(HWND hWnd, unsigned int SampleCount, unsigned int MaxDepth);
	void ClearWindow();
	void RenderFrameBuffer();
	void RenderToWindow();

	~SoftwareRenderer();

private:
	void CreateWorld();
private:
	unsigned int m_Width;
	unsigned int m_Height;
	float m_AspectRatio;
	float m_ViewportWidth;
	float m_ViewportHeight;
	Vector3D m_ViewportU;
	Vector3D m_ViewportV;
	Vector3D m_DeltaU;
	Vector3D m_DeltaV;
	Camera m_Camera;
	std::unique_ptr<HittableList> m_World;
	HWND m_hWnd;
	unsigned char* m_FrameBuffer;
	D2D1Class* m_D2D1;
	std::unique_ptr <VThreadPool> m_ThreadPool;
	
};