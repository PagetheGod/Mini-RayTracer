#include "Public/D3D11Class.h"
#include <vector>

D3D11Class::D3D11Class(HWND Hwnd, unsigned int Width, unsigned int Height) : m_Device(nullptr), m_DeviceContext(nullptr), m_SwapChain(nullptr), m_BackBuffer(nullptr)
, m_RasterState(nullptr), m_RTV(nullptr), m_Hwnd(Hwnd), m_Width(Width), m_Height(Height)
{
	

}

bool D3D11Class::InitializeD3D11()
{
	HRESULT Result;

	IDXGIFactory1* DXGIFactory = nullptr;
	IDXGIAdapter* DXGIAdapter = nullptr;
	IDXGIOutput* DXGIAdapterOutput = nullptr;


	Result = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)(&DXGIFactory));
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create D3D11 Factory! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	Result = DXGIFactory->EnumAdapters(0, &DXGIAdapter);
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create D3D11 Adapter! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}
	Result = DXGIAdapter->EnumOutputs(0, &DXGIAdapterOutput);
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create D3D11 Output! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	unsigned int NumModes = 0;
	Result = DXGIAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &NumModes, NULL);
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to get D3D11 display mode number! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	std::vector<DXGI_MODE_DESC> DisplayModes(NumModes);

	Result = DXGIAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &NumModes, DisplayModes.data());
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to get D3D11 display mode list! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}
	unsigned int RefreshRateNum = 0;
	unsigned int RefreshRateDe = 0;

	for (const DXGI_MODE_DESC& DisplayMode : DisplayModes)
	{
		if (DisplayMode.Width == (unsigned int)m_Width)
		{
			if (DisplayMode.Height == (unsigned int)m_Height)
			{
				RefreshRateNum = DisplayMode.RefreshRate.Numerator;
				RefreshRateDe = DisplayMode.RefreshRate.Denominator;
			}
		}
	}

	DXGI_ADAPTER_DESC AdapterDesc;
	Result = DXGIAdapter->GetDesc(&AdapterDesc);
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to get D3D11 adapter description! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	//Get the GPU memory in MBs
	m_VideoMemory = AdapterDesc.DedicatedVideoMemory;

	DXGIFactory->Release();
	DXGIAdapter->Release();
	DXGIAdapterOutput->Release();

	DXGIFactory = nullptr;
	DXGIAdapter = nullptr;
	DXGIAdapterOutput = nullptr;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	ZeroMemory(&SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.BufferDesc.Width = m_Width;
	SwapChainDesc.BufferDesc.Height = m_Height;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	if (m_IsVsyncEnabled)
	{
		SwapChainDesc.BufferDesc.RefreshRate.Numerator = RefreshRateNum;
		SwapChainDesc.BufferDesc.RefreshRate.Denominator = RefreshRateDe;
	}
	else
	{
		SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = m_Hwnd;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.Flags = 0;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	


	SwapChainDesc.Windowed = true;

	D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_1;

	

	Result = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		&FeatureLevel,
		1,
		D3D11_SDK_VERSION,
		&SwapChainDesc,
		&m_SwapChain,
		&m_Device,
		nullptr,
		&m_DeviceContext
	);
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create device and swap chain! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	//Create the back buffer for swap chain
	Result = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(&m_BackBuffer));
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create get back buffer! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	//Create the RTV for the swap chain back buffer
	Result = m_Device->CreateRenderTargetView(m_BackBuffer, nullptr, &m_RTV);
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create RTV for back buffer! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	m_DeviceContext->OMSetRenderTargets(1, &m_RTV, nullptr);

	D3D11_RASTERIZER_DESC RasterDesc{};
	RasterDesc.CullMode = D3D11_CULL_BACK;
	RasterDesc.FillMode = D3D11_FILL_SOLID;

	Result = m_Device->CreateRasterizerState(&RasterDesc, &m_RasterState);
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create raster state! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	m_DeviceContext->RSSetState(m_RasterState);

	m_Viewport.Width = (float)m_Width;
	m_Viewport.Height = (float)m_Height;
	m_Viewport.TopLeftX = 0.f;
	m_Viewport.TopLeftY = 0.f;
	m_Viewport.MinDepth = 0.f;
	m_Viewport.MaxDepth = 1.f;

	m_DeviceContext->RSSetViewports(1, &m_Viewport);

	return true;
}

void D3D11Class::ClearBackground()
{
	float Color[4];
	Color[0] = 0.f;
	Color[1] = 0.f; 
	Color[2] = 0.f; 
	Color[3] = 1.f;
	m_DeviceContext->ClearRenderTargetView(m_RTV, Color);
	PresentScene();
}

void D3D11Class::PresentScene()
{
	if (m_IsVsyncEnabled)
	{
		m_SwapChain->Present(1, 0);
	}
	else
	{
		m_SwapChain->Present(0, 0);
	}
}

D3D11Class::~D3D11Class()
{
	if (m_Device)
	{
		m_Device->Release();
		m_Device = nullptr;
	}
	if (m_DeviceContext)
	{
		m_DeviceContext->Release();
		m_DeviceContext = nullptr;
	}
	if (m_SwapChain)
	{
		m_SwapChain->Release();
		m_SwapChain = nullptr;
	}
	if(m_BackBuffer)
	{
		m_BackBuffer->Release();
		m_BackBuffer = nullptr;
	}
	if (m_RasterState)
	{
		m_RasterState->Release();
		m_RasterState = nullptr;
	}
	if (m_RTV)
	{
		m_RTV->Release();
		m_RTV = nullptr;
	}
}
