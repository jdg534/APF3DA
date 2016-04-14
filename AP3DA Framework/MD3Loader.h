#ifndef _MD3_LOADER_H_
#define _MD3_LOADER_H_

// this is based off source code found in the Frank luna book
#include <d3d11.h>

#include <vector>

#include "Structures.h"
#include "MD3ModelStructs.h"



struct MD3Material
{
	SurfaceInfo materialLightValues;
	std::string diffuseMapFile;
};

class MD3Loader
{
public:
	bool LoadM3d(const std::string& filename,
		std::vector<SimpleVertex>& vertices,
		std::vector<unsigned int>& indices,
		std::vector<MD3ModelSubSet>& subsets,
		std::vector<MD3Material>& mats,
		SkinnedMeshSkeleton & skinInfo);

private:
	void ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<MD3Material>& mats);
	void ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<MD3ModelSubSet>& subsets);
	void ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<SimpleVertex>& vertices);
	void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<SimpleVertex>& vertices);
	void ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<unsigned int>& indices);
	void ReadBoneOffsets(std::ifstream& fin, UINT numBones, std::vector<DirectX::XMFLOAT4X4>& boneOffsets);
	void ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex);
	void ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips, std::map<std::string, AnimationClip>& animations);
	void ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation);
};


#endif