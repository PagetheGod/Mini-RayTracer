#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>



using namespace DirectX;

struct GlobalBufferType
{
	XMFLOAT3 CameraPos;
	unsigned int Padding1 = 0;
	XMFLOAT3 ViewportUpperLeft;
	unsigned int Padding2 = 0;
	XMFLOAT3 FirstPixelPos;
	unsigned int Padding3 = 0;
	XMFLOAT3 DeltaU;
	unsigned int Padding4 = 0;
	XMFLOAT3 DeltaV;
	unsigned int ObjectCount;
	XMUINT2 ScreenSize;
	unsigned int Depth;
	unsigned int SampleCount;
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
	float FuzzOrRI;
	uint32_t Type;
};

class ComputeShaderManager
{
public:
	ComputeShaderManager(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext, unsigned int ScreenWidth = 1920, unsigned int ScreenHeight = 1080);
	~ComputeShaderManager();
	bool InitializeShaders(unsigned int ObjectCount, unsigned int Depth, unsigned int SampleCount);
	bool SetShaderParams(const XMFLOAT3& CameraPos, const XMFLOAT3& ViewportUpperLeft, const XMFLOAT3& FirstPixelPos, const XMFLOAT3& DeltaU, const XMFLOAT3& DeltaV,
		const SphereTransformBufferType* SphereTransforms, const SphereMaterialBufferType* SphereMaterials);
	void DispatchShader();
	
	ID3D11Texture2D* GetComputeOutputBuffer() { return m_ComputeOutputBuffer; }

public:
	
private:
	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_DeviceContext;
	ID3D11ComputeShader* m_ComputeShader;
	ID3D11Buffer* m_CSConstantBuffer;
	ID3D11Buffer* m_SampleOffsetBuffer;
	ID3D11Buffer* m_SphereTransformBuffer;
	ID3D11Buffer* m_SphereMaterialBuffer;
	ID3D11Texture2D* m_ComputeOutputBuffer;
	ID3D11ShaderResourceView* m_TransformSRV;
	ID3D11ShaderResourceView* m_MaterialSRV;
	ID3D11UnorderedAccessView* m_ComputeOutputUAV;
	unsigned int m_ObjectCount;
	unsigned int m_Width;
	unsigned int m_Height;
	unsigned int m_MaxDepth;
	unsigned int m_SampleCount;
};