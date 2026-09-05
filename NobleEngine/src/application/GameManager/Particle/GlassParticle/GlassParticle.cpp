#include "GlassParticle.h"

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

	particles_.resize(maxParticle_);
	initializeComputes_.resize(maxParticle_);
	emitComputes_.resize(maxParticle_);
	updateComputes_.resize(maxParticle_);

	for (int i = 0; i < maxParticle_; ++i) {

		particleSRVIDs_.push_back(Game::Resource::CreateCompute(sizeof(Particle), 1024));
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
		emitComputes_[i]->psoConfig_.cs = "assets/shaders/Particle/EmitParticle.CS.hlsl";
		emitComputes_[i]->SetupFromShaders();
		emitComputes_[i]->RegisterOutput(particleSRVIDs_[i]);
		emitComputes_[i]->RegisterOutput(freeListIndexSRVIDs_[i]);
		emitComputes_[i]->RegisterOutput(freeListSRVIDs_[i]);

		updateComputes_[i] = std::make_unique<ComputeObject>();
		updateComputes_[i]->psoConfig_.cs = "assets/shaders/Particle/UpdateParticle.CS.hlsl";
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

	float deltaTime = Game::Time::GetScaledDeltaTimeMs()*0.001f;
	emitterSphere_.frequencyTime += deltaTime;
	if (emitterSphere_.frequency <= emitterSphere_.frequencyTime)
	{
		emitterSphere_.frequencyTime -= emitterSphere_.frequency;
		emitterSphere_.emit = 1;
	} else
	{
		emitterSphere_.emit = 0;
	}

	for (int i = 0; i < maxParticle_; ++i) {
		emitComputes_[i]->SetUAVData(0, Game::Resource::GetUAV(particleSRVIDs_[i]));
		emitComputes_[i]->SetUAVData(1, Game::Resource::GetUAV(freeListIndexSRVIDs_[i]));
		emitComputes_[i]->SetUAVData(2, Game::Resource::GetUAV(freeListSRVIDs_[i]));
		emitComputes_[i]->SetCBufferData(0, &emitterSphere_);
		
		//if (i != 0) {
		//
		//} else {
		//	emitComputes_[i]->SetCBufferData(0, &emitterSphere_);
		//}

		Vector3 rand = { Game::Math::Rand::RandFloat(-1.0f, 1.0f, 1), Game::Math::Rand::RandFloat(-1.0f, 1.0f, 1), Game::Math::Rand::RandFloat(-1.0f, 1.0f, 1) };
		emitComputes_[i]->SetCBufferData(1, &rand);
	

		updateComputes_[i]->SetUAVData(0, Game::Resource::GetUAV(particleSRVIDs_[i]));
		updateComputes_[i]->SetUAVData(1, Game::Resource::GetUAV(freeListIndexSRVIDs_[i]));
		updateComputes_[i]->SetUAVData(2, Game::Resource::GetUAV(freeListSRVIDs_[i]));
		updateComputes_[i]->SetCBufferData(0, &deltaTime);
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

void GlassParticle::Load(const std::string directoryName,const int max)
{
	modelIDs_.clear();

	for (int i = 0; i < max; ++i) {
	    int32_t modelID = Game::Asset::Model::Load(directoryName + std::to_string(i)+".obj");
		modelIDs_.push_back(modelID);
	}
}
