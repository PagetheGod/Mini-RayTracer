#include "../Public/Application.h"
#include "../Public/SoftwareRenderer.h"
#include "../Public/HardwareRenderer.h"
#include "../../resource.h"
#include <string.h>
#include <array>

#ifdef _MSC_VER
//MSVC instrinsics
#include <intrin.h>
#else
//GCC or Clang
#include <cpuid.h>
#endif




Application::Application() : m_WindowHandle(NULL), m_WindowClass(NULL), m_SettingsWindowHandle(NULL), m_StartButtonHandle(NULL), m_RenderTimeLabel(NULL), m_SoftwareRenderer(nullptr), m_HardwareRenderer(nullptr)
{
	//Initialize the vectors so we can easily retreive the values using the indices we get back from the combo boxes
	m_Resolutions = { {640, 480}, {800, 600}, {1024, 768}, {1280, 720}, {1920, 1080}, {2560, 1440} };
	m_SampleCounts = { 3, 5, 10, 15, 20, 50, 75, 100, 150, 200, 500 };
	m_MaxDepths = { 5, 10, 15, 25, 50 };

}

bool Application::Initialize(HINSTANCE InhInstance, int InpCmdShow)
{
	long long Result = DialogBoxParamA(InhInstance, MAKEINTRESOURCEA(IDD_DIALOG2), NULL, &Application::SettingsDialogProc, (LPARAM)this);
	if (Result == FALSE)
	{
		//The user chose to quit in the dialog box, exit the application
		return false;
	}	


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


	float AspectRatio = 16.f / 9.f;
	m_Height = (unsigned int)(m_Width / AspectRatio);
	m_Height >= 1 ? m_Height : m_Height = 1;


	//Center the window. Note that we are working with 0 based coordinates that start at the top left of the screen, not NDC
	int PosX = (GetSystemMetrics(SM_CXSCREEN) - m_Width) / 2;
	int PosY = (GetSystemMetrics(SM_CYSCREEN) - m_Height) / 2;

	//Adjust the actual window client size to our assumed pixel counts - we do this because the actual client size is assumed size - paddings + borders and stuffs
	RECT RC = RECT(0, 0, m_Width, m_Height);
	AdjustWindowRectEx(&RC, WS_OVERLAPPEDWINDOW, false, 0);

	int ActualWidth = RC.right - RC.left;
	int ActualHeight = RC.bottom - RC.top;

	//CreateWindowEx returns 0/NULL on failure
	m_WindowHandle = CreateWindowEx(
		0,
		WindowClassName,
		L"Ray Tracing in One Weekend",
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
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

	if (m_RendererType == RenderType::Software)
	{
		m_SoftwareRenderer = std::make_unique<SoftwareRenderer>(m_Width, m_Height, AspectRatio);
		bool Result = m_SoftwareRenderer->Initialize(m_WindowHandle, m_SampleCount, m_MaxDepth);
		if (!Result)
		{
			return false;
		}
	}
	else
	{
		m_HardwareRenderer = std::make_unique<HardwareRenderer>(m_Width, m_Height, AspectRatio);
		bool Result = m_HardwareRenderer->Intialize(m_WindowHandle, m_SampleCount, m_MaxDepth);
		if (!Result)
		{
			return false;
		}
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
		//Create the start render button
		AppPtr->m_StartButtonHandle = CreateWindowEx(0,
			L"BUTTON",
			L"Start Render",
		    WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			10, 10,
			100, 35,
			hWnd,
			NULL,
			WindowInitStruct->hInstance,
			NULL);
		//Create the render time label
		//We are using a static text to display the GPU render time because message box actually cause hang
		//For reasons unknown to me :(
		AppPtr->m_RenderTimeLabel = CreateWindowEx(0,
			L"STATIC",
			L"Starting Render...",
			WS_VISIBLE | WS_CHILD | SS_LEFT | SS_SUNKEN,
			120, 15,
			350, 20,
			hWnd,
			NULL,
			WindowInitStruct->hInstance,
			NULL);
		SendMessage(AppPtr->m_RenderTimeLabel, WM_SETFONT,
			(WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
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
			return 0;
		}
		case WM_COMMAND:
		{
			if ((HWND)lParam == m_StartButtonHandle && (HIWORD(wParam) == BN_CLICKED))
			{
				//Destory the start render button so we don't click it multiple times
				DestroyWindow(m_StartButtonHandle);
				ShowWindow(m_StartButtonHandle, SW_HIDE);
				m_StartButtonHandle = NULL;
				if (m_RendererType == RenderType::Software)
				{
					//Clear the window before we start the rendering so the start button goes away
					m_SoftwareRenderer->ClearWindow();
					m_SoftwareRenderer->RenderFrameBuffer();
				}
				else
				{
					m_HardwareRenderer->ClearBackground();
					m_HardwareRenderer->RenderScene();
					//Set the static text that will display the render time
					SetWindowTextW(m_RenderTimeLabel, m_HardwareRenderer->GetRenderTimeString().c_str());
				}
				return 0;
			}
		}
		case WM_PAINT:
		{
			if (m_IsFirstPaint)
			{
				if (m_RendererType == RenderType::Software)
				{
					m_SoftwareRenderer->ClearWindow();
				}
				else
				{
					m_HardwareRenderer->ClearBackground();
				}
				m_IsFirstPaint = false;
				UpdateWindow(m_StartButtonHandle);
				UpdateWindow(m_RenderTimeLabel);
			}
			else
			{
				if (m_RendererType == RenderType::Software)
				{
					m_SoftwareRenderer->RenderToWindow();
				}
				
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

//Callback function handling the startup dialog box which allows user to set the ray tracer's settings
long long Application::SettingsDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//Same deal, it's a C API and essentially a static function, so we have to get the pointer to the application instance from the window data
	Application* AppPtr = nullptr;

	if (uMsg == WM_INITDIALOG)
	{
		AppPtr = reinterpret_cast<Application*>(lParam);
		SetLastError(0);
		long long Result = SetWindowLongPtr(hDlg, DWLP_USER, (long long)AppPtr);
		if (!Result && GetLastError() != 0)
		{
			unsigned long int ErrorCode = GetLastError();
			wchar_t ErrorMessage[128];
			wsprintf(ErrorMessage, L"Failed to set settings dialog box instance app pointer! Error code: %lu", ErrorCode);
			MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
			return FALSE;
		}
		HWND ComboListHandle;
		//The dialog box is created, initialize the drop down list contents
		ComboListHandle = GetDlgItem(hDlg, IDC_COMBO_RESOLUTION);

		//Trying out std::format for the drop down list items. This replaced StringCbPrintfExW().
		//However, do note that this comes with the cost of extra allocations. In the scope of this project it's not an issue
		//But it's a tradeoff we should not just ignore
		std::wstring ListContents;
		ListContents.reserve(256);

		for (const auto& [Width, Height] : AppPtr->m_Resolutions)
		{
			if (Width >= 1920)
			{
				ListContents = std::format(L"{}x{} (compatibility issues with laptops)", Width, Height);
			}
			else
			{
				ListContents = std::format(L"{}x{}", Width, Height);
			}
			SendMessageW(ComboListHandle, CB_ADDSTRING, 0, (LPARAM)ListContents.data());
		}
		SendMessageW(ComboListHandle, CB_SETCURSEL, 3, 0); //Set default selection to 1920x1080
		//Initialize render type combo list
		ComboListHandle = GetDlgItem(hDlg, IDC_RENDER_TYPE);
		ListContents.clear();

		AppPtr->GetCPUName();
		ListContents = std::format(L"Software ({})", AppPtr->m_CPUName);
		SendMessageW(ComboListHandle, CB_ADDSTRING, 0, (LPARAM)ListContents.data());

		ListContents.clear();
		AppPtr->GetGPUName();
		ListContents = std::format(L"Hardware ({})", AppPtr->m_GPUName);
		SendMessageW(ComboListHandle, CB_ADDSTRING, 0, (LPARAM)ListContents.data());

		SendMessageW(ComboListHandle, CB_SETCURSEL, 1, 0); //Set default selection to hardware rendering
		//Initialize sample count combo list
		ComboListHandle = GetDlgItem(hDlg, IDC_COMBO_SAMPLE);
		ListContents.clear();
		for (const unsigned int SampleCount : AppPtr->m_SampleCounts)
		{
			if (SampleCount >= 150)
			{
				ListContents = std::format(L"{} (increases render time)", SampleCount);
			}
			else
			{
				ListContents = std::format(L"{}", SampleCount);
			}
			SendMessageW(ComboListHandle, CB_ADDSTRING, 0, (LPARAM)ListContents.data());
		}
		SendMessageW(ComboListHandle, CB_SETCURSEL, 7, 0); //Set default selection to 100 samples per pixel
		//Initialize max depth combo list
		ComboListHandle = GetDlgItem(hDlg, IDC_COMBO_DEPTH);
		ListContents.clear();
		for (const unsigned int MaxDepth : AppPtr->m_MaxDepths)
		{
			if (MaxDepth >= 25)
			{
				ListContents = std::format(L"{} (increases render time)", MaxDepth);
			}
			else
			{
				ListContents = std::format(L"{}", MaxDepth);
			}
			SendMessageW(ComboListHandle, CB_ADDSTRING, 0, (LPARAM)ListContents.data());
		}
		SendMessageW(ComboListHandle, CB_SETCURSEL, 2, 0);//Set default selection to 15 max depth	
		return TRUE;

	}
	else
	{
		AppPtr = reinterpret_cast<Application*>(GetWindowLongPtr(hDlg, DWLP_USER));
	}

	if (AppPtr)
	{
		return AppPtr->HandleSettingsDialogMessage(hDlg, uMsg, wParam, lParam);
	}
	return FALSE;
}

long long Application::HandleSettingsDialogMessage(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					//User cliked ok, retreive the settings and store them in the application instance
					size_t ResolutionIndex = SendMessageW(GetDlgItem(hDlg, IDC_COMBO_RESOLUTION), CB_GETCURSEL, 0, 0);
					size_t RenderTypeIndex = SendMessageW(GetDlgItem(hDlg, IDC_RENDER_TYPE), CB_GETCURSEL, 0, 0);
					size_t SampleCountIndex = SendMessageW(GetDlgItem(hDlg, IDC_COMBO_SAMPLE), CB_GETCURSEL, 0, 0);
					size_t MaxDepthIndex = SendMessageW(GetDlgItem(hDlg, IDC_COMBO_DEPTH), CB_GETCURSEL, 0, 0);
					
					m_Width = m_Resolutions[ResolutionIndex].first;
					m_Height = m_Resolutions[ResolutionIndex].second;
					m_RendererType = RenderType(RenderTypeIndex);
					m_SampleCount = m_SampleCounts[SampleCountIndex];
					m_MaxDepth = m_MaxDepths[MaxDepthIndex];
					EndDialog(hDlg, TRUE);
					return TRUE;
				}
				case IDCANCEL:
				{
					if (MessageBox(hDlg, L"Are you sure you want to quit?", L"Mini Ray Tracer", MB_OKCANCEL) == IDOK)
					{
						EndDialog(hDlg, FALSE);
					}
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

void Application::GetCPUName()
{
	//According to the docs, calling __cpuid with 0x80000000 as the function_id argument gets the number of highest extend ID
	std::array<int, 4> CPUIDInfo;
#ifdef _MSC_VER
	//MSVC
	
	__cpuid(CPUIDInfo.data(), 0x80000000);
#else
	//GCC or Clang
	unsigned int EAX, EBX, ECX, EDX;
	int Result = __get_cpuid(0x80000000, &EAX, &EBX, &ECX, &EDX);
	if (!Result)
	{
		//GCC and Clang return 0 if the CPU does not support the leaf
		m_CPUName = L"Unknown CPU";
		return;
	}
#endif
	int NumExtendedIDs = CPUIDInfo[0];
	std::vector<std::array<int, 4>> ExtendedCPUIDInfo;
	for (int i = 0x80000000; i <= NumExtendedIDs; i++)
	{
	#ifdef _MSC_VER
		__cpuidex(CPUIDInfo.data(), i, 0);
	#else
		__get_cpuid_count(i, 0, &EAX, &EBX, &ECX, &EDX);
		CPUIDInfo[0] = static_cast<int>(EAX);
		CPUIDInfo[1] = static_cast<int>(EBX);
		CPUIDInfo[2] = static_cast<int>(ECX);
		CPUIDInfo[3] = static_cast<int>(EDX);
	#endif
		ExtendedCPUIDInfo.push_back(CPUIDInfo);
	}
	char CPUBrandString[64];
	memset(CPUBrandString, 0, sizeof(CPUBrandString));
	//If numexid is avaliable upto 0x80000004
	if (NumExtendedIDs >= 0x80000004)
	{
		memcpy(CPUBrandString, &ExtendedCPUIDInfo[2][0], sizeof(CPUIDInfo));
		memcpy(CPUBrandString + 16, &ExtendedCPUIDInfo[3][0], sizeof(CPUIDInfo));
		memcpy(CPUBrandString + 32, &ExtendedCPUIDInfo[4][0], 16);
	}
	else
	{
	#ifdef _MSC_VER
		strcpy_s(CPUBrandString, "Unknown CPU");
	#else
		strcpy(CPUBrandString, "Unknown CPU");
	#endif
	}
	//Store the CPU name in the m_CPUName member variable, which is used for display in the settings dialog box
	//Do remember to trim the extra white spaces that come with the CPU brand string, otherwise the display will look weird
	size_t i = strlen(CPUBrandString) - 1;
	while (CPUBrandString[i] == ' ' && i >= 0)
	{
		CPUBrandString[i] = '\0';
		i--;
	}

	
	wchar_t CPUBrandStringW[64];
#ifdef _MSC_VER
	size_t ConvertedChars = 0;
	mbstowcs_s(&ConvertedChars, CPUBrandStringW, CPUBrandString, i + 1);
#else
	std::mbstowcs(CPUBrandStringW, CPUBrandString, i + 1);
#endif
	m_CPUName = CPUBrandStringW;

}

void Application::GetGPUName()
{
	HRESULT Result;

	IDXGIFactory1* DXGIFactory = nullptr;
	IDXGIAdapter* DXGIAdapter = nullptr;

	Result = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)(&DXGIFactory));
	if (FAILED(Result))
	{
		m_GPUName = L"Unknown GPU";
		return;
	}

	Result = DXGIFactory->EnumAdapters(0, &DXGIAdapter);
	if (FAILED(Result))
	{
		DXGIFactory->Release();
		m_GPUName = L"Unknown GPU";
		return;
	}

	DXGI_ADAPTER_DESC AdapterDesc;
	Result = DXGIAdapter->GetDesc(&AdapterDesc);
	if (FAILED(Result))
	{
		DXGIFactory->Release();
		DXGIAdapter->Release();
		m_GPUName = L"Unknown GPU";
		return;
	}

	//Get the actual GPU name and memory. They are wide chars which means we have to have a wchar_t array
	unsigned long long VideoMemGB = AdapterDesc.DedicatedVideoMemory / (1024 * 1024 * 1024);
	m_GPUName = std::format(L"{} - {} GB", AdapterDesc.Description, VideoMemGB);
	DXGIFactory->Release();
	DXGIAdapter->Release();
}


Application::~Application()
{
	m_WindowHandle = NULL;
}
