#include "Public/ComputeShaderManager.h"
#include "Public/Camera.h"
#include <string>

ComputeShaderManager::ComputeShaderManager(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext, unsigned int ScreenWidth, unsigned int ScreenHeight) : m_Device(Device), m_DeviceContext(DeviceContext),
m_CSConstantBuffer(nullptr), m_SampleOffsetBuffer(nullptr), m_SphereTransformBuffer(nullptr), m_SphereMaterialBuffer(nullptr), m_ComputeOutputBuffer(nullptr), m_TransformSRV(nullptr), m_MaterialSRV(nullptr),
m_ComputeOutputUAV(nullptr), m_CSBeginQuery(nullptr), m_CSEndQuery(nullptr), m_DisjointQuery(nullptr), m_Width(ScreenWidth), m_Height(ScreenHeight)
{
}



bool ComputeShaderManager::InitializeShaders(unsigned int ObjectCount, unsigned int Depth, unsigned int SampleCount)
{
	HRESULT Result;
	ID3DBlob* ShaderBlob = nullptr;
	m_MaxDepth = Depth;
	m_SampleCount = SampleCount;
	m_ObjectCount = ObjectCount;
	//Get the .exe path so we know where to load the shader
	//Tbh just /Shaders/[Shader .cso file name] should work for a project this tiny so this is overkill
	
	wchar_t ExecutablePath[256];
	//Gets the full path of current executable, NULL means current process
	GetModuleFileName(NULL, ExecutablePath, 256);
	std::wstring ExecutableDirectory(ExecutablePath);
	//find_last_of finds the last separator and makes a substring up until that char, this gives us the absolute path to the exe
	ExecutableDirectory = ExecutableDirectory.substr(0, ExecutableDirectory.find_last_of(L"\\/") + 1);
	//Final full path to the shader file
	std::wstring ShaderPath = ExecutableDirectory + L"Shaders/RayTraceShader.cso";

	Result = D3DReadFileToBlob(ShaderPath.c_str(), &ShaderBlob);
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to read compute shader file! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}
	Result = m_Device->CreateComputeShader(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), nullptr, &m_ComputeShader);
	if (FAILED(Result))
	{
		ShaderBlob->Release();
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create compute shader object! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	

	D3D11_BUFFER_DESC GlobalBufferDesc{};

	GlobalBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	GlobalBufferDesc.ByteWidth = sizeof(GlobalBufferType);
	GlobalBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	GlobalBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; 
	GlobalBufferDesc.MiscFlags = 0;
	GlobalBufferDesc.StructureByteStride = 0;


	Result = m_Device->CreateBuffer(&GlobalBufferDesc, nullptr, &m_CSConstantBuffer);
	if (FAILED(Result))
	{
		ShaderBlob->Release();
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create sphere global buffer! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	D3D11_BUFFER_DESC SphereTransformBufferDesc;
	
	SphereTransformBufferDesc.Usage = D3D11_USAGE_DYNAMIC;//Note that because we did not set initial resource so we need to map. Which means usage should be dynamic
	SphereTransformBufferDesc.ByteWidth = sizeof(SphereTransformBufferType) * m_ObjectCount;
	SphereTransformBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	SphereTransformBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	SphereTransformBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;//We have to set this flag to let dx11 know that this is a structured buffer
	SphereTransformBufferDesc.StructureByteStride = sizeof(SphereTransformBufferType);

	Result = m_Device->CreateBuffer(&SphereTransformBufferDesc, nullptr, &m_SphereTransformBuffer);
	if (FAILED(Result))
	{
		ShaderBlob->Release();
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create sphere transform buffer! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}
	//Create the SRVs so the compute shader can use the structured buffers
	Result = m_Device->CreateShaderResourceView(m_SphereTransformBuffer, nullptr, &m_TransformSRV);
	if (FAILED(Result))
	{
		ShaderBlob->Release();
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create sphere transform SRV! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}


	D3D11_BUFFER_DESC SphereMaterialBufferDesc;

	SphereMaterialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	SphereMaterialBufferDesc.ByteWidth = sizeof(SphereMaterialBufferType) * m_ObjectCount;
	SphereMaterialBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	SphereMaterialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	SphereMaterialBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;//We have to set this flag to let dx11 know that this is a structured buffer
	SphereMaterialBufferDesc.StructureByteStride = sizeof(SphereMaterialBufferType);

	Result = m_Device->CreateBuffer(&SphereMaterialBufferDesc, nullptr, &m_SphereMaterialBuffer);
	if (FAILED(Result))
	{
		ShaderBlob->Release();
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create sphere material buffer! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	D3D11_TEXTURE2D_DESC ComputeOutputBufferDesc{};

	ComputeOutputBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//Later we have to copy this using GPU, so the format needs to match the back buffer
	ComputeOutputBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	ComputeOutputBufferDesc.CPUAccessFlags = 0;
	ComputeOutputBufferDesc.Width = m_Width;
	ComputeOutputBufferDesc.Height = m_Height;
	ComputeOutputBufferDesc.SampleDesc.Count = 1;
	ComputeOutputBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	ComputeOutputBufferDesc.ArraySize = 1;
	ComputeOutputBufferDesc.MipLevels = 1;
	ComputeOutputBufferDesc.MiscFlags = 0;

	Result = m_Device->CreateTexture2D(&ComputeOutputBufferDesc, nullptr, &m_ComputeOutputBuffer);
	if (FAILED(Result))
	{
		ShaderBlob->Release();
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create compute output buffer! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	
	Result = m_Device->CreateShaderResourceView(m_SphereMaterialBuffer, nullptr, &m_MaterialSRV);
	if (FAILED(Result))
	{
		ShaderBlob->Release();
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create sphere material SRV! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	Result = m_Device->CreateUnorderedAccessView(m_ComputeOutputBuffer, nullptr, &m_ComputeOutputUAV);
	if (FAILED(Result))
	{
		ShaderBlob->Release();
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create sphere material SRV! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	
	ShaderBlob->Release();
	ShaderBlob = nullptr;
	return true;
}

bool ComputeShaderManager::SetShaderParams(const XMFLOAT3& CameraPos, const XMFLOAT3& ViewportUpperLeft, const XMFLOAT3& FirstPixelPos, const XMFLOAT3& DeltaU, const XMFLOAT3& DeltaV,
	const SphereTransformBufferType* SphereTransforms, const SphereMaterialBufferType* SphereMaterials)
{
	HRESULT Result;
	D3D11_MAPPED_SUBRESOURCE MappedResource;

	//Fill all the buffers, starting with the two constant buffers
	GlobalBufferType* GlobalBufferData = nullptr;
	
	Result = m_DeviceContext->Map(m_CSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to map global constant buffer! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	GlobalBufferData = static_cast<GlobalBufferType*>(MappedResource.pData);

	GlobalBufferData->CameraPos = CameraPos;
	GlobalBufferData->ViewportUpperLeft = ViewportUpperLeft;
	GlobalBufferData->FirstPixelPos = FirstPixelPos;
	GlobalBufferData->DeltaU = DeltaU;
	GlobalBufferData->DeltaV = DeltaV;
	GlobalBufferData->ObjectCount = m_ObjectCount;
	GlobalBufferData->ScreenSize = XMUINT2(m_Width, m_Height);
	GlobalBufferData->Depth = m_MaxDepth;
	GlobalBufferData->SampleCount = m_SampleCount;

	m_DeviceContext->Unmap(m_CSConstantBuffer, 0);

	m_DeviceContext->CSSetConstantBuffers(0, 1, &m_CSConstantBuffer);

	//Structured Buffers
	SphereTransformBufferType* SphereTransformBufferData = nullptr;

	Result = m_DeviceContext->Map(m_SphereTransformBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to map sphere transform structured buffer! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	SphereTransformBufferData = static_cast<SphereTransformBufferType*>(MappedResource.pData);

	memcpy(SphereTransformBufferData, SphereTransforms, sizeof(SphereTransformBufferType) * m_ObjectCount);

	m_DeviceContext->Unmap(m_SphereTransformBuffer, 0);

	m_DeviceContext->CSSetShaderResources(0, 1, &m_TransformSRV);

	//Material Buffer
	SphereMaterialBufferType* SphereMaterialBufferData = nullptr;

	Result = m_DeviceContext->Map(m_SphereMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to map sphere material structured buffer! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	SphereMaterialBufferData = static_cast<SphereMaterialBufferType*>(MappedResource.pData);

	memcpy(SphereMaterialBufferData, SphereMaterials, sizeof(SphereMaterialBufferType) * m_ObjectCount);

	m_DeviceContext->Unmap(m_SphereMaterialBuffer, 0);

	m_DeviceContext->CSSetShaderResources(1, 1, &m_MaterialSRV);

	//Set the output buffer for compute shader
	m_DeviceContext->CSSetUnorderedAccessViews(0, 1, &m_ComputeOutputUAV, nullptr);

	return true;
}

void ComputeShaderManager::DispatchShader()
{	
	m_DeviceContext->CSSetShader(m_ComputeShader, nullptr, 0);

	D3D11_QUERY_DESC QueryDesc{};
	QueryDesc.Query = D3D11_QUERY_TIMESTAMP;
	HRESULT Result = m_Device->CreateQuery(&QueryDesc, &m_CSBeginQuery);
	if (FAILED(Result))
	{
		m_ShouldTime = false;
	}
	Result = m_Device->CreateQuery(&QueryDesc, &m_CSEndQuery);
	if (FAILED(Result))
	{
		m_ShouldTime = false;
	}
	QueryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	Result = m_Device->CreateQuery(&QueryDesc, &m_DisjointQuery);
	if (FAILED(Result))
	{
		m_ShouldTime = false;
	}
	if (m_ShouldTime)
	{
		m_DeviceContext->Begin(m_DisjointQuery);
		m_DeviceContext->End(m_CSBeginQuery);
	}
	//ceil(a / b) = (a + b - 1) / b
	//This pushes the integer division to the next multiple so we don't truncate in case the pixel number isn't multiple of 8
	m_DeviceContext->Dispatch((m_Width + 7 ) / 8, (m_Height + 7) / 8, 1);
	if (m_ShouldTime)
	{
		m_DeviceContext->End(m_CSEndQuery);
		m_DeviceContext->End(m_DisjointQuery);
	}
	
}

void ComputeShaderManager::GetGPURenderTime(std::wstring& RenderTimeString)
{
	//Function to fill in a target render time string
	//Will fill in a corresponding message if we failed to create the query objects
	if (!m_ShouldTime)
	{
		RenderTimeString = L"Failed to create DX11 queries. No GPU render time is available";
	}
	else
	{
		while (m_DeviceContext->GetData(m_DisjointQuery, nullptr, 0, 0) == S_FALSE)
		{
			Sleep(1);
		}
		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT DisjointTS;
		m_DeviceContext->GetData(m_DisjointQuery, &DisjointTS, sizeof(DisjointTS), 0);
		if (DisjointTS.Disjoint)
		{
			RenderTimeString = L"GPU frame was disjoint. No render time is available";
		}
		else
		{
			uint64_t CSStart, CSEnd;
			m_DeviceContext->GetData(m_CSBeginQuery, &CSStart, sizeof(uint64_t), 0);
			m_DeviceContext->GetData(m_CSEndQuery, &CSEnd, sizeof(uint64_t), 0);

			//Result is in raw clock ticks
			double RenderTime = ((double)CSEnd - (double)CSStart)/(double)DisjointTS.Frequency;

			RenderTimeString = std::format(L"Render Complete! Time used: {:.3f} seconds", RenderTime);
		}
	}
}



ComputeShaderManager::~ComputeShaderManager()
{
	if (m_ComputeShader)
	{
		m_ComputeShader->Release();
		m_ComputeShader = nullptr;
	}
	if (m_SphereTransformBuffer)
	{
		m_SphereTransformBuffer->Release();
		m_SphereTransformBuffer = nullptr;
	}
	if(m_CSConstantBuffer)
	{
		m_CSConstantBuffer->Release();
		m_CSConstantBuffer = nullptr;
	}

	if (m_SphereMaterialBuffer)
	{
		m_SphereMaterialBuffer->Release();
		m_SphereMaterialBuffer = nullptr;
	}
	if (m_ComputeOutputBuffer)
	{
		m_ComputeOutputBuffer->Release();
		m_ComputeOutputBuffer = nullptr;
	}
	if (m_TransformSRV)
	{
		m_TransformSRV->Release();
		m_TransformSRV = nullptr;
	}
	if (m_MaterialSRV)
	{
		m_MaterialSRV->Release();
		m_MaterialSRV = nullptr;
	}
	if (m_ComputeOutputUAV)
	{
		m_ComputeOutputUAV->Release();
		m_ComputeOutputUAV = nullptr;
	}
	if (m_CSBeginQuery)
	{
		m_CSBeginQuery->Release();
		m_CSBeginQuery = nullptr;
	}
	if (m_CSEndQuery)
	{
		m_CSEndQuery->Release();
		m_CSEndQuery = nullptr;
	}
	if (m_DisjointQuery)
	{
		m_DisjointQuery->Release();
		m_DisjointQuery = nullptr;
	}
}
