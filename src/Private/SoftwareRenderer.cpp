#include "../Public/SoftwareRenderer.h"



SoftwareRenderer::SoftwareRenderer(int Width) : m_Width(Width), m_OutFileStream(std::ofstream())
{
	m_AspectRatio = 16.f / 9.f;
	m_Height = (int)(m_Width / m_AspectRatio);
	m_Height >= 1 ? m_Height : m_Height = 1;

	m_ViewportWidth = 2.f;
	m_ViewportHeight = m_ViewportWidth / ((float)m_Width / (float)m_Height);
}
bool SoftwareRenderer::Initialize(const char* OutFileName)
{
	m_OutFileStream.open(OutFileName, std::ios::out | std::ios::binary);
	if (!m_OutFileStream.is_open())
	{
		MessageBox(NULL, L"Failed to open the file!", L"Error", MB_OK);
		return false;
	}

	//Initialize camera parameters and delta U,V
	//The camera center is also the origin of our coordinate system
	m_FocalLength = 1.f;
	m_CameraCenter = Point3D(0.f, 0.f, 0.f);

	m_ViewportU = Vector3D(m_ViewportWidth, 0.f, 0.f);
	m_ViewportV = Vector3D(0.f, -m_ViewportHeight, 0.f);//This is negative because the viewport Y is inverted compared to right hand coordinate system
	m_DeltaU = m_ViewportU / (float)(m_Width);
	m_DeltaV = m_ViewportV / (float)(m_Height);

	return true;
}

void SoftwareRenderer::RenderPPM()
{
	/*
	* Few things to note about getting the viewport upper left and the first pixel position:
	* 1. The camera is pointing straight down the negative Z axis(Right-hand system btw), and pointing at the center of the viewport, hence subtracting focal length in Z
	* 2. The viewport contains all the pixels(from 0th to Width - 1), however, we add half-pixel length spacings in both left-right and top-bottom, so the viewport can be divided nicely into Width x Height areas
	* 3. Step 2 means we need to add half of the delta U and V to get to the first pixel location
	*/
	Vector3D ViewportUpperLeft = m_CameraCenter - Vector3D(0.f, 0.f, m_FocalLength) - (m_ViewportU / 2.f) - (m_ViewportV / 2.f);
	Point3D FirstPixelPos = ViewportUpperLeft + 0.5f * (m_DeltaU + m_DeltaV);

	m_OutFileStream << "P3\n" << m_Width << ' ' << m_Height << "\n255\n";


	for (int i = 0; i < m_Height; i++)
	{
		for (int j = 0; j < m_Width; j++)
		{
			Point3D PixelPos = FirstPixelPos + (j * m_DeltaU) + (i * m_DeltaV);
			Vector3D RayDirection = PixelPos - m_CameraCenter;
			Ray CurrentRay = Ray(m_CameraCenter, RayDirection);

			Color PixelColor = Color(0.f, 0.f, 0.f);
			Vector3D UnitDirection = CurrentRay.Direction().Normalize();
			float tx = 0.5f * (UnitDirection.X + 1.f);//We are working with a unit vector with X in [-1,1] so we have to map X from [-1,1] to [0,1] first
			//Bilinear interpolation stuffs. Tbh I don't even know what am I accomplishing with this logic
			Color PixelColorX = (1.f - tx) * Color(0.f, 0.f, 1.f) + tx * Color(0.f, 0.1f, 1.f);
			float ty = 0.5f * (UnitDirection.Y + 1.f);
			PixelColor = (1.f - ty) * PixelColorX + ty * Color(1.f, 0.f, 0.f);
			WriteColor(m_OutFileStream, PixelColor);
		}
	}
	MessageBox(NULL, L"Render Complete!", L"Info", MB_OK);
	m_OutFileStream.close();
}


