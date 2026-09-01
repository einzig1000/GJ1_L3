#include "TitlePhase.h"
#include <Utilities/Json/JsonManager.h>
#include <Utilities/functions.h>
#include <externals/MagicEnum/magic_enum.hpp>


TitlePhase::TitlePhase()
{
}

TitlePhase::~TitlePhase()
{}

void TitlePhase::Initialize()
{
	// フェーズ初期化
	nextPhase_ = Phase::Phase_None;
}


void TitlePhase::Update()
{
}


void TitlePhase::Draw()
{
}


void TitlePhase::DrawImGui()
{
}
