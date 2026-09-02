#pragma once
#include <EngineDefinition/EngineDefinition.h>

class AnimationBank;

class AnimationComputer
{
public:
	AnimationComputer(AnimationBank* bank);
	~AnimationComputer();

	void ComputeAnimationData(int32_t animationID, SkinInstance& skin, const SkinBindData& bind, float& time);

private:

	AnimationBank* bank_;

	// 1,骨ごとのlocal情報を更新し
	void ApplyAnimation(Skeleton& skeleton, const AnimationData& animation, float time);

	// 2,骨ごとのlocal情報からSkeltonSpaceの情報を更新する
	void UpdateSkeleton(Skeleton& skeleton);

	// 3,SkeltonSpaceの情報からSkinClusterの情報を更新する
	void UpdatePalette(const Skeleton& skeleton, const SkinBindData& bind, std::vector<WellForGPU>& palette);
};

