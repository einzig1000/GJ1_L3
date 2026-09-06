#pragma once

#include <cstdint>
#include "../CollisionTag/CollisionTag.h"
#include "../../../engine/EngineDefinition/EngineDefinition.h"
#include"../CollisionInfo.h"
#include"Game.h"

class Camera;

namespace ParentMatrix {
	Vector3 GetWorldTransformByMatrix(const Matrix4x4& mat);
	//複合的な親子関係は対応していない
	Vector3 GetParentScaleByMatrix(const Matrix4x4& mat);
}


/// @brief 衝突判定オブジェクト
class Collider {
public:

	enum ColliderType {
		kColliderType_Sphere,
		kColliderType_AABB,
		//XZ平面上の円
		kColliderType_XZ_Circle,
		//OBB対応は必要に迫られたらやる
		//kColliderType_OBB,
		kColliderType_Max
	};

private:

	Sphere sphere_ = { .center = {0.0f},.radius = {1.0f} };
	AABB aabb_;

	uint32_t collisionAttribute_ = 0xffffffff;	// 衝突属性
	uint32_t collisionMask_ = 0xffffffff;		// 衝突マスク

	ColliderType type_ = ColliderType::kColliderType_Sphere;
	//CollisionInfo collisionInfo_;
	//速度と質量が入っている。
	PhysicsBody physicsBody_;
	Matrix4x4* parentWorldMat_ = nullptr;

	EulerTransforms tempWorldTransform_ = {0.0f};

	bool isCalculatedThisFrame_ = false; // 今フレーム計算済みかどうかのフラグ


#ifdef _DEBUG

	//=======================================
	//==========コライダーデバック表示==========
	//=======================================

	bool isDrawCollider_ = true;
	// 描画オブジェクト
	std::unique_ptr<RenderObject> colliderObj_;
	Vector4 colliderColor_ = { 0.0f };

	//=======================================
#endif

public:
	static void Load();

	Collider();

	~Collider();

	void InitCalcuatedTisFrameFlag();
	/// @brief 衝突時コールバック関数
	virtual void OnCollision(Collider* collider) {
		(void)collider;
		//コライダーを赤くする
		OnCollisionCollider();
	};

	/// @brief ワールドトランスフォームを取得する関数の作成
	/// @return 
	const EulerTransforms& CalculateWorldTransform();

	/// @brief 球を設定する
	/// @param sphere 球
	/// @param isCircleXZ 円にするかどうか
	void SetSphere(const Sphere& sphere,bool isCircleXZ = true) {

		type_ = isCircleXZ ? ColliderType::kColliderType_XZ_Circle : ColliderType::kColliderType_Sphere;
		sphere_ = sphere;
		
		ResetColliderType(type_);
	};

	const Sphere& GetSphere() {
		return sphere_;
	}

	/// @brief ワールド行列のアドレスを設定する
	/// @param worldMat 
	void SetWorldMatrixAddress(Matrix4x4& worldMat) {
		assert(&worldMat);
		parentWorldMat_ = &worldMat;
	};

	const AABB& GetAABB() const { return aabb_; }
	/// @brief AABBをセットする
	/// @param aabb 
	void SetAABB(const AABB& aabb) {
		type_ = ColliderType::kColliderType_AABB;
		aabb_ = aabb;

		ResetColliderType(type_);
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
	//void SetCollisionInfo(const CollisionInfo& info) { collisionInfo_ = info; };
	//CollisionInfo& GetCollisionInfo() {
	//	return collisionInfo_;
	//}

	void SetMass(const float mass) { physicsBody_.mass = mass; };

	void SetVelocity(const Vector3 velocity) { physicsBody_.velocity = velocity; }
	
	const PhysicsBody GetPhysicsBody() { return physicsBody_; }

	//=======================================
	//==========コライダーデバック表示==========
	//=======================================

	void SetIsDrawCollider(const bool flag);
	void Update(const int32_t cameraID);
	void Draw();
	private:
	void ResetColliderType(const ColliderType& type);
	//=======================================
};

