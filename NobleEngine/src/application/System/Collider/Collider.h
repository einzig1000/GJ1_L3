#pragma once

#include <cstdint>
#include "../CollisionTag/CollisionTag.h"
#include "../../../engine/EngineDefinition/EngineDefinition.h"
#include"../CollisionInfo.h"

class Camera;

namespace ParentMatrix {
	Vector3 GetWorldTransformByMatrix(const Matrix4x4& mat);
}

/// @brief 衝突判定オブジェクト
class Collider {
public:

	enum ColliderType {
		kColliderType_Sphere,
		kColliderType_AABB,
		//OBB対応は必要に迫られたらやる
		kColliderType_OBB,
		kColliderType_Max
	};

private:
	float radius_ = 1.0f;	// 衝突半径

	AABB aabb_;

	uint32_t collisionAttribute_ = 0xffffffff;	// 衝突属性
	uint32_t collisionMask_ = 0xffffffff;		// 衝突マスク

	ColliderType type_ = ColliderType::kColliderType_Sphere;
	CollisionInfo collisionInfo_;
	
	//新しく中心と座標を追加
	Vector3 center_ = {0.0f,0.0f,0.0f};
	Matrix4x4* worldMat_ = nullptr;

	Vector3 tempWorldTransform_ = {0.0f};

	bool isCalculatedThisFrame_ = false; // 今フレーム計算済みかどうかのフラグ

public:
	Collider();

	virtual ~Collider();

	void InitCalcuatedTisFrameFlag();
	/// @brief 衝突時コールバック関数
	virtual void OnCollision(Collider* collider) {
		(void)collider;
		//コライダーを赤くする
		OnCollisionCollider();
	};

	/// @brief ワールド座標を取得する関数の作成
	/// @return 
	const Vector3& CalculateWorldPos();
	/// @brief 中心点を設定する
	/// @param center 
	void SetCenter(const Vector3& center) {
		center_ = center;
	};
	/// @brief 中心を取得する
	/// @return 中心
	const Vector3& GetCenter() {
		return center_;
	}

	/// @brief ワールド行列のアドレスを設定する
	/// @param worldMat 
	void SetWorldMatrixAddress(Matrix4x4& worldMat) {
		assert(&worldMat);
		worldMat_ = &worldMat;
	};

	/// @brief 衝突半径を取得する
	/// @return 衝突半径
	float GetRadius() const { return radius_; }

	/// @brief 衝突半径を設定する
	/// @param radius 衝突半径
	void SetRadius(float radius) { 
		type_ = ColliderType::kColliderType_Sphere;
		radius_ = radius; }

	const AABB& GetAABB() const { return aabb_; }

	void SetAABB(const AABB& aabb) {
		type_ = ColliderType::kColliderType_AABB;
		aabb_ = aabb;
	};

	ColliderType GetType() const { return type_; }
	void SetType(const ColliderType& type) { type_ = type; }
	/// @brief 衝突属性を取得する
	/// @return 衝突属性
	uint32_t GetCollisionAttribute() const { return collisionAttribute_; }

	/// @brief 衝突属性を設定する
	/// @param attribute 衝突属性
	void SetCollisionAttribute(uint32_t attribute) { collisionAttribute_ = attribute; }

	/// @brief 衝突マスクを取得する
	/// @return 衝突マスク
	uint32_t GetCollisionMask() const { return collisionMask_; }

	/// @brief 衝突マスクを設定する
	/// @param mask 衝突マスク
	void SetCollisionMask(uint32_t mask) { collisionMask_ = mask; }

	void OnCollisionCollider();
	void SetCollisionInfo(const CollisionInfo& info) { collisionInfo_ = info; };
	CollisionInfo& GetCollisionInfo() {
		return collisionInfo_;
	}

	void ColliderUpdate();
};

