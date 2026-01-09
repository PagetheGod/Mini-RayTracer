#include <Windows.h>
#include "Public/Application.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) 
{
	bool Result;
	Application App = Application();
	Result = App.Initialize(hInstance, nCmdShow, 256, 256);
	if (!Result)
	{
		return -1;
	}
	App.Run();
	App.Shutdown();

	return 0;
}
