#include "../Public/D2D1Class.h"

D2D1Class::D2D1Class() : m_Factory(nullptr), m_RenderTarget(nullptr), m_Bitmap(nullptr), m_hWnd(NULL)
{

}

bool D2D1Class::InitFactory()
{
	HRESULT HResult;

	HResult = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_Factory);
	if (FAILED(HResult))
	{
		unsigned long int ErrorCode = GetLastError();
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create D2D1 Factory! Error code: %lu", ErrorCode);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}
	return true;
}

HRESULT D2D1Class::CreateGraphicResources(HWND hWnd)
{
	HRESULT Result = S_OK;
	m_hWnd = hWnd;
	if (!m_RenderTarget)
	{
		RECT RC = RECT(0, 0, 0, 0);
		GetClientRect(hWnd, &RC);

		m_Width = RC.right;
		m_Height = RC.bottom;

		D2D1_SIZE_U Size = D2D1::SizeU(RC.right, RC.bottom);

		D2D1_RENDER_TARGET_PROPERTIES RTProps = D2D1_RENDER_TARGET_PROPERTIES();
		RTProps.type = D2D1_RENDER_TARGET_TYPE_SOFTWARE;
		RTProps.pixelFormat = D2D1_PIXEL_FORMAT(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);

		D2D1_HWND_RENDER_TARGET_PROPERTIES HwndRTProps;
		HwndRTProps.hwnd = hWnd;
		HwndRTProps.pixelSize = Size;
		HwndRTProps.presentOptions = D2D1_PRESENT_OPTIONS_NONE;

		Result = m_Factory->CreateHwndRenderTarget(RTProps, HwndRTProps, &m_RenderTarget);

		if (SUCCEEDED(Result))
		{
			D2D1_BITMAP_PROPERTIES BitmapProps = D2D1_BITMAP_PROPERTIES();
			BitmapProps.pixelFormat = D2D1_PIXEL_FORMAT(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
			Result = m_RenderTarget->CreateBitmap(Size, BitmapProps, &m_Bitmap);
			if (FAILED(Result))
			{
				wchar_t ErrorMessage[128];
				wsprintf(ErrorMessage, L"Failed to create D2D1 Bitmap! HRESULT error code: %d", Result);
				MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
			}
		}
		else
		{
			wchar_t ErrorMessage[128];
			wsprintf(ErrorMessage, L"Failed to create D2D1 render target! HRESULT error code: %d", Result);
			MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		}
	}

	return Result;
}

void D2D1Class::ClearBackground()
{
	HRESULT Result = CreateGraphicResources(m_hWnd);
	if (SUCCEEDED(Result))
	{
		PAINTSTRUCT PS;
		BeginPaint(m_hWnd, &PS);

		m_RenderTarget->BeginDraw();

		m_RenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

		Result = m_RenderTarget->EndDraw();
		if (FAILED(Result) || Result == D2DERR_RECREATE_TARGET)
		{
			DestroyGraphicResources();
			unsigned long int ErrorCode = GetLastError();
			wchar_t ErrorMessage[128];
			wsprintf(ErrorMessage, L"Failed to execute D2D1 draw call! Error code: %lu", ErrorCode);
			MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		}
		EndPaint(m_hWnd, &PS);
	}
}

void D2D1Class::RenderBitmap(void* FrameBuffer)
{
	HRESULT Result = CreateGraphicResources(m_hWnd);
	if (SUCCEEDED(Result))
	{
		RECT UpdateRegion{ 0, 0, 0, 0 };

		size_t ScanlineOffset = sizeof(unsigned char) * 4 * m_Width;

		PAINTSTRUCT PS;
		BeginPaint(m_hWnd, &PS);
		UpdateRegion = PS.rcPaint;
		
		m_RenderTarget->BeginDraw();

		//m_RenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

		D2D1_RECT_F D2D1RectF = D2D1::RectF((float)UpdateRegion.left, (float)UpdateRegion.top, (float)UpdateRegion.right, (float)UpdateRegion.bottom);
		D2D1_RECT_U D2D1RectU = D2D1::RectU(UpdateRegion.left, UpdateRegion.top, UpdateRegion.right, UpdateRegion.bottom);
		unsigned char* Start = static_cast<unsigned char*>(FrameBuffer) + UpdateRegion.top * ScanlineOffset;
		Result = m_Bitmap->CopyFromMemory(&D2D1RectU, Start, ScanlineOffset);
		if (FAILED(Result))
		{
			unsigned long int ErrorCode = GetLastError();
			wchar_t ErrorMessage[128];
			wsprintf(ErrorMessage, L"Failed to copy from frame buffer! Error code: %lu", ErrorCode);
			MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		}
		else
		{
			m_RenderTarget->DrawBitmap(m_Bitmap, &D2D1RectF, 1.f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2D1RectF);
		}
		Result = m_RenderTarget->EndDraw();
		if (FAILED(Result) || Result == D2DERR_RECREATE_TARGET)
		{
			DestroyGraphicResources();
			unsigned long int ErrorCode = GetLastError();
			wchar_t ErrorMessage[128];
			wsprintf(ErrorMessage, L"Failed to execute D2D1 draw call! Error code: %lu", ErrorCode);
			MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		}
		EndPaint(m_hWnd, &PS);
	}
}

void D2D1Class::DestroyGraphicResources()
{

	if (m_RenderTarget)
	{
		m_RenderTarget->Release();
		m_RenderTarget = nullptr;
	}
	if (m_Bitmap)
	{
		m_Bitmap->Release();
		m_Bitmap = nullptr;
	}
}

void D2D1Class::Shutdown()
{
	DestroyGraphicResources();
	if (m_Factory)
	{
		m_Factory->Release();
		m_Factory = nullptr;
	}
}


