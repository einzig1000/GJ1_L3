#pragma once
#include <GameManager/Phase/IPhase.h>
#include <GameObject/Effect/Particle/Particle.h>


struct CollisionResult
{
	Vector2 position;
	Vector2 velocity;
};


class TestPhase :
	public IPhase
{
public:
	TestPhase();
	~TestPhase() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void DrawImGui() override;
	void ChangePhase(Phase phase) override { nextPhase_ = phase; }

private:
	// カメラID
	int32_t c_main_ = -1;
	// モデルID
	int32_t m_cube_ = -1;
	int32_t m_plane_ = -1;
	int32_t m_human_ = -1;
	int32_t m_table_ = -1;
	int32_t m_cocktail_ = -1;
	std::vector<int32_t> m_alcohol_;
	// テクスチャID
	int32_t t_uvChecker_ = -1;
	int32_t t_monsterBall_ = -1;
	int32_t t_table_ = -1;
	int32_t t_cocktail_ = -1;
	std::vector<int32_t> t_alcohol_;
	// アニメーションID
	int32_t a_sneakWalk_ = -1;
	// サウンドID
	int32_t s_aura_ = -1;


	void Initialize_SimpleModels();
	void Update_SimpleModels();
	void Draw_SimpleModels();
	void DrawImGui_SimpleModels();
	std::unique_ptr<RenderObject> simpleModels_;
	int32_t instanceCount_ = 10;
	// ディスクリプタヒープスロット
	int32_t worldMatrixHeapSlot_ = -1;
	int32_t colorHeapSlot_ = -1;
	int32_t textureIndexHeapSlot_ = -1;
	std::vector<EulerTransforms> transforms_;
	std::vector<Matrix4x4> worldMatrices_;
	std::vector<Vector4> colors_;
	std::vector<int32_t> textureIndices_;


	void Initialize_Sprite();
	void Update_Sprite();
	void Draw_Sprite();
	void DrawImGui_Sprite();
	std::unique_ptr<RenderObject> sprite_;
	EulerTransforms spriteTransform_;
	Vector4 spriteColor_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	int32_t spriteTexture_ = -1;


	void Initialize_Animation();
	void Update_Animation();
	void Draw_Animation();
	void DrawImGui_Animation();
	std::unique_ptr<RenderObject> animation_;
	std::unique_ptr<ComputeObject> animationCompute_;
	float animationTime_ = 0.0f;
	int32_t resultHeapSlot_ = -1;
	uint32_t numVertices_ = 0;
	SkinInstance skinInstance_;
	const ModelData* modelData_ = nullptr;

	struct Object
	{
		VectorDynamics translate = VectorDynamics{ Vector3{ 0.0f, 1.280f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f } };
		VectorDynamics rotate = VectorDynamics{ Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f } };
		VectorDynamics scale = VectorDynamics{ Vector3{ 1.0f, 1.0f, 1.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f } };

		// 円
		float radius = -1.0f;
		// 四角形
		Vector2 size = Vector2{ 0.0f, 0.0f };

		std::unique_ptr<RenderObject> renderObject;
	};


	void Initialize_Collider();
	void Update_Collider();
	void Draw_Collider();
	void DrawImGui_Collider();
	std::vector<Object> colliderObjects_;
	std::unique_ptr<RenderObject> table_;
	std::unique_ptr<RenderObject> cocktail_;
	VectorDynamics translate = VectorDynamics{ Vector3{ 1.6f, 1.280f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f } };
	VectorDynamics rotate = VectorDynamics{ Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f } };
	VectorDynamics scale = VectorDynamics{ Vector3{ 1.0f, 1.0f, 1.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f } };
	float radius = -1.0f;
	Vector2 hanasitatokinoruisekisokudo = Vector2{ 0.0f, 0.0f };



	Particle particle_;
};