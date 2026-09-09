#pragma once
#include <Game.h> 

class CreateMaskTexture
{
public:
	CreateMaskTexture();
	~CreateMaskTexture();

	void Initialize();
	void Update();
	void Draw();
	void DrawImGui();

	int32_t GetMaskTextureID() { return maskTextureID_; };

private:

	int32_t cameraID_ = -1;

	Matrix4x4 world_;
	Matrix4x4 wvp_;
	Vector4 color_ = { 1.0f, 1.0f, 0.0f, 1.0f };
	int32_t maskTextureID_ = -1;

	CounterF counter_;
	CounterF counter2_;
	int32_t stage;

	EulerTransforms transforms_;
	std::unique_ptr<RenderObject> render;
};

