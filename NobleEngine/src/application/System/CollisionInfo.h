#pragma once
#include "../../engine/EngineDefinition/EngineDefinition.h"

struct PhysicsBody {
	//質量
	float mass = 1.0f;
	//速度
	Vector3 velocity = { 0.0f };
	//デフォルトe = 1
	float coefficiendOfRestituion = 1.0f;

	Vector3 penetration;//めり込み量
};