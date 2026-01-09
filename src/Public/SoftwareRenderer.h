#pragma once

#include <string>
#include <fstream>
#include <Windows.h>

#define WIN32_LEAN_AND_MEAN
class SoftwareRenderer
{
public:
	SoftwareRenderer(int Width = 1920, int Height = 1080);
	bool Initialize(const char* OutFileName);
	void RenderPPM();


	~SoftwareRenderer() = default;

private:
	int m_Width;
	int m_Height;
	std::ofstream m_OutFileStream;
};