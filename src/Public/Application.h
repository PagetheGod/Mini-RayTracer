#pragma once

#include <Windows.h>


class SoftwareRenderer;

class Application
{
public:
	Application();
	bool Initialize(HINSTANCE InhInstance, int InpCmdShow, int Width = 1920, int Height = 1080);
	void Run();
	//This function has to be static otherwise it won't match the CreateWindowEx signature
	//It has to do with probably the fact that non-static member functions have an implicit "this" parameter, which did not exist in C(remember that Win32 is C)
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Shutdown();
	~Application() = default;


private:
	WNDCLASS m_WindowClass;
	HWND m_WindowHandle;
	SoftwareRenderer* m_Renderer;
	bool m_IsFirstPaint = true;
};