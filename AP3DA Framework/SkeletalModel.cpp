#include "SkeletalModel.h"
//#include "MD3Loader.h"
#include "SkeletalModelLoader.h"
#include "TextureManager.h"

#include <fstream>

SkeletalModel::SkeletalModel(ID3D11Device * d3dD, TextureManager * tm, std::string modelFile, std::wstring texturePath)
{
	SkeletalModelLoader md3l;
	std::vector<SkeletalModelMaterial> md3Mats;

	md3l.LoadM3d(modelFile, m_vertices, m_indercies, m_subsets, md3Mats, m_skinnedMeshSkeleton);
	
	

	m_modelGeomatry.setVertices(d3dD, &m_vertices[0], m_vertices.size());
	m_modelGeomatry.SetIndices(d3dD, &m_indercies[0], m_indercies.size());
	m_modelGeomatry.SetSubsetTable(m_subsets);

	m_nSubsets = m_subsets.size();

	for (unsigned int i = 0; i < m_nSubsets; i++)
	{
		m_materials.push_back(md3Mats[i].materialLightValues);

		// load the actual diffuse maps here, the parser only deals with getting the file names

		ID3D11ShaderResourceView * diffuseMap;

		Texture * t = tm->getTextureWithID(md3Mats[i].diffuseMapFile);
		if (t == nullptr)
		{
			if (tm->addTexture(md3Mats[i].diffuseMapFile))
			{
				t = tm->getTextureWithID(md3Mats[i].diffuseMapFile);
			}
		}
		diffuseMap = t->imageMapPtr;
		m_diffuseMaps.push_back(diffuseMap);
	}
}

SkeletalModel::SkeletalModel(ID3D11Device * d3dD, TextureManager * tm, std::string modelFile, std::vector<std::string> animationFiles)
{
	SkeletalModelLoader md5Loader;

	std::vector<SkeletalModelMaterial> materials;


}

SkeletalModel::~SkeletalModel()
{
}

void SkeletalModel::shutdown()
{
	// textures cleaned up by the texture manager
	m_modelGeomatry.shutdown();
}

void SkeletalModelInstance::update(float dt)
{
	assert(dt > 0.0f);
	timePoint += dt;
	theModel->m_skinnedMeshSkeleton.getFinalTransforms(currentAnimationClipName, timePoint, finalTransforms);
	float clipEndTime = theModel->m_skinnedMeshSkeleton.GetClipEndTime(currentAnimationClipName);
	
	if (timePoint > theModel->m_skinnedMeshSkeleton.GetClipEndTime(currentAnimationClipName))
	{
		timePoint = 0.0f;
	}
}