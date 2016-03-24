#include "MD3Model.h"
#include "MD3Loader.h"
#include "TextureManager.h"


MD3Model::MD3Model(ID3D11Device * d3dD, TextureManager * tm, std::string modelFile, std::wstring texturePath)
{
	MD3Loader md3l;
	std::vector<MD3Material> md3Mats;

	md3l.LoadM3d(modelFile, m_vertices, m_indercies, m_subsets, md3Mats, m_skinnedMeshSkeleton);

	m_modelGeomatry.setVertices(d3dD, &m_vertices[0], m_vertices.size());
	m_modelGeomatry.SetIndices(d3dD, &m_indercies[0], m_vertices.size());
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

MD3Model::~MD3Model()
{
}

void MD3ModelInstance::update(float dt)
{
	timePoint += dt;
	theModel->m_skinnedMeshSkeleton.getFinalTransforms(currentAnimationClipName, timePoint, finalTransforms);
	if (timePoint > theModel->m_skinnedMeshSkeleton.GetClipEndTime(currentAnimationClipName))
	{
		timePoint = 0.0f;
	}
}