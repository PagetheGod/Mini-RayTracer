#pragma once

#include <Windows.h>
#include <cstdint>
#include <memory>

enum RenderType : uint8_t
{
	Software,
	Hardware
};


class SoftwareRenderer;
class HardwareRenderer;

class Application
{
public:
	Application(const RenderType RendererType = RenderType::Software);
	bool Initialize(HINSTANCE InhInstance, int InpCmdShow, int Width = 1920, int Height = 1080);
	void Run();
	//This function has to be static otherwise it won't match the CreateWindowEx signature
	//It has to do with probably the fact that non-static member functions have an implicit "this" parameter, which did not exist in C(remember that Win32 is C)
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//Function used by the dialog box, it's logically similar to the normal window procedure
	static long long CALLBACK SettingsDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	long long HandleSettingsDialogMessage(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Shutdown();
	~Application();


private:
	WNDCLASS m_WindowClass;
	HWND m_WindowHandle;
	HWND m_SettingsWindowHandle;
	//For now I am going to keep both of these because we actully have two separate renderer class.
	//Maybe in the future we can unify the two renderer into one class.
	//By the way we can use std::variant. But that still occupies the memory for two pointers and it adds complexity.
	std::unique_ptr<SoftwareRenderer> m_SoftwareRenderer;
	std::unique_ptr<HardwareRenderer> m_HardwareRenderer;
	bool m_IsFirstPaint = true;
	RenderType m_RendererType;
};