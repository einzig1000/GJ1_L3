#include "AnimationComputer.h"
#include <AssetManager/Animation/AnimationBank/AnimationBank.h>
#include <Utilities/Logger/Logger.h>

namespace
{
	Vector3 CalculateValue(float time, const AnimationCurve<Vector3>& curve)
	{
		assert(!curve.keyFrames.empty());

		if (curve.keyFrames.size() == 1 || time <= curve.keyFrames[0].time)
			return curve.keyFrames[0].value;

		for (size_t i = 0; i < curve.keyFrames.size() - 1; ++i)
		{
			const auto& k0 = curve.keyFrames[i];
			const auto& k1 = curve.keyFrames[i + 1];

			if (k0.time <= time && time <= k1.time)
			{
				float t = (time - k0.time) / (k1.time - k0.time);
				return k0.value * (1.0f - t) + k1.value * t;
			}
		}

		return curve.keyFrames.back().value;
	}

	Quaternion CalculateValue(float time, const AnimationCurve<Quaternion>& curve)
	{
		assert(!curve.keyFrames.empty());

		if (curve.keyFrames.size() == 1 || time <= curve.keyFrames[0].time)
			return curve.keyFrames[0].value;

		for (size_t i = 0; i < curve.keyFrames.size() - 1; ++i)
		{
			const auto& k0 = curve.keyFrames[i];
			const auto& k1 = curve.keyFrames[i + 1];

			if (k0.time <= time && time <= k1.time)
			{
				float t = (time - k0.time) / (k1.time - k0.time);
				return Quaternion::Slerp(k0.value, k1.value, t);
			}
		}

		return curve.keyFrames.back().value;
	}
}


AnimationComputer::AnimationComputer(AnimationBank* bank)
	: bank_(bank)
{}

AnimationComputer::~AnimationComputer()
{}

// AnimationComputer.cpp
void AnimationComputer::ComputeAnimationData(int32_t animationID, SkinInstance& skin, const SkinBindData& bind, float& time)
{
	AnimationData* animationData = bank_->GetAnimationData(animationID);
	time = fmod(time, animationData->duration);
	ApplyAnimation(skin.skeleton, *animationData, time);
	UpdateSkeleton(skin.skeleton);
	UpdatePalette(skin.skeleton, bind, skin.palette);
}

void AnimationComputer::UpdatePalette(const Skeleton& skeleton, const SkinBindData& bind, std::vector<WellForGPU>& palette)
{
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex)
	{
		assert(jointIndex < bind.inverseBindPoseMatrices.size());

		const Matrix4x4 skeletonSpace = bind.inverseBindPoseMatrices[jointIndex] * skeleton.joints[jointIndex].skeletonSpaceMatrix;

		palette[jointIndex].skeletonSpaceMatrix = skeletonSpace;
		palette[jointIndex].skeletonSpaceInverseTransposeMatrix = skeletonSpace.Inverse().Transpose();
	}
}

void AnimationComputer::ApplyAnimation(Skeleton& skeleton, const AnimationData& animation, float time)
{
	for (Joint& joint : skeleton.joints)
	{
		if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end())
		{
			const NodeAnimation& nodeAnimation = it->second;
			joint.transform.translate = CalculateValue(time, nodeAnimation.translate);
			joint.transform.rotate = CalculateValue(time, nodeAnimation.rotate);
			joint.transform.scale = CalculateValue(time, nodeAnimation.scale);
		}
	}
}

void AnimationComputer::UpdateSkeleton(Skeleton& skeleton)
{
	for (Joint& joint : skeleton.joints)
	{
		joint.localMatrix = Matrix4x4::MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		if (joint.parentIndex.has_value())
		{
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[joint.parentIndex.value()].skeletonSpaceMatrix;
		}
		else
		{
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}