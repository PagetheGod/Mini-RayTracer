#include "../Public/ComputeShaderManager.h"

ComputeShaderManager::ComputeShaderManager(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) : m_Device(Device), m_DeviceContext(DeviceContext), m_CSConstantBuffer(nullptr)
, m_SampleOffsetBuffer(nullptr), m_SphereTransformBuffer(nullptr), m_SphereMaterialBuffer(nullptr), m_TransformSRV(nullptr), m_MaterialSRV(nullptr), m_ObjectCount(0)
{

}


bool ComputeShaderManager::InitializeShaders(unsigned int ObjectCount)
{
	HRESULT Result;
	ID3DBlob* ShaderBlob = nullptr;
	Result = D3DReadFileToBlob(L"RayTraceComputeShader.cso", &ShaderBlob);
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to read compute shader file! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}
	Result = m_Device->CreateComputeShader(ShaderBlob, sizeof(ShaderBlob), nullptr, &m_ComputeShader);
	if (FAILED(Result))
	{
		ShaderBlob->Release();
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create compute shader object! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	D3D11_BUFFER_DESC GlobalBufferDesc{};

	GlobalBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	GlobalBufferDesc.ByteWidth = sizeof(GlobalBufferType);
	GlobalBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	GlobalBufferDesc.CPUAccessFlags = 0;
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

	D3D11_BUFFER_DESC SampleOffsetBufferDesc{};

	SampleOffsetBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	SampleOffsetBufferDesc.ByteWidth = sizeof(SampleOffsetBufferDesc);
	SampleOffsetBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	SampleOffsetBufferDesc.CPUAccessFlags = 0;
	SampleOffsetBufferDesc.MiscFlags = 0;
	SampleOffsetBufferDesc.StructureByteStride = 0;

	Result = m_Device->CreateBuffer(&SampleOffsetBufferDesc, nullptr, &m_SampleOffsetBuffer);
	if (FAILED(Result))
	{
		ShaderBlob->Release();
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create sphere global buffer! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	D3D11_BUFFER_DESC SphereTransformBufferDesc;
	
	SphereTransformBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	SphereTransformBufferDesc.ByteWidth = sizeof(SphereTransformBufferType) * m_ObjectCount;
	SphereTransformBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	SphereTransformBufferDesc.CPUAccessFlags = 0;
	SphereTransformBufferDesc.MiscFlags = 0;
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

	SphereMaterialBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	SphereMaterialBufferDesc.ByteWidth = sizeof(SphereMaterialBufferType) * m_ObjectCount;
	SphereMaterialBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	SphereMaterialBufferDesc.CPUAccessFlags = 0;
	SphereMaterialBufferDesc.MiscFlags = 0;
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

	Result = m_Device->CreateShaderResourceView(m_SphereMaterialBuffer, nullptr, &m_MaterialSRV);
	if (FAILED(Result))
	{
		ShaderBlob->Release();
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to create sphere material SRV! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}


	m_ObjectCount = ObjectCount;
	ShaderBlob->Release();
	ShaderBlob = nullptr;
	return true;
}

bool ComputeShaderManager::SetShaderParams(XMFLOAT3& CameraPos, XMFLOAT3& ViewportUpperLeft, XMFLOAT3& FirstPixelPos, XMFLOAT3& DeltaU, XMFLOAT3& DeltaV, const XMFLOAT2* SampleOffset,
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
	GlobalBufferData->Infinity = Constants::g_Infinity;

	m_DeviceContext->Unmap(m_CSConstantBuffer, 0);

	m_DeviceContext->CSSetConstantBuffers(0, 1, &m_CSConstantBuffer);

	//Buffer for anti-alias offsets. We use the same 100(or more) random offsets for all the pixels

	SampleOffsetBufferType* SampleOffsetBufferData = nullptr;

	Result = m_DeviceContext->Map(m_SampleOffsetBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	if (FAILED(Result))
	{
		wchar_t ErrorMessage[128];
		wsprintf(ErrorMessage, L"Failed to map sample offset constant buffer! Error code: %04X", Result);
		MessageBox(NULL, ErrorMessage, L"Error", MB_OK);
		return false;
	}

	SampleOffsetBufferData = static_cast<SampleOffsetBufferType*>(MappedResource.pData);

	memcpy(SampleOffsetBufferData->SampleOffsets, SampleOffset, sizeof(XMFLOAT2) * 100);

	m_DeviceContext->Unmap(m_SampleOffsetBuffer, 0);

	m_DeviceContext->CSSetConstantBuffers(1, 1, &m_SampleOffsetBuffer);

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

	memcpy(SphereTransformBufferData, SphereTransforms, sizeof(SphereTransformBufferData) * m_ObjectCount);

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


	return true;
}

void ComputeShaderManager::DispatchShader()
{
	
}



ComputeShaderManager::~ComputeShaderManager()
{
	if (m_Device)
	{
		m_Device->Release();
		m_Device = nullptr;
	}
	if (m_DeviceContext)
	{
		m_DeviceContext->Release();
		m_DeviceContext = nullptr;
	}
	if (m_CSConstantBuffer)
	{
		m_CSConstantBuffer->Release();
		m_CSConstantBuffer = nullptr;
	}
	if (m_SphereTransformBuffer)
	{
		m_SphereTransformBuffer->Release();
		m_SphereTransformBuffer = nullptr;
	}
	if (m_SphereMaterialBuffer)
	{
		m_SphereMaterialBuffer->Release();
		m_SphereMaterialBuffer = nullptr;
	}
	if (m_TransformSRV)
	{
		m_TransformSRV->Release();
		m_TransformSRV = nullptr;
	}
}
