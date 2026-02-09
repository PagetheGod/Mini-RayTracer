#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <directxmath.h>

using namespace DirectX;

class D3D11Class
{
public:
	D3D11Class(HWND Hwnd, int Width = 1920, int Height = 1080);
	~D3D11Class();
	bool InitializeD3D11();
	void ClearBackground();
	void PresentScene();

	ID3D11Device* GetDevice() const { return m_Device; }
	ID3D11DeviceContext* GetDeviceContext() const { return m_DeviceContext; }
	//This is not const because we will use CopyResources to copy the compute shader output to the back buffer
	ID3D11Texture2D* GetBackBuffer() { return m_BackBuffer; }
private:
	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_DeviceContext;
	IDXGISwapChain* m_SwapChain;
	ID3D11Texture2D* m_BackBuffer;
	ID3D11RasterizerState* m_RasterState;
	ID3D11RenderTargetView* m_RTV;
	D3D11_VIEWPORT m_Viewport;
	HWND m_Hwnd;
	unsigned int m_Width;
	unsigned int m_Height;
	bool m_IsVsyncEnabled = false;
	unsigned long int m_VideoMemory = 0;
};