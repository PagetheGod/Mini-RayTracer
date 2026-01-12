#include "../Public/Application.h"
#include <string.h>
#include "../Public/SoftwareRenderer.h"


Application::Application() : m_WindowHandle(NULL), m_WindowClass(NULL)
{

}

bool Application::Initialize(HINSTANCE InhInstance, int InpCmdShow, int Width, int Height)
{
	const wchar_t WindowClassName[] = L"Ray Tracer Window Class";

	WNDCLASS WindowClass{};

	WindowClass.hInstance = InhInstance;
	WindowClass.lpszClassName = WindowClassName;
	WindowClass.lpfnWndProc = &Application::WindowProc;

	if (!RegisterClass(&WindowClass))
	{
		unsigned long int ErrorCode = GetLastError();
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to register window class! Error code: %lu", ErrorCode);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	//Center the window. Note that we are working with 0 based coordinates that start at the top left of the screen, not NDC
	int PosX = (GetSystemMetrics(SM_CXSCREEN) - Width) / 2;
	int PosY = (GetSystemMetrics(SM_CYSCREEN) - Height) / 2;

	//Adjust the actual window client size to our assumed pixel counts - we do this because the actual client size is assumed size - paddings + borders and stuffs
	RECT RC = RECT(0, 0, Width, Height);
	AdjustWindowRectEx(&RC, WS_OVERLAPPEDWINDOW, false, 0);

	int ActualWidth = RC.right - RC.left;
	int ActualHeight = RC.bottom - RC.top;

	//CreateWindowEx returns 0/NULL on failure
	m_WindowHandle = CreateWindowEx(
		0,
		WindowClassName,
		L"Ray Tracing in One Weekend",
		WS_OVERLAPPEDWINDOW,
		PosX,
		PosY,
		ActualWidth, ActualHeight,
		NULL,
		NULL,
		InhInstance,
		this
	);
	
	if (!m_WindowHandle)
	{
		unsigned long int ErrorCode = GetLastError();
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create window! Error code: %lu", ErrorCode);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	//For software rendering, especially for a ppm output, we only need to pass the width
	m_Renderer = new SoftwareRenderer(Width);
	bool Result = m_Renderer->Initialize("output.ppm", m_WindowHandle);
	if (!Result)
	{
		return false;
	}

	ShowWindow(m_WindowHandle, InpCmdShow);

	return true;
}

void Application::Run()
{
	MSG Msg{};
	while (GetMessage(&Msg, m_WindowHandle, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessageW(&Msg);
	}
}

LRESULT CALLBACK Application::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Application* AppPtr = nullptr;
	if (uMsg == WM_CREATE)
	{
		//We just created the window, set the pointer to the application in the instance pointer
		CREATESTRUCT* WindowInitStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
		AppPtr = reinterpret_cast<Application*>(WindowInitStruct->lpCreateParams);
		SetLastError(0);
		long long Result = SetWindowLongPtr(hWnd, GWLP_USERDATA, (long long)AppPtr);
		if (!Result && GetLastError() != 0)
		{
			unsigned long int ErrorCode = GetLastError();
			wchar_t ErrorMessage[128];
			wsprintf(ErrorMessage, L"Failed to set window instance app pointer! Error code: %lu", ErrorCode);
			MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
			return -1;
		}
	}
	else
	{
		AppPtr = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	if (AppPtr)
	{
		return AppPtr->HandleMessage(hWnd, uMsg, wParam, lParam);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT Application::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CLOSE:
		{
			DestroyWindow(hWnd);
			return 0;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				DestroyWindow(hWnd);
			}
			else if (wParam == VK_RETURN)
			{
				m_Renderer->RenderFrameBuffer();
			}
			return 0;
		}
		case WM_PAINT:
		{
			if (m_IsFirstPaint)
			{
				m_Renderer->ClearWindow();
				m_IsFirstPaint = false;
			}
			else
			{
				m_Renderer->RenderToWindow();
			}
			return 0;
		}
		default:
		{
			break;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Application::Shutdown()
{
	m_WindowHandle = NULL;
	m_Renderer->Shutdown();
	delete m_Renderer;
}
