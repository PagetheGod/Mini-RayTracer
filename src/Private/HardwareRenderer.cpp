#include "../Public/HardwareRenderer.h"
#include "../Public/ComputeShaderManager.h"
#include "../Public/Timer.h"

HardwareRenderer::HardwareRenderer(int Width, int Height, float AspectRatio) : m_Width(Width), m_Height(Height), m_AspectRatio(AspectRatio), m_World(nullptr), 
m_ComputeShaderManager(nullptr), m_Device(nullptr), m_DeviceContext(nullptr), m_CSTransformBuffer(nullptr), m_CSMaterialBuffer(nullptr)
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

	m_Device = m_D3D11->GetDevice();
	m_DeviceContext = m_D3D11->GetDeviceContext();

	//Construct the compute shader object, but do not initialize it, because at this point we do not know how many objects there are in the scene
	m_ComputeShaderManager = (std::make_unique<ComputeShaderManager>(m_Device, m_DeviceContext, m_Width, m_Height));

	//Create the world and initialize the shader manager
	CreateWorld();

	if (!m_ComputeShaderManager->InitializeShaders(m_World->GetNumObjects(), m_Camera.GetMaxDepth(), m_Camera.GetSampleCount()))
	{
		MessageBox(NULL, L"Failed to initialize compute shader!", L"Error", MB_OK);
		return false;
	}


	//Remember that our camera center is also the center of our coordinate system
	m_Camera = Camera();
	m_Camera.SetSampleCount(500);
	m_Camera.SetMaxDepth(50);

	float VFovAngle = Utility::DegreeToRadian(m_Camera.VerticalFOV);
	float h = std::tan(VFovAngle / 2.f);
	m_ViewportHeight = 2.f * h * m_Camera.FocusDistance;
	m_ViewportWidth = m_ViewportHeight * ((float)m_Width / (float)m_Height);


	m_ViewportU = m_ViewportWidth * m_Camera.CameraU;
	m_ViewportV = m_ViewportHeight * (-m_Camera.CameraV);//This is negative because the viewport Y is inverted compared to right hand coordinate system
	m_DeltaU = m_ViewportU / (float)(m_Width);
	m_DeltaV = m_ViewportV / (float)(m_Height);

	return true;
}

bool HardwareRenderer::RenderScene()
{
	bool Result = true;
	/*
	* Few things to note about getting the viewport upper left and the first pixel position:
	* 1. The camera is pointing straight down the negative Z axis(Right-hand system btw), and pointing at the center of the viewport, hence subtracting focal length in Z
	* 2. The viewport contains all the pixels(from 0th to Width - 1), however, we add half-pixel length spacings in both left-right and top-bottom, so the viewport can be divided nicely into Width x Height areas
	* 3. Step 2 means we need to add half of the delta U and V to get to the first pixel location
	*/

	//Initialize the constant buffer data that will be used by the compute shader
	GetShaderBuffers();

	Point3D CameraCenter = m_Camera.CameraCenter;
	Vector3D ViewportUpperLeft = CameraCenter - m_Camera.CameraW * m_Camera.FocusDistance - (m_ViewportU / 2.f) - (m_ViewportV / 2.f);
	Point3D FirstPixelPos = ViewportUpperLeft + 0.5f * (m_DeltaU + m_DeltaV);
	double RenderTime = 0.0;
	VTimer RenderTimer;
	
	const XMFLOAT3 CameraCenterPos(CameraCenter.X, CameraCenter.Y, CameraCenter.Z);
	const XMFLOAT3 ViewportUpperLeftPos(ViewportUpperLeft.X, ViewportUpperLeft.Y, ViewportUpperLeft.Z);
	const XMFLOAT3 FirstPixelPosFloat3(FirstPixelPos.X, FirstPixelPos.Y, FirstPixelPos.Z);
	const XMFLOAT3 DeltaUFloat3(m_DeltaU.X, m_DeltaU.Y, m_DeltaU.Z);
	const XMFLOAT3 DeltaVFloat3(m_DeltaV.X, m_DeltaV.Y, m_DeltaV.Z);

	Result = m_ComputeShaderManager->SetShaderParams(CameraCenterPos, ViewportUpperLeftPos, FirstPixelPosFloat3, DeltaUFloat3, DeltaVFloat3, 
		m_CSTransformBuffer, m_CSMaterialBuffer);
	if (!Result)
	{
		MessageBox(NULL, L"Failed to set compute shader parameters!", L"Error", MB_OK);
		return false;
	}

	m_ComputeShaderManager->DispatchShader();

	return Result;
}

void HardwareRenderer::GetShaderBuffers()
{
	m_CSTransformBuffer = m_World->GetCSTransformBuffer();
	m_CSMaterialBuffer = m_World->GetCSMaterialBuffer();
}



void HardwareRenderer::CreateWorld()
{
	//Create the materials and spheres in the world, I am keeping both my and the book's implementations so I can do some benchmark
	/*
	* Note on the calculation of RI for the glass sphere and the bubble. The glass is straightforward, it's just 1.5
	* For the bubble, we need to remember that the RI of a surface can be interpreted as the RI of itself divided by the enclosing object
	* Therefore, we have 1.f(air bubble) / 1.5f(glass layer)
	*/
	if (USEBULKHIT)
	{
		m_World = std::make_unique<HittableList>();
		MaterialScatterData MatScatterData(0.f, Color(0.5f, 0.5f, 0.5f));
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
	else
	{
		/*
		std::shared_ptr<Material> GroundMat = std::make_shared<Lambertian>(Color(0.5f, 0.5f, 0.5f));
		m_World = std::make_unique<HittableList>(std::make_shared<Sphere>(Point3D(0.f, -1000.f, 0.f), 1000.f, GroundMat));
		for (int a = -11; a < 11; a++)
		{
			for (int b = -11; b < 11; b++)
			{
				float ChooseMat = Utility::RandomFloat();
				Point3D SphereCenter(a + 0.9f * Utility::RandomFloat(), 0.2f, b + 0.9f * Utility::RandomFloat());

				if ((SphereCenter - Point3D(4.f, 0.2f, 0.f)).Length() > 0.9f)
				{
					std::shared_ptr<Material> SphereMat;

					if (ChooseMat < 0.8)
					{
						// diffuse
						Color albedo = Color::RandomVector() * Color::RandomVector();
						SphereMat = std::make_shared<Lambertian>(albedo);
						m_World->Add(make_shared<Sphere>(SphereCenter, 0.2, SphereMat));
					}
					else if (ChooseMat < 0.95)
					{
						// metal
						Color albedo = Color::RandomVector();
						float fuzz = Utility::RandomFloat(0.f, 0.5f);
						SphereMat = std::make_shared<Metal>(albedo, fuzz);
						m_World->Add(std::make_shared<Sphere>(SphereCenter, 0.2, SphereMat));
					}
					else
					{
						// glass
						SphereMat = std::make_shared<Dielectric>(1.5);
						m_World->Add(std::make_shared<Sphere>(SphereCenter, 0.2, SphereMat));
					}
				}
			}
		}

		std::shared_ptr<Material> Mat1 = std::make_shared<Dielectric>(1.5);
		m_World->Add(std::make_shared<Sphere>(Point3D(0, 1, 0), 1.0, Mat1));

		std::shared_ptr<Material> Mat2 = std::make_shared<Lambertian>(Color (0.4, 0.2, 0.1));
		m_World->Add(std::make_shared<Sphere>(Point3D(-4, 1, 0), 1.0, Mat2));

		std::shared_ptr<Material> Mat3 = std::make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
		m_World->Add(std::make_shared<Sphere>(Point3D(4, 1, 0), 1.0, Mat3));*/
	}
}

HardwareRenderer::~HardwareRenderer()
{
	if (m_CSMaterialBuffer)
	{
		delete[] m_CSMaterialBuffer;
		m_CSMaterialBuffer = nullptr;
	}
	if (m_CSTransformBuffer)
	{
		delete[] m_CSTransformBuffer;
		m_CSTransformBuffer = nullptr;
	}
}

