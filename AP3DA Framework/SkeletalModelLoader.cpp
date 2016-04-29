#include "SkeletalModelLoader.h"

#include <fstream>
#include <string>

bool SkeletalModelLoader::LoadM3d(const std::string& filename,
	std::vector<SimpleVertex>& vertices,
	std::vector<unsigned int>& indices,
	std::vector<SkeletalModelSubSet>& subsets,
	std::vector<SkeletalModelMaterial>& mats,
	SkinnedMeshSkeleton & skinInfo)
{
	std::ifstream md3File(filename);

	unsigned int nMaterials = 0,
		nVertices = 0,
		nTriangles = 0,
		nBones = 0,
		nAnimationClips = 0;

	std::string ignorStr = "";

	if (md3File.good())
	{
		// call the bellow loading functions
		md3File >> ignorStr; // ***************m3d-File-Header***************
		md3File >> ignorStr >> nMaterials;
		md3File >> ignorStr >> nVertices;
		md3File >> ignorStr >> nTriangles;
		md3File >> ignorStr >> nBones;
		md3File >> ignorStr >> nAnimationClips;

		std::vector<DirectX::XMFLOAT4X4> boneOffsets;
		std::vector<int> boneIndexToParentIndex;
		std::map<std::string, AnimationClip> animationClips;

		ReadMaterials(md3File, nMaterials, mats);
		ReadSubsetTable(md3File, nMaterials, subsets);
		ReadSkinnedVertices(md3File, nVertices, vertices);
		// now check to see if the bones on each vertex are in an ok range

		std::vector<unsigned int> badVertexIndercies;
		std::vector<BYTE> badWeightIndexValueForVert;

		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			for (unsigned int j = 0; j < 4; j++)
			{
				if (vertices[i].BoneIndices[j] >= nBones)
				{
					BYTE theValue = vertices[i].BoneIndices[j];
					int badVertexIndex = i;
					badVertexIndercies.push_back(badVertexIndex);
					badWeightIndexValueForVert.push_back(theValue);
				}
				
			}
			
		}
		ReadTriangles(md3File, nTriangles, indices);
		ReadBoneOffsets(md3File, nBones, boneOffsets);
		ReadBoneHierarchy(md3File, nBones, boneIndexToParentIndex);
		ReadAnimationClips(md3File, nBones, nAnimationClips, animationClips);

		skinInfo.set(boneIndexToParentIndex, boneOffsets, animationClips);

		return true;
	}
	return false;
}

void SkeletalModelLoader::ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<SkeletalModelMaterial>& mats)
{
	std::string ignorStr = "";
	mats.resize(numMaterials);

	std::string diffuseMapFileName;

	fin >> ignorStr; // ***************Materials*********************
	for (auto i = 0; i < numMaterials; i++)
	{
		fin >> ignorStr; // Ambient:
		fin >> mats[i].materialLightValues.AmbientMtrl.x >> mats[i].materialLightValues.AmbientMtrl.y >> mats[i].materialLightValues.AmbientMtrl.z;
		mats[i].materialLightValues.AmbientMtrl.w = 1.0f;
		fin >> ignorStr; // Diffuse:
		fin >> mats[i].materialLightValues.DiffuseMtrl.x >> mats[i].materialLightValues.DiffuseMtrl.y >> mats[i].materialLightValues.DiffuseMtrl.z;
		mats[i].materialLightValues.DiffuseMtrl.w = 1.0f;
		fin >> ignorStr;// Specular: 
		fin >> mats[i].materialLightValues.SpecularMtrl.x >> mats[i].materialLightValues.SpecularMtrl.y >> mats[i].materialLightValues.SpecularMtrl.z;
		mats[i].materialLightValues.SpecularMtrl.w = 1.0f;
		while (ignorStr != "DiffuseMap:")
		{
			fin >> ignorStr;
		}
		
		fin >> diffuseMapFileName;
		fin >> ignorStr >> ignorStr;// NormalMap: head_norm.dds
		mats[i].diffuseMapFile.resize(diffuseMapFileName.size(), ' ');
		std::copy(diffuseMapFileName.begin(), diffuseMapFileName.end(), mats[i].diffuseMapFile.begin());
	}
}
	
void SkeletalModelLoader::ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<SkeletalModelSubSet>& subsets)
{
	std::string skipStr = "";

	subsets.resize(numSubsets);

	fin >> skipStr; // ***************SubsetTable*******************

	for (auto i = 0; i < numSubsets; i++)
	{
		fin >> skipStr;// SubsetID:
		fin >> subsets[i].Id;
		fin >> skipStr;// VertexStart:
		fin >> subsets[i].VertexStart;
		fin >> skipStr;// VertexCount:
		fin >> subsets[i].VertexCount;
		fin >> skipStr;// FaceStart:
		fin >> subsets[i].FaceStart;
		fin >> skipStr;// FaceCount:
		fin >> subsets[i].FaceCount;
	}
}
	
void SkeletalModelLoader::ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<SimpleVertex>& vertices)
{
	std::string	skipStr = "";
	vertices.resize(numVertices);

	fin >> skipStr;// ***************Vertices**********************

	for (auto i = 0; i < numVertices; i++)
	{
		fin >> skipStr; // Position:
		fin >> vertices[i].PosL.x >> vertices[i].PosL.y >> vertices[i].PosL.z;
		std::getline(fin, skipStr);// Tangent: -0.257403 0.5351538 - 0.8045831 1
		fin >> skipStr;// Normal:
		fin >> vertices[i].NormL.x >> vertices[i].NormL.y >> vertices[i].NormL.z;
		fin >> skipStr;// Tex-Coords:
		fin >> vertices[i].Tex.x >> vertices[i].Tex.y;
	}
}
	
void SkeletalModelLoader::ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<SimpleVertex>& vertices)
{
	// call this one, not the above version
	std::string	skipStr = "";
	vertices.resize(numVertices);

	fin >> skipStr;// ***************Vertices**********************

	int boneIndices[4];
	float weights[4];

	for (auto i = 0; i < numVertices; i++)
	{
		fin >> skipStr; // Position:
		fin >> vertices[i].PosL.x >> vertices[i].PosL.y >> vertices[i].PosL.z;
		// Tangent: -0.257403 0.5351538 - 0.8045831 1
		fin >> skipStr >> skipStr >> skipStr >> skipStr >> skipStr;
		fin >> skipStr;// Normal:
		fin >> vertices[i].NormL.x >> vertices[i].NormL.y >> vertices[i].NormL.z;
		fin >> skipStr;// Tex-Coords:
		fin >> vertices[i].Tex.x >> vertices[i].Tex.y;

		fin >> skipStr;// BlendWeights:
		fin >> weights[0] >> weights[1] >> weights[2] >> weights[3];

		fin >> skipStr;// BlendIndices:
		fin >> boneIndices[0] >> boneIndices[1] >> boneIndices[2] >> boneIndices[3];

		vertices[i].Weights.x = weights[0];
		vertices[i].Weights.y = weights[1];
		vertices[i].Weights.z = weights[2];

		vertices[i].BoneIndices[0] = (BYTE)boneIndices[0];
		vertices[i].BoneIndices[1] = (BYTE)boneIndices[1];
		vertices[i].BoneIndices[2] = (BYTE)boneIndices[2];
		vertices[i].BoneIndices[3] = (BYTE)boneIndices[3];
	}
}
	
void SkeletalModelLoader::ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<unsigned int>& indices)
{
	std::string skipStr = "";
	indices.resize(numTriangles * 3);

	fin >> skipStr; // ***************Triangles*********************
	for (auto i = 0; i < numTriangles; i++)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
}
	
void SkeletalModelLoader::ReadBoneOffsets(std::ifstream& fin, UINT numBones, std::vector<DirectX::XMFLOAT4X4>& boneOffsets)
{
	std::string skipStr = "";
	boneOffsets.resize(numBones);

	fin >> skipStr;// ***************BoneOffsets*******************
	for (unsigned int i = 0; i < numBones; i++)
	{
		DirectX::XMStoreFloat4x4(&boneOffsets[i], DirectX::XMMatrixIdentity());

		fin >> skipStr;
		/*
		fin >> boneOffsets[i]._11 >> boneOffsets[i]._12 >> boneOffsets[i]._13 >> boneOffsets[i]._14;
		fin >> boneOffsets[i]._21 >> boneOffsets[i]._22 >> boneOffsets[i]._23 >> boneOffsets[i]._24;
		fin >> boneOffsets[i]._31 >> boneOffsets[i]._32 >> boneOffsets[i]._33 >> boneOffsets[i]._34;
		fin >> boneOffsets[i]._41 >> boneOffsets[i]._42 >> boneOffsets[i]._43 >> boneOffsets[i]._44;
		*/

		fin >>
			boneOffsets[i](0, 0) >> boneOffsets[i](0, 1) >> boneOffsets[i](0, 2) >> boneOffsets[i](0, 3) >>
			boneOffsets[i](1, 0) >> boneOffsets[i](1, 1) >> boneOffsets[i](1, 2) >> boneOffsets[i](1, 3) >>
			boneOffsets[i](2, 0) >> boneOffsets[i](2, 1) >> boneOffsets[i](2, 2) >> boneOffsets[i](2, 3) >>
			boneOffsets[i](3, 0) >> boneOffsets[i](3, 1) >> boneOffsets[i](3, 2) >> boneOffsets[i](3, 3);
		
	}
}

void SkeletalModelLoader::ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex)
{
	std::string junk = "";
	fin >> junk; // ***************BoneHierarchy*****************

	boneIndexToParentIndex.resize(numBones);

	for (auto i = 0; i < numBones; i++)
	{
		fin >> junk; // ParentIndexOfBone<Nth bone>:
		fin >> boneIndexToParentIndex[i];
	}
}
	
void SkeletalModelLoader::ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips, std::map<std::string, AnimationClip>& animations)
{
	std::string skipStr = "";
	fin >> skipStr; // ***************AnimationClips****************
	

	for (auto NthClip = 0; NthClip < numAnimationClips; NthClip++)
	{
		std::string clipName = "";
		fin >> skipStr; // "AnimationClip"
		fin >> clipName;
		fin >> skipStr; // {

		AnimationClip ac;
		ac.m_boneAnimations.resize(numBones);

		for (auto boneIndex = 0; boneIndex < numBones; boneIndex++)
		{
			ReadBoneKeyframes(fin, numBones, ac.m_boneAnimations[boneIndex]);
		}
		fin >> skipStr; // }

		animations[clipName] = ac;
	}
}
	
void SkeletalModelLoader::ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation)
{
	std::string junk = "";

	unsigned int nKeyFrames = 0;
	fin >> junk >> junk >> nKeyFrames; // Bone0 #Keyframes: <num key frames here>
	fin >> junk; // {
	boneAnimation.Keyframes.resize(nKeyFrames);

	for (auto i = 0; i < nKeyFrames; i++)
	{
		float t = 0.0f;
		DirectX::XMFLOAT3 trans(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 scale(1.0f, 1.0f, 1.0f);
		DirectX::XMFLOAT4 rotQuat(0.0f, 0.0f, 0.0f, 1.0f);
		fin >> junk; // Time:
		fin >> t;
		fin >> junk; // Pos:
		fin >> trans.x >> trans.y >> trans.z;
		fin >> junk; // Scale:
		fin >> scale.x >> scale.y >> scale.z;
		fin >> junk; // Quat:
		fin >> rotQuat.x >> rotQuat.y >> rotQuat.z >> rotQuat.w;

		boneAnimation.Keyframes[i].TimePos = t;
		boneAnimation.Keyframes[i].Translation = trans;
		boneAnimation.Keyframes[i].Scale = scale;
		boneAnimation.Keyframes[i].RotationQuaternion = rotQuat;
	}

	fin >> junk; // }
}