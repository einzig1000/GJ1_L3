#include "GlassParticle.h"
#include"../GameObject/Effect/Particle/Particle.h"

GlassParticle::GlassParticle()
{
    textureID_ = Game::Asset::Texture::Load("assets/engine/texture/particle/circle2.png");
	Load();

	emitterSphere_.count = 1;
	emitterSphere_.frequency = 0.5f;
	emitterSphere_.frequencyTime = 0.0f;
	emitterSphere_.translate = { 0.0f, 0.0f, 0.0f };
	emitterSphere_.radius = 0.1f;
	emitterSphere_.emit = 0;

	emitterSphere_.speedRange = 5.0f;
	emitterSphere_.reflectDirection = {0.0f,0.0f,0.0f};
	
	hitPosition_.tableCenter = {0.0f,1.0f,0.0f};//テーブルセンター
	hitPosition_.tableRadius = 10.0f;//テーブル半径
	hitPosition_.tableThickness = 0.0625f;//テーブルの高さ
	hitPosition_.floorHeight = 0.0f;//床の高さ
	hitPosition_.pieceRadius = 0.02f;//破片の半径
	hitPosition_.coefficiendOfRestituion = 0.8f;//反発係数
	hitPosition_.pieceMass = 0.05f;//破片の質量 
	particles_.resize(maxParticle_);
	initializeComputes_.resize(maxParticle_);
	emitComputes_.resize(maxParticle_);
	updateComputes_.resize(maxParticle_);

	for (int i = 0; i < maxParticle_; ++i) {

		particleSRVIDs_.push_back(Game::Resource::CreateCompute(sizeof(ParticleData), 1024));
		freeListIndexSRVIDs_.push_back(Game::Resource::CreateCompute(sizeof(uint32_t), 1024));
		freeListSRVIDs_.push_back(Game::Resource::CreateCompute(sizeof(uint32_t), 1024));

		particles_[i] = std::make_unique<RenderObject>();
		particles_[i]->modelID_ = modelIDs_.at(i);
		particles_[i]->psoConfig_.ps = "assets/shaders/Particle/Particle.PS.hlsl";
		particles_[i]->psoConfig_.vs = "assets/shaders/Particle/Particle.VS.hlsl";
		particles_[i]->SetupFromShaders();
		particles_[i]->instanceNum_ = 1024;

		initializeComputes_[i] = std::make_unique<ComputeObject>();
		initializeComputes_[i]->psoConfig_.cs = "assets/shaders/Particle/InitializeParticle.CS.hlsl";
		initializeComputes_[i]->SetupFromShaders();
		initializeComputes_[i]->SetUAVData(0, Game::Resource::GetUAV(particleSRVIDs_[i]));
		initializeComputes_[i]->SetUAVData(1, Game::Resource::GetUAV(freeListIndexSRVIDs_[i]));
		initializeComputes_[i]->SetUAVData(2, Game::Resource::GetUAV(freeListSRVIDs_[i]));
		initializeComputes_[i]->RegisterOutput(particleSRVIDs_[i]);
		initializeComputes_[i]->RegisterOutput(freeListIndexSRVIDs_[i]);
		initializeComputes_[i]->RegisterOutput(freeListSRVIDs_[i]);
		initializeComputes_[i]->Dispatch();

		emitComputes_[i] = std::make_unique<ComputeObject>();
		emitComputes_[i]->psoConfig_.cs = "assets/shaders/Particle/EmitGlassParticle.CS.hlsl";
		emitComputes_[i]->SetupFromShaders();
		emitComputes_[i]->RegisterOutput(particleSRVIDs_[i]);
		emitComputes_[i]->RegisterOutput(freeListIndexSRVIDs_[i]);
		emitComputes_[i]->RegisterOutput(freeListSRVIDs_[i]);

		updateComputes_[i] = std::make_unique<ComputeObject>();
		updateComputes_[i]->psoConfig_.cs = "assets/shaders/Particle/UpdateGlassParticle.CS.hlsl";
		updateComputes_[i]->SetupFromShaders();
		updateComputes_[i]->RegisterOutput(particleSRVIDs_[i]);
		updateComputes_[i]->RegisterOutput(freeListIndexSRVIDs_[i]);
		updateComputes_[i]->RegisterOutput(freeListSRVIDs_[i]);
	}
}

GlassParticle::~GlassParticle()
{
}

void GlassParticle::Initialize()
{
}



void GlassParticle::Update(int32_t cameraID)
{

	const float deltaTime = Game::Time::GetScaledDeltaTimeMs()*0.001f;
	//emitterSphere_.frequencyTime += deltaTime;
	//if (emitterSphere_.frequency <= emitterSphere_.frequencyTime)
	//{
	//	emitterSphere_.frequencyTime -= emitterSphere_.frequency;
	//	emitterSphere_.emit = 1;
	//} else
	//{
	//	emitterSphere_.emit = 0;
	//}

	for (int i = 0; i < maxParticle_; ++i) {
		emitComputes_[i]->SetUAVData(0, Game::Resource::GetUAV(particleSRVIDs_[i]));
		emitComputes_[i]->SetUAVData(1, Game::Resource::GetUAV(freeListIndexSRVIDs_[i]));
		emitComputes_[i]->SetUAVData(2, Game::Resource::GetUAV(freeListSRVIDs_[i]));
		emitComputes_[i]->SetCBufferData(0, &emitterSphere_);

		Vector3 rand = { Game::Math::Rand::RandFloat(-1.0f, 1.0f, 1), Game::Math::Rand::RandFloat(-1.0f, 1.0f, 1), Game::Math::Rand::RandFloat(-1.0f, 1.0f, 1) };
		emitComputes_[i]->SetCBufferData(1, &rand);

		updateComputes_[i]->SetUAVData(0, Game::Resource::GetUAV(particleSRVIDs_[i]));
		updateComputes_[i]->SetUAVData(1, Game::Resource::GetUAV(freeListIndexSRVIDs_[i]));
		updateComputes_[i]->SetUAVData(2, Game::Resource::GetUAV(freeListSRVIDs_[i]));
		updateComputes_[i]->SetCBufferData(0, &deltaTime);
		//ヒット座標をセットする
		updateComputes_[i]->SetCBufferData(1, &hitPosition_);

	}

	Matrix4x4x2 perView;
	perView.m1 = Game::Camera::Getter::GetViewProjectionMatrix(cameraID);
	perView.m2 = Matrix4x4::MakeIdentity4x4();

	for (int i = 0; i < maxParticle_; ++i) {
	
		particles_[i]->SetSBufferData(0, ShaderType::VertexShader, Game::Resource::GetSRV(particleSRVIDs_[i]));
		particles_[i]->SetCBufferData(0, ShaderType::VertexShader, &perView);
	}

}

void GlassParticle::Draw()
{
	for (int i = 0; i < maxParticle_; ++i) {
		emitComputes_[i]->Dispatch();
		updateComputes_[i]->Dispatch();
		particles_[i]->Draw();
	}
}

void GlassParticle::SetEmitterPos(Vector3 pos)
{
	emitterSphere_.translate = pos;
}

void GlassParticle::DebugImGui()
{
	ImGui::Begin("System");

	if (ImGui::TreeNode("GlassParticle")) {
		// 位置座標 (3Dベクター)
		ImGui::DragFloat3("Translate", &emitterSphere_.translate.x, 0.01f);

		// スカラーパラメータ
		ImGui::SliderFloat("Radius", &emitterSphere_.radius, 0.0f, 5.0f);

		int count = emitterSphere_.count;
		ImGui::SliderInt("Count", &count, 1, 100);
		emitterSphere_.count = count;

		ImGui::SliderFloat("Frequency", &emitterSphere_.frequency, 0.0f, 5.0f);

		ImGui::Separator(); // パラメータの区切り線

		// 物理・移動力
		ImGui::SliderFloat("Speed Range", &emitterSphere_.speedRange, 0.0f, 10.0f);
		ImGui::SliderFloat3("reflectDirection", &emitterSphere_.reflectDirection.x, -10.0f, 10.0f);

		ImGui::Separator();

		// 1回発生させるトリガーボタンの例
		if (ImGui::Button("Emit Force")) {
			emitterSphere_.emit = 1;
		}
		
		if (ImGui::TreeNode("Hit Position")) {
			// テーブルの中心座標 (3Dベクター)
			ImGui::DragFloat3("Table Center", &hitPosition_.tableCenter.x, 0.01f);

			// テーブル・床の形状パラメータ
			ImGui::SliderFloat("Table Radius", &hitPosition_.tableRadius, 0.0f, 10.0f);
			ImGui::SliderFloat("Table Thickness", &hitPosition_.tableThickness,0.0f, 10.0f);
			ImGui::SliderFloat("Floor Height", &hitPosition_.floorHeight, -5.0f, 10.0f);

			// 破片・物理パラメータ
			ImGui::SliderFloat("Piece Radius", &hitPosition_.pieceRadius, 0.001f, 1.0f);
			ImGui::SliderFloat("Piece Mass", &hitPosition_.pieceMass, 0.0f, 20.0f);

			ImGui::SliderFloat("coefficiendOfRestituion", &hitPosition_.coefficiendOfRestituion, 0.0f, 20.0f);


			ImGui::TreePop();
		}
		ImGui::TreePop();

	}

	ImGui::End();
}

void GlassParticle::SetTableCenterAndRadius(const Vector3& center, const float radius)
{
	hitPosition_.tableCenter = center;
	hitPosition_.pieceRadius = radius;
}

void GlassParticle::Load(const std::string directoryName,const int max)
{
	modelIDs_.clear();

	for (int i = 0; i < max; ++i) {
	    int32_t modelID = Game::Asset::Model::Load(directoryName + std::to_string(i)+".obj");
		modelIDs_.push_back(modelID);
	}
}
