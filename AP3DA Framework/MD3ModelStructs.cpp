#include "MD3ModelStructs.h"

float BoneAnimation::GetStartTime()
{
	return Keyframes[0].TimePos;
}

float BoneAnimation::GetEndTime()
{
	return Keyframes[Keyframes.size() - 1].TimePos;
}

void BoneAnimation::Interpolate(float t, DirectX::XMFLOAT4X4& M)
{
	using namespace DirectX;
	if (t <= Keyframes.front().TimePos) // the time point is before the first frame, (return first frame)
	{
		XMVECTOR scale = XMLoadFloat3(&Keyframes.front().Scale);
		XMVECTOR trans = XMLoadFloat3(&Keyframes.front().Translation);
		XMVECTOR rotationQuat = XMLoadFloat4(&Keyframes.front().RotationQuaternion);

		XMVECTOR zeroIn4D = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(scale, zeroIn4D, rotationQuat, trans));
	}
	else if (t >= Keyframes.back().TimePos) // gone part the final frame, so just returnt he final frame
	{
		XMVECTOR scale = XMLoadFloat3(&Keyframes.back().Scale);
		XMVECTOR trans = XMLoadFloat3(&Keyframes.back().Translation);
		XMVECTOR rotationQuat = XMLoadFloat4(&Keyframes.back().RotationQuaternion);

		XMVECTOR zeroIn4D = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(scale, zeroIn4D, rotationQuat, trans));
	}
	else
	{
		// need to lerp between keyframes
		for (auto i = 0; i < Keyframes.size() - 1; i++)
		{
			if (t >= Keyframes[i].TimePos &&
				t <= Keyframes[i + 1].TimePos)
			{
				float lerpVal = (t - Keyframes[i].TimePos) /
					(Keyframes[i + 1].TimePos - Keyframes[i].TimePos);

				XMVECTOR keyFrameAScale = XMLoadFloat3(&Keyframes[i].Scale);
				XMVECTOR keyFrameBScale = XMLoadFloat3(&Keyframes[i + 1].Scale);

				XMVECTOR keyFrameATrans = XMLoadFloat3(&Keyframes[i].Translation);
				XMVECTOR keyFrameBTrans = XMLoadFloat3(&Keyframes[i + 1].Translation);

				XMVECTOR keyFrameARotationQuat = XMLoadFloat4(&Keyframes[i].RotationQuaternion);
				XMVECTOR keyFrameBRotationQuat = XMLoadFloat4(&Keyframes[i + 1].RotationQuaternion);

				XMVECTOR lerpedScale = XMVectorLerp(keyFrameAScale, keyFrameBScale, lerpVal);
				XMVECTOR lerpedTrans = XMVectorLerp(keyFrameATrans, keyFrameBTrans, lerpVal);
				XMVECTOR slerpedRotationQuat = XMQuaternionSlerp(keyFrameARotationQuat, keyFrameBRotationQuat, lerpVal);

				XMVECTOR identityQuatForMul = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

				XMStoreFloat4x4(&M, XMMatrixAffineTransformation(lerpedScale, identityQuatForMul, slerpedRotationQuat, lerpedTrans));

				break; // leave the loop one lerped between the keyframes
			}
		}
	}
}

float AnimationClip::getClipStartTime()
{
	float rv = FLT_MAX; // FLT_MAX = max value a float can represent
	for (auto i = 0; i < boneAnimations.size(); i++)
	{
		float baStart = boneAnimations[i].GetStartTime();
		if (rv > baStart)
		{
			rv = baStart;
		}
	}
	return rv;
}

float AnimationClip::getClipEndTime()
{
	float rv = FLT_MIN;
	for (auto i = 0; i < boneAnimations.size(); i++)
	{
		float baEnd = boneAnimations[i].GetEndTime();
		if (rv < baEnd)
		{
			rv = baEnd;
		}
	}
	return rv;
}

void AnimationClip::interpolate(float t, std::vector<DirectX::XMFLOAT4X4> & boneTransforms)
{
	for (auto i = 0; i < boneAnimations.size(); i++)
	{
		boneAnimations[i].Interpolate(t, boneTransforms[i]);
	}
}