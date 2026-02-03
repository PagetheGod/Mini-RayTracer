#include "../Public/HardwareRenderer.h"

HardwareRenderer::HardwareRenderer(int Width, int Height, float AspectRatio) : m_Width(Width), m_Height(Height), m_AspectRatio(AspectRatio), m_World(nullptr), 
m_FrameBuffer(nullptr)
{

}

bool HardwareRenderer::Intialize(HWND hWnd)
{
	m_hWnd = hWnd;

	m_D3D11 = std::make_unique<D3D11Class>(hWnd);

	if (!m_D3D11->InitializeD3D11())
	{
		MessageBox(NULL, L"Failed to initialize D3D11 class!", L"Error", MB_OK);
		return false;
	}


	//Remember that our camera center is also the center of our coordinate system
	m_Camera = Camera();
	m_Camera.SetSampleCount(100);
	m_Camera.SetMaxDepth(20);

	float VFovAngle = Utility::DegreeToRadian(m_Camera.VerticalFOV);
	float h = std::tan(VFovAngle / 2.f);
	m_ViewportHeight = 2.f * h * m_Camera.FocusDistance;
	m_ViewportWidth = m_ViewportHeight * ((float)m_Width / (float)m_Height);


	m_ViewportU = m_ViewportWidth * m_Camera.CameraU;
	m_ViewportV = m_ViewportHeight * (-m_Camera.CameraV);//This is negative because the viewport Y is inverted compared to right hand coordinate system
	m_DeltaU = m_ViewportU / (float)(m_Width);
	m_DeltaV = m_ViewportV / (float)(m_Height);

	//Intialize the frame buffer and the D2D1 class used for software rendering
	m_FrameBuffer = new unsigned char[m_Width * m_Height * 4];//Each pixel needs four bytes for B8G8R8A8
	memset(m_FrameBuffer, 0, m_Width * m_Height * 4);
	if (!m_FrameBuffer)
	{
		MessageBox(NULL, L"Failed to allocate frame buffer!", L"Error", MB_OK);
		return false;
	}

	return true;
}
