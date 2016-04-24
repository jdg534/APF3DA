#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#include <d3d11.h> // needed for ID3D11Buffer deffinition found in Geomatry structure
#include <directxmath.h>
#include <vector>
#include <string>

#include <wtypes.h>

// start of Vertex Structures

struct SimpleVertex
{
	DirectX::XMFLOAT3 PosL;
	DirectX::XMFLOAT3 NormL;
	DirectX::XMFLOAT2 Tex;

	// when impleremnting m3d animation, from frank luna book
	DirectX::XMFLOAT3 Weights;
	BYTE BoneIndices[4];
	
	// when attempting to try MD5 animation
	int startWeight;
	int weightCount;
	
};

struct SkinnedVertex
{
	DirectX::XMFLOAT3 PosL;
	DirectX::XMFLOAT3 NormL;
	DirectX::XMFLOAT2 Tex;

	DirectX::XMFLOAT3 Weights;
	BYTE BoneIndices[4];// based off the frank luna book, page 662
};

// now the the Skeletal animation related structures
// for MD5, didn't work

struct Joint
{
	std::string name;
	int parentID;

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 orientation;
};

struct Weight
{
	int jointID;
	float bias;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal; // added upon adding the animation code
};

struct BoundingBox // added when implermenting Skeletal animation
{
	DirectX::XMFLOAT3 min, max;
};

struct FrameData
{
	unsigned int frameIndex;
	std::vector<float> frameData;
};

struct AnimationJointInformation
{
	std::string name;
	int parentID;

	int flags;
	int startIndex;
};

struct ModelAnimation
{
	int nFrames;
	int nJoints;
	int fps;

	int nAnimatedComponents;

	float frameTime;
	float totalAnimationTime;
	float currentAnimationTime;

	std::vector<AnimationJointInformation> jointInfo;
	std::vector<BoundingBox> frameBoundingBox;
	std::vector<Joint> baseFrameJoints;
	std::vector<FrameData> frameData;
	std::vector<std::vector<Joint>> frameSkeleton;
};

// end of skeletal animation related structures



// End of vertex Structures



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
	float drawingMode;
	float terrainScaledBy;
	DirectX::XMMATRIX WorldInverseTranspose;

	DirectX::XMFLOAT4X4 boneMatrices[96];
};

struct MD3ModelConstBuffer
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Projection;

	SurfaceInfo surface;

	Light light;

	DirectX::XMFLOAT3 EyePosW;
	float HasTexture;

	DirectX::XMMATRIX WorldInverseTranspose;
};

struct MD3ModelBoneMatrixConstBuffer
{
	DirectX::XMFLOAT4X4 boneMatrices[96]; // going with what is in the book, max 96 bones to be allowed, this seems unnessary
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

struct Diamond // this is using for the diamond square algorithum implermentation
{
	DirectX::XMFLOAT2 left, right, top, bottom;
};

struct Triangle
{
	DirectX::XMFLOAT3 v0, v1, v2;
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
		for (auto i = 0; i < vecs.size(); i++)
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


// structs moved from the provided file: GameObject.h

struct Geometry
{
	ID3D11Buffer * vertexBuffer;
	ID3D11Buffer * indexBuffer;
	int numberOfIndices;

	UINT vertexBufferStride;
	UINT vertexBufferOffset;
};

struct Material
{
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 specular;
	float specularPower;
};


#endif