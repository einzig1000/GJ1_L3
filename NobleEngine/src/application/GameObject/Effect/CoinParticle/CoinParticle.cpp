#include "CoinParticle.h"

CoinParticle::CoinParticle()
{
    textureID_ = Game::Asset::Texture::Load("assets/application/model/Coin/Coin.png");
    modelID_ = Game::Asset::Model::Load("assets/application/model/Coin/Coin.obj");

    Initialize();

    particleSRVID_ = Game::Resource::CreateCompute(sizeof(CoinParticleData), 1024);
    freeListIndexSRVID_ = Game::Resource::CreateCompute(sizeof(uint32_t), 1024);
    freeListSRVID_ = Game::Resource::CreateCompute(sizeof(uint32_t), 1024);

    particle_ = std::make_unique<RenderObject>();
    particle_->modelID_ = modelID_;
  
    particle_->psoConfig_.ps = "assets/shaders/Particle/Coin/CoinParticle.PS.hlsl";
    particle_->psoConfig_.vs = "assets/shaders/Particle/Coin/CoinParticle.VS.hlsl";
    particle_->SetupFromShaders();
    particle_->instanceNum_ = 1024;

    initializeCompute_ = std::make_unique<ComputeObject>();
    initializeCompute_->psoConfig_.cs = "assets/shaders/Particle/Coin/InitializeCoinParticle.CS.hlsl";
    initializeCompute_->SetupFromShaders();
    initializeCompute_->SetUAVData(0, Game::Resource::GetUAV(particleSRVID_));
    initializeCompute_->SetUAVData(1, Game::Resource::GetUAV(freeListIndexSRVID_));
    initializeCompute_->SetUAVData(2, Game::Resource::GetUAV(freeListSRVID_));
    initializeCompute_->RegisterOutput(particleSRVID_);
    initializeCompute_->RegisterOutput(freeListIndexSRVID_);
    initializeCompute_->RegisterOutput(freeListSRVID_);
    initializeCompute_->Dispatch();

    emitCompute_ = std::make_unique<ComputeObject>();
    emitCompute_->psoConfig_.cs = "assets/shaders/Particle/Coin/EmitCoinParticle.CS.hlsl";
    emitCompute_->SetupFromShaders();
    emitCompute_->RegisterOutput(particleSRVID_);
    emitCompute_->RegisterOutput(freeListIndexSRVID_);
    emitCompute_->RegisterOutput(freeListSRVID_);

    updateCompute_ = std::make_unique<ComputeObject>();
    updateCompute_->psoConfig_.cs = "assets/shaders/Particle/Coin/UpdateCoinParticle.CS.hlsl";
    updateCompute_->SetupFromShaders();
    updateCompute_->RegisterOutput(particleSRVID_);
    updateCompute_->RegisterOutput(freeListIndexSRVID_);
    updateCompute_->RegisterOutput(freeListSRVID_);

}

CoinParticle::~CoinParticle()
{
}

void CoinParticle::Initialize()
{
    emitterForCoin_.count = 1;
    emitterForCoin_.frequency = 0.5f;
    emitterForCoin_.frequencyTime = 0.0f;
    emitterForCoin_.translate = { 0.0f, 0.0f, 0.0f };
    emitterForCoin_.randomRange = 1.0f;
    emitterForCoin_.emit = 0;

    emitterForCoin_.lifeTime = 2.0f; // ★ ここを追加 (0より大きい値を設定)

    endpos_.endPos = { 0.0f,0.0f,0.0f };
    endpos_.easeStartTime = 0.75f;
}

void CoinParticle::Update(int32_t cameraID)
{

    const float deltaTime = Game::Time::GetScaledDeltaTimeMs() * 0.001f;


    emitCompute_->SetUAVData(0, Game::Resource::GetUAV(particleSRVID_));
    emitCompute_->SetUAVData(1, Game::Resource::GetUAV(freeListIndexSRVID_));
    emitCompute_->SetUAVData(2, Game::Resource::GetUAV(freeListSRVID_));
    emitCompute_->SetCBufferData(0, &emitterForCoin_);

    Vector3 rand = { Game::Math::Rand::RandFloat(-1.0f, 1.0f, 1), Game::Math::Rand::RandFloat(-1.0f, 1.0f, 1), Game::Math::Rand::RandFloat(-1.0f, 1.0f, 1) };
    emitCompute_->SetCBufferData(1, &rand);

    updateCompute_->SetUAVData(0, Game::Resource::GetUAV(particleSRVID_));
    updateCompute_->SetUAVData(1, Game::Resource::GetUAV(freeListIndexSRVID_));
    updateCompute_->SetUAVData(2, Game::Resource::GetUAV(freeListSRVID_));
    updateCompute_->SetCBufferData(0, &deltaTime);
    //ヒット座標をセットする
    updateCompute_->SetCBufferData(1, &endpos_);


    Matrix4x4x2 perView;
    perView.m1 = Game::Camera::Getter::GetViewProjectionMatrix(cameraID);
    perView.m2 = Matrix4x4::MakeIdentity4x4();

    particle_->SetSBufferData(0, ShaderType::VertexShader, Game::Resource::GetSRV(particleSRVID_));
    particle_->SetCBufferData(0, ShaderType::VertexShader, &perView);
    particle_->SetCBufferData(0, ShaderType::PixelShader, &textureID_);

    //毎フレームエミっとしない
    emitterForCoin_.emit = 0;

}

void CoinParticle::Draw(int32_t renderTargetID)
{
    emitCompute_->Dispatch();
    updateCompute_->Dispatch();
    particle_->Draw(renderTargetID);

}
void CoinParticle::SetPosition(const Vector3& startPos, const Vector3& endPos)
{
    emitterForCoin_.translate = startPos;
    endpos_.endPos = endPos;
}
void CoinParticle::DebugImGui() {

    ImGui::Begin("System");

    if (ImGui::TreeNode("CoinParticle")) {
        // 位置座標 (3Dベクター)
        ImGui::DragFloat3("Translate", &emitterForCoin_.translate.x, 0.01f);
        ImGui::SliderFloat("Range", &emitterForCoin_.randomRange, 0.0f, 10.0f);
        // テーブルの中心座標 (3Dベクター)
        ImGui::DragFloat3("endPos", &endpos_.endPos.x, 0.01f);

        int count = emitterForCoin_.count;
        ImGui::SliderInt("Count", &count, 1, 100);
        emitterForCoin_.count = count;

        ImGui::SliderFloat("Frequency", &emitterForCoin_.frequency, 0.0f, 5.0f);
        ImGui::SliderFloat("frequencyTime", &emitterForCoin_.frequencyTime, 0.0f, 5.0f);

        ImGui::SliderFloat("lifeTime", &emitterForCoin_.lifeTime, 0.0f, 5.0f);
        ImGui::DragFloat("easeStartTime", &endpos_.easeStartTime, 0.01f);
        ImGui::Separator(); // パラメータの区切り線

        // 1回発生させるトリガーボタンの例
        if (ImGui::Button("Emit Force")) {
            emitterForCoin_.emit = 1;
        }
        ImGui::TreePop();

    }

    ImGui::End();
}


void CoinParticle::Emit(const int32_t emitCount)
{
    //射出位置
    emitterForCoin_.count = emitCount;

    if (emitterForCoin_.emit == 0) {
        emitterForCoin_.emit = 1;
    }
}

