#pragma once
#include <EngineDefinition/EngineDefinition.h>
#include <EngineDefinition/EngineConstexprs.h>
#include <definition/constexprs.h>
#include <string>
#include <string_view>
#include <stdexcept>
#include <externals/MagicEnum/magic_enum.hpp>


// ゲームのフェーズ
enum class Phase
{
    Phase_None,
    Phase_Test,
    Phase_Title,
    Phase_GameScene,
	Phase_SikouteiDevelop,
	//仮に衝突テストフェーズを作成する
	Phase_CollisionTest,
    Phase_Result,

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
