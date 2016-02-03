#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#include <directxmath.h>

struct SimpleVertex
{
	DirectX::XMFLOAT3 PosL;
	DirectX::XMFLOAT3 NormL;
	DirectX::XMFLOAT2 Tex;
};

struct SurfaceInfo
{
	DirectX::XMFLOAT4 AmbientMtrl;
	DirectX::XMFLOAT4 DiffuseMtrl;
	DirectX::XMFLOAT4 SpecularMtrl;
};

struct Light
{
	DirectX::XMFLOAT4 AmbientLight;
	DirectX::XMFLOAT4 DiffuseLight;
	DirectX::XMFLOAT4 SpecularLight;

	float SpecularPower;
	DirectX::XMFLOAT3 LightVecW;
};

struct ConstantBuffer
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Projection;

	SurfaceInfo surface;

	Light light;

	DirectX::XMFLOAT3 EyePosW;
	float HasTexture;
};

#endif