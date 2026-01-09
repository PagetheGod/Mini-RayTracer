#include "../Public/SoftwareRenderer.h"



SoftwareRenderer::SoftwareRenderer(int Width, int Height) : m_Width(Width), m_Height(Height), m_OutFileStream(std::ofstream())
{

}
bool SoftwareRenderer::Initialize(const char* OutFileName)
{
	m_OutFileStream.open(OutFileName, std::ios::out | std::ios::binary);
	if (!m_OutFileStream.is_open())
	{
		MessageBox(NULL, L"Failed to open the file!", L"Error", MB_OK);
		return false;
	}

	return true;
}

void SoftwareRenderer::RenderPPM()
{
	m_OutFileStream << "P3\n" << m_Width << ' ' << m_Height << "\n255\n";

	const double RenderWidth = (double)m_Width - 1.f;
	const double RenderHeight = (double)m_Height - 1.f;

	for (int i = 0; i < m_Width; i++)
	{
		for (int j = 0; j < m_Height; j++)
		{
			double Red = (1.f - j / RenderWidth);
			double Green = i / RenderHeight;
			double Blue = 0.f;

			int OutRed = (255.99f * Red);
			int OutGreen = (255.99f * Green);
			int OutBlue = (255.99f * Blue);

			m_OutFileStream << OutRed << ' ' << OutGreen << ' ' << OutBlue << '\n';
		}
	}
	MessageBox(NULL, L"Render Complete!", L"Info", MB_OK);
	m_OutFileStream.close();
}


