#include "CreateMaskTexture.h"

CreateMaskTexture::CreateMaskTexture()
{
	maskTextureID_ = Game::Asset::RenderTexture::CreateRenderTexture(
		Game::Window::GetWidth(),
		Game::Window::GetHeight(),
		"MaskTexture");

	render = std::make_unique<RenderObject>();
	render->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModel.VS.hlsl";
	render->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModelNonTexture.PS.hlsl";
	render->modelID_ = Game::Asset::Model::Load("assets/application/model/Alcohol/Cocktail/Cocktail.obj");
	render->SetupFromShaders();

	cameraID_ = Game::Camera::AddCamera("MaskTextureCamera");
	Game::Camera::Setter::SetPhiTarget(0.24f, 0.0f, EaseType::IN_BACK, cameraID_);
	Game::Camera::Setter::SetDistanceTarget(1.2f, 0.0f, EaseType::IN_BACK, cameraID_);

	stage = 0;
}

CreateMaskTexture::~CreateMaskTexture()
{}

void CreateMaskTexture::Initialize()
{
	counter_.Initialize(50);
	counter2_.Initialize(-1);
	transforms_.translate = { 0.0f, 0.0f, 0.0f };
	transforms_.rotate = { 0.0f, 0.0f, 0.0f };
	transforms_.scale = { 1.0f, 1.0f, 1.0f };
	stage = 0;
}

void CreateMaskTexture::Update()
{
	Game::Camera::Update(cameraID_);

	if (stage == 0 && counter_.GetProgress() >= 1.0f)
	{
		stage++;
		counter2_.Initialize(50);
	}
	if (stage == 1 && counter2_.GetProgress() >= 1.0f)
	{
		stage++;
	}


	transforms_.translate = Game::Math::Ease::Easing(Vector3{ -2.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, EaseType::OUT_BACK, counter_.GetProgress());
	transforms_.scale = Game::Math::Ease::Easing(Vector3{ 1.0f, 1.0f, 1.0f }, Vector3{ 10.0f, 11.0f, 10.0f }, EaseType::IN_BACK, counter2_.GetProgress());




	Matrix4x4 viewPro = Game::Camera::Getter::GetViewProjectionMatrix(cameraID_);
	world_ = transforms_.GetWorldMatrix();
	wvp_ = world_ * viewPro;
}

void CreateMaskTexture::Draw()
{
	render->SetCBufferData(0, ShaderType::VertexShader, &wvp_);
	render->SetCBufferData(1, ShaderType::VertexShader, &world_);
	render->SetCBufferData(0, ShaderType::PixelShader, &color_);

	render->Draw(maskTextureID_);
}

void CreateMaskTexture::DrawImGui()
{
	ImGui::Begin("MaskTexture");
	ImGui::DragFloat3("Position", &transforms_.translate.x, 0.1f);
	ImGui::DragFloat3("Rotation", &transforms_.rotate.x, 0.1f);
	ImGui::DragFloat3("Scale", &transforms_.scale.x, 0.1f);
	ImGui::End();
}
