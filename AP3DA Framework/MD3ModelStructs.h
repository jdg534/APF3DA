#ifndef _MD3_MODEL_STRUCTS_H_
#define _MD3_MODEL_STRUCTS_H_

#include <d3d11.h>
#include <DirectXMath.h>

#include <vector>
#include <map>

// the following is largly based off:
// Frank Luna's "Introduction to 3D Game Programming with Directx 11"

struct KeyFrame
{
	KeyFrame()
	{
		TimePos = 0.0f;
		Translation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		Scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		RotationQuaternion = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	float TimePos;
	DirectX::XMFLOAT3 Translation;
	DirectX::XMFLOAT3 Scale;
	DirectX::XMFLOAT4 RotationQuaternion;
};

struct BoneAnimation
{
	float GetStartTime();
	float GetEndTime();
	void Interpolate(float t, DirectX::XMFLOAT4X4& M);
	std::vector<KeyFrame> Keyframes;
};

struct AnimationClip
{
	float getClipStartTime();

	float getClipEndTime();
	void interpolate(float t, std::vector<DirectX::XMFLOAT4X4> & boneTransforms);
	std::vector<BoneAnimation> boneAnimations;
};

// now the skinned data class
class SkinnedMeshSkeleton
{
public:
	unsigned int getBoneCount();

	float GetClipStartTime(const std::string& clipName);
	float GetClipEndTime(const std::string& clipName);

	void set(std::vector<int> & boneHierarchy,
		std::vector<DirectX::XMFLOAT4X4>& boneOffsets,
		std::map<std::string, AnimationClip>& animations);

	void getFinalTransforms(const std::string & animationClipName, float timePoint, std::vector<DirectX::XMFLOAT4X4> & finalTransforms);

private:
	std::vector<int> m_boneHierarchy;

	std::vector<DirectX::XMFLOAT4X4> m_boneOffsets;

	std::map<std::string, AnimationClip> m_animations;
};



struct MD3ModelSubSet
{
	MD3ModelSubSet() :
		Id(-1),
		VertexStart(0), VertexCount(0),
		FaceStart(0), FaceCount(0)
	{
	}

	unsigned int Id;
	unsigned int VertexStart;
	unsigned int VertexCount;
	unsigned int FaceStart;
	unsigned int FaceCount;
};



#endif