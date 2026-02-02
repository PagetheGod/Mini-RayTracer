#pragma once


#include <d3d11.h>


class D3D11Class
{
public:
	D3D11Class(HWND Hwnd, int Width = 1920, int Height = 1080);
	bool InitializeD3D11();
private:
	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_DeviceContext;
	IDXGISwapChain* m_SwapChain;
	ID3D11RenderTargetView* m_RTV;
	ID3D11ShaderResourceView* m_SRV;
	ID3D11UnorderedAccessView* m_UAV;
	D3D11_VIEWPORT m_Viewport;
	HWND m_Hwnd;
	unsigned int m_Width;
	unsigned int m_Height;
	bool m_IsVsyncEnabled = false;
	unsigned long int m_VideoMemory = 0;
};