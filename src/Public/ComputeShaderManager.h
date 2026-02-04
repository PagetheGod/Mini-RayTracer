#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "Camera.h"

using namespace DirectX;

struct GlobalBufferType
{
	XMFLOAT3 CameraPos;
	XMFLOAT3 ViewportUpperLeft;
	XMFLOAT3 FirstPixelPos;
	XMFLOAT3 DeltaU;
	XMFLOAT3 DeltaV;
	float Infinity;
};

struct SampleOffsetBufferType
{
	XMFLOAT2 SampleOffsets[100];
};

struct SphereTransformBufferType
{
	XMFLOAT3 SphereCenterPos;
	float Radius;
};
struct SphereMaterialBufferType
{
	XMFLOAT3 Albedo;
	float Fuzz;
	float RefractionIndex;
	uint32_t Type;
};


class ComputeShaderManager
{
public:
	ComputeShaderManager(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
	~ComputeShaderManager();
	bool InitializeShaders(unsigned int ObjectCount);
	bool SetShaderParams(XMFLOAT3& CameraPos, XMFLOAT3& ViewportUpperLeft, XMFLOAT3& FirstPixelPos, XMFLOAT3& DeltaU, XMFLOAT3& DeltaV, const XMFLOAT2* SampleOffset, 
		const SphereTransformBufferType* SphereTransforms, const SphereMaterialBufferType* SphereMaterials);
	void DispatchShader();
	

public:
	
private:
	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_DeviceContext;
	ID3D11ComputeShader* m_ComputeShader;
	ID3D11Buffer* m_CSConstantBuffer;
	ID3D11Buffer* m_SampleOffsetBuffer;
	ID3D11Buffer* m_SphereTransformBuffer;
	ID3D11Buffer* m_SphereMaterialBuffer;
	ID3D11ShaderResourceView* m_TransformSRV;
	ID3D11ShaderResourceView* m_MaterialSRV;
	unsigned int m_ObjectCount;
};