#pragma once

#include <Windows.h>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>
#include <format>

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
	Application();
	bool Initialize(HINSTANCE InhInstance, int InpCmdShow);
	void Run();
	//This function has to be static otherwise it won't match the CreateWindowEx signature
	//It has to do with probably the fact that non-static member functions have an implicit "this" parameter, which did not exist in C(remember that Win32 is C)
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//Function used by the dialog box, it's logically similar to the normal window procedure
	static long long CALLBACK SettingsDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	long long HandleSettingsDialogMessage(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//Helpers to get CPU and GPU names. Mostly aethetics
	void GetCPUName();
	void GetGPUName();
	~Application();


private:
	WNDCLASS m_WindowClass;
	HWND m_WindowHandle;
	HWND m_SettingsWindowHandle;
	//For now I am going to keep both of these because we actully have two separate renderer class.
	//Maybe we can unify the two renderer into one class.
	//By the way we can use std::variant. But that still occupies the memory for two pointers and it adds complexity.
	std::unique_ptr<SoftwareRenderer> m_SoftwareRenderer;
	std::unique_ptr<HardwareRenderer> m_HardwareRenderer;
	bool m_IsFirstPaint = true;
	RenderType m_RendererType;
	unsigned int m_Width = 0;
	unsigned int m_Height = 0;
	unsigned int m_SampleCount = 0;
	unsigned int m_MaxDepth = 0;
	//Dialog box stuffs
	wchar_t m_CPUName[128];
	wchar_t m_GPUName[128];
	std::wstring m_STDCPUName;
	std::wstring m_STDGPUName;
	//Using vectors so we can potentially add more resolutions, sample, and max depth options in the future
	//Doubt that would ever happen though
	std::vector<std::pair<int, int>> m_Resolutions;
	std::vector<unsigned int> m_SampleCounts;
	std::vector<unsigned int> m_MaxDepths;
	
};