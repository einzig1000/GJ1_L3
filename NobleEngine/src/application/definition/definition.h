#pragma once
#include <EngineDefinition/EngineDefinition.h>
#include <EngineDefinition/EngineConstexprs.h>
#include <definition/constexprs.h>
#include <string>
#include <string_view>
#include <stdexcept>
#include <externals/MagicEnum/magic_enum.hpp>


enum class CameraPhase
{
	// 盤面確認中。Theta/Phiともにマウス操作可能
	Free,
	// 射出角度調整中。Phiロック状態。Thetaはマウス操作可能
	ShotAngleSetup,
	// グラススライド中。Theta/Phiともにロック状態。
	GlassFollowing,
	// グラスキャッチ中。Theta/Phiともにロック状態。
	CatchFollowing,

};

// ゲームのフェーズ
enum class Phase
{
    Phase_None,
    Phase_Test,
    Phase_Title,
    Phase_GameScene,
	Phase_SikouteiDevelop,
    Phase_Result,
	Phase_Tutorial,

	Phase_Max,
};


enum class GlassType
{
    Glass,
    Bottle,
    Champagne,
    Gin,
    JapaneseSake,
    Plumwine,
    Whiskey,
    GLASS_MAX,
};
