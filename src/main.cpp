#include "Public/Application.h"

/*
* A simple ray tracer built following the Peter Shirley's Ray Tracing In One Week Book: https://raytracing.github.io/books/RayTracingInOneWeekend.html#overview
* With changes such as:
* 1. Renders to a GUI window using the WIN32 API and D2D1(for software rendering), DX11(for hardware rendering)
* 2. Implemented multi-threaded rendering using a basic thread pool
* 3. Added hardware rendering using DX11 and compute shader
* 4. Added a startup dialog box for user to set the ray tracer's settings such as resolution, render type, sample count and max depth
*/
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) 
{
	bool Result;
	Application App = Application();
	Result = App.Initialize(hInstance, nCmdShow);
	if (!Result)
	{
		return 0;
	}
	App.Run();
	return 0;
}
