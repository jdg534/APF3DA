#ifndef _MD3_MODEL_STRUCTS_H_
#define _MD3_MODEL_STRUCTS_H_

#include <d3d11.h>
#include <DirectXMath.h>

#include <vector>

struct KeyFrame
{
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

#endif