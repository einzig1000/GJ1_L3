#pragma once
#include "../../engine/EngineDefinition/EngineDefinition.h"

struct CollisionInfo {
	bool collided;
	Vector3 normal;//法線
	float penetration;//めり込み量
};
