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

	int32_t cameraID = Game::Camera::AddCamera("MaskTextureCamera");
	Game::Camera::Setter::SetPhiTarget(0.0, 0.0, EaseType::IN_BACK, cameraID);
	Game::Camera::Setter::SetThetaTarget(0.0, 0.0, EaseType::IN_BACK, cameraID);

	Matrix4x4 viewPro = Game::Camera::Getter::GetViewProjectionMatrix(cameraID);
	EulerTransforms transform = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.0f,0.0f} };
	Matrix4x4 world = transform.GetWorldMatrix();
	Matrix4x4 wvp = world * viewPro;

	Vector4 color = { 1.0f, 1.0f, 0.0f, 1.0f };

	render->SetCBufferData(0, ShaderType::VertexShader, &wvp);
	render->SetCBufferData(1, ShaderType::VertexShader, &world);
	render->SetCBufferData(0, ShaderType::PixelShader, &color);

	render->Draw(maskTextureID_);
}

CreateMaskTexture::~CreateMaskTexture()
{}
