#include "../Public/SoftwareRenderer.h"
#include "../Public/D2D1Class.h"


SoftwareRenderer::SoftwareRenderer(int Width, int Height, float AspectRatio) : m_Width(Width), m_Height(Height), m_AspectRatio(AspectRatio), m_OutFileStream(std::ofstream()), 
m_FrameBuffer(nullptr), m_D2D1(nullptr)
{
	m_ViewportHeight = 2.f;
	m_ViewportWidth = m_ViewportHeight * ((float)m_Width / (float)m_Height);
}
bool SoftwareRenderer::Initialize(const char* OutFileName, HWND hWnd)
{
	bool Result = false;

	m_OutFileStream.open(OutFileName, std::ios::out | std::ios::binary);
	if (!m_OutFileStream.is_open())
	{
		MessageBox(NULL, L"Failed to open the file!", L"Error", MB_OK);
		return false;
	}
	
	m_hWnd = hWnd;

	//Initialize camera parameters and delta U,V
	//The camera center is also the origin of our coordinate system
	m_FocalLength = 1.f;
	m_CameraCenter = Point3D(0.f, 0.f, 0.f);

	m_ViewportU = Vector3D(m_ViewportWidth, 0.f, 0.f);
	m_ViewportV = Vector3D(0.f, -m_ViewportHeight, 0.f);//This is negative because the viewport Y is inverted compared to right hand coordinate system
	m_DeltaU = m_ViewportU / (float)(m_Width);
	m_DeltaV = m_ViewportV / (float)(m_Height);

	//Intialize the frame buffer and the D2D1 class used for software rendering
	m_FrameBuffer = new unsigned char[m_Width * m_Height * 4];//Each pixel needs four bytes for B8G8R8A8
	memset(m_FrameBuffer, 0, m_Width * m_Height * 4);
	if (!m_FrameBuffer)
	{
		MessageBox(NULL, L"Failed to allocate frame buffer!", L"Error", MB_OK);
		m_OutFileStream.close();
		return false;
	}

	m_D2D1 = new D2D1Class();
	if (!m_D2D1)
	{
		MessageBox(NULL, L"Failed to allocate D2D1 Class!", L"Error", MB_OK);
		delete[] m_FrameBuffer;
		m_OutFileStream.close();
		return false;
	}
	Result = m_D2D1->InitFactory();
	if (!Result)
	{
		MessageBox(NULL, L"Failed to init D2D1 Class!", L"Error", MB_OK);
		delete[] m_FrameBuffer;
		m_OutFileStream.close();
		return false;
	}
	if (FAILED(m_D2D1->CreateGraphicResources(hWnd)))
	{
		MessageBox(NULL, L"Failed to init D2D1 Class!", L"Error", MB_OK);
		delete[] m_FrameBuffer;
		m_D2D1->Shutdown();
		m_OutFileStream.close();
		return false;
	}

	return true;
}

void SoftwareRenderer::ClearWindow()
{
	m_D2D1->ClearBackground();
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

void SoftwareRenderer::RenderFrameBuffer()
{
	Vector3D ViewportUpperLeft = m_CameraCenter - Vector3D(0.f, 0.f, m_FocalLength) - (m_ViewportU / 2.f) - (m_ViewportV / 2.f);
	Point3D FirstPixelPos = ViewportUpperLeft + 0.5f * (m_DeltaU + m_DeltaV);

	for (int i = 0; i < m_Height; i++)
	{
		for (int j = 0; j < m_Width; j++)
		{
			Point3D PixelPos = FirstPixelPos + (j * m_DeltaU) + (i * m_DeltaV);
			Vector3D RayDirection = PixelPos - m_CameraCenter;
			Ray CurrentRay = Ray(m_CameraCenter, RayDirection);

			Color PixelColor = Color(0.f, 0.f, 0.f);
			Point3D SphereCenter = Point3D(0.f, 0.f, -1.f);
			float t = TestHitSphere(SphereCenter, 0.5f, CurrentRay);
			if (t > 0.f)
			{
				Point3D HitPoint = CurrentRay.At(t);
				Vector3D Normal = HitPoint - SphereCenter;
				Normal = Normal.Normalize();
				Normal = 0.5f * Vector3D(Normal.X + 1.f, Normal.Y + 1.f, Normal.Z + 1.f);
				PixelColor = Normal;
			}
			else
			{
				Vector3D UnitDirection = CurrentRay.Direction().Normalize();
				float tx = 0.5f * (UnitDirection.X + 1.f);//We are working with a unit vector with X in [-1,1] so we have to map X from [-1,1] to [0,1] first
				//Bilinear interpolation stuffs. Tbh I don't even know what am I accomplishing with this logic
				Color PixelColorX = (1.f - tx) * Color(0.f, 0.f, 1.f) + tx * Color(0.f, 0.1f, 1.f);
				float ty = 0.5f * (UnitDirection.Y + 1.f);
				PixelColor = (1.f - ty) * PixelColorX + ty * Color(1.f, 0.f, 0.f);
			}
			//The D2D1 class is expecting B8G8R8, so we convert the float color value to byte and fill every pixel in the buffer accordingly
			size_t PixelIndex = (i * m_Width + j) * 4;
			unsigned char AdjustedRed = (unsigned char)(255.999f * PixelColor.R());
			unsigned char AdjustedGreen = (unsigned char)(255.999f * PixelColor.G());
			unsigned char AdjustedBlue = (unsigned char)(255.999f * PixelColor.B());

			m_FrameBuffer[PixelIndex] = AdjustedBlue;
			m_FrameBuffer[PixelIndex + 1] = AdjustedGreen;
			m_FrameBuffer[PixelIndex + 2] = AdjustedRed;
		}
		//We send a draw call after every scanline
		InvalidateRect(m_hWnd, nullptr, false);
		UpdateWindow(m_hWnd);
	}
	MessageBox(NULL, L"Render Complete!", L"Info", MB_OK);
	DestroyWindow(m_hWnd);
}

void SoftwareRenderer::RenderToWindow()
{
	m_D2D1->RenderBitmap(m_FrameBuffer);
}

void SoftwareRenderer::Shutdown()
{
	if (m_D2D1)
	{
		m_D2D1->Shutdown();
		delete m_D2D1;
		m_D2D1 = nullptr;
	}
	delete[] m_FrameBuffer;
	m_OutFileStream.close();
}

float SoftwareRenderer::TestHitSphere(const Point3D& Center, float Radius, const Ray& R)
{
	//A simple test function to do ray sphere intersection
	Vector3D RayDir = R.Direction();
	Point3D RayOrigin = R.Origin();
	Vector3D RayOriToCenter = Center - RayOrigin;
	
	/*
	* 1.Now let's do some simplifications, first notice that b has a factor of -2 in it
	* 2.Let's assume for some number h, b = -2h, this will allows to simply b to just h, and the discriminant to h square minus ac, the four got factor out
	* 3.We have h = -2 * RayDir dot RayOriToCenter/ -2, solve this equation and we get h = RayDir Dot RayOriToCenter
	* 4.Also we can simply tho self dot products to length squared
	*/

	float a = RayDir.LengthSquared();
	float h = (RayDir.Dot(RayOriToCenter));
	float c = RayOriToCenter.LengthSquared() - Radius * Radius;


	float Discriminant = h * h - a * c;
	
	if (Discriminant < 0.f)
	{
		return -1.f;
	}
	return (h - std::sqrt(Discriminant)) / a;
}



