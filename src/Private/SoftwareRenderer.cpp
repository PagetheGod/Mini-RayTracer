#include "../Public/SoftwareRenderer.h"
#include "../Public/D2D1Class.h"


SoftwareRenderer::SoftwareRenderer(int Width, int Height, float AspectRatio) : m_Width(Width), m_Height(Height), m_AspectRatio(AspectRatio), m_OutFileStream(std::ofstream()), 
m_World(nullptr), m_FrameBuffer(nullptr), m_D2D1(nullptr)
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

	//Remember that our camera center is also the center of our coordinate system
	m_Camera = Camera();
	m_Camera.SetSampleCount(100);

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
	Point3D CameraCenter = m_Camera.CameraCenter;
	Vector3D ViewportUpperLeft = CameraCenter - Vector3D(0.f, 0.f, m_Camera.FocalLength) - (m_ViewportU / 2.f) - (m_ViewportV / 2.f);
	Point3D FirstPixelPos = ViewportUpperLeft + 0.5f * (m_DeltaU + m_DeltaV);

	m_OutFileStream << "P3\n" << m_Width << ' ' << m_Height << "\n255\n";


	for (int i = 0; i < m_Height; i++)
	{
		for (int j = 0; j < m_Width; j++)
		{
			Point3D PixelPos = FirstPixelPos + (j * m_DeltaU) + (i * m_DeltaV);
			Vector3D RayDirection = PixelPos - CameraCenter;
			Ray CurrentRay = Ray(CameraCenter, RayDirection);

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
	/*
	* Few things to note about getting the viewport upper left and the first pixel position:
	* 1. The camera is pointing straight down the negative Z axis(Right-hand system btw), and pointing at the center of the viewport, hence subtracting focal length in Z
	* 2. The viewport contains all the pixels(from 0th to Width - 1), however, we add half-pixel length spacings in both left-right and top-bottom, so the viewport can be divided nicely into Width x Height areas
	* 3. Step 2 means we need to add half of the delta U and V to get to the first pixel location
	*/
	Point3D CameraCenter = m_Camera.CameraCenter;
	Vector3D ViewportUpperLeft = CameraCenter - Vector3D(0.f, 0.f, m_Camera.FocalLength) - (m_ViewportU / 2.f) - (m_ViewportV / 2.f);
	Point3D FirstPixelPos = ViewportUpperLeft + 0.5f * (m_DeltaU + m_DeltaV);

	//Add spheres into the world.
	m_World = new HittableList(std::make_shared<Sphere>(Sphere(Point3D(0.f, 0.f, -1.f), 0.5f)));
	m_World->Add(std::make_shared<Sphere>(Sphere(Point3D(0.f, -100.5f, -1.f), 100.f)));

	for (int i = 0; i < m_Height; i++)
	{
		for (int j = 0; j < m_Width; j++)
		{
			Point3D PixelPos = FirstPixelPos + (j * m_DeltaU) + (i * m_DeltaV);
			Vector3D RayDirection = PixelPos - CameraCenter;
			Ray CurrentRay = Ray(CameraCenter, RayDirection);

			Color PixelColor = Color(0.f, 0.f, 0.f);
			PixelColor = m_Camera.CalculateHitColor(*m_World, PixelPos, m_DeltaU, m_DeltaV);
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
	delete m_World;
	m_OutFileStream.close();
}


Color SoftwareRenderer::CalculateHitColor(const Ray& R, HittableList& World)
{
	HitRecord TempHitRecord;
	if (World.Hit(R, Interval(0.f, Constants::g_Infinity), TempHitRecord))
	{
		return 0.5f * Color(TempHitRecord.HitNormal + Color(1.f, 1.f, 1.f));
	}
	Vector3D UnitDirection = R.Direction().Normalize();
	float t = 0.5f * (UnitDirection.X + 1.f);//We are working with a unit vector with X in [-1,1] so we have to map X from [-1,1] to [0,1] first
	return (1.f - t) * Color(0.9f, 0.9f, 0.9f) + t * Color(0.5f, 0.7f, 1.f);
}



