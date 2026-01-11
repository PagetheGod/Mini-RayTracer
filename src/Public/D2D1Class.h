#pragma once

#include "Windows.h"
#include <d2d1.h>

#define WIN32_LEAN_AND_MEAN

class D2D1Class
{
public:
	D2D1Class();
	bool InitFactory();
	HRESULT CreateGraphicResources(HWND hWnd);
	HRESULT RenderBitmap(void* FrameBuffer);

	void Shutdown();
	~D2D1Class() = default;


private:
	void DestroyGraphicResources();
private:
	ID2D1Factory* m_Factory;
	ID2D1HwndRenderTarget* m_RenderTarget;
	ID2D1Bitmap* m_Bitmap;
	HWND m_hWnd;
	size_t m_Width = 0;
	size_t m_Height = 0;
};