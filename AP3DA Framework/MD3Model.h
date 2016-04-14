#ifndef _MD3_MODEL_H_
#define _MD3_MODEL_H_

#include <d3d11.h>
#include <DirectXMath.h>

#include <map>
#include <vector>
#include <string>

// code for handling MD3 models is based off code found in the frank luna book

// start of required structures
// start with the KeyFrame struct as seen on page 646

#include "MD3ModelStructs.h"
#include "MD3ModelGeomatry.h"

#include "Structures.h"

#include "TextureManager.h"

// need to go to page 670

// end of required structures

class MD3Model
{
public: // keep simple for now
	MD3Model(ID3D11Device * d3dD, TextureManager * tm, std::string modelFile, std::wstring texturePath);


	~MD3Model();
	
	unsigned int m_nSubsets;

	std::vector<SurfaceInfo> m_materials;
	std::vector<ID3D11ShaderResourceView *> m_diffuseMaps;

	// tutorial requires CPU side versions of vertex data
	std::vector<SimpleVertex> m_vertices;
	std::vector<unsigned int> m_indercies;
	std::vector<MD3ModelSubSet> m_subsets;

	

	
	SkinnedMeshSkeleton m_skinnedMeshSkeleton;
	MD3ModelGeomatry m_modelGeomatry;

	
};

struct MD3ModelInstance
{
	MD3Model * theModel;
	float timePoint;
	std::string currentAnimationClipName;
	DirectX::XMFLOAT4X4 WorldMat;
	std::vector<DirectX::XMFLOAT4X4> finalTransforms;

	void update(float dt);
};


#endif