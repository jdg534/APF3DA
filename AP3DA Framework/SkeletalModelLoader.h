#ifndef _SKELETAL_LOADER_H_
#define _SKELETAL_LOADER_H_

// this is based off source code found in the Frank luna book
#include <d3d11.h>

#include <vector>

#include "Structures.h"
#include "SkeletalModelStructs.h"



struct SkeletalModelMaterial
{
	SurfaceInfo materialLightValues;
	std::string diffuseMapFile;
};

class SkeletalModelLoader
{
public:
	bool LoadM3d(const std::string& filename,
		std::vector<SimpleVertex>& vertices,
		std::vector<unsigned int>& indices,
		std::vector<SkeletalModelSubSet>& subsets,
		std::vector<SkeletalModelMaterial>& mats,
		SkinnedMeshSkeleton & skinInfo);

private:
	void ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<SkeletalModelMaterial>& mats);
	void ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<SkeletalModelSubSet>& subsets);
	void ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<SimpleVertex>& vertices);
	void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<SimpleVertex>& vertices);
	void ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<unsigned int>& indices);
	void ReadBoneOffsets(std::ifstream& fin, UINT numBones, std::vector<DirectX::XMFLOAT4X4>& boneOffsets);
	void ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex);
	void ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips, std::map<std::string, AnimationClip>& animations);
	void ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation);
};


#endif