#include <Windows.h>
#include "Public/Application.h"
/*
* A simple ray tracer built following the Peter Shirley's Ray Tracing In One Week Book: https://raytracing.github.io/books/RayTracingInOneWeekend.html#overview
* With changes such as:
* 1. Renders to a GUI window using the WIN32 API and D2D1(for software rendering), DX11(for potential hardware rendering)
* 2. Implemented multi-threaded rendering using a basic thread pool
* 3. Added hardware rendering using DX11 and compute shader
*/
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) 
{
	bool Result;
	Application App = Application();
	Result = App.Initialize(hInstance, nCmdShow);
	if (!Result)
	{
		return -1;
		App.Shutdown();
	}
	App.Run();
	App.Shutdown();
	return 0;
}
