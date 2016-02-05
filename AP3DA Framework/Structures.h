#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#include <directxmath.h>
#include <vector>


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

struct Facet
{
	DirectX::XMFLOAT3 v0, v1, v2;
	DirectX::XMFLOAT3 surfaceNormal;
};

// start of Windows.bmp file headers / pixel types

struct BmpMagicNumber
{
	char magicNumber[2];
};

struct BmpFileHeader
{
	unsigned __int32 fileSize;
	unsigned __int16 creator1;
	unsigned __int16 creator2;
	unsigned __int32 bmpOffset; // offset to the actual image data
};

struct BmpInfoHeader
{
	unsigned __int32 headerSize;
	signed __int32 width;
	signed __int32 height;
	unsigned __int16 numberOfImagePlanes;
	unsigned __int16 bitsPerPixel;
	unsigned __int32 compressionType;
	unsigned __int32 sizeInBytes;
	signed __int32 imageWidth;
	signed __int32 imageHeight;
	unsigned __int32 numColours;
	unsigned __int32 numImportedColour;
};

struct BmpPx24Bit
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
};

struct BmpPx32Bit
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char alpha;
};

// end of Window.bmp file headers / pixel types

struct VertexForVertexNormalCalc
{
	DirectX::XMFLOAT3 pos;
	std::vector<DirectX::XMFLOAT3> surfaceNormals;
};

struct Square
{
	// to be used on the height map generation
	float top, bottom, left, right;
};

struct FaultFormationLine
{
	unsigned int startSideIndex;
	unsigned int endSideIndex;
};

struct FaultFormationQuad
{
	DirectX::XMFLOAT2 topLeft, topRight, bottomLeft, bottomRight;

	bool pointInQuad(DirectX::XMFLOAT2 p)
	{
		// get vectors to each point, sum them, 
		// if the length of the sum vecor = (0,0,0)
		std::vector<DirectX::XMFLOAT2> vecs;
		DirectX::XMFLOAT2 tmp;
		tmp.x = topLeft.x - p.x;
		tmp.y = topLeft.y - p.y;
		vecs.push_back(tmp);

		tmp.x = topRight.x - p.x;
		tmp.y = topRight.y - p.y;
		vecs.push_back(tmp);

		tmp.x = bottomLeft.x - p.x;
		tmp.y = bottomLeft.y - p.y;
		vecs.push_back(tmp);

		tmp.x = bottomRight.x - p.x;
		tmp.y = bottomRight.y - p.y;
		vecs.push_back(tmp);

		DirectX::XMFLOAT2 sum(0.0f, 0.0f);
		for (int i = 0; i < vecs.size(); i++)
		{
			sum.x += vecs[i].x;
			sum.y += vecs[i].y;
		}
		if (sum.x == 0.0f && sum.y == 0.0f)
		{
			return true;
		}
		return false;
	}
};


#endif