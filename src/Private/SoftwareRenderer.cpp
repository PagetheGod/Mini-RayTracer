#include "../Public/SoftwareRenderer.h"
#include "../Public/D2D1Class.h"
#include "../Public/Timer.h"
#include "../Public/VMaterial.h"
#include "../Public/SubMaterials.h"


SoftwareRenderer::SoftwareRenderer(unsigned int Width, unsigned int Height, float AspectRatio) : m_Width(Width), m_Height(Height), m_AspectRatio(AspectRatio), m_OutFileStream(std::ofstream()),
m_World(nullptr), m_FrameBuffer(nullptr), m_D2D1(nullptr), m_ThreadPool(nullptr)
{

}

//Some of the viewport, fov, and aspect ratio logics can be moved into the camera class
//But for this project such set up is fine. Or maybe we should move these logic into the constructor for RAII
bool SoftwareRenderer::Initialize(const char* OutFileName, HWND hWnd, unsigned int SampleCount, unsigned int MaxDepth)
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
	m_Camera.SetSampleCount(SampleCount);
	m_Camera.SetMaxDepth(MaxDepth);

	float VFovAngle = Utility::DegreeToRadian(m_Camera.VerticalFOV);
	float h = std::tan(VFovAngle / 2.f);
	m_ViewportHeight = 2.f * h * m_Camera.FocusDistance;
	m_ViewportWidth = m_ViewportHeight * ((float)m_Width / (float)m_Height);
	
	
	m_ViewportU = m_ViewportWidth * m_Camera.CameraU;
	m_ViewportV = m_ViewportHeight *  (-m_Camera.CameraV);//This is negative because the viewport Y is inverted compared to right hand coordinate system
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

	//Only start the thread pool if everything goes well
	m_ThreadPool = std::make_unique<VThreadPool>(16, true);

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
	Vector3D ViewportUpperLeft = CameraCenter - Vector3D(0.f, 0.f, m_Camera.FocusDistance) - (m_ViewportU / 2.f) - (m_ViewportV / 2.f);
	Point3D FirstPixelPos = ViewportUpperLeft + 0.5f * (m_DeltaU + m_DeltaV);

	m_OutFileStream << "P3\n" << m_Width << ' ' << m_Height << "\n255\n";
	for (size_t i = 0; i < m_Height; i++)
	{
		for (size_t j = 0; j < m_Width; j++)
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
	Vector3D ViewportUpperLeft = CameraCenter - m_Camera.CameraW * m_Camera.FocusDistance - (m_ViewportU / 2.f) - (m_ViewportV / 2.f);
	Point3D FirstPixelPos = ViewportUpperLeft + 0.5f * (m_DeltaU + m_DeltaV);
	double RenderTime = 0.0;
	VTimer RenderTimer;
	
	

	CreateWorld();
	std::vector<std::future<unsigned int>> Futures;
	Futures.reserve(1080);
	RenderTimer.Start();
	for (unsigned int i = 0; i < m_Height; i++)
	{
		Futures.push_back(m_ThreadPool->SubmitTask([this, CameraCenter, FirstPixelPos, i]()
		{
			for (unsigned int j = 0; j < m_Width; j++)
			{
				Point3D PixelPos = FirstPixelPos + (j * m_DeltaU) + (i * m_DeltaV);

				Color PixelColor = Color(0.f, 0.f, 0.f);
				PixelColor = m_Camera.CalculateHitColor(*m_World, PixelPos, m_DeltaU, m_DeltaV);
				//The D2D1 class is expecting B8G8R8, so we convert the float color value to byte and fill every pixel in the buffer accordingly
				size_t PixelIndex = (i * m_Width + j) * 4;
					
				//Apply gamma correction to color components and adjust them to between 0 and 255
				unsigned char AdjustedRed = (unsigned char)LinearToGamma(PixelColor.R());
				unsigned char AdjustedGreen = (unsigned char)LinearToGamma(PixelColor.G());
				unsigned char AdjustedBlue = (unsigned char)LinearToGamma(PixelColor.B());

				AdjustedRed = (unsigned char)(255.999f * PixelColor.R());
				AdjustedGreen = (unsigned char)(255.999f * PixelColor.G());
				AdjustedBlue =  (unsigned char)(255.999f * PixelColor.B());

				m_FrameBuffer[PixelIndex] = AdjustedBlue;
				m_FrameBuffer[PixelIndex + 1] = AdjustedGreen;
				m_FrameBuffer[PixelIndex + 2] = AdjustedRed;
			}
			return i;
		}));
	}

	for (auto& Future : Futures)
	{
		int Scanline = Future.get();
		RECT UpdateRegion{ 0, Scanline, m_Width, Scanline + 1};
		InvalidateRect(m_hWnd, &UpdateRegion, false);
		UpdateWindow(m_hWnd);
	}

	RenderTimer.Stop();
	RenderTime = (double)RenderTimer.GetLastDuration() / 1000.0;
	std::vector<char> CompleteMessage(128);
	StringCbPrintfExA(CompleteMessage.data(), 128, NULL, NULL, STRSAFE_NULL_ON_FAILURE, "Render Complete! Time used: %0.3f", RenderTime);
	MessageBoxExA(NULL, CompleteMessage.data(), LPCSTR{"Info"}, MB_OK, NULL);

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

void SoftwareRenderer::CreateWorld()
{
	//Create the materials and spheres in the world, I am keeping both my and the book's implementations so I can do some benchmark
	/*
	* Note on the calculation of RI for the glass sphere and the bubble. The glass is straightforward, it's just 1.5
	* For the bubble, we need to remember that the RI of a surface can be interpreted as the RI of itself divided by the enclosing object
	* Therefore, we have 1.f(air bubble) / 1.5f(glass layer)
	*/
	m_World = std::make_unique<HittableList>();
	MaterialScatterData MatScatterData(0.f, Color(0.5f, 0.5f, 0.5f)) ;
	m_World->VAddSphere(SphereObjectData(Point3D(0.f, -1000.f, 0.f), 1000.f), MatScatterData, MaterialType::Lambertian);
	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
		{
			SphereObjectData SphereData;
			float ChooseMat = Utility::RandomFloat();
			Point3D SphereCenter(a + 0.9f * Utility::RandomFloat(), 0.2f, b + 0.9f * Utility::RandomFloat());

			if ((SphereCenter - Point3D(4.f, 0.2f, 0.f)).Length() > 0.9f)
			{
				std::shared_ptr<Material> SphereMat;

				if (ChooseMat < 0.8f)
				{
					// diffuse
					Color Albedo = Color::RandomVector() * Color::RandomVector();
					MaterialScatterData MatScatterData;
					MatScatterData.Albedo = Albedo;
					SphereData.Center = SphereCenter;
					SphereData.Radius = 0.2f;
					m_World->VAddSphere(SphereData, MatScatterData, MaterialType::Lambertian);
				}
				else if (ChooseMat < 0.95f)
				{
					// metal
					Color Albedo = Color::RandomVector();
					float Fuzz = Utility::RandomFloat(0.f, 0.5f);
					MaterialScatterData MatScatterData;
					MatScatterData.Albedo = Albedo;
					MatScatterData.FuzzOrRI = Fuzz;
					SphereData.Center = SphereCenter;
					SphereData.Radius = 0.2f;
					m_World->VAddSphere(SphereData, MatScatterData, MaterialType::Metal);
				}
				else
				{
					// glass
					MaterialScatterData MatScatterData;
					MatScatterData.FuzzOrRI = 1.5f;
					SphereData.Center = SphereCenter;
					SphereData.Radius = 0.2f;
					m_World->VAddSphere(SphereData, MatScatterData, MaterialType::Dielectric);
				}
			}
		}
	}
	MaterialScatterData ScatterData;
	ScatterData.FuzzOrRI = 1.5f;
	m_World->VAddSphere(SphereObjectData(Point3D(0.f, 1.f, 0.f), 1.f), ScatterData, MaterialType::Dielectric);

	ScatterData.Albedo = Color(0.4f, 0.2f, 0.1f);
	m_World->VAddSphere(SphereObjectData(Point3D(-4, 1, 0), 1.f), ScatterData, MaterialType::Lambertian);

	ScatterData.Albedo = Color(0.7f, 0.6f, 0.5f);
	ScatterData.FuzzOrRI = 0.f;
	m_World->VAddSphere(SphereObjectData(Point3D(4, 1, 0), 1.f), ScatterData, MaterialType::Metal);
}

